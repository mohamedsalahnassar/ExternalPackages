// swift-tools-version:5.7
import PackageDescription

#if os(macOS)
private let addCryptoSwift = false
#else
private let addCryptoSwift = true
#endif

let frameworkDependencies: [Target.Dependency] = [
    .product(name: "IDEUtils", package: "swift-syntax"),
    .product(name: "SourceKittenFramework", package: "SourceKitten"),
    .product(name: "SwiftSyntax", package: "swift-syntax"),
    .product(name: "SwiftSyntaxBuilder", package: "swift-syntax"),
    .product(name: "SwiftParser", package: "swift-syntax"),
    .product(name: "SwiftOperators", package: "swift-syntax"),
    "Yams",
]
+ (addCryptoSwift ? ["CryptoSwift"] : [])

let package = Package(
    name: "SwiftLint",
    platforms: [.macOS(.v12)],
    products: [
        .executable(name: "swiftlint", targets: ["swiftlint"]),
        .library(name: "SwiftLintFramework", targets: ["SwiftLintFramework"]),
        .plugin(name: "SwiftLintPlugin", targets: ["SwiftLintPlugin"])
    ],
    dependencies: [
        .package(path: "../swift-argument-parser"),
        .package(path: "../swift-syntax"),
        .package(path: "../sourcekitten"),
        .package(path: "../yams"),
        .package(path: "../swiftytexttable"),
        .package(path: "../collectionconcurrencykit")
    ] + (addCryptoSwift ? [.package(path: "../CryptoSwift")] : []),
    targets: [
        .plugin(
            name: "SwiftLintPlugin",
            capability: .buildTool(),
            dependencies: [
                .target(name: "swiftlint")
            ]
        ),
        .executableTarget(
            name: "swiftlint",
            dependencies: [
                .product(name: "ArgumentParser", package: "swift-argument-parser"),
                "CollectionConcurrencyKit",
                "SwiftLintFramework",
                "SwiftyTextTable",
            ]
        ),
        .testTarget(
            name: "CLITests",
            dependencies: [
                "swiftlint"
            ]
        ),
        .target(
            name: "SwiftLintFramework",
            dependencies: frameworkDependencies
        ),
        .testTarget(
            name: "SwiftLintTestHelpers",
            dependencies: [
                "SwiftLintFramework"
            ]
        ),
        .testTarget(
            name: "SwiftLintFrameworkTests",
            dependencies: [
                "SwiftLintFramework",
                "SwiftLintTestHelpers"
            ],
            exclude: [
                "Resources",
            ]
        ),
        .testTarget(
            name: "GeneratedTests",
            dependencies: [
                "SwiftLintFramework",
                "SwiftLintTestHelpers"
            ]
        ),
        .testTarget(
            name: "IntegrationTests",
            dependencies: [
                "SwiftLintFramework",
                "SwiftLintTestHelpers"
            ]
        ),
        .testTarget(
            name: "ExtraRulesTests",
            dependencies: [
                "SwiftLintFramework",
                "SwiftLintTestHelpers"
            ]
        ),
    ]
)
