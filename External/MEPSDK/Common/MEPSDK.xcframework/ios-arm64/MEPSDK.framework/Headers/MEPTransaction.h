//
//  MEPTransaction.h
//  MEPSDK
//
//  Copyright © 2019 Moxtra. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 MEPTransactionData contains detail info about the specific transaction which is needed when interactive with transaction button.
 */
@interface MEPTransaction : NSObject

/**
 The id of the chat.
 */
@property (nonatomic, copy, readonly) NSString *chatId;

/**
 The id of the transaction.
 */
@property (nonatomic, assign, readonly) NSUInteger transactionId;

/**
 The id of the step just processed.
 */
@property (nonatomic, assign, readonly) NSUInteger stepId;

/**
 The id of the transaction button just clicked.
 */
@property (nonatomic, copy, readonly) NSString *buttonId;

/**
 The payload of transaction, contains more detail info about the transaction.
 */
@property (nonatomic, copy, readonly) NSString *payload;

@end

NS_ASSUME_NONNULL_END
