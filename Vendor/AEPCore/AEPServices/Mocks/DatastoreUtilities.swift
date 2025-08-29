/*
 Copyright 2023 Adobe. All rights reserved.
 This file is licensed to you under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License. You may obtain a copy
 of the License at http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software distributed under
 the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS
 OF ANY KIND, either express or implied. See the License for the specific language
 governing permissions and limitations under the License.
 */


import Foundation
import AEPServices

extension UserDefaults {
    public static func clear() {
        for _ in 0 ... 5 {
            for key in UserDefaults.standard.dictionaryRepresentation().keys {
                UserDefaults.standard.removeObject(forKey: key)
            }
        }
    }
}

extension FileManager {

    func clearCache() {
        if let _ = self.urls(for: .cachesDirectory, in: .userDomainMask).first {
            try? self.removeItem(at: URL(fileURLWithPath: "Library/Caches/com.adobe.module.signal"))
            try? self.removeItem(at: URL(fileURLWithPath: "Library/Caches/com.adobe.module.identity"))
            try? self.removeItem(at: URL(fileURLWithPath: "Library/Caches/com.adobe.mobile.diskcache", isDirectory: true))
            try? self.removeItem(at: URL(fileURLWithPath: "Library/Caches/com.adobe.eventHistory"))
        }
    }
}

extension NamedCollectionDataStore {
    static func clear() {
        UserDefaults.clear()
        FileManager.default.clearCache()
        let directory = FileManager.default.urls(for: .libraryDirectory, in: .allDomainsMask)[0].appendingPathComponent("com.adobe.aep.datastore", isDirectory: true).path
        guard let filePaths = try? FileManager.default.contentsOfDirectory(atPath: directory) else {
            return
        }
        for filePath in filePaths {
            try? FileManager.default.removeItem(atPath: directory + "/" + filePath)
        }
    }
}

