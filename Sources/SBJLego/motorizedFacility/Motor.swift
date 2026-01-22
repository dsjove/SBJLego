//
//  Motor.swift
//  Infrastructure
//
//  Created by David Giovannini on 3/27/25.
//

import Foundation
import BLEByJove

public protocol MotorProtocol {
	associatedtype Power: ControlledProperty where Power.P == Double
	associatedtype Calibration: ControlledProperty where Calibration.P == Double

	var power: Power { get }
	var calibration: Calibration? { get }
	var increment: Double? { get }
	var readOnlyCalibration: Bool { get }
}

public extension MotorProtocol {
	var increment: Double? { nil }
	var calibration: Calibration? { nil }
	var readOnlyCalibration: Bool { false }

	func reset() {
		self.power.reset()
		self.calibration?.reset()
	}

	func fullStop() {
		self.power.control = 0
	}
}

public struct BTMotor: MotorProtocol {
	public typealias Power = BTProperty<ScaledTransformer<Int8>>
	public typealias Calibration = BTProperty<ScaledTransformer<UInt8>>

	public let power: Power
	public let calibration: Calibration?
	public let increment: Double? = 0.01

	public init(device: any BTBroadcaster) {
		self.power = Power(
			broadcaster: device,
			controlChar: BTCharacteristicIdentity(
				component: FacilityPropComponent.motor,
				category: FacilityPropCategory.power,
				channel: BTPropChannel.control),
			feedbackChar: BTCharacteristicIdentity(
				component: FacilityPropComponent.motor,
				category: FacilityPropCategory.power,
				channel: BTPropChannel.feedback),
			transfomer: ScaledTransformer(127))
			
		self.calibration = Calibration(
			broadcaster: device,
			characteristic: BTCharacteristicIdentity(
				component: FacilityPropComponent.motor,
				category: FacilityPropCategory.calibration),
			transfomer: ScaledTransformer(127),
			defaultValue: 0.25)
	}
}
