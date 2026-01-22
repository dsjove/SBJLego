//
//  FacilityRepository.swift
//  BLEByJove
//
//  Created by David Giovannini on 1/18/26.
//

import Foundation
import Observation
import SBJKit
import BLEByJove

public typealias FacilityEntry = Identified<any Facility>

@Observable
public final class FacilityRepository: RFIDConsumer, PFTransmitter {
	public private(set) var scanners: [any DeviceScanning] = []

	private let createFacilities: (any DeviceIdentifiable) -> [any Facility]
	private var facilitiesByDeviceID: [ObjectIdentifier : [UUID: [FacilityEntry]]] = [:]
	public private(set) var facilities: [FacilityEntry] = []

	private var rfidTokensByDeviceID: [UUID: [ObserveToken]] = [:]

	public init(createFacilities: @escaping (any DeviceIdentifiable) -> [any Facility]) {
		self.createFacilities = createFacilities
	}

	public func addScanner<S: DeviceScanner>(_ scanner: S) {
		scanners.append(scanner)
		observeValue(of: scanner, \.anyDevices, with: self) { scanner, devices, this in
			this?.sync(devices, scoped: ObjectIdentifier(scanner))
		}
	}

	public func setScanning(_ scanning: Bool) {
		for scanner in scanners {
			scanner.scanning = scanning
		}
	}

	private func sync(_ scannerDevices: [any DeviceIdentifiable], scoped: ObjectIdentifier) {
		let scannerDeviceIds = Set(scannerDevices.map(\.id))
		var scope = facilitiesByDeviceID[scoped] ?? [:]

		// Given never seen device before
		for device in scannerDevices where scope[device.id] == nil {
			// Create the facilities
			let newFacilities = createFacilities(device)
			scope[device.id] = newFacilities.map { .init($0, id: $0.id) }
			// Observe RFIDs if we can
			for facility in newFacilities {
				if let rfidProducer = facility as? (any AnyObject & RFIDProducing) {
					observeRFID(rfidProducer, deviceID: device.id)
				}
			}
			newFacilities.forEach { if $0.autoConnects { $0.connect() } }
		}

		// Observe RFID producers
		func observeRFID<P: AnyObject & RFIDProducing>(_ rfidProducer: P, deviceID: UUID) {
			let token = observeValue(of: rfidProducer, \.currentRFID, with: self) { _, value, this in
				guard let this, let value else { return }
				this.consumeRFID(value)
			}
			var tokensForDevice = rfidTokensByDeviceID[deviceID] ?? []
			tokensForDevice.append(token)
			rfidTokensByDeviceID[deviceID] = tokensForDevice
		}

		// If devices have disappeared
		for deviceID in scope.keys where !scannerDeviceIds.contains(deviceID) {
			// Remove all facilities for the device
			scope.removeValue(forKey: deviceID)
			// Cancel all tokens for the device
			if let tokens = rfidTokensByDeviceID.removeValue(forKey: deviceID) {
				for token in tokens { token.cancel() }
			}
		}
		facilitiesByDeviceID[scoped] = scope
		rebuildFacilitiesSorted()
	}

	public func consumeRFID(_ detection: SampledRFIDDetection) {
		for scanner in scanners {
			( (scanner as? RFIDConsumer) )?.consumeRFID(detection)
		}
	}

	public func transmit(cmd: PFCommand) {
		facilities
			.lazy
			.compactMap { $0.value as? PFTransmitter }
			.first?.transmit(cmd: cmd)
	}

	public var pfConnectionState: ConnectionState {
		facilities
			.lazy
			.compactMap { $0.value as? PFTransmitter }
			.first?.pfConnectionState ?? .disconnected
	}

	private func rebuildFacilitiesSorted() {
		let all = facilitiesByDeviceID.values.flatMap { $0.values.flatMap { $0 } }
		facilities = all.sorted { lhs, rhs in
			let lhsName = lhs.value.name
			let rhsName = rhs.value.name
			let cmp: ComparisonResult = lhsName.localizedStandardCompare(rhsName)
			if cmp != .orderedSame {
				return cmp == .orderedAscending
			}
			return lhs.id.uuidString < rhs.id.uuidString
		}
	}
}
