import Foundation

/// Unsigned LEB128 (protobuf-style varint) support.
///
/// Signed integers deliberately do not conform here: unsigned LEB128 and signed LEB128 have
/// different encodings, and treating a negative two's-complement value as UInt64 is surprising.
public extension FixedWidthInteger where Self: UnsignedInteger {
    /// Initialize from unsigned LEB128 contained in `leb128`.
    /// Returns `nil` if the data is incomplete or overflows this integer width.
    init?(leb128: Data) {
        var value: UInt64 = 0
        var shift: UInt64 = 0

        for (index, byte) in leb128.enumerated() {
            let payload = UInt64(byte & 0x7F)
            guard shift < 64 else { return nil }

            if shift == 63 && payload > 1 { return nil }
            value |= payload << shift

            if (byte & 0x80) == 0 {
                guard index < 10, let exact = Self(exactly: value) else { return nil }
                self = exact
                return
            }

            guard index < 9 else { return nil }
            shift += 7
        }

        return nil
    }

    /// Encode this integer as unsigned LEB128.
    var leb128: Data {
        var output = Data()
        var value = UInt64(self)

        repeat {
            var byte = UInt8(value & 0x7F)
            value >>= 7
            if value != 0 { byte |= 0x80 }
            output.append(byte)
        } while value != 0

        return output
    }

    /// Number of bytes required to encode this integer as unsigned LEB128.
    var leb128Size: Int {
        var count = 1
        var value = UInt64(self)
        while value >= 0x80 {
            count += 1
            value >>= 7
        }
        return count
    }

    /// Compatibility spelling retained for source compatibility.
    @available(*, deprecated, renamed: "leb128Size")
    var lebi18Size: Int { leb128Size }
}
