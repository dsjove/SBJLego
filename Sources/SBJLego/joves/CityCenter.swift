//
//  CityCenter.swift
//  Infrastructure
//
//  Created by David Giovannini on 3/22/25.
//

//TODO: Move Out of infrastructure package

import Foundation
import SBJKit
import BLEByJove
import Combine
import Observation

public struct TrainRegistration {
	public let info: PFMeta
	public let sound: SoundPlayer.Source
	public let symbol: ArduinoR4Matrix?

    public init(info: PFMeta, sound: SoundPlayer.Source, symbol: ArduinoR4Matrix?) {
		self.info = info
		self.sound = sound
		self.symbol = symbol
	}
}

public struct TrainDetection {
	let rfid: SampledRFIDDetection
	let registration: TrainRegistration
}

@Observable
public final class CityCenter: Facility, RFIDProducing, PFTransmitter {
	public static let Service = BTServiceIdentity(name: "City Center")
	public var id: UUID { device.id }
	private let device: BTDevice
	private var sink: Set<AnyCancellable> = []

	public let streetLights: BTLighting
	public let logoDisplay: ArduinoDisplay
	public let fpTransmitter: PFBTRransmitter
	public let rail: RFIDProducer;

	public private(set) var connectionState: ConnectionState {
		didSet {
			switch connectionState {
				case .connected:
					break
				case .connecting:
					break
				case .disconnected:
					reset()
			}
		}
	}

	public var pfConnectionState: ConnectionState {
		self.connectionState
	}

	public private(set) var currentTrain: TrainDetection?

	public init(device: BTDevice) {
		self.device = device
		self.connectionState = device.connectionState
		self.streetLights = BTLighting(device: device)
		self.logoDisplay = ArduinoDisplay(device: device)
		self.rail = RFIDProducer(
			device: device,
			component: FacilityPropComponent.motion,
			category: FacilityPropCategory.address,
			subCategory: EmptySubCategory(0))
		self.fpTransmitter = PFBTRransmitter(
			device: device,
			component: FacilityPropComponent.motion,
			category: FacilityPropCategory.power)

		device.$connectionState.dropFirst().sink { [weak self] in
			self?.connectionState = $0
		}.store(in: &sink)

		//TODO: if another device removes self.currentTrain, set to nil
		observeValue(of: rail, \.currentRFID, with: self) { _, value, this in
			if let this, let value {
				this.updateCurrentRail(value)
			}
		}
	}

	public var category: FacilityCategory { .transportation }
	public var image: ImageName { .system("building") }
	public var name : String { CityCenter.Service.name }

	public func connect() {
		device.connect()
	}

	public func disconnect() {
		device.disconnect()
	}

	public var currentRFID: BLEByJove.SampledRFIDDetection? {
		rail.currentRFID
	}

	public func transmit(cmd: PFCommand) {
		self.fpTransmitter.transmit(cmd: cmd)
	}

	private func updateCurrentRail(_ detection: SampledRFIDDetection) {
		let registration = CityCenter.trains[detection.rfid.id] ?? CityCenter.trains[Data()]!
		self.currentTrain = TrainDetection(rfid: detection, registration: registration)
		let sound: SoundPlayer.Source
		let symbol: ArduinoR4Matrix?
		if let train = self.currentTrain {
			if train.rfid.anotherRound {
				sound = train.registration.sound
				symbol = train.registration.symbol
			}
			else {
				sound = .system(1306)
				symbol = nil
			}
		}
		else {
			sound = .none
			symbol = ArduinoR4Matrix()
		}
		SoundPlayer.shared.play(sound)
		if let symbol {
			self.logoDisplay.power.control = symbol
		}
	}

	public func reset() {
		self.streetLights.reset()
		self.logoDisplay.reset()
		self.rail.reset()
		self.currentTrain = nil
	}

	public func fullStop() {
		self.streetLights.fullStop()
		self.logoDisplay.fullStop()
		self.currentTrain = nil
	}
}
