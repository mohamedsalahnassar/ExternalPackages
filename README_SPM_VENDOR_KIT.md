
# SPM Vendor Kit

This bundle contains:
- `build_shell_from_json.py` — clones each dependency, auto-localizes `.binaryTarget(url:)` to local `.xcframework` paths, and generates a `Shell/` package with path dependencies.
- `validate_vendor.py` — validates vendored packages and the Shell package; can optionally build.
- `vendor_config.json` — package list, versions, git URLs; the script reads from here.
- `DESTINATION_iOS_Simulator.example.json` — sample `--destination` file for iOS Simulator builds.

## Quick start
1) Place all files at your repo root.
2) Edit `vendor_config.json` to fill any missing `git` URLs or adjust `targetVersion` as needed.
3) (Optional) export a token if your binary ZIPs are private on GHES:
   ```bash
   export GHES_TOKEN=<your-ghes-personal-access-token>
   ```
4) Vendor & generate the Shell package:
   ```bash
   python3 build_shell_from_json.py vendor_config.json
   ```
5) Validate:
   ```bash
   python3 validate_vendor.py --config vendor_config.json
   # or also build the shell (recommended on macOS):
   python3 validate_vendor.py --config vendor_config.json --build --destination DESTINATION_iOS_Simulator.example.json
   ```

### Notes
- The vendoring script backs up each modified `Package.swift` as `Package.swift.orig` before patching.
- If a package has no `Package.swift` (pure CocoaPods), the script will warn you to add one.
- Re-run the vendoring script any time you change `vendor_config.json`; it is idempotent.
