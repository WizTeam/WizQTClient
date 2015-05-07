//
//  RMStoreAppReceiptVerificator.h
//  RMStore
//
//  Created by Hermes on 10/15/13.
//  Copyright (c) 2013 Robot Media. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#import <Foundation/Foundation.h>
#import "RMStore.h"

/**
 Reference implementation of an app receipt verificator. If security is a concern you might want to avoid using a verificator whose code is open source.
 */
__attribute__((availability(ios,introduced=7.0)))
@interface RMStoreAppReceiptVerificator : NSObject<RMStoreReceiptVerificator>

/**
 The value that will be used to validate the bundle identifier included in the app receipt. Given that it is possible to modify the app bundle in jailbroken devices, setting this value from a hardcoded string might provide better protection.
 @return The given value, or the app's bundle identifier by defult.
 */
@property (nonatomic, strong) NSString *bundleIdentifier;

/**
 The value that will be used to validate the bundle version included in the app receipt. Given that it is possible to modify the app bundle in jailbroken devices, setting this value from a hardcoded string might provide better protection.
 @return The given value, or the app's bundle version by defult.
 */
@property (nonatomic, strong) NSString *bundleVersion;

/**
 Verifies the app receipt by checking the integrity of the receipt, comparing its bundle identifier and bundle version to the values returned by the corresponding properties and verifying the receipt hash.
 @return YES if the receipt is verified, NO otherwise.
 @discussion If validation fails in iOS, Apple recommends to refresh the receipt and try again.
 */
- (BOOL)verifyAppReceipt;

@end
