//
//  ArduinoMatrix.swift
//  Infrastructure
//
//  Created by David Giovannini on 3/27/25.
//

import BLEByJove

public struct ArduinoDisplay {
	public typealias Power = BTProperty<BTValueTransformer<ArduinoR4Matrix>>
	public private(set) var power: Power

	public init(device: any BTBroadcaster) {
		self.power = Power(
			broadcaster: device,
			characteristic: BTCharacteristicIdentity(
				component: FacilityPropComponent.display,
				category: FacilityPropCategory.power))
	}

	public func reset() {
		self.power.reset()
	}

	public func fullStop() {
	}
}

