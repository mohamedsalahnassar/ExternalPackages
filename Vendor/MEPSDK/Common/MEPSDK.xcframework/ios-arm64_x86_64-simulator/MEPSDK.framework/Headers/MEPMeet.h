//
//  MEPMeet.h
//  MEPSDK
//
//  Copyright © 2020 Moxtra. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, MEPMeetFeature)
{
    MEPMeetFeatureChat                    = 1 << 0,
    MEPMeetFeatureScreenShare             = 1 << 1,
    MEPMeetFeatureFileShare               = 1 << 2,
    MEPMeetFeatureCoBrowse                = 1 << 3,
    MEPMeetFeatureInviteParticipants      = 1 << 4,
    MEPMeetFeatureAll            = MEPMeetFeatureChat | MEPMeetFeatureScreenShare | MEPMeetFeatureFileShare | MEPMeetFeatureCoBrowse | MEPMeetFeatureInviteParticipants
};

NS_ASSUME_NONNULL_BEGIN

@interface MEPMeetEndState : NSObject

@property (nonatomic, assign) BOOL audioUsed;
@property (nonatomic, assign) BOOL videoUsed;
@property (nonatomic, assign) BOOL screenShareUsed;
@property (nonatomic, assign) BOOL coBrowseUsed;
@property (nonatomic, assign) BOOL missCall;

@end

@class MEPUser;
@interface MEPMeet : NSObject

/**
The id of this meet.
*/
@property (nonatomic, copy, readonly) NSString *meetID;

/**
Topic of the meet.
*/
@property (nonatomic, copy, readonly) NSString *topic;

/**
has password.
*/
@property (nonatomic, readonly) BOOL hasPassword;

/**
password of the meet.
*/
@property (nonatomic, copy, readonly, nullable) NSString *password;

/**
Is meeting in progress.
*/
@property (nonatomic, assign, readonly) BOOL isInProgress;

/**
Agenda of the meet.
*/
@property (nonatomic, copy, readonly) NSString *agenda;

/**
 Start time of this meet. If the meet has not started yet, the value in this property will be nil.
 */
@property (nonatomic, strong, readonly, nullable) NSDate *startTime;

/**
 End time of this meet. If the meet has not ended yet, the value in this property will be nil.
 */
@property (nonatomic, strong, readonly, nullable) NSDate *endTime;

/**
 Scheduled start time of this meet. If the meet was not scheduled, the value in this property will be nil.
 */
@property (nonatomic, strong, readonly, nullable) NSDate *scheduledStartTime;

/**
 Scheduled end time of this meet. If the meet was not scheduled, the value in this property will be nil.
 */
@property (nonatomic, strong, readonly, nullable) NSDate *scheduledEndTime;

/**
 If the scheduled meeting accepted by current user.
 */
@property (nonatomic, assign, readonly, getter = isAccepted) BOOL accepted;

/**
Host of the meet.
*/
@property (nonatomic, strong) MEPUser *host;

/**
Particiapnts of the meet
 */
@property (nonatomic, strong, readonly) NSArray<MEPUser *> *participants;

/*
End state of the meet, only has value when is an ended meet
Refer MEPMeetEndState for details
 */
@property (nonatomic, strong, nullable) MEPMeetEndState *endState;

@end

NS_ASSUME_NONNULL_END
