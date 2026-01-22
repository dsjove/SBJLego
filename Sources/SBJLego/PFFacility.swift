//
//  PFFacility.swift
//  Infrastructure
//
//  Created by David Giovannini on 1/17/26.
//

import Foundation
import BLEByJove
import SBJKit

@Observable
public class PFFacility: MotorizedFacility {
	private let device: PFDevice
	public let category: FacilityCategory

	public var id: UUID { device.id }
	public var name: String { device.name }
	public var image: ImageName { device.image }

	public private(set) var motor: PFMotor
	public private(set) var lighting: PFLighting?

	public init(device: PFDevice, category: FacilityCategory) {
		self.device = device
		self.category = category
		self.motor = PFMotor(device: device, port: .A)
		self.lighting = PFLighting(device: device, port: .B)
	}

	public var hasConnectionState: Bool { false }
	public var connectionState: BLEByJove.ConnectionState { device.transmitter.pfConnectionState }
	public func connect() {}
	public func disconnect() {}

	public func reset() {
		self.motor.reset()
		self.lighting?.reset()
	}
}
