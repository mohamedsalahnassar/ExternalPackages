//
//  MEPRelationListModel.h
//  MEPSDK
//
//  Created by John Hu on 2021/9/26.
//  Copyright © 2021 Moxtra. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN
@class MEPRelation;
@protocol MEPRelationListModelDelegate;

@interface MEPRelationListModel : NSObject
@property (nonatomic, weak) id<MEPRelationListModelDelegate> __nullable delegate;

/**
 Return an array containing all relations.
 @discussion From 9.1, server will only return relation chats in limited count(2000 by default) for better performence.
 Should the index of a relation's chats exceed this threshold, its chat data will not be available
 you can contact server admin to change the cap.
 Or you can use new API fetchRelationsWithCompletionHandler get complete data.
 */
@property (nonatomic, strong) NSArray<MEPRelation *> *__nullable relations MEP_DEPRECATED_WILL_BE_REMOVED_IN_PLEASE_USE("", "fetchRelationsWithCompletionHandler");;

/**
 Fetch complete relation data
 * @param handler A block object to be executed when the action completes
 */
- (void)fetchRelationsWithCompletionHandler:(void(^__nullable)(NSError *__nullable errorOrNil, NSArray<MEPRelation *> *relations))handler;

/**
* Get relation object from unique id
* @param uniqueId Relation user's uniqueId
* @param handler A block object to be executed when the action completes
*/
- (void)relationWithUniqueId:(NSString *)uniqueId
           completionHandler:(void(^__nullable)(NSError *__nullable errorOrNil, MEPRelation *__nullable relation))handler;

/**
* Get relation object from email
* @param uniqueId Relation user's email
* @param handler A block object to be executed when the action completes
*/
- (void)relationWithEmail:(NSString *)email
        completionHandler:(void(^__nullable)(NSError *__nullable errorOrNil, MEPRelation *__nullable relation))handler;

@end

@protocol MEPRelationListModelDelegate<NSObject>
@optional
/**
Tells the delegate that some relations have been created.
@param relationListModel The related relation list model.
@param createdRelations  An array containing created relations.
*/
- (void)relationListModel:(MEPRelationListModel *)relationListModel didCreateRelations:(NSArray<MEPRelation *> *)createdRelations;

/**
Tells the delegate that some relations have been updated.
 @param relationListModel The related relation list model.
@param updatedRelations  An array containing updated relations.
*/
- (void)relationListModel:(MEPRelationListModel *)relationListModel didUpdateRelations:(NSArray<MEPRelation *> *)updatedRelations;

/**
Tells the delegate that some relations have been deleted.
 @param relationListModel The related relation list model.
@param deletedRelations  An array containing deleted relations.
*/
- (void)relationListModel:(MEPRelationListModel *)relationListModel didDeleteRelations:(NSArray<MEPRelation *> *)deletedRelations;
@end

NS_ASSUME_NONNULL_END
