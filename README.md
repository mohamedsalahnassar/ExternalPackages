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
Cloned repos end up in `DEST/repos/`. Add `CP_GIT_SHALLOW=0` to disable shallow clones.
