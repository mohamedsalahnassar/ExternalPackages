// swift-tools-version:5.3
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "Insider",
    products: [
        .library(
            name: "InsiderMobile",
            targets: ["InsiderMobile"]),
        .library(
            name: "InsiderGeofence",
            targets: ["InsiderGeofence"]),
        .library(
            name: "InsiderMobileAdvancedNotification",
            targets: ["InsiderMobileAdvancedNotification"])
    ],
    targets: [
        .binaryTarget(name: "InsiderMobile", path: "Binaries/InsiderMobile.xcframework"),
        .binaryTarget(name: "InsiderGeofence", path: "Binaries/InsiderGeofence.xcframework"),
        .binaryTarget(name: "InsiderMobileAdvancedNotification", path: "Binaries/InsiderMobileAdvancedNotification.xcframework"),
        .target(
            name: "InsiderNotificationContent",
            dependencies: ["InsiderMobileAdvancedNotification"],
            path: "./",
            resources: [
                .process("InsiderInterface.storyboard")
            ]
        ),

    ]
)
