//
//  InsiderEvent.h
//  InsiderShop
//
//  Created by Insider on 20.06.2019.
//  Copyright © 2019 Insider. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface InsiderEvent : NSObject <NSCopying>

-(instancetype)initWithName:(NSString *)name;
-(instancetype)initInternalWithName:(NSString *)name;

/**
 This method allows you to add multiple parameters to the InsiderEvent object at once.
 @discussion The parameter takes a value of type NSDictionary.
 
 @warning Your parameter keys must be all lowercase and not contain any special or non-Latin characters, otherwise the event will be ignored. Check our documentation for more information.
 */
-(InsiderEvent*(^)(NSDictionary *))addParameters;

/**
 This method allows you to add a NSString type parameter to the InsiderEvent object.
 @discussion The first parameter is the key and the second parameter is the value of the parameter that is going to be added to the InsiderEvent object.
 
 @warning Your parameter key should be all lowercase and should not include any special or non Latin characters, otherwise event will be ignored. For more information check our documentation.
 */
-(InsiderEvent*(^)(NSString *, NSString *))addParameterWithString;

/**
 This method allows you to add a int type parameter to the InsiderEvent object.
 @warning Your parameter key should be all lowercase and should not include any special or non Latin characters, otherwise event will be ignored. For more information check our documentation.
 
 @discussion The first parameter is the key and the second parameter is the value of the parameter that is going to be added to the InsiderEvent object.
 */
-(InsiderEvent*(^)(NSString *, NSInteger))addParameterWithInt;

/**
 This method allows you to add a double type parameter to the InsiderEvent object.
 @discussion The first parameter is the key and the second parameter is the value of the parameter that is going to be added to the InsiderEvent object.
 
 @warning Your parameter key should be all lowercase and should not include any special or non Latin characters, otherwise event will be ignored. For more information check our documentation.
 */
-(InsiderEvent*(^)(NSString *, double))addParameterWithDouble;

/**
 This method allows you to add a bool type parameter to the InsiderEvent object.
 @discussion The first parameter is the key and the second parameter is the value of the parameter that is going to be added to the InsiderEvent object.
 
 @warning Your parameter key should be all lowercase and should not include any special or non Latin characters, otherwise event will be ignored. For more information check our documentation.
 */
-(InsiderEvent*(^)(NSString *, BOOL))addParameterWithBoolean;

/**
 This method allows you to add a NSDate type parameter to the InsiderEvent object.
 @discussion The first parameter is the key and the second parameter is the value of the parameter that is going to be added to the InsiderEvent object.
 
 @warning Your parameter key should be all lowercase and should not include any special or non Latin characters, otherwise event will be ignored. For more information check our documentation.
 */
-(InsiderEvent*(^)(NSString *, NSDate *))addParameterWithDate;

/**
 This method allows you to add a NSArray type parameter to the InsiderEvent object.
 @discussion The first parameter is the key and the second parameter is the value of the parameter that is going to be added to the InsiderEvent object.
 
 @warning Your parameter key should be all lowercase and should not include any special or non Latin characters, otherwise event will be ignored. For more information check our documentation.
 */
-(InsiderEvent*(^)(NSString *, NSArray *))addParameterWithArray;

/**
 This method allows you to build your InsiderEvent object, this is the way of saying to InsiderSDK that this event is actually triggered.
 @discussion You need to call this method after you finishing the building up your InsiderEvent object.
 
 @warning Without calling this method your event will not be triggered.
 */
-(void)build;

/**
 This method will return the name of the InsiderEvent object.
 
 @return Name of the InsiderEvent object.
 */
-(NSString *)getName;

/**
 This method will return the parameters of the InsiderEvent object.
 
 @return Parameters of the InsiderEvent object.
 */
-(NSDictionary *)getParameters;

@end
