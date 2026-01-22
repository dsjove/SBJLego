//
//  CCMotor.swift
//  SBJLego
//
//  Created by David Giovannini on 1/22/26.
//

import Foundation
import BLEByJove

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
