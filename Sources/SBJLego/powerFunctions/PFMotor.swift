//
//  PFMotor.swift
//  SBJLego
//
//  Created by David Giovannini on 1/22/26.
//

import Foundation
import BLEByJove

public struct PFMotor: MotorProtocol {
	public typealias Power = TransformedProperty<ScaledTransformer<Int8>>
	public typealias Calibration = TransformedProperty<ScaledTransformer<UInt8>>

	public let power: Power
	public let calibration: Calibration?
	public var readOnlyCalibration: Bool { true }
	public let increment: Double? = 0.0625

	public init(device: PFDeviceTransmitter, port: PFPort) {
		self.power = Power(
			sendControl: { value in
				device.transmit(port: port, power: value)
				return abs(value) < (256 / 32) ? 0 : value
			},
			transfomer: ScaledTransformer((127)))

		self.calibration = Calibration(
			sendControl: { value in
				return value
			},
			transfomer: ScaledTransformer((127)),
			defaultValue: 0.0625)
	}
}
