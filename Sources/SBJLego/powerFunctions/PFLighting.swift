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
	public let increment: Double? = 128.0/16.0

	public init(device: PFDevice, port: PFPort) {
		self.power = Value(
			sendControl: { value in
				device.send(port: port, power: Int8(value))
				return value
			},
			transfomer: ScaledTransformer((127)))
	}
}
