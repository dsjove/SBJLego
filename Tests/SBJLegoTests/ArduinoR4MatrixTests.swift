import XCTest
@testable import SBJLego

final class ArduinoR4MatrixTests: XCTestCase {
    func testPackAndUnpackRoundTrip() throws {
        var matrix = ArduinoR4Matrix()
        matrix[0, 0] = true
        matrix[0, 11] = true
        matrix[7, 11] = true

        let packed = matrix.pack()
        XCTAssertEqual(packed.count, ArduinoR4Matrix.packedSize)
        XCTAssertEqual(try ArduinoR4Matrix(unpack: packed), matrix)
    }

    func testUnpackHonorsCursor() throws {
        var matrix = ArduinoR4Matrix()
        matrix[3, 4] = true
        let data = Data([0xaa, 0xbb]) + matrix.pack() + Data([0xcc])
        var cursor = 2
        let decoded = try ArduinoR4Matrix(unpack: data, &cursor)
        XCTAssertEqual(decoded, matrix)
        XCTAssertEqual(cursor, 2 + ArduinoR4Matrix.packedSize)
    }

    func testScrollWithFillHandlesNegativeAndOversizedDistances() {
        var matrix = ArduinoR4Matrix()
        matrix[0, 0] = true
        matrix.scroll(true, -1, false)
        XCTAssertFalse(matrix[0, 0])

        matrix.fill(.on)
        matrix.scroll(true, 100, false)
        for row in 0..<matrix.rows {
            for column in 0..<matrix.columns { XCTAssertFalse(matrix[row, column]) }
        }
    }

    func testScrollWithoutFillRotatesForOversizedDistance() {
        var matrix = ArduinoR4Matrix()
        matrix[0, 0] = true
        matrix.scroll(true, 13, nil)
        XCTAssertTrue(matrix[0, 1])
    }

    func testKnownPackedWordsAndExports() throws {
        let matrix = try ArduinoR4Matrix(packed: [0x80000000, 0x00000001, 0x12345678])
        XCTAssertEqual(matrix.exportSwift(name: "matrix"), "let matrix: [UInt32] = [0x80000000, 0x00000001, 0x12345678]\n")
        XCTAssertEqual(matrix.exportJSON(name: "matrix"), "\"matrix\" : [\"80000000\", \"00000001\", \"12345678\"]\n")
    }
}
