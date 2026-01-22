//
//  ESPLighting.swift
//  SBJLego
//
//  Created by David Giovannini on 1/22/26.
//

import Foundation
import SBJKit
import BLEByJove
import Network

public struct ESPLighting: LightingProtocol {
	public typealias Value = TransformedProperty<ScaledTransformer<UInt8>>

	public var power: Value
	public var calibration: Value?
	public var sensed: Value?
	public let hasDimmer: Bool = true
	public let hasSensor: Bool = false

	public init(controlURL: URL) {
		let throttle = RequestThrottler()
		self.power = Value(sendControl: { value in
			let url = controlURL.appending(queryItems: [
				.init(name: "var", value: "led_intensity"),
				.init(name: "val", value: value.description),
			])
			throttle.sendRequest(url: url)
			return value
		}, transfomer: ScaledTransformer(255))
	}
}
