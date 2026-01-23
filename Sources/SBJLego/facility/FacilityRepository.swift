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

	private var facilitiesByDeviceID: [ObjectIdentifier : [UUID: [FacilityEntry]]] = [:]
	public private(set) var facilities: [FacilityEntry] = []

	private var rfidTokensByDeviceID: [UUID: [ObserveToken]] = [:]

	public init() {
	}

	public func addScanner<S: DeviceScanner>(
		_ scanner: S,
		_ facilities: @escaping (S.Device)->[Facility]) {
		scanners.append(scanner)
		observeValue(of: scanner, \.devices, with: self) { scanner, _, this in
			this?.sync(scanner, facilities)
		}
	}

	public func setScanning(_ scanning: Bool) {
		for scanner in scanners {
			scanner.scanning = scanning
		}
	}

	private func sync<S: DeviceScanner>(_ scanner: S, _ facilities: (S.Device)->[Facility]) {
		let scannerDevices = scanner.devices
		let scannerDeviceIds = Set(scannerDevices.map(\.id))
		var scope = facilitiesByDeviceID[ObjectIdentifier(scanner)] ?? [:]

		// Given never seen device before
		for device in scannerDevices where scope[device.id] == nil {
			// Create the facilities
			let newFacilities = facilities(device)
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
			//Reset any current RFIDs
			resetRFID(deviceID)
			// Remove all facilities for the device
			scope.removeValue(forKey: deviceID)
			// Cancel all tokens for the device
			if let tokens = rfidTokensByDeviceID.removeValue(forKey: deviceID) {
				for token in tokens { token.cancel() }
			}
		}
		facilitiesByDeviceID[ObjectIdentifier(scanner)] = scope
		rebuildFacilitiesSorted()
	}

	public func consumeRFID(_ detection: SampledRFIDDetection) {
		for scanner in scanners {
			( (scanner as? RFIDConsumer) )?.consumeRFID(detection)
		}
	}

	public func resetRFID(_ uuid: UUID) {
		for facility in facilities {
			if let facility = facility.value as? RFIDProducing {
				if facility.currentRFID?.rfid.uuid == uuid {
					facility.resetRFID()
				}
			}
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
