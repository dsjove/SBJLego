//
//  Lighting.swift
//  Infrastructure
//
//  Created by David Giovannini on 3/27/25.
//

import Foundation
import BLEByJove

public protocol LightingProtocol {
	associatedtype Value: ControlledProperty where Value.P == Double

	var power: Value { get }
	var calibration: Value? { get }
	var sensed: Value? { get }

	var hasDimmer : Bool { get }
	var increment: Double? { get }
}

public extension LightingProtocol {
	var increment: Double? { nil }

	func reset() {
		self.power.reset()
		self.calibration?.reset()
		self.sensed?.reset()
	}

	func fullStop() {
		self.power.control = 0
	}
}

public struct BTLighting: LightingProtocol {
	public typealias Value = BTProperty<ScaledTransformer<UInt8>>
	
	public let power: Value
	public let calibration: Value?
	public let sensed: Value?
	public let increment: Double? = 0.01
	public let hasDimmer: Bool = true

	public init(device: any BTBroadcaster) {
		self.power = Value(
			broadcaster: device,
			controlChar: BTCharacteristicIdentity(
				component: FacilityPropComponent.lights,
				category: FacilityPropCategory.power,
				channel: BTPropChannel.control),
			feedbackChar: BTCharacteristicIdentity(
				component: FacilityPropComponent.lights,
				category: FacilityPropCategory.power,
				channel: BTPropChannel.feedback),
			transfomer: ScaledTransformer(255))

		self.calibration = Value(
			broadcaster: device,
			characteristic: BTCharacteristicIdentity(
				component: FacilityPropComponent.lights,
				category: FacilityPropCategory.calibration),
			transfomer: ScaledTransformer(255),
			defaultValue: 1.0)

		self.sensed = Value(
			broadcaster: device,
			characteristic: BTCharacteristicIdentity(
				component: FacilityPropComponent.lights,
				category: FacilityPropCategory.sensed,
				channel: BTPropChannel.feedback),
			transfomer: ScaledTransformer(255))
	}
}
