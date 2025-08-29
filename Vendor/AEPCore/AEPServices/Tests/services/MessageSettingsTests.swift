/*
 Copyright 2021 Adobe. All rights reserved.
 This file is licensed to you under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License. You may obtain a copy
 of the License at http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software distributed under
 the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS
 OF ANY KIND, either express or implied. See the License for the specific language
 governing permissions and limitations under the License.
 */
#if os(iOS)
    @testable import AEPServices
    import XCTest

    class MessageSettingsTests: XCTestCase {
    
        var parent: String!
        var settings: MessageSettings!
    
        override func setUp() {
            parent = "parent"
            settings = MessageSettings(parent: parent)
        }
    
        func testConstructor() throws {
            XCTAssertNotNil(settings)
            XCTAssertEqual(parent, settings.parent as? String)
        }
        
        func testConstructorNilParent() throws {
            let s = MessageSettings()
            XCTAssertNotNil(s)
            XCTAssertNil(s.parent)
        }
    
        func testWidth() throws {
            settings.setWidth(552)
            XCTAssertEqual(552, settings.width)
        }
    
        func testWidthNil() throws {
            settings.setWidth(nil)
            XCTAssertNil(settings.width)
        }
    
        func testHeight() throws {
            settings.setHeight(1337)
            XCTAssertEqual(1337, settings.height)
        }
    
        func testHeightNil() throws {
            settings.setHeight(nil)
            XCTAssertNil(settings.height)
        }
    
        func testVerticalAlign() throws {
            settings.setVerticalAlign(.bottom)
            XCTAssertEqual(.bottom, settings.verticalAlign)
        }
    
        func testVerticalAlignNil() throws {
            settings.setVerticalAlign(nil)
            XCTAssertEqual(.center, settings.verticalAlign, "center should be default value for vertical align.")
        }
    
        func testHorizontalAlign() throws {
            settings.setHorizontalAlign(.right)
            XCTAssertEqual(.right, settings.horizontalAlign)
        }
    
        func testHorizontalAlignNil() throws {
            settings.setHorizontalAlign(nil)
            XCTAssertEqual(.center, settings.horizontalAlign, "center should be default value for horizontal align.")
        }
    
        func testVerticalInset() throws {
            settings.setVerticalInset(12)
            XCTAssertEqual(12, settings.verticalInset)
        }
    
        func testVerticalInsetNil() throws {
            settings.setVerticalInset(nil)
            XCTAssertNil(settings.verticalInset)
        }
    
        func testHorizontalInset() throws {
            settings.setHorizontalInset(34)
            XCTAssertEqual(34, settings.horizontalInset)
        }
    
        func testHorizontalInsetNil() throws {
            settings.setHorizontalInset(nil)
            XCTAssertNil(settings.horizontalInset)
        }
        
        func testUiTakeover() throws {
            settings.setUiTakeover(true)
            XCTAssertEqual(true, settings.uiTakeover)
        }
    
        func testUiTakeoverNil() throws {
            settings.setUiTakeover(nil)
            XCTAssertEqual(false, settings.uiTakeover, "false should be default value for ui takeover.")
        }
    
        func testBackdropColor() throws {
            let expectedColor = UIColor(red: 0, green: 0, blue: 0, alpha: 0)
            settings.setBackdropColor("000000")
            XCTAssertEqual(expectedColor, settings.getBackgroundColor())
        }
    
        func testBackdropRed() throws {
            let expectedColor = UIColor(red: 1, green: 0, blue: 0, alpha: 0)
            settings.setBackdropColor("FF0000")
            XCTAssertEqual(expectedColor, settings.getBackgroundColor())
        }
    
        func testBackdropGreen() throws {
            let expectedColor = UIColor(red: 0, green: 1, blue: 0, alpha: 0)
            settings.setBackdropColor("00FF00")
            XCTAssertEqual(expectedColor, settings.getBackgroundColor())
        }
    
        func testBackdropBlue() throws {
            let expectedColor = UIColor(red: 0, green: 0, blue: 1, alpha: 0)
            settings.setBackdropColor("0000FF")
            XCTAssertEqual(expectedColor, settings.getBackgroundColor())
        }
    
        func testBackdropColorNil() throws {
            let expectedColor = UIColor(red: 1, green: 1, blue: 1, alpha: 0)
            settings.setBackdropColor(nil)
            XCTAssertEqual(expectedColor, settings.getBackgroundColor())
        }
    
        func testBackdropOpacity() throws {
            let expectedColor = UIColor(red: 1, green: 1, blue: 1, alpha: 0.5)
            settings.setBackdropOpacity(0.5)
            XCTAssertEqual(expectedColor, settings.getBackgroundColor())
        }
    
        func testBackdropOpacityNil() throws {
            let expectedColor = UIColor(red: 1, green: 1, blue: 1, alpha: 0)
            settings.setBackdropOpacity(nil)
            XCTAssertEqual(expectedColor, settings.getBackgroundColor())
        }
    
        func testGetBackgroundColorOverrideOpacity() throws {
            let expectedColor = UIColor(red: 1, green: 1, blue: 1, alpha: 0)
            settings.setBackdropOpacity(0.5)
            XCTAssertEqual(expectedColor, settings.getBackgroundColor(opacity: 0.0))
        }
    
        func testCornerRadius() throws {
            settings.setCornerRadius(15.0)
            XCTAssertEqual(15.0, settings.cornerRadius)
        }
    
        func testCornerRadiusNil() throws {
            settings.setCornerRadius(nil)
            XCTAssertNil(settings.cornerRadius)
        }
    
        func testGestures() throws {
            let gestures: [MessageGesture: URL] = [
                .swipeUp: URL(string: "https://stuff")!,
                .swipeDown: URL(string: "https://moreStuff")!
            ]
            settings.setGestures(gestures)
            XCTAssertEqual(2, settings.gestures?.count)
            XCTAssertEqual(URL(string: "https://stuff")!, settings.gestures?[.swipeUp])
            XCTAssertEqual(URL(string: "https://moreStuff")!, settings.gestures?[.swipeDown])
        }
    
        func testGesturesNil() throws {
            settings.setGestures(nil)
            XCTAssertNil(settings.gestures)
        }
    
        func testDisplayAnimation() throws {
            settings.setDisplayAnimation(.bottom)
            XCTAssertEqual(.bottom, settings.displayAnimation)
        }
    
        func testDisplayAnimationNil() throws {
            settings.setDisplayAnimation(nil)
            XCTAssertEqual(MessageAnimation.none, settings.displayAnimation)
        }
    
        func testDismissAnimation() throws {
            settings.setDismissAnimation(.fade)
            XCTAssertEqual(.fade, settings.dismissAnimation)
        }
    
        func testDismissAnimationNil() throws {
            settings.setDismissAnimation(nil)
            XCTAssertEqual(MessageAnimation.none, settings.dismissAnimation)
        }
    }
#endif
