//
//  MotorizedFacility.swift
//  JovesLanding
//
//  Created by David Giovannini on 1/22/26.
//

import SBJKit
import BLEByJove
import Foundation
import Network

public typealias IPv4AddressProperty = BTProperty<BTValueTransformer<IPv4Address>>

public protocol MotorizedFacility: Facility {
	associatedtype Lighting: LightingProtocol
	associatedtype Motor: MotorProtocol

	var motor: Motor { get }
	var lighting: Lighting? { get }
}

public extension MotorizedFacility {
	func fullStop() {
		motor.fullStop()
		lighting?.fullStop()
	}
}
