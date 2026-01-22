//
//  CCLighting.swift
//  SBJLego
//
//  Created by David Giovannini on 1/22/26.
//

import Foundation
import BLEByJove

public struct CCLighting: LightingProtocol {
	public typealias Value = TransformedProperty<ScaledTransformer<Int16>>
	
	public let power: Value
	public let calibration: Value? = nil
	public let sensed: Value? = nil
	public let hasDimmer: Bool = true

	public init(cube: CircuitCube) {
		self.power = Value(sendControl: { value in
			Task {
				await cube.power(set: value, on: [.b, .c], dropKey: "lighting")
			}
			return value
		}, transfomer: ScaledTransformer(255))
	}
}
