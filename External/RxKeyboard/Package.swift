// swift-tools-version:5.1

import PackageDescription

let package = Package(
  name: "RxKeyboard",
  products: [
    .library(name: "RxKeyboard", targets: ["RxKeyboard"]),
  ],
  dependencies: [
    .package(path: "../RxSwift"),
  ],
  targets: [
    .target(name: "RxKeyboard", dependencies: ["RxSwift", "RxCocoa"]),
  ]
)
