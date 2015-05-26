//
//  RMStoreUserDefaultsPersistence.m
//  RMStore
//
//  Created by Hermes on 10/16/13.
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

#import "RMStoreUserDefaultsPersistence.h"
#import "RMStoreTransaction.h"

NSString* const RMStoreTransactionsUserDefaultsKey = @"RMStoreTransactions";

@implementation RMStoreUserDefaultsPersistence

#pragma mark - RMStoreTransactionPersistor

- (void)persistTransaction:(SKPaymentTransaction*)paymentTransaction
{
    NSUserDefaults *defaults = [self userDefaults];
    NSDictionary *purchases = [defaults objectForKey:RMStoreTransactionsUserDefaultsKey] ? : @{};
    
    SKPayment *payment = paymentTransaction.payment;
    NSString *productIdentifier = payment.productIdentifier;

    NSArray *transactions = purchases[productIdentifier] ? : @[];
    NSMutableArray *updatedTransactions = [NSMutableArray arrayWithArray:transactions];
    
    RMStoreTransaction *transaction = [[RMStoreTransaction alloc] initWithPaymentTransaction:paymentTransaction];
    NSData *data = [self dataWithTransaction:transaction];
    [updatedTransactions addObject:data];
    [self setTransactions:updatedTransactions forProductIdentifier:productIdentifier];
}

#pragma mark - Public

- (void)removeTransactions
{
    NSUserDefaults *defaults = [self userDefaults];
    [defaults removeObjectForKey:RMStoreTransactionsUserDefaultsKey];
    [defaults synchronize];
}

- (BOOL)consumeProductOfIdentifier:(NSString*)productIdentifier
{
    NSUserDefaults *defaults = [self userDefaults];
    NSDictionary *purchases = [defaults objectForKey:RMStoreTransactionsUserDefaultsKey] ? : @{};
    NSArray *transactions = purchases[productIdentifier] ? : @[];
    for (NSData *data in transactions)
    {
        RMStoreTransaction *transaction = [self transactionWithData:data];
        if (!transaction.consumed)
        {
            transaction.consumed = YES;
            NSData *updatedData = [self dataWithTransaction:transaction];
            NSMutableArray *updatedTransactions = [NSMutableArray arrayWithArray:transactions];
            NSInteger index = [updatedTransactions indexOfObject:data];
            updatedTransactions[index] = updatedData;
            [self setTransactions:updatedTransactions forProductIdentifier:productIdentifier];
            return YES;
        }
    }
    return NO;
}

- (NSInteger)countProductOfdentifier:(NSString*)productIdentifier
{
    NSArray *transactions = [self transactionsForProductOfIdentifier:productIdentifier];
    NSInteger count = 0;
    for (RMStoreTransaction *transaction in transactions)
    {
        if (!transaction.consumed) { count++; }
    }
    return count;
}

- (BOOL)isPurchasedProductOfIdentifier:(NSString*)productIdentifier
{
    NSArray *transactions = [self transactionsForProductOfIdentifier:productIdentifier];
    return transactions.count > 0;
}

- (NSSet*)purchasedProductIdentifiers
{
    NSUserDefaults *defaults = [self userDefaults];
    NSDictionary *purchases = [defaults objectForKey:RMStoreTransactionsUserDefaultsKey];
    NSSet *productIdentifiers = [NSSet setWithArray:purchases.allKeys];
    return productIdentifiers;
}

- (NSArray*)transactionsForProductOfIdentifier:(NSString*)productIdentifier
{
    NSUserDefaults *defaults = [self userDefaults];
    NSDictionary *purchases = [defaults objectForKey:RMStoreTransactionsUserDefaultsKey];
    NSArray *obfuscatedTransactions = purchases[productIdentifier] ? : @[];
    NSMutableArray *transactions = [NSMutableArray arrayWithCapacity:obfuscatedTransactions.count];
    for (NSData *data in obfuscatedTransactions)
    {
        RMStoreTransaction *transaction = [self transactionWithData:data];
        [transactions addObject:transaction];
    }
    return transactions;
}

- (NSUserDefaults *)userDefaults
{
    return [NSUserDefaults standardUserDefaults];
}

#pragma mark - Obfuscation

- (NSData*)dataWithTransaction:(RMStoreTransaction*)transaction
{
    NSMutableData *data = [[NSMutableData alloc] init];
    NSKeyedArchiver *archiver = [[NSKeyedArchiver alloc] initForWritingWithMutableData:data];
    [archiver encodeObject:transaction];
    [archiver finishEncoding];
    return data;
}

- (RMStoreTransaction*)transactionWithData:(NSData*)data
{
    NSKeyedUnarchiver *unarchiver = [[NSKeyedUnarchiver alloc] initForReadingWithData:data];
    RMStoreTransaction *transaction = [unarchiver decodeObject];
    [unarchiver finishDecoding];
    return transaction;
}

#pragma mark - Private

- (void)setTransactions:(NSArray*)transactions forProductIdentifier:(NSString*)productIdentifier
{
    NSUserDefaults *defaults = [self userDefaults];
    NSDictionary *purchases = [defaults objectForKey:RMStoreTransactionsUserDefaultsKey] ? : @{};
    NSMutableDictionary *updatedPurchases = [NSMutableDictionary dictionaryWithDictionary:purchases];
    updatedPurchases[productIdentifier] = transactions;
    [defaults setObject:updatedPurchases forKey:RMStoreTransactionsUserDefaultsKey];
    [defaults synchronize];
}

@end
