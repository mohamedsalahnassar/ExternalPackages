//
//  MEPClient.h
//  MEPSDK
//
//  Copyright © 2017 Moxtra. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "MEPUser.h"
#import "MEPChat.h"
#import "MEPMeet.h"
#define MEP_DEPRECATED_WILL_BE_REMOVED_IN_PLEASE_USE(VERSION, METHOD) __attribute__((deprecated("This method has been deprecated and will be removed in Moxtra MEPSDK " VERSION ". Please use `" METHOD "` instead.")))

NS_ASSUME_NONNULL_BEGIN
@class MEPClient;
@protocol MEPTimeline;
@class MEPTransaction;
@class MEPParticipant;
@class MEPLiveChat;
@class MEPLeaveMessageData;

/**
 The MEPClientDelegate protocol defines methods that are called by the client object in response to important events in the lifetime of your client.
 */
@protocol MEPClientDelegate <NSObject>
@optional

/**
 If implemented will trigger when unread count changes
 
 @param client The mep sdk client
 @param unreadCount Count after updated
 */
- (void)client:(MEPClient *)client  didUpdateUnreadCount:(NSUInteger)unreadCount;

/**
 If implemeted will trigger when user clicks close button. Usually to take actions like unlink or hide the screen etc
 
  @param client The mep sdk client
  @param sender The ui element where trigger this event
 */
- (void)client:(MEPClient *)client didTapClose:(id _Nullable)sender;


/**
 If implemeted will trigger when user clicks new chat button. Usually to let 3rd party to create chat by self.
 
 @param client The mep sdk client
 @param sender The ui element where trigger this event
 */
- (void)client:(MEPClient *)client didTapNewChat:(id _Nullable)sender;


/**
 If implemeted will trigger when a meet ended

 @param client The mep sdk client
 @param meetID The call's id
 @param isMissCall YES means miss call, otherwise not missed call.
 */
- (void)client:(MEPClient *)client meetDidEnd:(NSString *)meetID isMissCall:(BOOL)isMissCall;


/**
 If implemeted will trigger when user tap transaction button
 
 @param client The mep sdk client
 @param transaction The transaction tapped
 @discussion 3rd party need call unlink or localUnlink to logout if implemeted this callback, otherwise we will call unlink by default.
 */
- (void)client:(MEPClient *)client didTapTransaction:(MEPTransaction *)transaction;

/**
 If implemeted will trigger when current account did logout
 
 @param client The mep sdk client
 @discussion 3rd party need call unlink or localUnlink to logout if implemeted this callback, otherwise we will call unlink by default.
 */
- (void)clientDidLogout:(MEPClient *)client;

/**
If implemeted will trigger when received upgrade recommend from server

@param client The mep sdk client
@discussion 3rd party can handle the upgrade recommend if implemeted this callback, otherwise we will popup alert UI by default.
*/
- (void)clientDidReceiveUpgradeRecommend:(MEPClient *)client;

/**
If implemeted will trigger when tap add member in chat setting.

@param client The mep sdk client
@param chatID The chat's id
@discussion 3rd party can overtake this action if implemeted this callback and use restApi to add if need.
*/
- (void)client:(MEPClient *)client didTapAddMemberInChat:(NSString *)chatID;

/**
If implemeted will trigger when tap remove member in chat setting.

@param client The mep sdk client
@param chatID The chat's id
@param userToRemove the user to be removed
@discussion 3rd party can overtake this action if implemeted this callback, and use restApi to remove if need.
*/
- (void)client:(MEPClient *)client didTapRemoveMemberInChat:(NSString *)chatID withUser:(MEPUser *)userToRemove;

/**
If implemeted will trigger notify when user open chat.

@param client The mep sdk client
@param chat The chat
@discussion 3rd party fetch properties from chat if need
*/
- (void)client:(MEPClient *)client didOpenChat:(MEPChat *)chat;

/**
If implemeted will trigger notify when user tap call inside chat.

@param client The mep sdk client
@param chat The chat
@discussion 3rd party fetch properties from chat if need
*/
- (void)client:(MEPClient *)client didTapCallInChat:(MEPChat *)chat MEP_DEPRECATED_WILL_BE_REMOVED_IN_PLEASE_USE("6.12", "- (void)client:(MEPClient *)client didTapCall:(MEPChat * _Nullable)chat");


/**
If implemeted will trigger notify when user tap call inside chat/contact profile/calendar, etc.

@param client The mep sdk client
@param chat The chat if tap call button inside chat
@param peerOrNil The related user you want to make call.
@discussion if chat is null, it means this action happened not inside chat, maybe calendar, contact profile. Chat is conflict with peer,  If you want to fetch chat members, pls refer class MEPChat.
*/
- (void)client:(MEPClient *)client didTapCall:(MEPChat * _Nullable)chat withPeer:(MEPUser *_Nullable)peerOrNil;

/**
If implemeted will trigger notify when user tap start meeting from tabbar/calendar/chat/contact profile
/
@param client The mep sdk client
@param chat The chat if tap call button from chat
@param peer The user if tap call button from contact profile
@discussion if chat is null, it means this action happened outside of a chat, happens in calendar or contact profile. Argument chat is conflict with argument peer, they won't have value in same time. If you want to fetch chat members, pls refer class MEPChat.
*/
- (void)client:(MEPClient *)client didTapMeetNow:(MEPChat * _Nullable)chatOrNil withPeer:(MEPUser *_Nullable)peerOrNil;

/**
If implemeted will trigger notify when user tap schedule meeting from tabbar/calendar/chat
/
@param client The mep sdk client
@param chat The chat if tap call button from chat
@discussion if chat is null, it means this action happened outside of a chat, happens in calendar.
*/
- (void)client:(MEPClient *)client didTapScheduleMeet:(MEPChat * _Nullable)chatOrNil;

/**
If implemeted will trigger notify when user tap join meet.

@param client The mep sdk client
@param meet The meet to join
@discussion 3rd party fetch properties from meet if need. And 3rd party can use joinMeet* related apis to join meet.
*/
- (void)client:(MEPClient *)client didTapJoinMeet:(MEPMeet *)meet;


/**
If implemeted will trigger notify when user tap view meet.

@param client The mep sdk client
@param meet The meet
@discussion 3rd party fetch properties from meet if need
*/
-(void)client:(MEPClient *)client didTapViewMeet:(MEPMeet *)meet;

/**
If implemeted will trigger notify when user tap edit meet.

@param client The mep sdk client
@param meet The meet
@discussion 3rd party fetch properties from meet if need
*/
-(void)client:(MEPClient *)client didTapEditMeet:(MEPMeet *)meet;

/**
Use this callback to handle resending invitation.
@param client The mep sdk client.
@param chat The chat. See MEPChat for detail.
@param relationID Relation ID.
*/
- (void)client:(MEPClient *)client handleResendingInvitationWithChat:(nullable MEPChat *)chat relationID:(UInt64)relationID;

/**
If implemented will trigger notify when user tap invite button in meet panel.

@param client The mep sdk client
@param meet The meet which is on going
@discussion Invite button in a live meet panel can only show up when this callback implemented. 3rd party can fetch properties from meet if need.
*/
- (void)client:(MEPClient *)client didTapInviteInLiveMeet:(MEPMeet *)meet;

/** If implemented will trigger notify when host switched in meet.
@param client The mep sdk client
@param meet The meet which changed host
@param newHost The new host switched to */
- (void)client:(MEPClient *)client meet:(MEPMeet *)meet didHostChange:(MEPParticipant *)newHost;

/**
If implemented will trigger notify when live chat updated
@param client The mep sdk client
@param updatedLiveChats Updated live chats
@discussion Only name or status changed will fire this callback, others like message updated won't fire the callback.
*/
- (void)client:(MEPClient *)client liveChatsDidUpdated:(NSArray<MEPLiveChat *> *)updatedLiveChats;

/** If implemented will trigger notify when external user left a message in live chat
@param client The mep sdk client
@param data Refer MEPLeaveMessageData for more
*/
- (void)client:(MEPClient *)client didLeaveMessageForLiveChat:(MEPLeaveMessageData *)data;

/**
If implemented will trigger notify when user going to submit a e-sign
@param client The mep sdk client
@param payload The payload of this e-sign submit action
@param viewController From which view controller this action happens
@param completionHandler A block object which should be called when you decide user can submit this e-sign or not.
@discussion This completionHandler block is required to be called immediately after your process done. First parameters is boolean, to indicate if SDK should submit esign, for example, per 2FA case, once completed the authentication, you should call pass 'YES' to first parameter.
Second parameter is optional JWT,to provide security check per esign .
*/
- (void)client:(MEPClient *)client willSubmitEsign:(NSString * __nullable)payload fromViewController:(UIViewController *)viewController completionHandler:(void(^_Nullable)(BOOL pass, NSString * __nullable jwt))completionHandler;

/** It will be called when user tries to post messages or files, or tries to update messages.
Use this delegate to set custom info for chat content.
@param client The mep sdk client
@param chat   The chat where message post to
@param content  See MEPChatContent
@param handler  See MEPCustomChatContentHandler
*/
- (void)client:(MEPClient *)client handleCustomInfoInChat:(MEPChat *)chat content:(MEPChatContent *)content handler:(MEPCustomChatContentHandler)handler;

/** It will be called when current user sent a message to chat
@param client The mep sdk client
@param feed  Feed been sent, see MEPChatFeed
@param chat  Which chat this message sent to, See MEPChat
*/
- (void)client:(MEPClient *)client didSendMessage:(MEPChatFeed *)feed inChat:(MEPChat *)chat;

/** It will be called before user send a message
@param client The mep sdk client
@param feed  Feed will be send, see MEPChatFeed
@param handler A block object which should be called when you decide user can send this the message or not.
@discussion This completionHandler block is required to be called immediately after your process done.
 */
- (void)client:(MEPClient *)client willSendMessage:(MEPChatFeed *)feed handler:(void(^_Nullable)(BOOL sendOrNot))handler;

/** It will be called when user accepted a scheduled meeting
@param client The mep sdk client
@param meet  The accepted meeting
 */
- (void)client:(MEPClient *)client didAcceptScheduledMeet:(MEPMeet *)meet;

/** It will be called when user accepted a scheduled meeting
@param client The mep sdk client
@param meet  The declined meeting
 */
- (void)client:(MEPClient *)client didDeclineScheduledMeet:(MEPMeet *)meet;

@end


/**
 The MEPLinkConfig class provides some optional network configuration used in linking.
 */
@interface MEPLinkConfig : NSObject

/**
 The certificate organization, optional.
 */
@property (nonatomic, copy, nullable) NSString *certOrganization;

/**
 The public key of ssl cert, optional.
 @discussion Example:
 -----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2laxodWJbCHOIldeQadh\nJn9ii3b+JVTQx/nl6Br5IOCllJFTnACMOAwTXQ1aAoAMTxof73/XQet5FctfUoza\n/Unh5jSv2S07rATNPPxbp0QnRe/KaMeCmExGv88NEXJBo6IkuWp2xzcaVkR8/UD8\nRKuH6GlEjfjvGBAvQRtlUKCvm9nkcwjKinVCjNt4DhXn2RNrNFmz4vcBcnomezyl\nj/uo0j0DZaOnd3PFrj4OGQ+OpRmsoazpuYrqYWN6IToirU/9ogQnsnFtsQ7gXM5J\nALcEQ8A/sJFbVi91jhP6oKapvakQNrvzeeiUHEjlIF9NzVVMsCx1HrPXPgAo5bO8\nWwIDAQAB\n-----END PUBLIC KEY-----\n
*/
@property (nonatomic, copy, nullable) NSString *certPublicKey;

/**
 A boolean value indicating whether ignore the bad certificate.
 */
@property (nonatomic, assign, getter=isIgnoreBadCert) BOOL ignoreBadCert;
@end

@interface MEPLeaveMessageData: NSObject
/**
 The id of chat where this message drop to
*/
@property (nonatomic, copy) NSString *chatId;
/**
 The id of todo created along with the message
*/
@property (nonatomic, copy) NSNumber *todoId;
@end

@interface MEPClient : NSObject

@property (nonatomic, readonly) NSDate *lastActiveDate;

@property(nonatomic, weak, nullable) id<MEPClientDelegate> delegate;

/**
 The MEPClient class provides interfaces for user authentication and UI operations.
 */
+ (nonnull instancetype) sharedInstance;

- (instancetype)init UNAVAILABLE_ATTRIBUTE;
+ (instancetype)new UNAVAILABLE_ATTRIBUTE;


/**
 Set up domain before link
 
 @param baseDomain        The server domain url. There are 2 basedomains you might to use:
                 "www.moxtra.com"  for production environment,
                 "sandbox.moxtra.com" for sanbox environment
                 During development, you will point your app at Moxtra's "sandbox" development environment. Set the base domain to 'production' when pointing your app to production environment.
 @param linkConfig        The optional link configuration.
 */

- (void)setupWithDomain:(NSString *)baseDomain linkConfig:(MEPLinkConfig *_Nullable)linkConfig;


/**
 Initialize MEP with access token
 
 @param accessToken Access token used for authentication
 @param completionHandler A block object to be executed when the action completes
 */
- (void)linkUserWithAccessToken:(nonnull NSString *)accessToken
                       completionHandler:(void (^ _Nullable)(NSError * _Nullable errorOrNil))completionHandler;

/**
 Unlink MEP along with disable notification
 */
- (void)unlink;

/**
 Unlink MEP without disable notification
 */
- (void)localUnlink;

/**
 Request MEP link state
 
 @return If MEP is linked or not
 */
- (BOOL)isLinked;

/**
 Request count of unread messages
 
 @return The count of unread messages
 */
- (NSUInteger)getUnreadMessageCount;

/**
 Request count of unread messages with specific type
 @discussion This API only supports external users for type: MEPChatTypeLiveChat and MEPChatTypeServiceRequest yet.
 @return The count of unread messages
 */
- (NSUInteger)getUnreadMessageCountWithType:(MEPChatType)type;

@end

@interface MEPClient(OpenChat)

/**
 Open chat with chat ID and scroll to the specified feed if present.
 
 @param chatID The chat to open
 @param feedSequence A string indicating the sequence of the scrolling target feed
 @param completionHandler A block object to be executed when the action completes
 */
- (void)openChat:(NSString *)chatID withFeedSequence:(NSNumber * _Nullable)feedSequence completionHandler:(void (^ _Nullable)(NSError *_Nullable error))completionHandler;


/**
 Open relation chat with peer.

 @param peerUniqueID unique id of peer.
 @param completionHandler A block object to be executed when the action completes
 */
- (void)openRelationChatWithUniqueID:(NSString *)peerUniqueID completionHandler:(void (^ _Nullable)(NSError *_Nullable error))completionHandler;

/**
 Open relation chat with peer.
 
 @param peerEmail email of peer.
 @param completionHandler A block object to be executed when the action completes
 */
- (void)openRelationChatWithEmail:(NSString *)peerEmail completionHandler:(void (^ _Nullable)(NSError *_Nullable error))completionHandler;

/**
Open live support chat

@param completionHandler A block object to be executed when the action completes
*/
- (void)openLiveChatWithCompletion:(void (^ _Nullable)(NSError *_Nullable error))completionHandler;

/**
Open service request page, only for customer users.

@param completionHandler A block object to be executed when the action completes
*/
- (void)openServiceReqeustWithCompletion:(void (^ _Nullable)(NSError *_Nullable error))completionHandler;


/**
Open live support chat with specific channel ID

@param channelId The channel ID
@param message A message which would be sent automatically once live chat opended, optional.
@param completionHandler A block object to be executed when the action completes
*/
- (void)openLiveChatWithChannelId:(NSUInteger)channelId message:(NSString *_Nullable)message completion:(void (^ _Nullable)(NSError *_Nullable error))completionHandler;

@end

@interface MEPClient(transaction)
/**
Open transaction

@param completionHandler A block object to be executed when the action completes
*/
- (void)openTransaction:(NSString *)chatId withTransactionId:(NSUInteger)transactionId completion:(void (^ _Nullable)(NSError *_Nullable error))completionHandler;
@end

@interface MEPClient(ViewController)
/**
 Return the top view controller can let 3rd party to present view controller

 @return the top view controller
 @discussion maybe null, if not login or mepwindow not showing.
 */
- (nullable UIViewController *)getTopViewController;


/**
 Create timeline view controller.

 @param enableTabs YES means have all/internal/external, otherwise not have.
 @return the timeline view controller, not include navigation bar.
 */

- (nullable UIViewController *)createTimelineViewController:(BOOL)enableTabs MEP_DEPRECATED_WILL_BE_REMOVED_IN_PLEASE_USE("7.1", "createTimelineViewController");

/**
 Create timeline view controller.
 
 @return the timeline view controller, not include navigation bar.
 */
- (nullable UIViewController *)createTimelineViewController;

/**
Create dashboard controller.

@return the dashboard view controller, not include navigation bar.
*/

- (nullable UIViewController *)createDashboardViewController;

/**
Create chat view controller instance with specific chat id
@param chatID Id of the chat
*/

- (UIViewController *)createChatViewControllerWithChatID:(NSString *)chatID;

/**
Create live chat list view controller, only supports external user
*/
- (nullable UIViewController *)createLiveChatController;

/**
Create service reqeust list controller, only supports external user
*/
- (nullable UIViewController *)createServiceRequestController;

/**
Invoke Moxtra new conversation flow
@param completionHandler A block object to be executed when the controller created or error occurs.
@return The navigation view controller of create new chat.
*/
- (nullable UINavigationController *)createNewChatWithCompletionHandler:(void (^_Nullable)(NSError * _Nullable errorOrNil))completionHandler;

/**
Invoke Moxtra invite client flow
@param completionHandler A block object to be executed when the controller created or error occurs.
@return The navigation view controller of invite new client.
*/
- (nullable UINavigationController *)inviteClientWithCompletionHandler:(void (^_Nullable)(NSError * _Nullable errorOrNil))completionHandler;

/**
Invoke Moxtra invite internal flow
@param completionHandler A block object to be executed when the controller created or error occurs.
@return The navigation view controller of invite new internal.
*/
- (nullable UINavigationController *)inviteInternalWithCompletionHandler:(void (^_Nullable)(NSError * _Nullable errorOrNil))completionHandler;
@end

@interface MEPClient(MEPWindow)
/**
 Shows MEP window if hidden
 By default, the window level is UIWindowLevelDefault. After calling this API, you can change the window level by calling `[UIApplication sharedApplication].keyWindow.windowLevel = someLevel`.
 */
- (void)showMEPWindow;

/**
 Shows MEP window if hidden
 By default, the window level is UIWindowLevelDefault. After calling this API, you can change the window level by calling `[UIApplication sharedApplication].keyWindow.windowLevel = someLevel`.
 */
- (void)showMEPWindowLite;

/**
 Hides MEP window if in show state
 */
- (void)hideMEPWindow;

/**
 Kills the MEP window
 */
- (void)dismissMEPWindow;

/**
 Presents the meet ringer view controller modally.
 @discussion
 By default, SDK will show meet ringer automatically when user is linked.
 But sometimes if meeting started before user linked. SDK not able to present meet ringer automatically.
 In this case, leverage this function to present it whenever expected as long as:
 meeting existing,
 meeting not ended.
 current user has not joined yet.
 */
- (void)showMeetRinger:(NSString *_Nonnull)meetID;

@end


@interface MEPClient(languageSwitch)
/**
 Change language
 
 @param language Standard iso language code（ISO 639-1） to change language at runtime. ex. zh-Hant, zh-Hans, en etc
 */
- (void)changeLanguage:(NSString *_Nonnull)language;
@end

/*!
 *  @brief The value of this key is a string indicating the id of the notification corresponding chat. This key is absent if it is not related with a chat.
 */
FOUNDATION_EXPORT NSString *const MEPNotificationChatIDKey;
/*!
 *  @brief The value of this key is a number indicating the feed sequence in the corresponding chat. This key is absent if it is not related with a feed.
 */
FOUNDATION_EXPORT NSString *const MEPNotificationFeedSequenceKey;
/*!
 *  @brief The value of this key is a string indicating the id of the notification corresponding meet. This key is absent if it is not related with an instant meeting.
 */
FOUNDATION_EXPORT NSString *const MEPNotificationMeetIDKey;
/*!
 *  @brief The value of this key is a BOOL indicating  meet started or not.
 */
FOUNDATION_EXPORT NSString *const MEPNotificationMeetStartedKey;

@interface MEPClient(Notifications)
/**
 Tells the MEP SDK that Apple Push Services successfully completes the registration process.
 @param deviceToken Device token
 */
- (void)registerNotificationWithDeviceToken:(NSData *)deviceToken MEP_DEPRECATED_WILL_BE_REMOVED_IN_PLEASE_USE("", "registerNotificationWithDeviceToken:completionHandler");

/**
 Register device token to server
 @param deviceToken APNS device token or customized token with base64 encoding.
 @param completionHandler A block object to be executed when the action completes.
 @discussion This function requires a logged in user.
 */
- (void)registerNotificationWithDeviceToken:(NSData *)deviceToken completionHandler:(void (^ _Nullable)(NSError *_Nullable error))completionHandler;

/**
 Save customized device token to Moxtra server
 @param deviceToken customized token
 */
- (void)registerNotificationWithCustomizedToken:(NSString *)deviceToken;


/**
 Verify notification is from moxtra or not.

 @param userInfo The notification dictionary
 @return return YES means moxtra otherwise NO.
 */
- (BOOL)isMEPNotification:(NSDictionary *)userInfo;

/**
 Verify notification is a badge notification or not.
 
 @param userInfo The notification dictionary
 @return return YES means it is a badge notification otherwise NO.
 */
- (BOOL)isBadgeNotification:(NSDictionary *)userInfo;

/**
 Parse the notification to extract related info.
 
 @param userInfo          A dictionary that contains information related to the remote notification.
 @param completionHandler A block object to be executed when the action completes.
 @discussion the info argument of the completionHandler block may contain MEPNotificationMeetIDKey, MEPNotificationMeetStartedKey,  MEPNotificationChatIDKey, MEPNotificationFeedSequenceKey  as keys. For detail information, please refer to these constant declarations.
 */
- (void)parseRemoteNotification:(NSDictionary *)userInfo completionHandler:(void (^)(NSError * _Nullable error, NSDictionary * _Nullable info))completionHandler;

@end

@interface MEPStartMeetOptions : NSObject
/**
Meet topic, required.
*/
@property (nonatomic, copy, nonnull) NSString *topic;

/**
Chat id to generate meet feed, optional
*/
@property (nonatomic, copy, nullable) NSString *chatID;

/**
UniqueIDs , the Users you want to invite into meet
*/
@property (nonatomic, copy, nullable) NSArray<NSString *> *uniqueIDs;

/**
 Emails , the Users you want to invite into meet
 */
@property (nonatomic, copy, nullable) NSArray<NSString *> *emails;

/**
YES, auto Join voip, otherwise don't auto join.
*/
@property (nonatomic) BOOL autoJoinAudio;

/**
YES, auto start front camera, otherwise don't start camera
*/
@property (nonatomic) BOOL autoStartVideo;

/**
YES, auto start meeting recording. Default is NO.
*/
@property (nonatomic) BOOL autoRecording;

/**
YES, will hide recording start/stop inside meeting. Default is NO.
*/
@property (nonatomic) BOOL disableRecordingCtrl;

/**
Set YES to enable call UI with dialer tone when start a meeting with only one user. Default is NO.
This option only works when uniqueIDs got only one user, and autoJoinAudio set to YES.
*/
@property (nonatomic) BOOL instantCall;

@end


@interface MEPScheduleMeetOptions : NSObject
/**
Meet topic, required.
*/
@property (nonatomic, copy, nonnull) NSString *topic;

/**
Chat id to generate meet feed, optional
*/
@property (nonatomic, copy, nullable) NSString *chatID;

/**
UniqueIDs , the Users you want to invite into meet, optional
 */
@property (nonatomic, copy, nullable) NSArray<NSString *> *uniqueIDs;

/**
 Emails , the Users you want to invite into meet
 */
@property (nonatomic, copy, nullable) NSArray<NSString *> *emails;

/**
meet Start date,  required.
*/
@property(nonatomic, copy) NSDate *meetStartDate;

/**
meet end date, required.
*/
@property(nonatomic, copy) NSDate *meetEndDate;

/**
meet reminder me time Optioional. Default is -1.
@discussion Only -1, 0 or greater than 0 is allowed.  -1: no reminder.  0: Server will  notify when meet's meetStartDate reached.  >0 and less than 30*24*60: Server will notify when meet (meetStartDate - remindMeBeforeMinutes) reached.
*/
@property(nonatomic) NSInteger remindMeBeforeMinutes;
@end



@interface MEPClient(Meet)

/**
 Join meet with meet ID

 @param meetID meetID the you want to join
 @param completionHandler A block object to be executed when the action completes.
 @discussion Join meet with meet id when current account loggin.
 */
- (void)joinMeetWithMeetID:(nonnull NSString *)meetID completionHandler:(void (^ _Nullable)(NSError * _Nullable errorOrNil))completionHandler;

/**
 Join meet with meet ID

 @param meetID meetID the you want to join
 @param password password for this meet
 @param completionHandler A block object to be executed when the action completes.
 @discussion Join meet with meet id when current account loggin.
 */
- (void)joinMeetWithMeetID:(nonnull NSString *)meetID password:(nonnull NSString *)password completionHandler:(void (^ _Nullable)(NSError * _Nullable errorOrNil))completionHandler;

/**
 Join meet as anonymous with meet ID
 
 @param meetID meetID the you want to join
 @param displayName the name will display in meet
 @param completionHandler A block object to be executed when the action completes.
 @discussion Join meet with meet id when current no account logined.
 */
- (void)joinMeetAnonymouslyWithMeetID:(nonnull NSString *)meetID displayName:(nonnull NSString *)displayName completionHandler:(void (^ _Nullable)(NSError * _Nullable errorOrNil))completionHandler MEP_DEPRECATED_WILL_BE_REMOVED_IN_PLEASE_USE("6.7", "joinMeetAnonymouslyWithMeetID:displayName:email:completionHandler");

/**
 Join meet as anonymous with meet ID

 @param meetID meetID the you want to join
 @param displayName the name will display in meet
 @param emailOrNil the email
 @param completionHandler A block object to be executed when the action completes.
 @discussion Join meet with meet id when current no account logined.
 */
- (void)joinMeetAnonymouslyWithMeetID:(nonnull NSString *)meetID displayName:(nonnull NSString *)displayName email:(nullable NSString *)emailOrNil completionHandler:(void (^ _Nullable)(NSError * _Nullable errorOrNil))completionHandler;


/**
 Join meet as anonymous with meet ID

 @param meetID meetID the you want to join
 @param password password for this meet
 @param displayName the name will display in meet
 @param emailOrNil the email
 @param completionHandler A block object to be executed when the action completes.
 @discussion Join meet with meet id when current no account logined.
 */
- (void)joinMeetAnonymouslyWithMeetID:(nonnull NSString *)meetID password:(nonnull NSString *)password displayName:(nonnull NSString *)displayName email:(nullable NSString *)emailOrNil completionHandler:(void (^ _Nullable)(NSError * _Nullable errorOrNil))completionHandler;


/**
start meet with option
 
@param meetOptions meet related option
@param completionHandler A block object to be executed when the action completes.
 */
- (void)startMeetWithOption:(nonnull MEPStartMeetOptions *)meetOptions completionHandler:(void (^ _Nullable)(NSError * _Nullable errorOrNil, NSString *meetIDOrNil))completionHandler;


/**
schedule meet with option

@param meetOptions meet related option
@param completionHandler A block object to be executed when the action completes.
 */
- (void)scheduleMeetWithOption:(nonnull MEPScheduleMeetOptions *)meetOptions completionHandler:(void (^ _Nullable)(NSError * _Nullable errorOrNil, NSString *meetIDOrNil))completionHandler;


/**
fetch meet with meetID

@param meetID meet id the you want fetch
@param completionHandler A block object to be executed when the action completes.
 */
- (void)fetchMeetWithMeetID:(NSString *__nonnull)meetID withCompletion: (void (^)(NSError * _Nullable errorOrNil, MEPMeet * _Nullable meetOrNil))completionHandler;

/**
 handle call activity
 
 @param callActivity  typical user activity from call history or call UI
 @return YES means activity handled, otherwise NO.
 */

- (BOOL)handleCallKitUserActivity:(NSUserActivity *)callActivity;

@end

@interface MEPAddFileEntryOptions : NSObject

/**
 Enable/disable add whiteboard
 */
@property (nonatomic, assign) BOOL enableWhiteboard;

/**
 Enable/disable add location
 */
@property (nonatomic, assign) BOOL enableLocation;


/**
 Enable/disable add web
 */
@property (nonatomic, assign) BOOL enableWeb;

/**
 Enable/disable add file scan
 */
@property (nonatomic, assign) BOOL enableScan;

/**
 Enable/disable add contact
 */
@property (nonatomic, assign) BOOL enableContact;

/**
 Enable/disable more option, default is YES
 */
@property (nonatomic, assign) BOOL enableMore;

/**
 Enable/disable camera, default is YES.
 */
@property (nonatomic, assign) BOOL enableCamera;

/**
An array of file extensions that are acceptable e.g ["pdf", "png", "txt"]
The default value is nil, which means SDK does not check file types.
*/
@property (nonatomic, strong) NSArray<NSString *> *allowedFileTypes;

/**
 Max file size in bytes.
 The default value is 0, which means SDK does not check file size.
 @discussion SDK will take minimum value from org admin configuration and sdk configuration as max upload file size. So sdk configuration will be deprecated when its size greater than org admin configuration.
*/
@property (nonatomic, assign) UInt64 maxFileSize;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

/**
 Feature configurations
 */
@interface MEPFeatureConfig : NSObject

/**
  Enable/disable rename chat. YES means can rename, NO cannot. Default is YES.
 */
@property (nonatomic, assign) BOOL renameChatEnabled;

/**
 Enable/disable inactive relation chats. YES means can rename, NO cannot. Default is YES.
 */
@property (nonatomic, assign) BOOL hidesInactiveRelationChats;

/**
 Enable/disable notifications configurations. YES means hide, NO means show. Default is NO.
 */
@property (nonatomic, assign) BOOL hidesChatNotificationsSetting;

/**
 Enable/disable user presence feature. YES means enable, NO means disable. Default is YES.
 */
@property (nonatomic, assign) BOOL userPresenceEnabled;


/**
 Enable/disable out of office only. YES means enable, NO means disable. Default is NO.
 */
@property (nonatomic, assign) BOOL showOOOOnly;

/**
 Enable/disable file share features, save to album, open in..., etc. YES means enabled, NO means disable.  Default is YES
 */
@property (nonatomic, assign) BOOL downloadEnabled;

/**
 Enable/disable whiteboard in chat and meet. YES means enabled, NO means disable.  Default is YES
 */
@property (nonatomic, assign) BOOL whiteboardEnabled MEP_DEPRECATED_WILL_BE_REMOVED_IN_PLEASE_USE("6.6", "MEPFeatureConfig.addFileEntryOptions.enableWhiteboard");

/**
 Enable/disable user profile in chat and meet. YES means enabled, NO means disable.  Default is YES
 */
@property (nonatomic, assign) BOOL userProfileEnabled;

/**
 Enable/disable forward message in chat. YES means can forward, NO means cannot.  Default is YES
 */
@property (nonatomic, assign) BOOL forwardMessageEnabled;

/**
 Enable/disable leave chat. YES means can forward, NO means cannot.  Default is YES
 */
@property (nonatomic, assign) BOOL leaveChatEnabled;

/**
 Enable/disable edit chat message. YES means can edit, NO means cannot.  Default is YES
 */
@property (nonatomic, assign) BOOL editMessageEnabled;

/**
  Enable/disable edit voice message. YES means can, NO means cannot.  Default is YES
 */
@property (nonatomic, assign) BOOL voiceMessageEnabled;

/**
 Enable/disable edit save meet files when end meet . YES means can save, NO means cannot.  Default is NO
 */
@property (nonatomic, assign) BOOL saveMeetFilesEnabled;

/**
 Enable/disable pushkit notification. YES means to receive PushKit VOIP notifications, NO means not to receive. Default is NO
 */
@property (nonatomic, assign) BOOL pushKitEnabled;

/**
 Enable/disable transfer chat owner. YES means can transfer, NO means cannot.  Default is YES
 */
@property (nonatomic, assign) BOOL makeOwnerEnabled;

/**
 Enable/disable chat settings feature. Default is YES
 */
@property (nonatomic, assign) BOOL chatSettingsEnabled;

/**
 Hide the floating action button from timeline. Default is NO
 */
@property (nonatomic, assign) BOOL hideFABOnTimeline;

/**
 * Disable copy and paste for text. Default is NO.
 *
 */
@property (nonatomic, assign) BOOL disableCopyPaste;

/**
 * Enable/disable chat archive feature. Default is NO.
 */
@property (nonatomic, assign) BOOL disableArchiveChat;

/**
 * Replace with local action color.
 */
@property (nonatomic, strong) UIColor *actionColor;

/**
 * Replace with local action color in dark mode.
 */
@property (nonatomic, strong) UIColor *actionColorDarkMode;

/**
 Configure the time limit for how long an object can be delete/edit after it has been sent.  The unit of the time limit is milliseconds.
 Default value is '-1'.
 @discussion
 If time set to '1800000' , then message can't be delete/edit after 30 minutes.
 If set to '-1', means to follow time from org settings. (30 minutes by default from org setting)
 '0' means can not be deleted.
 When both org setting and sdk setting been set, the smaller one will take effect.
 */
@property (nonatomic, assign) NSInteger contentEditInterval;

/**
 Enable/disable add file entry options
 */
@property (nonatomic, readonly, nonnull) MEPAddFileEntryOptions *addFileEntryOptions;

/**
 Enabled chat tabs, see more in MEPChatTab.
 @discussion if you want to hide some specific tab, please use code like below:
 // Hide file tab
 [MEPFeatureConfig sharedInstance].enabledChatTabs &= ~MEPChatTabFiles;
 // Hide action tab
 [MEPFeatureConfig sharedInstance].enabledChatTabs &= ~MEPChatTabActions;
 */
@property (nonatomic, assign) MEPChatTab enabledChatTabs;

/**
 Enabled meet features, see more in MEPMeetFeature. Defaults is MEPMeetFeatureAll.
 @discussion if you want to disable specific feature, please use code like below:
 // Disable meeting chat
 [MEPFeatureConfig sharedInstance].enabledMeetFeatures &= ~MEPMeetFeatureChat;
 // Disable meeting file share
 [MEPFeatureConfig sharedInstance].enabledMeetFeatures &= ~MEPMeetFeatureFileShare;
 */
@property (nonatomic, assign) MEPMeetFeature enabledMeetFeatures;

/**
 Enabled file operations, see more in MEPFileOperation. Defaults is MEPMeetFeatureAll.
 @discussion if you want to disable specific feature, please use code like below:
 // Disable page rotate
 [MEPFeatureConfig sharedInstance].enabledFileOperations &= ~MEPFileOperationRotatePage;
 // Disable file rename
 [MEPFeatureConfig sharedInstance].enabledFileOperations &= ~MEPFileOperationRenameFile;
 */
@property (nonatomic, assign) MEPFileOperation enabledFileOperations;

/**
 If implemeted will trigger when add user button needs render
 @param chatID The chat where add user button in
 @return Display add button or not
 @discussion This block would only be invoked when all pre-conditions satisfied. For example, the chat must be a group chat and current user has privilege to invite new user
*/
@property (nonatomic, copy, nullable) BOOL(^canAddUserInChat)(MEPChat *chat);


/**
  Override MEPSDK interface style, default is UIUserInterfaceStyleUnspecified.
  This API only effect when using global UI API, showMEPWindow, showMEPWidnowLite, joinMeet, openRelationChat, etc)
 */
@property (nonatomic, assign) UIUserInterfaceStyle overrideUserInterfaceStyle;

+ (nonnull instancetype) sharedInstance;

@end

NS_ASSUME_NONNULL_END
