//
// Copyright 2022 Adobe. All rights reserved.
// This file is licensed to you under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License. You may obtain a copy
// of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under
// the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS
// OF ANY KIND, either express or implied. See the License for the specific language
// governing permissions and limitations under the License.
//

@testable import AEPServices
import XCTest

class AtomicCounterTests: XCTestCase {
    var atomicCounter: AtomicCounter!
    
    override func setUp() {
        atomicCounter = AtomicCounter()
    }
    
    func testAtomicCounterSimple() {
        var counter = 0
        let finalCount = 50
        for _ in 0 ..< finalCount {
            counter = atomicCounter.incrementAndGet()
        }
        
        XCTAssertEqual(counter, finalCount)
    }
    
    func testAtomicCounterMultiThread() {
        let threadCount = 10
        var counter = 0
        let expectation = XCTestExpectation()
        expectation.assertForOverFulfill = true
        expectation.expectedFulfillmentCount = threadCount
        for _ in 0 ..< threadCount {
            DispatchQueue.global().async {
                counter = self.atomicCounter.incrementAndGet()
                expectation.fulfill()
            }
        }
        
        wait(for: [expectation], timeout: 1.0)
        
        XCTAssertEqual(counter, threadCount)
    }
}

