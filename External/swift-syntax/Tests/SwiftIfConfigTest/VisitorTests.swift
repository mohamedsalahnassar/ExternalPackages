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
@_spi(Compiler) import SwiftIfConfig
import SwiftParser
import SwiftSyntax
@_spi(XCTestFailureLocation) @_spi(Testing) import SwiftSyntaxMacrosGenericTestSupport
import XCTest
import _SwiftSyntaxGenericTestSupport
import _SwiftSyntaxTestSupport

/// Visitor that ensures that all of the nodes we visit are active.
///
/// This cross-checks the visitor itself with the `SyntaxProtocol.isActive(in:)`
/// API.
class AllActiveVisitor: ActiveSyntaxAnyVisitor {
  let configuration: TestingBuildConfiguration

  init(
    configuration: TestingBuildConfiguration,
    configuredRegions: ConfiguredRegions? = nil
  ) {
    self.configuration = configuration

    if let configuredRegions {
      super.init(viewMode: .sourceAccurate, configuredRegions: configuredRegions)
    } else {
      super.init(viewMode: .sourceAccurate, configuration: configuration)
    }
  }

  open override func visitAny(_ node: Syntax) -> SyntaxVisitorContinueKind {
    XCTAssertEqual(node.isActive(in: configuration).state, .active)
    return .visitChildren
  }
}

class NameCheckingVisitor: ActiveSyntaxAnyVisitor {
  let configuration: TestingBuildConfiguration

  /// The set of names we are expected to visit. Any syntax nodes with
  /// names that aren't here will be rejected, and each of the names listed
  /// here must occur exactly once.
  var expectedNames: Set<String>

  init(
    configuration: TestingBuildConfiguration,
    expectedNames: Set<String>,
    configuredRegions: ConfiguredRegions? = nil
  ) {
    self.configuration = configuration
    self.expectedNames = expectedNames

    if let configuredRegions {
      super.init(viewMode: .sourceAccurate, configuredRegions: configuredRegions)
    } else {
      super.init(viewMode: .sourceAccurate, configuration: configuration)
    }
  }

  deinit {
    if !expectedNames.isEmpty {
      XCTFail("No nodes with expected names visited: \(expectedNames)")
    }
  }

  func checkName(name: String, node: Syntax) {
    if !expectedNames.contains(name) {
      XCTFail("syntax node with unexpected name \(name) found: \(node.debugDescription)")
    }

    expectedNames.remove(name)
  }

  open override func visitAny(_ node: Syntax) -> SyntaxVisitorContinueKind {
    if let identified = node.asProtocol(NamedDeclSyntax.self) {
      checkName(name: identified.name.text, node: node)
    } else if let identPattern = node.as(IdentifierPatternSyntax.self) {
      // FIXME: Should the above be an IdentifiedDeclSyntax?
      checkName(name: identPattern.identifier.text, node: node)
    }

    return .visitChildren
  }
}

public class VisitorTests: XCTestCase {
  let linuxBuildConfig = TestingBuildConfiguration(
    customConditions: ["DEBUG", "ASSERTS"],
    features: ["ParameterPacks"],
    attributes: ["available"]
  )

  let iosBuildConfig = TestingBuildConfiguration(
    platformName: "iOS",
    customConditions: ["DEBUG", "ASSERTS"],
    features: ["ParameterPacks"],
    attributes: ["available"]
  )

  let inputSource: SourceFileSyntax = """
    #if DEBUG
    #if os(Linux)
    #if hasAttribute(available)
    @available(*, deprecated, message: "use something else")
    #else
    @MainActor
    #endif
    func f() {
    }
    #elseif os(iOS)
    func g() {
      let a = foo
        #if hasFeature(ParameterPacks)
        .b
        #endif
        .c
    }
    #endif

    struct S {
      #if DEBUG
      var generationCount = 0
      #endif
    }

    func h() {
      switch result {
        case .success(let value):
          break
        #if os(iOS)
        case .failure(let error):
          break
        #endif
      }
    }

    func i() {
      a.b
      #if DEBUG
        .c
      #endif
      #if hasAttribute(available)
        .d()
      #endif
      #if os(iOS)
        .e[]
      #endif
    }
    #endif

    #if hasAttribute(available)
    func withAvail() { }
    #else
    func notAvail() { }
    #endif
    """

  func testAnyVisitorVisitsOnlyActive() throws {
    // Make sure that all visited nodes are active nodes.
    assertVisitedAllActive(linuxBuildConfig)
    assertVisitedAllActive(iosBuildConfig)
  }

  func testVisitsExpectedNodes() throws {
    // Check that the right set of names is visited.
    assertVisitedExpectedNames(
      linuxBuildConfig,
      expectedNames: ["f", "h", "i", "S", "generationCount", "value", "withAvail"]
    )

    assertVisitedExpectedNames(
      iosBuildConfig,
      expectedNames: ["g", "h", "i", "a", "S", "generationCount", "value", "error", "withAvail"]
    )
  }

  func testVisitorWithErrors() throws {
    var configuration = linuxBuildConfig
    configuration.badAttributes.insert("available")
    assertVisitedExpectedNames(
      configuration,
      expectedNames: ["f", "h", "i", "S", "generationCount", "value", "notAvail"],
      diagnosticCount: 3
    )
  }

  func testRemoveInactive() {
    assertRemoveInactive(
      inputSource.description,
      configuration: linuxBuildConfig,
      expectedSource: """

        @available(*, deprecated, message: "use something else")
        func f() {
        }

        struct S {
          var generationCount = 0
        }

        func h() {
          switch result {
            case .success(let value):
              break
          }
        }

        func i() {
          a.b
            .c
            .d()
        }
        func withAvail() { }
        """
    )
  }

  func testRemoveInactiveWithErrors() {
    var configuration = linuxBuildConfig
    configuration.badAttributes.insert("available")

    assertRemoveInactive(
      inputSource.description,
      configuration: configuration,
      diagnostics: [
        DiagnosticSpec(
          message: "unacceptable attribute 'available'",
          line: 3,
          column: 18
        ),
        DiagnosticSpec(
          message: "unacceptable attribute 'available'",
          line: 42,
          column: 20
        ),
        DiagnosticSpec(
          message: "unacceptable attribute 'available'",
          line: 51,
          column: 18
        ),
      ],
      expectedSource: """

        @MainActor
        func f() {
        }

        struct S {
          var generationCount = 0
        }

        func h() {
          switch result {
            case .success(let value):
              break
          }
        }

        func i() {
          a.b
            .c
        }
        func notAvail() { }
        """
    )
  }

  func testRemoveInactiveRetainingFeatureChecks() {
    assertRemoveInactive(
      """
      public func hasIfCompilerCheck(_ x: () -> Bool = {
      #if compiler(>=5.3)
        return true
      #else
        return false
      #endif

      #if $Blah
        return 0
      #else
        return 1
      #endif

      #if NOT_SET
        return 3.14159
      #else
        return 2.71828
      #endif
        }) {
      }
      """,
      configuration: linuxBuildConfig,
      retainFeatureCheckIfConfigs: true,
      expectedSource: """
        public func hasIfCompilerCheck(_ x: () -> Bool = {
        #if compiler(>=5.3)
          return true
        #else
          return false
        #endif

        #if $Blah
          return 0
        #else
          return 1
        #endif
          return 2.71828
          }) {
        }
        """
    )
  }

  func testRemoveCommentsAndSourceLocations() {
    let original: SourceFileSyntax = """

      /// This is a documentation comment
      func f() { }

      #sourceLocation(file: "if-configs.swift", line: 200)
      /** Another documentation comment
          that is split across
          multiple lines */
      func g() { }

      func h() {
        x +/*comment*/y
        // foo
      }
      """

    assertStringsEqualWithDiff(
      original.descriptionWithoutCommentsAndSourceLocations,
      """

       
      func f() { }


      func g() { }

      func h() {
        x + y
         
      }
      """
    )
  }
}

extension VisitorTests {
  /// Ensure that all visited nodes are active nodes according to the given
  /// build configuration.
  fileprivate func assertVisitedAllActive(_ configuration: TestingBuildConfiguration) {
    AllActiveVisitor(configuration: configuration).walk(inputSource)

    let configuredRegions = inputSource.configuredRegions(in: configuration)
    AllActiveVisitor(
      configuration: configuration,
      configuredRegions: configuredRegions
    ).walk(inputSource)
  }

  /// Ensure that we visit nodes with the set of names we were expecting to
  /// visit.
  fileprivate func assertVisitedExpectedNames(
    _ configuration: TestingBuildConfiguration,
    expectedNames: Set<String>,
    diagnosticCount: Int = 0
  ) {
    let firstVisitor = NameCheckingVisitor(
      configuration: configuration,
      expectedNames: expectedNames
    )
    firstVisitor.walk(inputSource)
    XCTAssertEqual(firstVisitor.diagnostics.count, diagnosticCount)

    let configuredRegions = inputSource.configuredRegions(in: configuration)
    let secondVisitor = NameCheckingVisitor(
      configuration: configuration,
      expectedNames: expectedNames,
      configuredRegions: configuredRegions
    )
    secondVisitor.walk(inputSource)
    XCTAssertEqual(secondVisitor.diagnostics.count, diagnosticCount)
  }
}

/// Assert that removing any inactive code according to the given build
/// configuration returns the expected source and diagnostics.
private func assertRemoveInactive(
  _ source: String,
  configuration: some BuildConfiguration,
  retainFeatureCheckIfConfigs: Bool = false,
  diagnostics expectedDiagnostics: [DiagnosticSpec] = [],
  expectedSource: String,
  file: StaticString = #filePath,
  line: UInt = #line
) {
  var parser = Parser(source)
  let tree = SourceFileSyntax.parse(from: &parser)

  for useConfiguredRegions in [false, true] {
    let fromDescription = useConfiguredRegions ? "configured regions" : "build configuration"
    let treeWithoutInactive: Syntax
    let actualDiagnostics: [Diagnostic]

    if useConfiguredRegions {
      let configuredRegions = tree.configuredRegions(in: configuration)
      actualDiagnostics = configuredRegions.diagnostics
      treeWithoutInactive = configuredRegions.removingInactive(
        from: tree,
        retainFeatureCheckIfConfigs: retainFeatureCheckIfConfigs
      )
    } else {
      (treeWithoutInactive, actualDiagnostics) = tree.removingInactive(
        in: configuration,
        retainFeatureCheckIfConfigs: retainFeatureCheckIfConfigs
      )
    }

    // Check the resulting tree.
    assertStringsEqualWithDiff(
      treeWithoutInactive.description,
      expectedSource,
      "Active code (\(fromDescription))",
      file: file,
      line: line
    )

    // Check the diagnostics.
    if actualDiagnostics.count != expectedDiagnostics.count {
      XCTFail(
        """
        Expected \(expectedDiagnostics.count) diagnostics, but got \(actualDiagnostics.count) via \(fromDescription):
        \(actualDiagnostics.map(\.debugDescription).joined(separator: "\n"))
        """,
        file: file,
        line: line
      )
    } else {
      for (actualDiag, expectedDiag) in zip(actualDiagnostics, expectedDiagnostics) {
        assertDiagnostic(
          actualDiag,
          in: .tree(tree),
          expected: expectedDiag,
          failureHandler: {
            XCTFail(
              $0.message + " via \(fromDescription)",
              file: $0.location.staticFilePath,
              line: $0.location.unsignedLine
            )
          }
        )
      }
    }
  }
}
