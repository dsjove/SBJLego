//
//  FacilityCategory.swift
//  Infrastructure
//
//  Created by David Giovannini on 3/22/25.
//

import Foundation
import BLEByJove

public enum FacilityPropComponent: UInt8, BTComponent {
	case system = 1
	case motor = 2
	case lights = 3
	case heart = 4
	case motion = 5
	case camera = 6
	case display = 7
	case sound = 8

	public var description: String {
		switch self {
		case .system:
			return "system"
		case .motor:
			return "motor"
		case .lights:
			return "lights"
		case .heart:
			return "heart"
		case .motion:
			return "motion"
		case .camera:
			return "camera"
		case .display:
			return "display"
		case .sound:
			return "sound"
		}
	}
}

public enum FacilityPropCategory: UInt8, BTCategory {
	case address = 0
	case calibration = 1
	case power = 2
	case state = 3
	case sensed = 4

	public var description: String {
		switch self {
		case .address:
			return "address"
		case .calibration:
			return "calibration"
		case .power:
			return "power"
		case .state:
			return "state"
		case .sensed:
			return "sensed"
		}
	}
}
