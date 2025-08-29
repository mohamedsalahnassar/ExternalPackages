//===----------------------------------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2024 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

import SwiftDiagnostics
import SwiftIfConfig
import SwiftParser
import SwiftSyntax
import SwiftSyntaxMacrosGenericTestSupport
import XCTest
import _SwiftSyntaxGenericTestSupport
import _SwiftSyntaxTestSupport

public class ActiveRegionTests: XCTestCase {
  let linuxBuildConfig = TestingBuildConfiguration(
    customConditions: ["DEBUG", "ASSERTS"],
    features: ["ParameterPacks"],
    attributes: ["available"]
  )

  func testActiveRegions() throws {
    try assertActiveCode(
      """
      1️⃣
      #if DEBUG
      2️⃣func f()
      #elseif ASSERTS
      3️⃣func g()

      #if compiler(>=8.0)
      4️⃣func h()
      #else
      5️⃣var i
      #endif
      #endif
      6️⃣token
      """,
      configuration: linuxBuildConfig,
      states: [
        "1️⃣": .active,
        "2️⃣": .active,
        "3️⃣": .inactive,
        "4️⃣": .unparsed,
        "5️⃣": .unparsed,
        "6️⃣": .active,
      ]
    )
  }

  func testActiveRegionsInPostfix() throws {
    try assertActiveCode(
      """
      1️⃣a.b()
      #if DEBUG
      2️⃣.c()
      #elseif ASSERTS
      3️⃣.d()
      #if compiler(>=8.0)
      4️⃣.e()
      #else
      5️⃣.f()
      #endif
      #endif
      6️⃣.g()
      """,
      configuration: linuxBuildConfig,
      states: [
        "1️⃣": .active,
        "2️⃣": .active,
        "3️⃣": .inactive,
        "4️⃣": .unparsed,
        "5️⃣": .unparsed,
        "6️⃣": .active,
      ],
      configuredRegionDescription:
        "[[1:6 - 3:5] = active, [3:5 - 10:7] = inactive, [5:5 - 7:5] = unparsed, [7:5 - 9:5] = unparsed)]"
    )
  }

  func testActiveRegionsWithErrors() throws {
    try assertActiveCode(
      """
      #if FOO > 10
      0️⃣class Foo {
      }
      #else
      1️⃣class Fallback {
      }
      #endif
      """,
      states: [
        "0️⃣": .unparsed,
        "1️⃣": .active,
      ]
    )
  }

  func testActiveRegionUnparsed() throws {
    try assertActiveCode(
      """
      #if false
       #if compiler(>=4.1)
       1️⃣let _: Int = 1
       #else
        // There should be no error here.
        2️⃣foo bar
       #endif
      #endif
      """,
      states: [
        "1️⃣": .unparsed,
        "2️⃣": .unparsed,
      ]
    )
  }
}

/// Assert that the various marked positions in the source code have the
/// expected active states.
private func assertActiveCode(
  _ markedSource: String,
  configuration: some BuildConfiguration = TestingBuildConfiguration(),
  states: [String: IfConfigRegionState],
  configuredRegionDescription: String? = nil,
  file: StaticString = #filePath,
  line: UInt = #line
) throws {
  // Pull out the markers that we'll use to dig out nodes to query.
  let (markerLocations, source) = extractMarkers(markedSource)

  var parser = Parser(source)
  let tree = SourceFileSyntax.parse(from: &parser)

  let configuredRegions = tree.configuredRegions(in: configuration)

  for (marker, location) in markerLocations {
    guard let expectedState = states[marker] else {
      XCTFail("Missing marker \(marker) in expected states", file: file, line: line)
      continue
    }

    guard let token = tree.token(at: AbsolutePosition(utf8Offset: location)) else {
      XCTFail("Unable to find token at location \(location)", file: file, line: line)
      continue
    }

    let (actualState, _) = token.isActive(in: configuration)
    XCTAssertEqual(actualState, expectedState, "isActive(in:) at marker \(marker)", file: file, line: line)

    let actualViaRegions = configuredRegions.isActive(token)
    XCTAssertEqual(
      actualViaRegions,
      expectedState,
      "isActive(inConfiguredRegions:) at marker \(marker)",
      file: file,
      line: line
    )

    if let configuredRegionDescription {
      XCTAssertEqual(
        configuredRegions.debugDescription,
        configuredRegionDescription,
        "configured region descsription",
        file: file,
        line: line
      )
    }
  }
}
