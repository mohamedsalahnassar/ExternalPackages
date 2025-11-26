// swift-tools-version:5.7

import Foundation
import PackageDescription

let package = Package(
  name: "swift-syntax-dev-utils",
  platforms: [
    .macOS(.v13)
  ],
  products: [
    .executable(name: "swift-syntax-dev-utils", targets: ["swift-syntax-dev-utils"])
  ],
  targets: [
    .executableTarget(
      name: "swift-syntax-dev-utils",
      dependencies: [
        .product(name: "ArgumentParser", package: "swift-argument-parser")
      ]
    )
  ]
)
package.dependencies += [
  .package(path: "../../swift-argument-parser")
]
