//
//  MEPSDKError.h
//  MEPSDK
//
//  Copyright © 2018 Moxtra. All rights reserved.
//

#import <Foundation/Foundation.h>


/**
 list of error codes which are returned when some error occurs. Used for debugging the error by the provider.
 */
FOUNDATION_EXTERN NSErrorDomain _Nonnull const MEPSDKErrorDomain;

/**
 - MEPUnkownError:unkown error happen
 - MEPDomainsError: setUpDomain firstly
 - MEPInvalidAccountError: Account invalid pls call linkWith* again
 - MEPNotLinkedError: call linkWith*** with success firstly.
 - MEPNetworkError: network error
 - MEPObjectNotFoundError: Object not found error, typical happen when join meet with meet id, open chat with chat id.
 - MEPAuthorizedError, Authorize error when login, typical happen when this kind account not allow to login in this app.
 - MEPAccountDisabled, account disabled.
 - MEPMeetEndedError, meet ended. Typical happen when join an ended meet.
 - MEPPermissionError, Permission error
 - MEPIncorrectMeetPasswordError, password wrong
 - MEPAnotherMeetInProgressError, another meet inprogress
 - MEPInvalidParamError, invalid parameter.
 */
typedef NS_ENUM(NSUInteger, MEPSDKErrorCode) {
    MEPUnkownError = 0,
    MEPDomainsError,
    MEPInvalidAccountError,
    MEPNotLinkedError,
    MEPNetworkError,
    MEPObjectNotFoundError,
	MEPAuthorizedError,
	MEPAccountDisabled,
    MEPAccountLocked,
    MEPMeetEndedError,
	MEPPermissionError,
    MEPIncorrectMeetPasswordError,
    MEPAnotherMeetInProgressError,
    MEPInvalidParamError,
	MEPFeedNoFoundInFlowError,
    MEPMeetLockedError
};

