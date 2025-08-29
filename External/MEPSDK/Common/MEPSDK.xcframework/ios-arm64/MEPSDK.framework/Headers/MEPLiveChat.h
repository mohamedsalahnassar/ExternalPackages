//
//  MEPLiveChat.h
//  MEPSDK
//
//  Created by jacob on 6/29/21.
//  Copyright © 2021 Moxtra. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, MEPLiveChatStatus) {
    MEPLiveChatOpen = 0,
    MEPLiveChatOpenWithoutTimeout,
    MEPLiveChatInProgress,
    MEPLiveChatClosed,
    MEPLiveChatTimeoutClosed,
    MEPLiveChatOfficeClosed
};

@interface MEPLiveChat : NSObject
/**
name of the live chat.
*/
@property (nonatomic, copy, readonly) NSString *name;

/**
status of this live chat.
@see MEPLiveChatStatus
*/
@property (nonatomic, assign, readonly) MEPLiveChatStatus status;
@end

NS_ASSUME_NONNULL_END
