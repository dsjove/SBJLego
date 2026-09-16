//
//  ArduinoR4Matrix.swift
//  BLEByJove
//
//  Created by David Giovannini on 3/23/25.
//

import Foundation
import BLEByJove

private extension Array {
    mutating func shift(increment: Int, fill: Element?) {
        guard !isEmpty, increment != 0 else { return }

        if let fill {
            let distance = Swift.min(abs(increment), count)
            if distance == count {
                self = Array(repeating: fill, count: count)
            } else if increment > 0 {
                removeLast(distance)
                insert(contentsOf: Array(repeating: fill, count: distance), at: 0)
            } else {
                removeFirst(distance)
                append(contentsOf: Array(repeating: fill, count: distance))
            }
            return
        }

        let distance = abs(increment) % count
        guard distance != 0 else { return }
        if increment > 0 {
            let moved = suffix(distance)
            removeLast(distance)
            insert(contentsOf: moved, at: 0)
        } else {
            let moved = prefix(distance)
            removeFirst(distance)
            append(contentsOf: moved)
        }
    }
}

public struct ArduinoR4Matrix: Equatable, BTSerializable {
    public let columns = 12
    public let rows = 8
    private(set) var grid: [[Bool]]

    public init() {
        grid = Array(repeating: Array(repeating: false, count: columns), count: rows)
    }

    public subscript(r: Int, c: Int) -> Bool {
        get { grid[r][c] }
        set { grid[r][c] = newValue }
    }

    public enum FillStyle {
        case off
        case on
        case toggle
        case random
    }

    public mutating func fill(_ style: FillStyle = .off) {
        for r in grid.indices {
            for c in grid[r].indices {
                switch style {
                case .off: grid[r][c] = false
                case .on: grid[r][c] = true
                case .toggle: grid[r][c].toggle()
                case .random: grid[r][c] = Bool.random()
                }
            }
        }
    }

    public mutating func flip(_ columns: Bool = false, _ rows: Bool = false) {
        if columns {
            for r in grid.indices { grid[r].reverse() }
        }
        if rows { grid.reverse() }
    }

    public mutating func scroll(_ columns: Bool = true, _ increment: Int = 1, _ fill: Bool? = nil) {
        if columns {
            for r in grid.indices { grid[r].shift(increment: increment, fill: fill) }
        } else {
            let fillRow = fill.map { Array(repeating: $0, count: self.columns) }
            grid.shift(increment: increment, fill: fillRow)
        }
    }

    public var packedSize: Int { Self.packedSize }
    public static var packedSize: Int { 12 }
    public static var packedCount: Int { 3 }

    private init(fetchChunk: (Int) throws -> UInt32) rethrows {
        var result = Array(repeating: Array(repeating: false, count: columns), count: rows)
        var r = 0
        var c = 0
        for i in 0..<Self.packedCount {
            let chunk = try fetchChunk(i)
            for bitCounter in 0..<32 {
                result[r][c] = ((chunk >> (31 - bitCounter)) & 1) != 0
                c += 1
                if c == columns {
                    c = 0
                    r += 1
                }
            }
        }
        grid = result
    }

    public init(unpack data: Data, _ cursor: inout Int) throws {
        guard cursor >= 0, cursor <= data.count, Self.packedSize <= data.count - cursor else {
            throw BTSerializeError.invalidDataLength
        }
        var localCursor = cursor
        try self.init { _ in try UInt32(unpack: data, &localCursor) }
        cursor = localCursor
    }

    public init(packed: [UInt32]) throws {
        guard packed.count >= Self.packedCount else { throw BTSerializeError.invalidDataLength }
        self.init { packed[$0] }
    }

    public init(packed: [String]) throws {
        guard packed.count >= Self.packedCount else { throw BTSerializeError.invalidDataLength }
        let words: [UInt32] = try packed.prefix(Self.packedCount).map { hex in
            let cleaned = hex.trimmingCharacters(in: .whitespacesAndNewlines)
            guard let value = UInt32(cleaned, radix: 16) else { throw BTSerializeError.invalidRawValue }
            return value
        }
        self.init { words[$0] }
    }

    private var packedWords: [UInt32] {
        var words: [UInt32] = []
        words.reserveCapacity(Self.packedCount)
        var bitCounter = 0
        var chunk: UInt32 = 0
        for row in grid {
            for value in row {
                if value { chunk |= 1 << (31 - bitCounter) }
                bitCounter += 1
                if bitCounter == 32 {
                    words.append(chunk)
                    bitCounter = 0
                    chunk = 0
                }
            }
        }
        return words
    }

    public func pack(btData data: inout Data) {
        for word in packedWords { word.pack(btData: &data) }
    }

    public func exportCPP(name: String) -> String {
        let elements = packedWords.map { "0x" + String(format: "%08xu", $0) }.joined(separator: ", ")
        return "const std::array<uint32_t, 3> \(name) = {\(elements)};\n"
    }

    public func exportSwift(name: String) -> String {
        let elements = packedWords.map { "0x" + String(format: "%08x", $0) }.joined(separator: ", ")
        return "let \(name): [UInt32] = [\(elements)]\n"
    }

    public func exportJSON(name: String) -> String {
        let elements = packedWords.map { "\"\(String(format: "%08x", $0))\"" }.joined(separator: ", ")
        return "\"\(name)\" : [\(elements)]\n"
    }
}
