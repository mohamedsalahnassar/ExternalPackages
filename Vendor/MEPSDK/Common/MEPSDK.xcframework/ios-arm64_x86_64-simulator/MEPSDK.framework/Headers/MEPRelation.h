//
//  MEPRelation.h
//  MEPSDK
//
//  Created by John Hu on 2021/9/26.
//  Copyright © 2021 Moxtra. All rights reserved.
//

#import <Foundation/Foundation.h>
@class MEPRelationUser;
@class MEPChat;
NS_ASSUME_NONNULL_BEGIN

@interface MEPRelation : NSObject
@property (nonatomic, strong) MEPRelationUser *user;
@property (nonatomic, strong, nullable) MEPChat *chat;
@end

NS_ASSUME_NONNULL_END

