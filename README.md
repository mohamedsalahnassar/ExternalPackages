# CocoaPods Clone Capture (v24)

**Autonomous detection** of the in-place patch target:
- Detects CocoaPods version via `pod --version`
- Searches Homebrew Cellar for that version first, then `opt/`
- Falls back to RubyGems discovery (and common system gem paths)
- You can still override with `CP_DL_GIT_RB=/full/path/to/git.rb`

## Usage
```bash
unzip cocoapods_capture_v24.zip
cd cocoapods_capture_v24

# Non-invasive (recommended)
./Scripts/fetch_pods.sh --podfile-dir ./Sample --dest ./output --verbose

# In-place patch (auto-detect target; backups & restores)
./Scripts/fetch_pods.sh --patch-downloader-git --podfile-dir ./Sample --dest ./output --verbose
# manual restore:
./Scripts/fetch_pods.sh --restore-downloader-git
```

Cloned repos end up in `DEST/repos/`. Set `CP_GIT_SHALLOW=0` to disable shallow clones up front.
