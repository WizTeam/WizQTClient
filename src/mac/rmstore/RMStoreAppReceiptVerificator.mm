//
//  RMStoreAppReceiptVerificator.m
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

#import "RMStoreAppReceiptVerificator.h"
#import "RMAppReceipt.h"

#ifdef BUILD4APPSTORE

@implementation RMStoreAppReceiptVerificator

//- (void)verifyTransaction:(SKPaymentTransaction*)transaction
//                           success:(void (^)())successBlock
//                           failure:(void (^)(NSError *error))failureBlock
//{
//    RMAppReceipt *receipt = [RMAppReceipt bundleReceipt];
//    const BOOL verified = [self verifyTransaction:transaction inReceipt:receipt success:successBlock failure:nil]; // failureBlock is nil intentionally. See below.
//    if (verified) return;

//    // Apple recommends to refresh the receipt if validation fails on iOS
//    [[RMStore defaultStore] refreshReceiptOnSuccess:^{
//        RMAppReceipt *receipt = [RMAppReceipt bundleReceipt];
//        [self verifyTransaction:transaction inReceipt:receipt success:successBlock failure:failureBlock];
//    } failure:^(NSError *error) {
//        [self failWithBlock:failureBlock error:error];
//    }];
//}

- (BOOL)verifyAppReceipt
{
    RMAppReceipt *receipt = [RMAppReceipt bundleReceipt];
    return [self verifyAppReceipt:receipt];
}

#pragma mark - Properties

- (NSString*)bundleIdentifier
{
    if (!_bundleIdentifier)
    {
        return [[NSBundle mainBundle] bundleIdentifier];
    }
    return _bundleIdentifier;
}

- (NSString*)bundleVersion
{
    if (!_bundleVersion)
    {
        return [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    }
    return _bundleVersion;
}

#pragma mark - Private

- (BOOL)verifyAppReceipt:(RMAppReceipt*)receipt
{
    NSLog(@"get receipt %@", receipt);    
    if (!receipt) return NO;

    NSLog(@"compare app bundleIdentifier : %@ and receipt bundleIdentifier : %@", self.bundleIdentifier, receipt.bundleIdentifier);
    if (![receipt.bundleIdentifier isEqualToString:self.bundleIdentifier]) return NO;

    NSLog(@"compare app verion : %@ and receipt version : %@", self.bundleVersion, receipt.appVersion);
    if (![receipt.appVersion isEqualToString:self.bundleVersion]) return NO;

    if (![receipt verifyReceiptHash]) return NO;

    return YES;
}

//- (BOOL)verifyTransaction:(SKPaymentTransaction*)transaction
//                inReceipt:(RMAppReceipt*)receipt
//                           success:(void (^)())successBlock
//                           failure:(void (^)(NSError *error))failureBlock
//{
//    const BOOL receiptVerified = [self verifyAppReceipt:receipt];
//    if (!receiptVerified)
//    {
//        [self failWithBlock:failureBlock message:NSLocalizedStringFromTable(@"The app receipt failed verification", @"RMStore", nil)];
//        return NO;
//    }
//    SKPayment *payment = transaction.payment;
//    const BOOL transactionVerified = [receipt containsInAppPurchaseOfProductIdentifier:payment.productIdentifier];
//    if (!transactionVerified)
//    {
//        [self failWithBlock:failureBlock message:NSLocalizedStringFromTable(@"The app receipt does not contain the given product", @"RMStore", nil)];
//        return NO;
//    }
//    if (successBlock)
//    {
//        successBlock();
//    }
//    return YES;
//}

//- (void)failWithBlock:(void (^)(NSError *error))failureBlock message:(NSString*)message
//{
//    NSError *error = [NSError errorWithDomain:RMStoreErrorDomain code:0 userInfo:@{NSLocalizedDescriptionKey : message}];
//    [self failWithBlock:failureBlock error:error];
//}

//- (void)failWithBlock:(void (^)(NSError *error))failureBlock error:(NSError*)error
//{
//    if (failureBlock)
//    {
//        failureBlock(error);
//    }
//}

@end

#endif
