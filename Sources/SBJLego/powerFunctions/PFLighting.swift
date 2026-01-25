//
//  PFLighting.swift
//  SBJLego
//
//  Created by David Giovannini on 1/22/26.
//

import Foundation
import BLEByJove

public struct PFLighting: LightingProtocol {
	public typealias Value = TransformedProperty<ScaledTransformer<UInt8>>
	
	public let power: Value
	public let calibration: Value? = nil
	public let sensed: Value? = nil
	public let hasDimmer: Bool = true
	public let increment: Double? = 0.0625

	public init(device: PFDeviceTransmitter, port: PFPort) {
		self.power = Value(
			sendControl: { value in
				device.transmit(port: port, power: Int8(value))
				return value
			},
			transfomer: ScaledTransformer((127)))
	}
}
