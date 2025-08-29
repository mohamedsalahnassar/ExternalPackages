//
//  MEPParticipant.h
//  MEPSDK
//
//  Copyright © 2021 Moxtra. All rights reserved.
//

#import "MEPUser.h"

NS_ASSUME_NONNULL_BEGIN

@interface MEPParticipant : MEPUser
/**
 participant id for a participant
 */
@property (nonatomic, strong, readonly, nullable) NSString *participantID;
@end

NS_ASSUME_NONNULL_END
