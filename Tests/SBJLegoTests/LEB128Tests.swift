import XCTest
@testable import SBJLego

final class LEB128Tests: XCTestCase {
    func testKnownUnsignedEncoding() throws {
        XCTAssertEqual(UInt64(0).leb128, Data([0x00]))
        XCTAssertEqual(UInt64(127).leb128, Data([0x7f]))
        XCTAssertEqual(UInt64(128).leb128, Data([0x80, 0x01]))
        XCTAssertEqual(UInt64(624485).leb128, Data([0xe5, 0x8e, 0x26]))
        XCTAssertEqual(UInt64(624485).leb128Size, 3)
    }

    func testKnownUnsignedDecoding() throws {
        XCTAssertEqual(UInt64(leb128: Data([0xe5, 0x8e, 0x26])), 624485)
        XCTAssertNil(UInt8(leb128: Data([0x80])))
        XCTAssertNil(UInt8(leb128: Data([0x80, 0x02])))
    }

    func testRoundTrip() throws {
        for value: UInt64 in [0, 1, 127, 128, 255, 16_384, UInt64.max] {
            XCTAssertEqual(UInt64(leb128: value.leb128), value)
        }
    }
}
