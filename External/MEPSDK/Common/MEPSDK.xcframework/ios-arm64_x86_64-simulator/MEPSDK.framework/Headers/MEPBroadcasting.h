//
//  MEPBroadcasting.h
//  MEPSDK
//
//  Copyright © 2020 Moxtra. All rights reserved.
//

#ifndef MEPBroadcasting_h
#define MEPBroadcasting_h

NS_ASSUME_NONNULL_BEGIN

@protocol MEPBroadcastingDelegate;
@interface MEPBroadcasting : NSObject
//Bundle identifyer for broadcasting. 3rd party must set this if want broadcasting feature. Default is nil
@property (nonatomic, copy, nullable) NSString *broadcastExtensionBundleIdentifier;

@property (nonatomic, weak, nullable) id<MEPBroadcastingDelegate> delegate;


+ (id)sharedInstance;

/**
 share image thru screen sharing.
 
 @param image        sampling image
 */
- (void)shareImage:(UIImage *)image;

/**
 let 3rd party to start share screen.
 */
- (void)startSharing;

/**
 let 3rd party to stop share screen.
 */
- (void)stopSharing;

@end


@protocol MEPBroadcastingDelegate<NSObject>
@optional
/**
 If implemented will trigger when screen share started inside meet
 
 @param boradcasting The boradcasting instance
 */
- (void)broadcastingScreenShareDidStarted:(MEPBroadcasting *)boradcasting;

/**
 If implemented will trigger when screen share stopped inside meet
 
 @param boradcasting The boradcasting instance
 @discussion you need stop remote capture extension session because screen sharing stopped.
 */
- (void)broadcastingScreenShareDidStopped:(MEPBroadcasting *)boradcasting;
@end
NS_ASSUME_NONNULL_END

#endif /* MEPBroadcasting_h */
