//
//  InsiderIdentifiers.h
//  SDK
//
//  Created by Insider on 20.04.2020.
//  Copyright © 2020 Insider. All rights reserved.
//

#import <Foundation/Foundation.h>

@class InsiderIdentifiers;

NS_ASSUME_NONNULL_BEGIN

@interface InsiderIdentifiers : NSObject
/**
 A dictionary containing all user identifiers that have been added.
 */
@property (nonatomic, readonly) NSDictionary<NSString *, NSString *> *dictionaryValue;

/**
 Adds an email address as a user identifier.
 @discussion Use this to associate a user's email with their Insider profile.
 This helps unify the user's identity across Insider products and enables communication through various channels.
 @note Pass the configured `InsiderIdentifiers` object to the Insider User Login method.
 */
- (InsiderIdentifiers *(^)(NSString *))addEmail;

/**
 Adds a phone number as a user identifier.
 @discussion Use this to associate a user's phone number with their Insider profile.
 This helps unify the user's identity across Insider products and enables communication through various channels.
 @note Pass the configured `InsiderIdentifiers` object to the Insider User Login method.
 */
- (InsiderIdentifiers *(^)(NSString *))addPhoneNumber;

/**
 Adds a user ID as a user identifier.
 @discussion Use this to associate a unique user ID with their Insider profile.
 This helps unify the user's identity across Insider products and enables communication through various channels.
 @note Pass the configured `InsiderIdentifiers` object to the Insider User Login method.
 */
- (InsiderIdentifiers *(^)(NSString *))addUserID;

/**
 Adds a custom identifier with a key-value pair.
 @discussion Use this to associate additional, custom identifiers with a user's Insider profile.
 @note Pass the configured `InsiderIdentifiers` object to the Insider User Login method.
 */
- (InsiderIdentifiers *(^)(NSString *key, NSString *value))addCustomIdentifier;

@end

NS_ASSUME_NONNULL_END
