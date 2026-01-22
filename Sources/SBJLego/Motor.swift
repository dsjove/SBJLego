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
}

public extension MotorProtocol {
	var increment: Double? { nil }
	var calibration: Calibration? { nil }

	func reset() {
		self.power.reset()
		self.calibration?.reset()
	}

	func fullStop() {
		self.power.control = 0
	}
}

public struct PFMotor: MotorProtocol {
	public typealias Power = TransformedProperty<ScaledTransformer<Int8>>
	public typealias Calibration = LocalControlledProperty<Double>

	public let power: Power
	public let calibration: Calibration? = nil
	public let increment: Double? = 128.0/16.0

	public init(device: PFDevice, port: PFPort) {
		self.power = Power(
			sendControl: { value in
				device.send(port: port, power: value)
				return value
			},
			transfomer: ScaledTransformer((127)))
	}
}

public struct CCMotor: MotorProtocol {
	public typealias Power = TransformedProperty<ScaledTransformer<Int16>>
	public typealias Calibration = TransformedProperty<ScaledTransformer<Int16>>
	
	public let power: Power
	public let calibration: Calibration?

	public init(cube: CircuitCube) {
		class Inner {
			let cube: CircuitCube
			weak var power: Power? = nil
			weak var calib: Calibration? = nil

			init(_ cube: CircuitCube) {
				self.cube = cube
			}

			func apply() -> Int16 {
				var signal = power?.controlMomento ?? 0
				let calib = calib?.controlMomento ?? 255/4
				if abs(signal) < calib {
					signal = 0
					power?.receiveFeedback(newFeedbackMomento: signal)
				}
				Task {
					await cube.power(set: signal, on: .a, dropKey: "motor")
				}
				return signal
			}
		}
		let inner = Inner(cube)

		let calibration = Calibration(
			sendControl: { value in
				let _ = inner.apply()
				return value;
			},
			transfomer: ScaledTransformer(255))
		calibration.control = 0.25

		let power = Power(
			sendControl: { value in
				return inner.apply()
			},
			transfomer: ScaledTransformer((255)))

		inner.power = power
		self.power = power

		inner.calib = calibration
		self.calibration = calibration
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
