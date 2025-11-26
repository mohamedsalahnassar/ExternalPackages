// swift-tools-version:4.0
import PackageDescription

// All dependencies are vendored locally; no remote package references are required.
let dependencies: [Package.Dependency] = []

let package = Package(
    name: "ZIPFoundation",
    products: [
        .library(name: "ZIPFoundation", targets: ["ZIPFoundation"])
    ],
	dependencies: dependencies,
    targets: [
        .target(name: "ZIPFoundation"),
		.testTarget(name: "ZIPFoundationTests", dependencies: ["ZIPFoundation"])
    ]
)
