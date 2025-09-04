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
Last updated: `2025-09-04T06:13:36Z`

| Package | Version | Size |
|---|---|---:|
| abseil-cpp-swiftpm | 0.20200225.4 | 7.5 MB |
| AEPAnalytics | 4.0.0 | 595.0 KB |
| AEPAssurance | 4.1.1 | 16.6 MB |
| AEPCore | 4.2.4 | 4.0 MB |
| AEPEdge | 4.3.1 | 30.6 MB |
| AEPEdgeConsent | 4.0.0 | 286.2 KB |
| AEPEdgeIdentity | 4.0.0 | 456.7 KB |
| AEPRulesEngine | 4.0.0 | 89.4 KB |
| AEPTarget | 4.0.3 | 1.0 MB |
| Alamofire | 4.9.1 | 6.7 MB |
| AppDynamicsAgent | 2023.10.1 | 27.0 MB |
| BigInt | 5.2.0 | 1.9 MB |
| boringssl-swiftpm | 0.7.2 | 115.8 MB |
| collectionconcurrencykit | 0.2.0 | 38.9 KB |
| CryptoSwift | 1.4.1 | 781.8 KB |
| DeviceKit | 5.5.0 | 359.1 KB |
| DGCharts | 5.0.0 | 20.8 MB |
| Firebase | 8.9.1 | 59.2 MB |
| FloatingPanel | 2.8.1 | 6.9 MB |
| googleappmeasurement | 8.9.1 | 64.2 MB |
| GoogleDataTransport | 9.1.2 | 996.3 KB |
| GoogleMaps | 8.3.1 | 84.8 MB |
| GooglePlaces | 8.3.0 | 11.2 MB |
| googleutilities | 7.13.3 | 774.5 KB |
| grpc-swiftpm | 1.28.4 | 35.2 MB |
| gtm-session-fetcher | 1.7.2 | 1.1 MB |
| Insider | 1.4.1 | 4.6 MB |
| IOSSecuritySuite | 2.0.2 | 221.8 KB |
| iProov | 11.1.1 | 15.4 MB |
| IQKeyboardManagerSwift | 6.3.0 | 4.2 MB |
| ISPageControl | 0.1.0 | 15.9 MB |
| Jumio | 4.11.0 | 49.1 MB |
| KeychainAccess | 3.2.1 | 1.2 MB |
| Kingfisher | 7.9.1 | 2.1 MB |
| leveldb | 1.22.5 | 958.4 KB |
| Lottie | 4.5.0 | 151.0 MB |
| Mantis | 2.8.0 | 31.6 MB |
| MEPSDK | 9.8.6 | 144.7 MB |
| MGSwipeTableCell | 1.6.14 | 2.3 MB |
| mobile-sdk-ios | 4.12.0 |  |
| nanopb | 0.3.9.8 | 1.1 MB |
| Promises | 2.0.0 | 625.1 KB |
| RxDataSources | 5.0.0 | 386.0 KB |
| RxKeyboard | 2.0.0 | 59.5 KB |
| RxSwift | 6.6.0 | 11.2 MB |
| RxSwiftExt | 6.2.1 | 11.5 MB |
| SkeletonView | 1.30.1 | 3.1 MB |
| sourcekitten | 0.33.1 | 11.7 MB |
| SVProgressHUD | 2.3.1 | 353.8 KB |
| swift-argument-parser | 1.2.3 | 1.4 MB |
| swift-protobuf | 1.31.0 | 78.9 MB |
| swift-syntax | HEAD | 9.5 MB |
| SwiftLint | 0.50.1 | 2.8 MB |
| swiftytexttable | 0.9.0 | 496.3 KB |
| swxmlhash | 7.0.2 | 368.8 KB |
| TensorFlowLiteC | 2.14.0 | 115.5 MB |
| yams | 5.4.0 | 1.4 MB |
| ZIPFoundation | 0.9.19 | 27.6 MB |
<!-- END VENDOR MATRIX -->


















## Notes
- To keep a package vendor‑only (not linked by the root package), set its `products` to an empty list.
- `exports` customizes re‑exports; omit to mirror `products`.
- `postCloneCommands` run inside each vendored folder.

## License
See individual package licenses in `External/<Package>/` and this repository’s license.

