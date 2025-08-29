//
//  MEPOutOfOfficeViewController.h
//  MEPSDK
//
//  Created by John on 2024/1/12.
//  Copyright © 2024 Moxtra. All rights reserved.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

/**
 MEPOutOfOfficeViewController
 only has value when:
 current user loged in and is internal user
 current org enabled user presence feature
*/
@interface MEPOutOfOfficeViewController : UINavigationController

#pragma mark - Unavailable
- (instancetype)initWithRootViewController:(UIViewController *)rootViewController UNAVAILABLE_ATTRIBUTE;
- (instancetype)initWithCoder:(NSCoder *)aDecoder UNAVAILABLE_ATTRIBUTE;
- (instancetype)initWithNibName:(NSString * _Nullable)nibNameOrNil bundle:(NSBundle * _Nullable)nibBundleOrNil UNAVAILABLE_ATTRIBUTE;
@end

NS_ASSUME_NONNULL_END
