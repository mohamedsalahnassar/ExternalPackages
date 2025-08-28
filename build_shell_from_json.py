#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os, sys, json, re, shutil, subprocess, urllib.request, urllib.error, zipfile, tempfile, uuid, hashlib, argparse, pathlib
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
def run(cmd, cwd=None):
    return subprocess.check_call(cmd, cwd=cwd)

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
    candidates = [base, f"v{base}", f"origin/{base}"]
    tag_candidates = [("tag", base), ("tag", f"v{base}")]

    for ref in candidates:
        try:
            run(["git", "fetch", "--depth", "1", "origin", ref], cwd=dest_dir)
            run(["git", "checkout", "FETCH_HEAD"], cwd=dest_dir)
            good(f"Checked out {ref}")
            if has_submodules(dest_dir):
                try: run(["git", "submodule", "update", "--init", "--recursive", "--depth", "1"], cwd=dest_dir)
                except subprocess.CalledProcessError:
                    run(["git", "submodule", "update", "--init", "--recursive"], cwd=dest_dir)
            return True, ref
        except subprocess.CalledProcessError:
            continue

    for kind, ref in tag_candidates:
        try:
            run(["git", "fetch", "--depth", "1", "origin", kind, ref], cwd=dest_dir)
            run(["git", "checkout", "FETCH_HEAD"], cwd=dest_dir)
            good(f"Checked out {kind} {ref}")
            if has_submodules(dest_dir):
                try: run(["git", "submodule", "update", "--init", "--recursive", "--depth", "1"], cwd=dest_dir)
                except subprocess.CalledProcessError:
                    run(["git", "submodule", "update", "--init", "--recursive"], cwd=dest_dir)
            return True, ref
        except subprocess.CalledProcessError:
            continue

    try:
        run(["git", "fetch", "--tags", "origin"], cwd=dest_dir)
        for ref in (f"v{base}", base):
            try:
                run(["git", "checkout", f"tags/{ref}"], cwd=dest_dir)
                good(f"Checked out tags/{ref}")
                if has_submodules(dest_dir):
                    try: run(["git", "submodule", "update", "--init", "--recursive", "--depth", "1"], cwd=dest_dir)
                    except subprocess.CalledProcessError:
                        run(["git", "submodule", "update", "--init", "--recursive"], cwd=dest_dir)
                return True, ref
            except subprocess.CalledProcessError:
                continue
    except subprocess.CalledProcessError as e:
        bad(f"Tag fetch failed: {e}")

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

# ---------- Download + Extract ----------
def download_file(url, dest, token=None, extra_headers=None):
    step(f"Downloading binary: {url}")
    try:
        if os.path.isfile(dest) and os.path.getsize(dest) > 0:
            info(f"[skip] download_file: {dest} already present")
            return True, None
    except Exception:
        pass
    headers = {
        "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15",
        "Accept": "application/zip, */*;q=0.8",
        "Accept-Language": "en-US,en;q=0.9",
        "Connection": "keep-alive",
    }
    if token:
        headers["Authorization"] = f"Bearer {token}"
    if extra_headers:
        headers.update(extra_headers)

    def attempt(h, tag):
        try:
            req = urllib.request.Request(url, headers=h)
            with urllib.request.urlopen(req, timeout=240) as r, open(dest, "wb") as f:
                shutil.copyfileobj(r, f)
            # Validate zip signature
            try:
                if not zipfile.is_zipfile(dest):
                    ct = ""
                    try:
                        ct = r.headers.get("Content-Type", "")
                    except Exception:
                        pass
                    # Read a few bytes to hint what we got
                    head = b""
                    try:
                        with open(dest, "rb") as _fh:
                            head = _fh.read(64)
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
    ok, err = attempt(headers, "initial")
    if ok: return True, None
    if "HTTP Error 403" in (err or "") or "HTTP Error 401" in (err or ""):
        if "Referer" not in headers:
            headers["Referer"] = "https://github.com"
        ok2, err2 = attempt(headers, "retry+referer")
        if ok2: return True, None
        return False, err2 or err
    return False, err
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

# ---------- Main ----------
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("config_pos", nargs="?", help="Path to vendor_config.normalized.json")
    ap.add_argument("--config", default=None)
    ap.add_argument("--vendor-dir", default=None)
    ap.add_argument("--force", action="store_true")
    args = ap.parse_args()
    # Backward-compatible positional config
    config_path = args.config or args.config_pos or "vendor_config.normalized.json"

    cfg = json.load(open(config_path, "r"))
    vendor_root = pathlib.Path(cfg.get("vendorDir","Vendor") if args.vendor_dir is None else args.vendor_dir)
    ensure_dir(vendor_root)

    rows_for_print = []
    dep_lines: List[str] = []
    overall = {"errors": [], "cloned": []}

    for p in cfg.get("packages", []):
        name = p.get("name")
        git = (p.get("git") or "").strip()
        target_ver = str(p.get("targetVersion") or p.get("currentVersion") or "")
        binary_headers = p.get("binaryHeaders") or p.get("headers") or {}
        if not binary_headers.get("Referer") and git:
            binary_headers["Referer"] = git
        products = p.get("products") or [name]

        row = {
            "Package": name or "UNKNOWN",
            "Version": target_ver or "HEAD",
            "Clone": "",
            "Binaries": 0,
            "Remaining": 0,
            "Errors": ""
        }

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

        # Parse Package.swift
        pkg_swift_path = pkg_dir / "Package.swift"
        if not pkg_swift_path.exists():
            # Not a swift package; still mark done
            _mark_pkg_done(str(vendor_root), name, target_ver, str(pkg_dir))
            rows_for_print.append(row)
            continue
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

        dep_lines.append(f'        .package(path: "../{cfg.get("vendorDir","Vendor")}/{name}"),')

    print_table(rows_for_print)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nInterrupted.")
        sys.exit(130)
