#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os, sys, json, re, shutil, subprocess, urllib.request, urllib.error, zipfile, tempfile, uuid, hashlib, argparse, pathlib
import base64
from urllib.parse import urlparse
from typing import List, Dict, Tuple

# ---------- Colors ----------
class C:
    X = "\x1b[0m"
    G = "\x1b[32m"
    R = "\x1b[31m"
    Y = "\x1b[33m"
    C = "\x1b[36m"
def good(msg): print(C.G + "✓ " + msg + C.X)
def bad(msg):  print(C.R + "✗ " + msg + C.X)
def warn(msg): print(C.Y + "▶ " + msg + C.X)
def step(msg): print("▶ " + msg)
def info(msg): print("• " + msg)
def dim(msg):  print(msg)

# ---------- Binary parsing regexes ----------
BIN_RE = re.compile(
    r'\.binaryTarget\s*\(\s*name\s*:\s*"(?P<name>[^"]+)"\s*,(?P<body>[^)]*)\)',
    re.S
)
URL_FIELD_RE    = re.compile(r'url\s*:\s*"(?P<url>[^"]+)"')
CHKSUM_FIELD_RE = re.compile(r'checksum\s*:\s*"(?P<sum>[a-fA-F0-9]{64})"')
PATH_FIELD_RE   = re.compile(r'path\s*:\s*"(?P<path>[^"]+)"')

# For dependency URL patching
# Match any .package(...) entry that contains a url: "..."
# This is robust to the presence of other arguments like name:, and spans newlines.
PKG_DEP_URL_RE = re.compile(r"\.package\s*\(\s*[^\)]*url\s*:\s*\"([^\"]+)\"[^\)]*\)", re.S)

# ---------- Done registry ----------
def _done_registry_path(vendor_dir):
    return os.path.join(vendor_dir, ".packages_done.json")

def _load_done_registry(vendor_dir):
    p = _done_registry_path(vendor_dir)
    try:
        with open(p, "r") as f:
            return json.load(f)
    except Exception:
        return {}

def _save_done_registry(vendor_dir, reg):
    try:
        with open(_done_registry_path(vendor_dir), "w") as f:
            json.dump(reg, f, indent=2)
    except Exception:
        pass

def _pkg_done_marker(pkg_dir):
    return os.path.join(pkg_dir, ".done.json")

def _is_pkg_done(vendor_dir, name, version, pkg_dir):
    reg = _load_done_registry(vendor_dir)
    entry = reg.get(name)
    if not entry:
        return False
    if str(entry.get("version","")) != str(version or "HEAD"):
        return False
    try:
        with open(_pkg_done_marker(pkg_dir), "r") as f:
            meta = json.load(f)
        return meta.get("version") == entry["version"]
    except Exception:
        return False

def _mark_pkg_done(vendor_dir, name, version, pkg_dir):
    ts = __import__("datetime").datetime.utcnow().isoformat() + "Z"
    reg = _load_done_registry(vendor_dir)
    reg[name] = {"version": str(version or "HEAD"), "timestamp": ts}
    _save_done_registry(vendor_dir, reg)
    try:
        with open(_pkg_done_marker(pkg_dir), "w") as f:
            json.dump({"name": name, "version": str(version or "HEAD"), "timestamp": ts}, f, indent=2)
    except Exception:
        pass

# ---------- Utils ----------
def ensure_dir(p):
    os.makedirs(p, exist_ok=True)
def run(cmd, cwd=None, env=None):
    return subprocess.check_call(cmd, cwd=cwd, env=env)

# ---------- Git checkout ----------
def checkout_repo(dest_dir, git_url, version):
    """
    Robust clone/checkout:
    - clone --no-checkout --filter=blob:none
    - try shallow fetch of base, vbase, origin/base
    - fallback to fetching tags fully then checkout tags/vbase or tags/base
    - initialize submodules (shallow first, full if needed)
    """
    ensure_dir(os.path.dirname(dest_dir))

    if os.path.exists(dest_dir) and not os.path.exists(os.path.join(dest_dir, ".git")):
        warn(f"{dest_dir} exists but is not a git repo; removing")
        shutil.rmtree(dest_dir)

    def has_submodules(path): return os.path.exists(os.path.join(path, ".gitmodules"))
    def is_git_repo(path): return os.path.isdir(os.path.join(path, ".git"))
    def ensure_repo_initialized(path, remote):
        ensure_dir(path)
        if not is_git_repo(path):
            run(["git", "init"], cwd=path)
            run(["git", "remote", "add", "origin", remote], cwd=path)
        else:
            try:
                run(["git", "remote", "set-url", "origin", remote], cwd=path)
            except subprocess.CalledProcessError:
                pass
        return True

    # If a specific version is requested, prefer an init + shallow fetch of that ref
    # to avoid large clones (especially on servers without partial-clone support).
    if version:
        try:
            ensure_repo_initialized(dest_dir, git_url)
        except subprocess.CalledProcessError as e:
            bad(f"git init/remote add failed: {e}")
            return False, "git init failed"
    else:
        if not os.path.exists(dest_dir):
            step(f"Cloning (no checkout) {git_url} → {dest_dir}")
            try:
                run(["git", "clone", "--no-checkout", "--filter=blob:none", git_url, dest_dir])
            except subprocess.CalledProcessError as e:
                bad(f"git clone failed: {e}")
                return False, "git clone failed"
        else:
            dim(f"{dest_dir} already exists")

    if not version:
        dim("No version specified; leaving repository at default branch/HEAD")
        try: run(["git", "checkout", "--detach"], cwd=dest_dir)
        except subprocess.CalledProcessError: pass
        if has_submodules(dest_dir):
            try: run(["git", "submodule", "update", "--init", "--recursive", "--depth", "1"], cwd=dest_dir)
            except subprocess.CalledProcessError:
                run(["git", "submodule", "update", "--init", "--recursive"], cwd=dest_dir)
        return True, "HEAD"

    base = str(version)[1:] if str(version).startswith("v") else str(version)
    # Try specific tags first, then branches
    shallow_env = dict(os.environ)
    # Avoid auto-downloading LFS blobs during checkout
    shallow_env.setdefault("GIT_LFS_SKIP_SMUDGE", "1")
    def _do_submodules():
        if has_submodules(dest_dir):
            try:
                run(["git", "submodule", "update", "--init", "--recursive", "--depth", "1"], cwd=dest_dir, env=shallow_env)
            except subprocess.CalledProcessError:
                run(["git", "submodule", "update", "--init", "--recursive"], cwd=dest_dir, env=shallow_env)

    def try_sequence():
        # Ensure path exists before each attempt
        ensure_repo_initialized(dest_dir, git_url)
        tag_names = [f"v{base}", base]
        for t in tag_names:
            try:
                run(["git", "fetch", "--depth", "1", "origin", "tag", t], cwd=dest_dir, env=shallow_env)
                run(["git", "checkout", "FETCH_HEAD"], cwd=dest_dir, env=shallow_env)
                good(f"Checked out tag {t}")
                _do_submodules()
                return True, f"tags/{t}"
            except subprocess.CalledProcessError:
                continue
        candidates = [base, f"v{base}", f"origin/{base}"]
        for ref in candidates:
            try:
                run(["git", "fetch", "--depth", "1", "origin", ref], cwd=dest_dir, env=shallow_env)
                run(["git", "checkout", "FETCH_HEAD"], cwd=dest_dir, env=shallow_env)
                good(f"Checked out {ref}")
                _do_submodules()
                return True, ref
            except subprocess.CalledProcessError:
                continue
        return False, None

    ok, ref = try_sequence()
    if ok:
        return True, ref
    # Retry once from a clean slate for transient pack/index errors
    shutil.rmtree(dest_dir, ignore_errors=True)
    ensure_repo_initialized(dest_dir, git_url)
    ok, ref = try_sequence()
    if ok:
        return True, ref

    bad(f"Could not checkout {version}")
    return False, f"checkout failed ({version})"

# ---------- Package.swift parsing ----------
def find_binary_targets(pkg_swift_text: str):
    results = []
    for m in BIN_RE.finditer(pkg_swift_text or ""):
        name = m.group("name")
        body = m.group("body") or ""
        url_m = URL_FIELD_RE.search(body)
        sum_m = CHKSUM_FIELD_RE.search(body)
        path_m = PATH_FIELD_RE.search(body)
        results.append({
            "name": name,
            "url": url_m.group("url") if url_m else None,
            "checksum": sum_m.group("sum") if sum_m else None,
            "path": path_m.group("path") if path_m else None,
        })
    return results

def patch_manifest_to_path(pkg_swift_path: str, target_name: str, rel_path: str):
    txt = pathlib.Path(pkg_swift_path).read_text()
    def repl(m):
        body = m.group("body")
        body = re.sub(r'url\s*:\s*"[^"]+"', f'path: "{rel_path}"', body)
        body = re.sub(r'checksum\s*:\s*"[a-fA-F0-9]{64}"', '', body)
        # Ensure only one of url/path exists (prefer path)
        body = re.sub(r'\s*,\s*,', ',', body)
        return f'.binaryTarget(name: "{target_name}", {body})'
    new = BIN_RE.sub(lambda m: repl(m) if m.group("name")==target_name else m.group(0), txt)
    pathlib.Path(pkg_swift_path).write_text(new)

def _normalize_git_url(u: str) -> str:
    try:
        u = (u or "").strip()
        if u.endswith('.git'):
            u = u[:-4]
        u = u.replace('git@github.com:', 'https://github.com/')
        # Remove trailing slashes
        while u.endswith('/'):
            u = u[:-1]
        return u.lower()
    except Exception:
        return (u or '').lower()

def patch_manifest_deps_to_local(pkg_swift_path: str, pkg_dir: str, vendor_root: str, cfg: Dict):
    """Replace remote .package(url: ...) deps with path-based ones if the URL matches a vendored package."""
    try:
        txt = pathlib.Path(pkg_swift_path).read_text()
    except Exception:
        return
    # Build lookup from normalized git url -> local relative path
    url_to_rel: Dict[str, str] = {}
    for e in (cfg.get('packages') or []):
        git = e.get('git') or ''
        name = e.get('name') or ''
        if not git or not name:
            continue
        nurl = _normalize_git_url(git)
        local_abs = os.path.join(vendor_root, name)
        rel = os.path.relpath(local_abs, start=os.path.dirname(pkg_swift_path))
        url_to_rel[nurl] = rel

    changed = False
    def repl(m):
        nonlocal changed
        full = m.group(0)
        url = m.group(1)
        nurl = _normalize_git_url(url)
        rel = url_to_rel.get(nurl)
        if not rel:
            return full
        # Preserve explicit dependency name label if present
        name_m = re.search(r'name\s*:\s*"([^"]+)"', full)
        name_part = f'name: "{name_m.group(1)}", ' if name_m else ''
        changed = True
        return f'.package({name_part}path: "{rel}")'

    new_txt = PKG_DEP_URL_RE.sub(repl, txt)
    if changed:
        # Normalize accidental double-closing parens introduced by replacing nested url entries
        new_txt = re.sub(r'(\.package\s*\(\s*path\s*:\s*\"[^\"]+\"\s*\))\s*\)', r"\1", new_txt, flags=re.S)
        # If a trailing '),\n' remains from the original entry, fold it into a single comma
        new_txt = re.sub(r'(\.package\s*\([^)]*path\s*:\s*\"[^\"]+\"[^)]*\))\s*\),', r"\1,", new_txt, flags=re.S)
    if changed and new_txt != txt:
        pathlib.Path(pkg_swift_path).write_text(new_txt)
        info(f"Patched dependencies to local paths in {pkg_swift_path}")

# ---------- Download + Extract ----------
def download_file(url, dest, token=None, extra_headers=None):
    step(f"Downloading binary: {url}")
    try:
        if os.path.isfile(dest) and os.path.getsize(dest) > 0:
            # Only skip if it's actually a valid zip
            if zipfile.is_zipfile(dest):
                info(f"[skip] download_file: {dest} already present")
                return True, None
            else:
                warn(f"Cached file invalid zip; re-downloading → {dest}")
                try:
                    os.remove(dest)
                except Exception:
                    pass
    except Exception:
        pass
    headers = {
        "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15",
        # Default to octet-stream which works for GitHub assets API
        "Accept": "application/octet-stream, application/zip;q=0.9, */*;q=0.8",
        "Accept-Language": "en-US,en;q=0.9",
        "Connection": "keep-alive",
    }
    # Prefer explicit token argument, else fall back to env var
    if not token:
        token = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
    if token:
        # GitHub accepts both Bearer and token; keep it simple
        if not str(token).startswith(("Bearer ", "token ")):
            headers["Authorization"] = f"token {token}"
        else:
            headers["Authorization"] = str(token)
    if extra_headers:
        headers.update(extra_headers)

    # Heuristics for Artifactory/JFrog-hosted zips (often require auth or '?download')
    try:
        u = urlparse(url)
        host = (u.netloc or "").lower()
        path = u.path or ""
    except Exception:
        host = ""
        path = ""

    # Special handling for GitHub release asset API endpoint which requires
    # Accept: application/octet-stream in order to return the binary bytes
    if host == "api.github.com" and "/releases/assets/" in path:
        headers["Accept"] = "application/octet-stream"

    if "jfrog" in host or "artifactory" in host:
        # Inject API key header if provided via env
        api_key = os.environ.get("ARTIFACTORY_API_KEY") or os.environ.get("APPD_ARTIFACTORY_API_KEY")
        if api_key and not headers.get("X-JFrog-Art-Api") and not headers.get("Authorization"):
            headers["X-JFrog-Art-Api"] = api_key
        # Inject bearer access token if provided via env
        bearer = os.environ.get("ARTIFACTORY_ACCESS_TOKEN") or os.environ.get("APPD_ARTIFACTORY_ACCESS_TOKEN")
        if bearer and not headers.get("Authorization"):
            headers["Authorization"] = f"Bearer {bearer}"
        # Basic auth if provided
        basic = os.environ.get("ARTIFACTORY_BASIC_AUTH")
        user = os.environ.get("ARTIFACTORY_USER") or os.environ.get("APPD_ARTIFACTORY_USER")
        pwd  = os.environ.get("ARTIFACTORY_PASSWORD") or os.environ.get("APPD_ARTIFACTORY_PASSWORD")
        if not headers.get("Authorization"):
            if basic:
                headers["Authorization"] = basic
            elif user and pwd:
                b = base64.b64encode(f"{user}:{pwd}".encode()).decode()
                headers["Authorization"] = f"Basic {b}"

    def _extract_urls_from_json_bytes(b: bytes):
        try:
            txt = b.decode("utf-8", errors="ignore")
            obj = json.loads(txt)
        except Exception:
            return []
        # Prioritize GitHub release asset fields when available
        found: List[str] = []
        try:
            if isinstance(obj, dict) and isinstance(obj.get("assets"), list):
                # Collect browser_download_url first, then asset url
                for a in obj["assets"]:
                    if isinstance(a, dict):
                        u1 = a.get("browser_download_url")
                        u2 = a.get("url")
                        if isinstance(u1, str) and u1.startswith("http"):
                            found.append(u1)
                        if isinstance(u2, str) and u2.startswith("http"):
                            found.append(u2)
        except Exception:
            pass
        def walk(o):
            if isinstance(o, dict):
                for k, v in o.items():
                    walk(v)
                    if isinstance(v, str) and v.startswith("http"):
                        found.append(v)
            elif isinstance(o, list):
                for it in o:
                    walk(it)
            elif isinstance(o, str):
                if o.startswith("http"):
                    found.append(o)
        walk(obj)
        # Prefer direct artifacts
        def score(u: str):
            u_l = u.lower()
            return (
                0 if u_l.endswith(".zip") else 1,
                0 if ".xcframework" in u_l or ".artifactbundle" in u_l else 1,
                0 if "browser_download_url" in u else 1,
                len(u_l)
            )
        found = list(dict.fromkeys(found))
        found.sort(key=score)
        return found

    def _extract_urls_from_html_bytes(b: bytes, base_url: str):
        """Very light HTML href/src/meta refresh extractor returning candidate URLs.
        We don't depend on an HTML parser to keep the script self‑contained.
        """
        try:
            txt = b.decode("utf-8", errors="ignore")
        except Exception:
            return []
        urls: List[str] = []
        # href/src attributes
        for m in re.finditer(r'(?:href|src)\s*=\s*[\'\"]([^\'\"]+)[\'\"]', txt, re.I):
            urls.append(m.group(1))
        # meta refresh
        m = re.search(r'<meta[^>]+http-equiv\s*=\s*[\'\"]refresh[\'\"][^>]+content\s*=\s*[\'\"]\s*\d+\s*;\s*url=([^\'\"]+)[\'\"]', txt, re.I)
        if m:
            urls.append(m.group(1))
        # simple JS redirections
        for m in re.finditer(r'location\.(?:href|assign)\s*=\s*[\'\"]([^\'\"]+)[\'\"]', txt, re.I):
            urls.append(m.group(1))
        # data attributes commonly used by download buttons
        for m in re.finditer(r'data-(?:redirect-url|download-url)\s*=\s*[\'\"]([^\'\"]+)[\'\"]', txt, re.I):
            urls.append(m.group(1))
        # resolve relative URLs
        try:
            from urllib.parse import urljoin
            urls = [urljoin(base_url, u) for u in urls]
        except Exception:
            pass
        # Prefer direct artifacts
        def score(u: str):
            u_l = u.lower()
            return (
                0 if u_l.endswith('.zip') else 1,
                0 if '.xcframework' in u_l or '.artifactbundle' in u_l else 1,
                0 if 'artifactory' in u_l else 1,
                len(u_l)
            )
        urls = list(dict.fromkeys(urls))
        urls.sort(key=score)
        return urls

    def _try_jfrog_storage_api(orig_url: str, h: Dict[str, str]) -> Tuple[bool, str]:
        # Convert /artifactory/<path> to /artifactory/api/storage/<path> and read downloadUri
        try:
            u = urlparse(orig_url)
            if '/artifactory/' not in u.path:
                return False, ''
            api_path = u.path.replace('/artifactory/', '/artifactory/api/storage/', 1)
            api_url = f"{u.scheme}://{u.netloc}{api_path}"
            req_h = dict(h)
            req_h['Accept'] = 'application/json'
            req = urllib.request.Request(api_url, headers=req_h)
            with urllib.request.urlopen(req, timeout=120) as r2:
                body2 = r2.read()
            try:
                obj = json.loads(body2.decode('utf-8', errors='ignore'))
                dl = obj.get('downloadUri') or obj.get('uri')
                if isinstance(dl, str) and dl.startswith('http'):
                    return True, dl
            except Exception:
                return False, ''
        except Exception:
            return False, ''
        return False, ''

    def _try_jfrog_download_api(orig_url: str, h: Dict[str, str]) -> Tuple[bool, str]:
        """Use Artifactory download API to get a direct redirect to filestore/S3.
        GET /artifactory/api/download/{repoKey}/{path}?useRedirect=true
        """
        try:
            u = urlparse(orig_url)
            if '/artifactory/' not in u.path:
                return False, ''
            parts = u.path.split('/artifactory/', 1)[1]
            # Avoid recursion if we're already using the download API
            if parts.startswith('api/download/'):
                return False, ''
            if not parts:
                return False, ''
            # parts starts with '<repoKey>/rest/of/path'
            api_url = f"{u.scheme}://{u.netloc}/artifactory/api/download/{parts}?useRedirect=true"
            # Return the download API URL; the caller will fetch it (urllib follows redirects)
            return True, api_url
        except Exception:
            return False, ''
        return False, ''

    def attempt(h, tag):
        try:
            req = urllib.request.Request(url, headers=h)
            with urllib.request.urlopen(req, timeout=240) as r:
                body = r.read()
                with open(dest, "wb") as f:
                    f.write(body)
            # Validate zip signature
            try:
                if not zipfile.is_zipfile(dest):
                    ct = ""
                    try:
                        ct = r.headers.get("Content-Type", "")
                    except Exception:
                        pass
                    # If JSON with nested URL, try to extract
                    if "json" in (ct or "").lower() or (body[:1] in (b"{", b"[")):
                        nested = _extract_urls_from_json_bytes(body)
                        for u2 in nested:
                            ok_nested, err_nested = download_file(u2, dest, token=token, extra_headers=h)
                            if ok_nested and zipfile.is_zipfile(dest):
                                good(f"Resolved nested download URL → {u2}")
                                return True, None
                        # fallthrough to treat as non-zip
                    # If HTML landing page (e.g., Artifactory link generator), try to extract next hop
                    if "html" in (ct or "").lower() or body.lstrip().startswith((b"<!DOCTYPE html", b"<html")):
                        # Try Artifactory storage API to get a direct downloadUri first
                        ok_api, api_url = _try_jfrog_storage_api(url, h)
                        if ok_api:
                            ok_api_dl, err_api_dl = download_file(api_url, dest, token=token, extra_headers=h)
                            if ok_api_dl and zipfile.is_zipfile(dest):
                                good(f"Resolved Artifactory storage API → {api_url}")
                                return True, None
                        # Try Artifactory download API to get a redirect to filestore/S3
                        ok_dl, dl_url = _try_jfrog_download_api(url, h)
                        if ok_dl:
                            ok_dl2, err_dl2 = download_file(dl_url, dest, token=token, extra_headers=h)
                            if ok_dl2 and zipfile.is_zipfile(dest):
                                good(f"Resolved Artifactory download API → {dl_url}")
                                return True, None
                        # Fallback to parsing HTML for links
                        hops = _extract_urls_from_html_bytes(body, url)
                        for u3 in hops:
                            ok_hop, err_hop = download_file(u3, dest, token=token, extra_headers=h)
                            if ok_hop and zipfile.is_zipfile(dest):
                                good(f"Followed HTML redirect/link → {u3}")
                                return True, None
                        # fallthrough
                    # Read a few bytes to hint what we got
                    head = b""
                    try:
                        head = body[:64]
                    except Exception:
                        pass
                    os.remove(dest)
                    return False, f"{tag}: not a zip file (Content-Type={ct}, head={head[:16]!r})"
            except Exception as _ve:
                # If validation itself fails, treat as error
                try: os.remove(dest)
                except Exception: pass
                return False, f"{tag}: zip validation failed: {_ve}"
            good(f"Saved → {dest}")
            return True, None
        except urllib.error.HTTPError as e:
            return False, f"{tag}: HTTP Error {e.code}: {e.reason}"
        except urllib.error.URLError as e:
            return False, f"{tag}: URLError: {e.reason}"
        except Exception as e:
            return False, f"{tag}: {e}"
    # Special preflight for JFrog/Artifactory: try API-based redirect before raw URL
    pre_err = None
    if ("jfrog" in (host or "")) or ("artifactory" in (host or "")):
        # Avoid preflight recursion if already using download API
        if '/artifactory/api/download/' not in (u.path or ''):
            ok_dl, dl_url = _try_jfrog_download_api(url, headers)
            if ok_dl:
                ok_pre, err_pre = download_file(dl_url, dest, token=token, extra_headers=headers)
                if ok_pre:
                    return True, None
                pre_err = err_pre
        else:
            ok_api, api_url = _try_jfrog_storage_api(url, headers)
            if ok_api:
                ok_pre2, err_pre2 = download_file(api_url, dest, token=token, extra_headers=headers)
                if ok_pre2:
                    return True, None
                pre_err = err_pre2

    ok, err = attempt(headers, "initial")
    if ok:
        return True, None
    # Retry with Referer if auth forbidden
    if "HTTP Error 403" in (err or "") or "HTTP Error 401" in (err or ""):
        if "Referer" not in headers:
            headers["Referer"] = "https://github.com"
        ok2, err2 = attempt(headers, "retry+referer")
        if ok2:
            return True, None
        # Fallthrough to potential '?download' retry
        err = err2 or err
    # If we hit GitHub asset API without octet-stream, retry forcing it
    if (host == "api.github.com" and "/releases/assets/" in path and "not a zip file" in (err or "")):
        h2 = dict(headers)
        h2["Accept"] = "application/octet-stream"
        ok3, err3 = attempt(h2, "retry+octet-stream")
        if ok3:
            return True, None
        err = err3 or err
    # Some Artifactory URLs need '?download' to return raw bytes
    if ((err or "").startswith("initial: not a zip file") or "not a zip file" in (err or "")) and "download" not in (url or "") and not (("jfrog" in (host or "")) or ("artifactory" in (host or ""))):
        sep = '&' if ('?' in url) else '?'
        url2 = f"{url}{sep}download"
        try:
            req_headers = dict(headers)
            ok3, err3 = download_file(url2, dest, token=token, extra_headers=req_headers)
            if ok3:
                return True, None
            return False, err3 or err or pre_err
        except Exception:
            return False, err or pre_err
    return False, err or pre_err
def extract_spm_artifact(zip_path, out_dir, expected_name=None):
    temp_dir = os.path.join(out_dir, f".extract_{uuid.uuid4().hex}")
    try:
        ensure_dir(temp_dir)
        with zipfile.ZipFile(zip_path, "r") as z:
            z.extractall(temp_dir)
        candidates = []
        for root, dirs, files in os.walk(temp_dir):
            for d in dirs:
                if d.endswith(".artifactbundle") or d.endswith(".xcframework"):
                    candidates.append(os.path.join(root, d))
        if not candidates:
            return None, temp_dir, "No .xcframework or .artifactbundle found in archive"
        def score(p):
            base = os.path.basename(p)
            return (0 if base.endswith(".artifactbundle") else 1,
                    0 if expected_name and expected_name.lower() in base.lower() else 1,
                    len(base))
        candidates.sort(key=score)
        chosen = candidates[0]
        dest_path = os.path.join(out_dir, os.path.basename(chosen))
        if os.path.exists(dest_path):
            if os.path.isdir(dest_path): shutil.rmtree(dest_path)
            else: os.remove(dest_path)
        shutil.move(chosen, dest_path)
        return dest_path, temp_dir, None
    except zipfile.BadZipFile:
        return None, temp_dir, "BadZipFile: file is not a valid zip"
    except Exception as e:
        return None, temp_dir, str(e)

# ---------- Table ----------
def print_table(rows):
    ANSI = re.compile(r'\x1b\[[0-9;]*m')
    def vis_len(x): return len(ANSI.sub('', str(x)))
    cols = ["Package","Version","Clone","Binaries","Remaining","Errors"]
    widths = {c: len(c) for c in cols}
    for r in rows:
        for c in cols:
            widths[c] = max(widths[c], vis_len(r.get(c, "")))
    def pad(x,w):
        s = str(x); extra = w - vis_len(s); return s + (" " * max(0, extra))
    line = " | ".join(c.ljust(widths[c]) for c in cols)
    sep  = "-+-".join("-"*widths[c] for c in cols)
    print("\n" + line); print(sep)
    for r in rows:
        print(" | ".join(pad(r.get(c, ""), widths[c]) for c in cols))
    print()

# ---------- Helpers ----------
def run_post_commands(pkg_dir: str, cmds: List[str], env: Dict = None) -> Tuple[bool, List[str]]:
    """Run post-clone commands inside the package directory. Returns (ok, errors)."""
    if not cmds:
        return True, []
    errs: List[str] = []
    for i, cmd in enumerate(cmds, 1):
        if not cmd or not str(cmd).strip():
            continue
        # Basic placeholder interpolation
        cmd_fmt = str(cmd)
        try:
            cmd_fmt = cmd_fmt.replace("{PKG_DIR}", pkg_dir)
            cmd_fmt = cmd_fmt.replace("{VENDOR_DIR}", os.path.dirname(pkg_dir))
            cmd_fmt = cmd_fmt.replace("{PKG_NAME}", os.path.basename(pkg_dir))
        except Exception:
            pass
        step(f"post[{os.path.basename(pkg_dir)}] #{i}: {cmd_fmt}")
        try:
            subprocess.check_call(cmd_fmt, shell=True, cwd=pkg_dir, env=env or os.environ.copy())
        except subprocess.CalledProcessError as e:
            msg = f"post command failed (#{i}): {cmd_fmt} → {e}"
            bad(msg)
            errs.append(msg)
    return len(errs) == 0, errs
def remove_git_dir(path: str):
    g = os.path.join(path, ".git")
    if os.path.isdir(g):
        try:
            shutil.rmtree(g)
            dim(f"Removed git metadata → {g}")
        except Exception as e:
            warn(f"Could not remove {g}: {e}")

def _products_for_package(cfg: Dict, name: str) -> List[str]:
    for e in (cfg.get("packages") or []):
        if (e.get("name") or "").strip() == name:
            # Respect explicit empty list (means: no products to wire)
            if "products" in e:
                prods = e.get("products")
                prods = prods if prods is not None else [name]
            else:
                prods = [name]
            return [p for p in (prods or []) if p]
    return [name]

_PKG_NAME_RE = re.compile(r"let\s+package\s*=\s*Package\s*\(\s*name\s*:\s*\"([^\"]+)\"", re.S)
_PRODUCT_LIB_RE = re.compile(r"\.library\s*\(\s*name\s*:\s*\"([^\"]+)\"", re.S)

def _declared_package_name(manifest_path: pathlib.Path) -> str:
    try:
        txt = manifest_path.read_text()
        m = _PKG_NAME_RE.search(txt)
        if m:
            return m.group(1)
    except Exception:
        pass
    # Fallback to directory name
    return manifest_path.parent.name

def _declared_products(manifest_path: pathlib.Path) -> List[str]:
    try:
        txt = manifest_path.read_text()
    except Exception:
        return []
    return [m.group(1) for m in _PRODUCT_LIB_RE.finditer(txt)]

def _scan_vendor_for_shell(vendor_root: pathlib.Path, cfg: Dict) -> List[Dict]:
    """Scan Vendor dir for all packages that have a Package.swift and build include list."""
    includes: List[Dict] = []
    if not vendor_root.exists():
        return includes
    for entry in sorted(vendor_root.iterdir(), key=lambda p: p.name.lower()):
        if not entry.is_dir():
            continue
        pkg_manifest = entry / "Package.swift"
        if pkg_manifest.exists():
            name = entry.name
            products = _products_for_package(cfg, name)
            # Allow optional config override for the dependency package name
            declared = None
            for e in (cfg.get("packages") or []):
                if (e.get("name") or "").strip() == name:
                    declared = (e.get("packageName") or "").strip() or None
                    break
            if not declared:
                declared = _declared_package_name(pkg_manifest)
            avail = _declared_products(pkg_manifest)
            includes.append({"name": name, "products": products, "declaredName": declared, "availableProducts": avail})
    return includes

def _cleanup_all_git(vendor_root: pathlib.Path):
    """Remove all .git directories under vendor_root."""
    if not vendor_root.exists():
        return
    for entry in vendor_root.iterdir():
        if entry.is_dir():
            remove_git_dir(str(entry))

# ---------- Main ----------
def _generate_shell_package(cfg: Dict, included_pkgs: List[Dict], vendor_root: str):
    """
    Create a Shell/Package.swift that depends on vendored packages via path.
    Includes a single library target that depends on each listed product.
    Only include packages that actually have a Package.swift in Vendor/<name>.
    """
    shell_name = cfg.get("shellPackageName", "ThirdPartyShell")
    vendor_dir = vendor_root or cfg.get("vendorDir", "Vendor")

    # Prepare dependency entries (path-based)
    dep_lines = []
    prod_lines = []
    seen_deps = set()
    seen_prod_pairs = set()
    for p in included_pkgs:
        name = p.get("name")
        if not name:
            continue
        products = p.get("products") or []
        available = set(p.get("availableProducts") or [])
        # Determine which products we will attach for this package
        to_attach = []
        for prod in products:
            if available and prod not in available:
                warn(f"Skipping unknown product '{prod}' in package '{name}' (available: {sorted(list(available))[:6]}...)")
                continue
            to_attach.append(prod)
        # Only include the package dependency if at least one product is attached
        if to_attach and name not in seen_deps:
            rel_path = os.path.relpath(os.path.join(str(vendor_dir), name), start="Shell")
            dep_lines.append(f'        .package(path: "{rel_path}"),')
            seen_deps.add(name)
        # Use dependency name as declared in dependencies list (path basename)
        pkg_label = name
        for prod in to_attach:
            # Attach each declared product from the package
            key = (prod, name)
            if key in seen_prod_pairs:
                continue
            prod_lines.append(f'                .product(name: "{prod}", package: "{pkg_label}"),')
            seen_prod_pairs.add(key)

    # Minimal Swift package manifest (mirrors earlier generator)
    pkg_txt = f"""// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "{shell_name}",
    platforms: [
        .iOS(.v15)
    ],
    products: [
        .library(name: "{shell_name}", targets: ["{shell_name}"])
    ],
    dependencies: [
{chr(10).join(dep_lines)}
    ],
    targets: [
        .target(
            name: "{shell_name}",
            dependencies: [
{chr(10).join(prod_lines)}
            ]
        )
    ]
)
"""

    shell_dir = pathlib.Path("Shell")
    src_dir = shell_dir / "Sources" / shell_name
    ensure_dir(str(src_dir))

    # Write manifest
    (shell_dir / "Package.swift").write_text(pkg_txt)

    # Re-export modules for convenience
    # Export only the products that were actually added to the target deps
    added_products = set()
    for p in included_pkgs:
        products = p.get("products") or []
        available = set(p.get("availableProducts") or [])
        for prod in products:
            if available and prod not in available:
                continue
            added_products.add(prod)
    # Filter out names that are not valid Swift module identifiers (e.g., contain hyphens)
    def is_valid_swift_ident(x: str) -> bool:
        return bool(re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", x or ""))
    exported = [m for m in sorted(added_products) if is_valid_swift_ident(m)]
    exports_src = "// Auto-generated re-exports\n" + "\n".join([f"@_exported import {m}" for m in exported])
    (src_dir / "Exports.swift").write_text(exports_src)

    good(f"Shell package generated → {shell_dir / 'Package.swift'}")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("config_pos", nargs="?", help="Path to vendor_config.normalized.json")
    ap.add_argument("--config", default=None)
    ap.add_argument("--vendor-dir", default=None)
    ap.add_argument("--force", action="store_true")
    ap.add_argument(
        "--package",
        dest="only_packages",
        default=None,
        help="Comma-separated package name(s) to process only these packages"
    )
    ap.add_argument("--include-unsupported", action="store_true", help="Process packages marked spmSupported=false")
    args = ap.parse_args()
    # Backward-compatible positional config
    config_path = args.config or args.config_pos or "vendor_config.normalized.json"

    cfg = json.load(open(config_path, "r"))
    vendor_root = pathlib.Path(cfg.get("vendorDir","Vendor") if args.vendor_dir is None else args.vendor_dir)
    ensure_dir(vendor_root)

    rows_for_print = []
    # Keep track of packages that are safe to include in Shell (have a Package.swift)
    shell_include: List[Dict] = []
    overall = {"errors": [], "cloned": []}

    # Optional filtering by package name(s)
    only_set = None
    if args.only_packages:
        only_set = {s.strip() for s in str(args.only_packages).split(",") if s.strip()}

    for p in cfg.get("packages", []):
        name = p.get("name")
        if only_set is not None and name not in only_set:
            continue
        git = (p.get("git") or "").strip()
        target_ver = str(p.get("targetVersion") or p.get("currentVersion") or "")
        binary_headers = p.get("binaryHeaders") or p.get("headers") or {}
        if not binary_headers.get("Referer") and git:
            binary_headers["Referer"] = git
        products = p.get("products") or [name]
        spm_supported = p.get("spmSupported")

        row = {
            "Package": name or "UNKNOWN",
            "Version": target_ver or "HEAD",
            "Clone": "",
            "Binaries": 0,
            "Remaining": 0,
            "Errors": ""
        }

        # Respect spmSupported=false unless explicitly included
        if spm_supported is False and not args.include_unsupported:
            msg = f"{name}: marked spmSupported=false in config; skipping"
            warn(msg)
            row["Clone"] = C.Y + "SKIP" + C.X
            row["Errors"] = msg
            rows_for_print.append(row)
            overall["cloned"].append(name)
            continue

        # Skip if truly done for this version
        pkg_dir = vendor_root / (name or "UNKNOWN")
        if not args.force and _is_pkg_done(str(vendor_root), name, target_ver, str(pkg_dir)):
            dim(f"{name}: already completed for version {target_ver}; skipping")
            row["Clone"] = C.Y + "SKIP" + C.X
            rows_for_print.append(row)
            overall["cloned"].append(name)
            continue

        if not name or not git:
            msg = f"{name}: missing git URL in config"
            bad(msg)
            row["Clone"] = C.R + "FAIL" + C.X
            row["Errors"] = msg
            rows_for_print.append(row)
            overall["errors"].append(msg)
            continue

        # Checkout repo
        ok, ref = checkout_repo(str(pkg_dir), git, target_ver)
        if not ok:
            msg = f"{name}: checkout failed for {target_ver} ({ref})"
            bad(msg)
            row["Clone"] = C.R + "FAIL" + C.X
            row["Errors"] = msg
            rows_for_print.append(row)
            overall["errors"].append(msg)
            continue
        else:
            row["Clone"] = C.G + "OK" + C.X

        # Run post-clone commands if specified in config
        post_cmds = p.get("postCloneCommands") or p.get("postCommands") or []
        if post_cmds:
            ok_post, errs_post = run_post_commands(str(pkg_dir), post_cmds)
            if not ok_post:
                row["Errors"] = (row["Errors"] + "; " if row["Errors"] else "") + "; ".join(errs_post)

        # Parse Package.swift
        pkg_swift_path = pkg_dir / "Package.swift"
        if not pkg_swift_path.exists():
            # Not a swift package; still mark done but skip from Shell generation
            _mark_pkg_done(str(vendor_root), name, target_ver, str(pkg_dir))
            rows_for_print.append(row)
            continue
        # First, patch remote dependency URLs inside manifest to local path-based ones
        try:
            patch_manifest_deps_to_local(str(pkg_swift_path), str(pkg_dir), str(vendor_root), cfg)
        except Exception as e:
            warn(f"Could not patch manifest deps for {name}: {e}")
        pkg_swift = pkg_swift_path.read_text()

        bins = find_binary_targets(pkg_swift)
        bins_dir = pkg_dir / "Binaries"
        ensure_dir(bins_dir)

        remaining = 0
        replaced = 0
        for bt in bins:
            tname = bt["name"]
            burl = bt.get("url")
            if not burl:
                # already local path
                continue

            # download zip
            zip_name = f"{tname}.zip"
            zip_path = bins_dir / zip_name
            ok, err = download_file(burl, str(zip_path), extra_headers=binary_headers)
            if not ok:
                msg = f"{name}:{tname}: download failed → {err}"
                bad(msg)
                remaining += 1
                row["Errors"] = (row["Errors"] + "; " if row["Errors"] else "") + msg
                continue

            # extract artifact
            art_path, tmp_dir, err = extract_spm_artifact(str(zip_path), str(bins_dir), expected_name=tname)
            try:
                if err or not art_path or not os.path.exists(art_path):
                    msg = f"{name}:{tname}: extraction failed → {err or 'unknown'}"
                    bad(msg)
                    remaining += 1
                    row["Errors"] = (row["Errors"] + "; " if row["Errors"] else "") + msg
                    continue
                # patch manifest to path
                rel = os.path.relpath(art_path, start=str(pkg_dir))
                patch_manifest_to_path(str(pkg_swift_path), tname, rel)
                good(f"{name}:{tname}: placed → {art_path}")
                replaced += 1
            finally:
                if tmp_dir and os.path.isdir(tmp_dir):
                    shutil.rmtree(tmp_dir, ignore_errors=True)

        row["Binaries"] = replaced
        row["Remaining"] = remaining

        if row["Errors"] == "" and remaining == 0:
            _mark_pkg_done(str(vendor_root), name, target_ver, str(pkg_dir))
        rows_for_print.append(row)
        # Eligible for Shell if it has a manifest
        shell_include.append({
            "name": name,
            "products": products
        })
        # Remove embedded .git to keep vendored code clean
        remove_git_dir(str(pkg_dir))

    print_table(rows_for_print)
    # Always regenerate Shell package by scanning all vendored packages with manifests
    # Also clean .git folders across vendor tree to ensure vendored copies are source-only
    _cleanup_all_git(vendor_root)
    all_shell_include = _scan_vendor_for_shell(vendor_root, cfg)
    _generate_shell_package(cfg, all_shell_include, str(vendor_root))

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nInterrupted.")
        sys.exit(130)
