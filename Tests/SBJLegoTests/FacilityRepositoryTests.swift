import Observation
import XCTest
import SBJFoundation
import BLEByJove
@testable import SBJLego

private struct TestDevice: DeviceIdentifiable {
    let id: UUID
    let name: String
}

@MainActor
@Observable
private final class TestScanner: DeviceScanner {
    var scanning = false
    var devices: [TestDevice] = []
}

@MainActor
@Observable
private final class TestFacility: Facility {
    let id: UUID
    let category: FacilityCategory = .transportation
    let image: ImageReference = .none
    let name: String
    private(set) var connectionState: ConnectionState = .disconnected
    private(set) var connectCount = 0

    init(id: UUID = UUID(), name: String) {
        self.id = id
        self.name = name
    }

    func connect() {
        connectCount += 1
        connectionState = .connected
    }

    func disconnect() { connectionState = .disconnected }
    func fullStop() {}
}

@MainActor
final class FacilityRepositoryTests: XCTestCase {
    func testScannerCreatesSortsAndRemovesFacilities() async {
        let repository = FacilityRepository()
        let scanner = TestScanner()
        var created: [UUID: TestFacility] = [:]

        repository.addScanner(scanner) { device in
            let facility = TestFacility(id: device.id, name: device.name)
            created[device.id] = facility
            return [facility]
        }

        let z = TestDevice(id: UUID(), name: "Zulu")
        let a = TestDevice(id: UUID(), name: "Alpha")
        scanner.devices = [z, a]
        await Task.yield()
        await Task.yield()

        XCTAssertEqual(repository.facilities.map { $0.value.name }, ["Alpha", "Zulu"])
        XCTAssertEqual(created[z.id]?.connectCount, 1)
        XCTAssertEqual(created[a.id]?.connectCount, 1)

        // Publishing the same device again must not recreate/reconnect its facility.
        scanner.devices = [z, a]
        await Task.yield()
        XCTAssertEqual(created[z.id]?.connectCount, 1)
        XCTAssertEqual(created[a.id]?.connectCount, 1)

        scanner.devices = [a]
        await Task.yield()
        await Task.yield()
        XCTAssertEqual(repository.facilities.map { $0.value.name }, ["Alpha"])
    }

    func testSetScanningForwardsToAllScanners() {
        let repository = FacilityRepository()
        let first = TestScanner()
        let second = TestScanner()
        repository.addScanner(first) { _ in [] }
        repository.addScanner(second) { _ in [] }

        repository.setScanning(true)
        XCTAssertTrue(first.scanning)
        XCTAssertTrue(second.scanning)
    }
}
