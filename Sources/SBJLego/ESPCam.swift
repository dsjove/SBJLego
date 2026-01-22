//
//  Christof.swift
//  Infrastructure
//
//  Created by David Giovannini on 5/19/25.
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

@Observable
public class ESPCam: Facility {
	public typealias Lighting = ESPLighting

	public static let Service = "espcam"
	public let id = UUID()
	public let url: URL
	public let imageURL: URL
	public let controlURL: URL
	public let lighting: Lighting

	public init(device: MDNSDevice) {
		self.name = device.name
		if case let NWEndpoint.service(service, _, _, _) = device.endpoint {
			url = URL(string: "http://\(service).local:80")!
			imageURL = url.appendingPathComponent("capture")
			controlURL = url.appendingPathComponent("control")
		}
		else {
			url = URL(string: "about:blank")!
			imageURL = URL(string: "about:blank")!
			controlURL = URL(string: "about:blank")!
		}
		lighting = .init(controlURL: controlURL)
	}

	public convenience init() {
		self.init(device: .init(preview: "Sample"))
	}

	public var category: FacilityCategory { .housing }
	public var image: ImageName { .system("eye") }
	public let name : String
	public let connectionState: BLEByJove.ConnectionState = .connected

	public func connect() {
	}

	public func disconnect() {
	}

	public func reset() {
	}

	public func fullStop() {
	}
}
