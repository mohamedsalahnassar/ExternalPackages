//
//  MEPMeetListModel.h
//  MEPSDK
//
//  Created by John Hu on 2021/10/12.
//  Copyright © 2021 Moxtra. All rights reserved.
//

#import <Foundation/Foundation.h>
NS_ASSUME_NONNULL_BEGIN

@class MEPMeet;
@protocol MEPMeetListModelDelegate;

@interface MEPMeetListModel : NSObject
@property (nonatomic, weak) id<MEPMeetListModelDelegate> delegate;
- (void)fetchMeetsFromTimestamp:(NSTimeInterval)fromTimestamp toTimestamp:(NSTimeInterval)toTimestamp withCompletionHandler:(void (^__nullable)(NSError  *__nullable errorOrNil, NSArray<MEPMeet *> *__nullable meets))completionHandler;

@end

@protocol MEPMeetListModelDelegate <NSObject>
@optional
/**
Tells the delegate that some meets have been created.
@param meetListModel The related meet list model.
@param createdMeets  An array containing created meets.
*/
- (void)meetListModel:(MEPMeetListModel *)meetListModel didCreateMeets:(NSArray<MEPMeet *> *)createdMeets;

/**
Tells the delegate that some meets have been updated.
@param meetListModel The related meet list model.
@param updatedMeets  An array containing updated meets.
*/
- (void)meetListModel:(MEPMeetListModel *)meetListModel didUpdateMeets:(NSArray<MEPMeet *> *)updatedMeets;

/**
Tells the delegate that some meets have been deleted.
@param meetListModel The related meet list model.
@param deletedMeets  An array containing deleted meets.
*/
- (void)meetListModel:(MEPMeetListModel *)meetListModel didDeleteMeets:(NSArray<MEPMeet *> *)deletedMeets;
@end

NS_ASSUME_NONNULL_END
