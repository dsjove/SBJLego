import Foundation
import Observation
import SBJKit
import BLEByJove

public struct FacilityCategory: Hashable, Sendable {
	let rawValue: String
	public init(_ rawValue: String) {
		self.rawValue = rawValue
	}
}

public extension FacilityCategory {
	static let transportation = FacilityCategory("transportation")
	static let housing = FacilityCategory("housing")
}

public protocol HardwareConnecting {
	var hasConnectionState: Bool { get }
	var autoConnects: Bool { get }
	var connectionState: ConnectionState { get }
	func connect()
	func disconnect()

	var heartBeat: Int { get }
	func fullStop()
}

public extension Facility {
	var autoConnects: Bool { true }
	var hasConnectionState: Bool { true }
	var heartBeat: Int { connectionState == .connected ? 0 : -1 }
}

public protocol Facility: HardwareConnecting, Identifiable {
	var id: UUID { get }

	var category: FacilityCategory { get }
	var name: String { get }
	var image: ImageName { get }

	var canSetName: Bool { get }
	func change(name: String)

	var battery: Double? { get }
}

public extension Facility {
	var canSetName: Bool { false }
	func change(name: String) {}
	var battery: Double? { nil }
}

@Observable
public class UnsupportedFacility: Facility {
	public let id = UUID()
	public let name: String
	public let category: FacilityCategory = .transportation
	public let image: ImageName = .system("questionmark.diamond")

	public let connectionState: BLEByJove.ConnectionState = .disconnected

	public init(name: String) {
		self.name = name
	}

	public func connect() {}
	public func fullStop() {}
	public func disconnect() {}
}
