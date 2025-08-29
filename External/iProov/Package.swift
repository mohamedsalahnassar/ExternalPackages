// swift-tools-version:5.3

import PackageDescription

let package = Package(
    name: "iProov",
    platforms: [.iOS(.v12)],
    products: [
        .library(
            name: "iProov",
            targets: ["iProovTargets"]
        )
    ],
    targets: [
        .binaryTarget(name: "iProov", path: "Binaries/iProov.xcframework"),
        .target(
            name: "iProovTargets",
            dependencies: [
                .target(name: "iProov", condition: .when(platforms: .some([.iOS])))
            ],
            path: "Sources",
            resources: [
                .copy("Resources/PrivacyInfo.xcprivacy")
            ]
        ),
    ]
)