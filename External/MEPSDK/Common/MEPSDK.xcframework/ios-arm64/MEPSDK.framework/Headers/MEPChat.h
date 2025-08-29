//
//  MEPChat.h
//  MEPSDK
//
//  Copyright © 2020 Moxtra. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, MEPFileOperation)
{
    MEPFileOperationRotatePage             = 1 << 0,
    MEPFileOperationRenameFile             = 1 << 1,
    MEPFileOperationRearrangePage          = 1 << 2,
    MEPFileOperationAll           = MEPFileOperationRotatePage | MEPFileOperationRenameFile | MEPFileOperationRearrangePage
};

@class MEPUser;
NS_ASSUME_NONNULL_BEGIN

@interface MEPChatFeed: NSObject
/**
When the feed created
*/
@property (nonatomic, strong, readonly) NSDate *createTime;

/**
Human readable feed content
*/
@property (nonatomic, copy, readonly) NSString *content;

/**
Id of the feed
*/
@property (nonatomic, copy, readonly) NSString *feedId;

/**
User who sent the feed(message)
*/
@property (nonatomic, strong, readonly) MEPUser *actor;
@end

/**
Set customInfo when post/edit message or upload file.
*/
typedef void(^MEPCustomChatContentHandler)(NSString *__nullable customInfo);


typedef NS_ENUM(NSUInteger, MEPChatContentType) {
    MEPChatContentTypeTextMessage, // text message, including message, message comment, file comment, page comment and page position comment, etc.
    MEPChatContentTypeVoiceMessage, // voice message
    MEPChatContentTypeFile // file, including file and page
};

@interface MEPChatContent: NSObject

// @see MEPChatContentType
@property (nonatomic, assign, readonly) MEPChatContentType type;
// A boolean value indicating whether it is going to update message
@property (nonatomic, assign, readonly) BOOL isUpdating;
// Custom info previously set on this content. It is always nil if this is not going to update message.
@property (nonatomic, strong, readonly) NSString *oldCustomInfo;
@end

@interface MEPChatTag: NSObject

@property (nonatomic, strong, readonly) NSString *name;
@property (nonatomic, strong, readonly) NSString *value;

@end


/**
- MEPChatTypeGroupChat:moxtra group chat
- MEPChatTypePrivateChat: 1-1 private chat
- MEPChatTypeRelationChat: relation chat
- MEPChatTypeWeChat: WeChat channel
- MEPChatTypeWhatsApp: WhatsApp channel
- MEPChatTypeLiveChat: ACD Live chat
- MEPChatTypeServiceRequest: Service request chat
- MEPChatTypeFlowChat: Flow chat.
*/
typedef NS_ENUM(NSUInteger, MEPChatType) {
    MEPChatTypeGroupChat,
    MEPChatTypePrivateChat,
    MEPChatTypeRelationChat,
    MEPChatTypeWeChat,
    MEPChatTypeWhatsApp,
    MEPChatTypeLiveChat,
    MEPChatTypeServiceRequest,
    MEPChatTypeFlowChat
};

typedef NS_OPTIONS(NSUInteger, MEPChatTab) {
    MEPChatTabFiles     = 1 << 0,
    MEPChatTabActions   = 1 << 1,
    MEPChatTabAll       = MEPChatTabFiles | MEPChatTabActions
};

@interface MEPChat : NSObject

@property (nonatomic, copy, readonly) NSString *chatID;
@property (nonatomic, assign, readonly) MEPChatType chatType;
/**
A boolean value indicating whether chat is active.
*/
@property (nonatomic, assign, readonly) BOOL isActive;
/**
 Topic of this chat.
 */
@property (nonatomic, strong, readonly, nullable) NSString *topic;

/**
When the last feed was posted.
*/
@property (nonatomic, strong, readonly) NSDate *lastFeedTime;

/**
The count of unread feeds in this chat.
*/
@property (nonatomic, assign, readonly) UInt64 unreadFeedsCount;

/**
The content of the last feed.
*/
@property (nonatomic, strong, readonly, nullable) NSString *lastFeedContent;

/**
The user who create the last feed.
*/
@property (nonatomic, strong, readonly, nullable) MEPUser *lastFeedUser;


- (instancetype)init UNAVAILABLE_ATTRIBUTE;
+ (instancetype)new UNAVAILABLE_ATTRIBUTE;

- (void)setTagWithName:(NSString *)name value:(NSString *)value completionHandler:(void(^__nullable)(NSError * __nullable error))completionHandler;
- (void)getTagsWithCompletionHandler:(void(^)(NSArray<MEPChatTag *> *__nullable tagsOrNil, NSError *__nullable errorOrNil))completionHandler;

- (void)getMembersWithCompletion:(void(^__nullable)(NSError * __nullable errorOrNil, NSArray<MEPUser *> *__nullable members))completionHandler;
/**
Fetch cover of this chat
@param completionHandler A block object to be executed when the action completes
*/
- (void)fetchCoverWithCompletionHandler:(void (^__nullable)(NSError * __nullable errorOrNil, UIImage * __nullable image))completionHandler;

@end

NS_ASSUME_NONNULL_END
