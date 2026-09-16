//
//  PFFacility.swift
//  Infrastructure
//
//  Created by David Giovannini on 1/17/26.
//

import Foundation
import BLEByJove
import SBJFoundation

public protocol PFFacilityMeta: PFMeta {
	var category: FacilityCategory { get }
	var name: String { get }
	var image: ImageReference { get }
}

@MainActor
@Observable
public class PFFacility<M: PFFacilityMeta>: @MainActor MotorizedFacility {
	private let device: PFDevice<M>

	public var id: UUID { device.id }
	public var category: FacilityCategory { device.info.category }
	public var name: String { device.info.name }
	public var image: ImageReference { device.info.image }

	public private(set) var motor: PFMotor
	public private(set) var lighting: PFLighting?

	public init(device: PFDevice<M>) {
		self.device = device
		self.motor = PFMotor(device: device, port: .A)
		self.lighting = PFLighting(device: device, port: .B)
	}

	public var hasConnectionState: Bool { false }
	public var connectionState: BLEByJove.ConnectionState { device.pfConnectionState }
	public func connect() {}
	public func disconnect() {}

	public func reset() {
		self.motor.reset()
		self.lighting?.reset()
	}
}
