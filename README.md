# ExternalPackages – Vendor Tooling

This repository vendors third‑party Swift Packages into `External/` and generates a single root package named `ExternalPackages`. It supports offline builds by converting remote binaries to local `path:` targets and rewriting remote dependencies to local paths.

## What It Does
- Binary targets: Converts `.binaryTarget(url: ...)` to local `path:` XCFrameworks under `External/<Pkg>/Binaries`.
- Localize deps: Rewrites `.package(url: ...)` entries in vendored manifests to local `package(path: ...)` where possible.
- Root package: Generates a top‑level `Package.swift` that depends on all vendored packages you configured and re‑exports their modules.
- Cleaning: Strips `.git`, CI configs, and stray archives from vendored trees; optional deep clean trims files not referenced by targets.

## Requirements
- macOS with Xcode toolchain and Python 3.
- Network access for the first run when downloading/cloning packages and binary zips.

## Quick Start
- Configure `external_packages.json` with the packages you need (see schema below).
- Generate or refresh vendors and root package:
  - `python3 external_packages_builder.py`
- Open the root `Package.swift` (package name `ExternalPackages`) in Xcode and build.

## Commands
- Vendoring/update: Processes packages in the config, converts binaries, rewrites manifests, and regenerates the root package.
  - `python3 external_packages_builder.py`
- Cleanup only: Cleans VCS/CI metadata and leftover `*.zip` archives; updates README matrix. No cloning/downloading.
  - `python3 external_packages_builder.py --cleanup-only`
- Prune removed: Deletes folders in `External/` that are not listed in the JSON; also updates root `Package.swift`.
  - `python3 external_packages_builder.py --prune`
- Deep clean: Trims vendored packages to only files referenced by their `Package.swift` (targets, resources, public headers).
  - `python3 external_packages_builder.py --deep-clean`
  - Combine with cleanup only:
    - `python3 external_packages_builder.py --cleanup-only --deep-clean`
- Process subset: Process only select packages by name (comma‑separated).
  - `python3 external_packages_builder.py --package AEPAnalytics,Kingfisher`
- Force reprocess: Ignore the “done” cache and redo work for matching packages.
  - `python3 external_packages_builder.py --force`
- Include unsupported: Process packages marked `"spmSupported": false`.
  - `python3 external_packages_builder.py --include-unsupported`
- Custom vendor dir or config path:
  - `python3 external_packages_builder.py --vendor-dir External --config external_packages.json`

## JSON Configuration

Top‑level fields
- `shellPackageName`: Name for the root package generated at repository root.
- `vendorDir`: Directory where packages are vendored (default `External`).
- `packages`: Array of package entries (schema below).

Per‑package fields
- `name`: Folder name under `vendorDir` and identifier in the config.
- `git`: Git URL of the package.
- `currentVersion`: Observed/current version (informational).
- `targetVersion`: Version or branch/tag to checkout (used for vendoring).
- `spmSupported`: When false, the package is skipped unless `--include-unsupported` is used.
- `products`: SwiftPM product names to add as dependencies in the root package. Empty list means vendored but not linked.
- `exports`: Module names to re‑export from the root package. Defaults to `products` when omitted.
- `packageName`: Optional override for the dependency package name used by SwiftPM if it differs from the folder name.
- `postCloneCommands`: Array of shell commands executed inside the package folder after checkout.
  - Placeholders: `{PKG_DIR}`, `{VENDOR_DIR}`, `{PKG_NAME}` are expanded.
- `binaryHeaders` (or `headers`): Extra HTTP headers used when downloading binary zips (for protected hosts).
- `xcframeworks`: Reserved for future use when pointing at pre‑fetched artifacts.
- `notes`: Free‑form text for maintainers.

Example package entry
```json
{
  "name": "AppDynamicsAgent",
  "git": "https://github.com/CiscoDevNet/AppDynamicsAgent.git",
  "currentVersion": "2022.5.0",
  "targetVersion": "2023.10.1",
  "spmSupported": true,
  "products": ["AppDynamicsAgent"],
  "exports": ["AppDynamicsAgent"],
  "postCloneCommands": [
    "python3 -c \"p='Package.swift'; s=open(p).read(); s=s.replace('old','new'); open(p,'w').write(s)\""
  ],
  "binaryHeaders": {"Referer": "https://github.com/CiscoDevNet/AppDynamicsAgent"},
  "notes": "SPM added in 23.10.x"
}
```

## Behavior Details
- Binary conversion: For each vendored manifest’s `.binaryTarget(url: ...)`, a zip is downloaded, the `.xcframework` or `.artifactbundle` extracted into `External/<Pkg>/Binaries/<Target>/`, and the manifest is rewritten to `path:`.
- Dependency localization: Any `.package(url: ...)` in vendored manifests is rewritten to `package(path: ...)` if the URL matches a package you vendored (normalized across `git@`/`https` forms).
- Done registry: The tool uses `External/.packages_done.json` and per‑package `.done.json` to skip repeat work unless `--force` is given.
- Re‑exports: The root package re‑exports modules listed in `exports` (or `products` if not specified). Names must be valid Swift identifiers to be re‑exported.
- Cleaning: Always removes `.git`, `.github`, `.circleci`, `.gitignore`, `.swiftlint.yml`. Deep clean additionally trims files not referenced by targets/resources/public headers.

## Common Workflows
- Remove a package completely
  - Edit `external_packages.json` and delete the package entry.
  - Regenerate and prune: `python3 external_packages_builder.py --prune`
  - Optional: Deep cleanup of the remaining vendors: `--deep-clean`.
- Add or update a package
  - Add/modify an entry with `git` and `targetVersion`.
  - Run: `python3 external_packages_builder.py` (or `--force` to redo work).
  - If the vendor has remote deps you already vendor, they are rewritten to local paths automatically.

## Authentication & Hosts
- GitHub: honors `GITHUB_TOKEN` / `GH_TOKEN`. For release asset URLs under `api.github.com`, uses `Accept: application/octet-stream`.
- JFrog/Artifactory: supports API Key (`X-JFrog-Art-Api`), Bearer tokens, Basic auth; tries the Download API and Storage API to resolve direct downloads.
- Custom headers: set per‑package `binaryHeaders` to inject arbitrary headers; `Referer` defaults to the package Git URL when available.

## Caches & Build Hygiene
- Xcode → File → Packages → Reset Package Caches; Product → Clean Build Folder.
- CLI: `swift package reset && swift package resolve`.
- Delete `~/Library/Developer/Xcode/DerivedData/*/SourcePackages` if stale.

## Vendored Packages – Live Matrix
The table below is generated by the script after each run.

<!-- BEGIN VENDOR MATRIX -->
Last updated: `N/A`

| Package | Version | Size |
|---|---|---:|
<!-- END VENDOR MATRIX -->

## Notes
- To keep a package vendor‑only (not linked by the root package), set its `products` to an empty list.
- `exports` customizes re‑exports; omit to mirror `products`.
- `postCloneCommands` run inside each vendored folder.

## License
See individual package licenses in `External/<Package>/` and this repository’s license.

