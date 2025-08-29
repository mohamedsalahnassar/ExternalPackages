//
//  MEPChatListModel.h
//  MEPSDK
//
//  Created by Moxtra on 2020/12/16.
//  Copyright © 2020 Moxtra. All rights reserved.
//

#import <Foundation/Foundation.h>
@class MEPChat;
@protocol MEPChatListModelDelegate;

NS_ASSUME_NONNULL_BEGIN
@interface MEPChatListModel : NSObject
/**
 Whether there is more chats not loaded.
 */
@property (nonatomic, assign) BOOL hasMoreChats;
@property (nonatomic, weak) id<MEPChatListModelDelegate> delegate;
/**
Return an array containing all chats.
@discussion From 9.1, server will only return chats in limited count(2000 by default) for better performence,
you can contact server admin to change the cap
Or you can use new API hasMoreChats and fetchMoreChats to load more chats at once.
*/
- (NSArray<MEPChat *> *)chats;

/**
 Fetch more chats from server to local cache.
 @param pageSize Number of chats to be fetched
 @param completionHandler A block object to be executed when the action completes, return errorOrNil if something went wrong.
 */
- (void)fetchMoreChats:(NSInteger)pageSize
        withCompletion:(void(^ _Nullable)(NSError * _Nullable errorOrNil, NSArray <MEPChat *> * _Nullable chats))completionHandler;
@end

@protocol MEPChatListModelDelegate<NSObject>
@optional
/**
Tells the delegate that some chats have been created.
@param chatListModel The related chat list model.
@param createdChats  An array containing created chats.
*/
- (void)chatListModel:(MEPChatListModel *)chatListModel didCreateChats:(NSArray<MEPChat *> *)createdChats;

/**
Tells the delegate that some chats have been updated.
@param chatListModel The related chat list model.
@param updatedChats  An array containing updated chats.
*/
- (void)chatListModel:(MEPChatListModel *)chatListModel didUpdateChats:(NSArray<MEPChat *> *)updatedChats;

/**
Tells the delegate that some chats have been deleted.
@param chatListModel The related chat list model.
@param deletedChats  An array containing deleted chats.
*/
- (void)chatListModel:(MEPChatListModel *)chatListModel didDeleteChats:(NSArray<MEPChat *> *)deletedChats;
@end

NS_ASSUME_NONNULL_END
