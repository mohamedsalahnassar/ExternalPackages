# CocoaPods Clone Capture (v25)

**Zero-config detection** for in-place patch:
- Scans Homebrew **Cellar** for the exact `pod --version`, then `opt/cocoapods/libexec`, then any Cellar.
- Searches under `libexec/**` patterns that match both `gems/` and `lib/ruby/gems/` layouts.
- Falls back to RubyGems discovery.
- If still not found, the runner **automatically falls back to the RUBYOPT (non-invasive) mode** so your run succeeds.

## Usage
```bash
unzip cocoapods_capture_v25.zip
cd cocoapods_capture_v25

# Non-invasive (recommended)
./Scripts/fetch_pods.sh --podfile-dir ./Sample --dest ./output --verbose

# In-place patch attempt (auto-detect; falls back to RUBYOPT if not found)
./Scripts/fetch_pods.sh --patch-downloader-git --podfile-dir ./Sample --dest ./output --verbose
```
Cloned repos end up in `DEST/repos/`. Pass `--unshallow` to fetch full Git histories instead of the default shallow clones.

## Options

The script accepts the following flags (they may be combined unless noted):

| Flag | Description |
| --- | --- |
| `--podfile-dir DIR` | Directory containing the `Podfile` to install (defaults to `Sample`). |
| `--dest DIR` | Output directory where capture results are written (defaults to `output`). |
| `--verbose` | Runs `pod install` in verbose mode. |
| `--unshallow` | Fetches full Git histories (`CP_GIT_SHALLOW=0`). |
| `--patch-downloader-git` | Attempts in-place patching of CocoaPods' `downloader/git.rb`; falls back to RUBYOPT on failure. |
| `--restore-downloader-git` | Restores `downloader/git.rb` to its original state and exits. |
| `--no-repo-update` | Passes the same flag to `pod install` to skip updating the specs repos. |
| `--keep-all` | Retains Pods, logs, and temporary directories instead of cleaning up. |
| `--final-dir-name NAME` | Changes the directory name under `DEST` where repositories are finalized (defaults to `repos`). |

The script leaves the CocoaPods specs repositories in their default location; only the library clone directories are captured via the patched `git.rb`.

## Example command combinations

**Basic non-invasive capture**
```bash
./Scripts/fetch_pods.sh --podfile-dir ./Sample --dest ./output
```

**Non-invasive with verbose output and skipping spec repo updates**
```bash
./Scripts/fetch_pods.sh \
  --podfile-dir ./Sample \
  --dest ./output \
  --verbose \
  --no-repo-update
```

**Fetch full histories and keep all generated files**
```bash
./Scripts/fetch_pods.sh \
  --podfile-dir ./Sample \
  --dest ./output \
  --unshallow \
  --keep-all
```

**In-place downloader patch**
```bash
./Scripts/fetch_pods.sh \
  --patch-downloader-git \
  --podfile-dir ./Sample \
  --dest ./output
```

**In-place patch with no repo update and custom output folder**
```bash
./Scripts/fetch_pods.sh \
  --patch-downloader-git \
  --podfile-dir ./Sample \
  --dest ./capture \
  --no-repo-update \
  --final-dir-name libs
```

**Restore CocoaPods' downloader after patching**
```bash
./Scripts/fetch_pods.sh --restore-downloader-git
```

These examples may be adapted to match your project paths. Repositories are written to `DEST/<final-dir-name>` (default `DEST/repos/`).

## Listing external dependencies

Use `Scripts/list_external_deps.sh` to print pods that come from external sources (e.g., git repositories) listed in a `Podfile.lock`:

```bash
./Scripts/list_external_deps.sh /path/to/Podfile.lock
```

Each line outputs the pod name, the source URL, and the tag/commit when available. Pods fetched from the central specs repo are ignored.
