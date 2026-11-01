// swift-tools-version: 6.4

import PackageDescription

let package = Package(
    name: "SBJLego",
    platforms: [
        .iOS(.v17),
        .watchOS(.v10),
        .tvOS(.v17),
        .visionOS(.v1),
    ],
    products: [
        .library(
            name: "SBJLego",
            targets: ["SBJLego"]),
    ],
    dependencies: [
        .package(path: "../BLEByJove"),
        .package(path: "../../SBJFoundation/Packages/SBJLocalization"),
    ],
    targets: [
        .target(
            name: "SBJLego",
            dependencies: ["BLEByJove", "SBJLocalization"],
            resources: [.process("Resources")],
            swiftSettings: [
                .enableUpcomingFeature("ApproachableConcurrency"),
                .defaultIsolation(nil),
            ]),
        .testTarget(
            name: "SBJLegoTests",
            dependencies: ["SBJLego"],
            swiftSettings: [
                .enableUpcomingFeature("ApproachableConcurrency"),
                .defaultIsolation(nil),
            ]),
    ]
)
