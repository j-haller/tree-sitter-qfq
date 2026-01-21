import XCTest
import SwiftTreeSitter
import TreeSitterQfq

final class TreeSitterQfqTests: XCTestCase {
    func testCanLoadGrammar() throws {
        let parser = Parser()
        let language = Language(language: tree_sitter_qfq())
        XCTAssertNoThrow(try parser.setLanguage(language),
                         "Error loading QFQ grammar")
    }
}
