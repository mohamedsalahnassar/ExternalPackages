//
//  MEPRelationUser.h
//  MEPSDK
//
//  Created by John Hu on 2022/1/5.
//  Copyright © 2022 Moxtra. All rights reserved.
//

#import <MEPSDK/MEPUser.h>

NS_ASSUME_NONNULL_BEGIN
typedef NS_ENUM(NSUInteger, MEPRelationUserStatus) {
    MEPRelationUserStatusUnknown = 0,
    MEPRelationUserStatusOnline = 100,
    MEPRelationUserStatusBusy = 200,
    MEPRelationUserStatusOffline = 300,
    MEPRelationUserStatusOutOfOffice = 400
};

@interface MEPRelationUser : MEPUser
/**
 Customized status title.
 */
@property (nonatomic, copy, readonly, nullable) NSString *statusTitle;

/**
 Customized status message.
 */
@property (nonatomic, copy, readonly, nullable) NSString *statusMessage;

/**
 Subscribe user status
 @discussion Call this function to subscribe status only when its userStatus property is MEPRelationUserStatusUnknown. Then listen status changes in callback didUpdateRelations from MEPRelationListModel.
 This is for performance consideration, SDK won't manage all relation user status when its amount too large.
 */
- (void)subscribeUserStatus;
@end

NS_ASSUME_NONNULL_END
