//
//  RMStoreUserDefaultsPersistence.h
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

#import <Foundation/Foundation.h>
#import "RMStore.h"
@class RMStoreTransaction;

/** Transaction persistence using NSUserDefaults. For heightened security, consider subclassing this class and overriding the Obfuscation category methods.
 */
@interface RMStoreUserDefaultsPersistence : NSObject<RMStoreTransactionPersistor>

/** Remove all transactions from user defaults.
 */
- (void)removeTransactions;

/** Consume the given product if available. Intended for consumable products.
 @param productIdentifier Identifier of the product to be consumed.
 @return YES if the product was consumed, NO otherwise.
 */
- (BOOL)consumeProductOfIdentifier:(NSString*)productIdentifier;

/** Returns the number of transactions for the given product that have not been consumed. Intended for consumable products.
 @return The number of transactions for the given product that have not been consumed.
 */
- (NSInteger)countProductOfdentifier:(NSString*)productIdentifier;

/**
 Indicates wheter the given product has been purchased. Intended for non-consumables.
 @param productIdentifier Identifier of the product.
 @return YES if there is at least one transaction for the given product, NO otherwise. Note that if the product is consumable this method will still return YES even if all transactions have been consumed.
 */
- (BOOL)isPurchasedProductOfIdentifier:(NSString*)productIdentifier;

/** Returns the product identifiers of all products that have a transaction.
 */
- (NSSet*)purchasedProductIdentifiers;

/**
 Returns all the transactions for the given product.
 @param productIdentifier Identifier of the product whose transactions will be returned.
 @return An array of RMStoreTransaction objects (not SKPaymentTransaction) for the given product.
 @see RMStoreTransaction
 */
- (NSArray*)transactionsForProductOfIdentifier:(NSString*)productIdentifier;

@end

/** Subclasess should override these methods to use their own obfuscation.
 */
@interface RMStoreUserDefaultsPersistence(Obfuscation)

/** Returns a data representation of the given transaction. The default implementation uses NSKeyedArchiver.
 @param transaction Transaction to be converted into data
 @return Data representation of the given transaction
 */
- (NSData*)dataWithTransaction:(RMStoreTransaction*)transaction;

/** Returns a transaction from the given data. The default implementation uses NSKeyedUnarchiver.
 @param data Data from which a transaction will be obtained
 @return Transaction from the given data
 */
- (RMStoreTransaction*)transactionWithData:(NSData*)data;

@end