// swift-tools-version: 5.9

import PackageDescription

let package = Package(
    name: "SBJLego",
    platforms: [
        .iOS(.v17),
        .watchOS(.v10),
    ],
    products: [
        .library(
            name: "SBJLego",
            targets: ["SBJLego"]),
    ],
    dependencies: [
        .package(path: "../BLEByJove"),
        .package(path: "../SBJKit"),
    ],
    targets: [
        .target(
            name: "SBJLego",
            dependencies: ["BLEByJove", "SBJKit"]),
        .testTarget(
            name: "SBJLegoTests",
            dependencies: ["SBJLego"]),
    ]
)
