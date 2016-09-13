//
//  RMStore.h
//  RMStore
//
//  Created by Hermes Pique on 12/6/09.
//  Copyright (c) 2013 Robot Media SL (http://www.robotmedia.net)
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
#import <StoreKit/StoreKit.h>

@protocol RMStoreContentDownloader;
@protocol RMStoreReceiptVerificator;
@protocol RMStoreTransactionPersistor;
@protocol RMStoreObserver;

extern NSString *const RMStoreErrorDomain;
extern NSInteger const RMStoreErrorCodeDownloadCanceled;
extern NSInteger const RMStoreErrorCodeUnknownProductIdentifier;
extern NSInteger const RMStoreErrorCodeUnableToCompleteVerification;

/** A StoreKit wrapper that adds blocks and notifications, plus optional receipt verification and purchase management.
 */
@interface RMStore : NSObject<SKPaymentTransactionObserver>

///---------------------------------------------
/// @name Getting the Store
///---------------------------------------------

/** Returns the singleton store instance.
 */
+ (RMStore*)defaultStore;

#pragma mark StoreKit Wrapper
///---------------------------------------------
/// @name Calling StoreKit
///---------------------------------------------

/** Returns whether the user is allowed to make payments.
 */
+ (BOOL)canMakePayments;

/** Request payment of the product with the given product identifier.
 @param productIdentifier The identifier of the product whose payment will be requested.
 */
- (void)addPayment:(NSString*)productIdentifier;

/** Request payment of the product with the given product identifier. `successBlock` will be called if the payment is successful, `failureBlock` if it isn't.
 @param productIdentifier The identifier of the product whose payment will be requested.
 @param successBlock The block to be called if the payment is sucessful. Can be `nil`.
 @param failureBlock The block to be called if the payment fails or there isn't any product with the given identifier. Can be `nil`.
 */
- (void)addPayment:(NSString*)productIdentifier
           success:(void (^)(SKPaymentTransaction *transaction))successBlock
           failure:(void (^)(SKPaymentTransaction *transaction, NSError *error))failureBlock;

/** Request payment of the product with the given product identifier. `successBlock` will be called if the payment is successful, `failureBlock` if it isn't.
 @param productIdentifier The identifier of the product whose payment will be requested.
 @param userIdentifier An opaque identifier of the user’s account, if applicable. Can be `nil`.
 @param successBlock The block to be called if the payment is sucessful. Can be `nil`.
 @param failureBlock The block to be called if the payment fails or there isn't any product with the given identifier. Can be `nil`.
 @see [SKPayment applicationUsername]
 */
- (void)addPayment:(NSString*)productIdentifier
              user:(NSString*)userIdentifier
           success:(void (^)(SKPaymentTransaction *transaction))successBlock
           failure:(void (^)(SKPaymentTransaction *transaction, NSError *error))failureBlock __attribute__((availability(ios,introduced=7.0)));

/** Request localized information about a set of products from the Apple App Store.
 @param identifiers The set of product identifiers for the products you wish to retrieve information of.
 */
- (void)requestProducts:(NSSet*)identifiers;

/** Request localized information about a set of products from the Apple App Store. `successBlock` will be called if the products request is successful, `failureBlock` if it isn't.
 @param identifiers The set of product identifiers for the products you wish to retrieve information of.
 @param successBlock The block to be called if the products request is sucessful. Can be `nil`. It takes two parameters: `products`, an array of SKProducts, one product for each valid product identifier provided in the original request, and `invalidProductIdentifiers`, an array of product identifiers that were not recognized by the App Store.
 @param failureBlock The block to be called if the products request fails. Can be `nil`.
 */
- (void)requestProducts:(NSSet*)identifiers
                success:(void (^)(NSArray *products, NSArray *invalidProductIdentifiers))successBlock
                failure:(void (^)(NSError *error))failureBlock;

/** Request to restore previously completed purchases.
 */
- (void)restoreTransactions;

/** Request to restore previously completed purchases. `successBlock` will be called if the restore transactions request is successful, `failureBlock` if it isn't.
 @param successBlock The block to be called if the restore transactions request is sucessful. Can be `nil`.
 @param failureBlock The block to be called if the restore transactions request fails. Can be `nil`.
 */
- (void)restoreTransactionsOnSuccess:(void (^)(NSArray *transactions))successBlock
                             failure:(void (^)(NSError *error))failureBlock;


/** Request to restore previously completed purchases of a certain user. `successBlock` will be called if the restore transactions request is successful, `failureBlock` if it isn't.
 @param userIdentifier An opaque identifier of the user’s account.
 @param successBlock The block to be called if the restore transactions request is sucessful. Can be `nil`.
 @param failureBlock The block to be called if the restore transactions request fails. Can be `nil`.
 */
- (void)restoreTransactionsOfUser:(NSString*)userIdentifier
                        onSuccess:(void (^)(NSArray *transactions))successBlock
                          failure:(void (^)(NSError *error))failureBlock __attribute__((availability(ios,introduced=7.0)));

#pragma mark Receipt
///---------------------------------------------
/// @name Getting the receipt
///---------------------------------------------

/** Returns the url of the bundle’s App Store receipt, or nil if the receipt is missing.
 If this method returns `nil` you should refresh the receipt by calling `refreshReceipt`.
 @see refreshReceipt
 */
+ (NSURL*)receiptURL __attribute__((availability(ios,introduced=7.0)));

/** Request to refresh the App Store receipt in case the receipt is invalid or missing.
 */
- (void)refreshReceipt __attribute__((availability(ios,introduced=7.0)));

/** Request to refresh the App Store receipt in case the receipt is invalid or missing. `successBlock` will be called if the refresh receipt request is successful, `failureBlock` if it isn't.
 @param successBlock The block to be called if the refresh receipt request is sucessful. Can be `nil`.
 @param failureBlock The block to be called if the refresh receipt request fails. Can be `nil`.
 */
- (void)refreshReceiptOnSuccess:(void (^)())successBlock
                        failure:(void (^)(NSError *error))failureBlock __attribute__((availability(ios,introduced=7.0)));

///---------------------------------------------
/// @name Setting Delegates
///---------------------------------------------

/**
 The content downloader. Required to download product content from your own server.
 @discussion Hosted content from Apple’s server (SKDownload) is handled automatically. You don't need to provide a content downloader for it.
 */
@property (nonatomic, weak) id<RMStoreContentDownloader> contentDownloader;

/** The receipt verificator. You can provide your own or use one of the reference implementations provided by the library.
 @see RMStoreAppReceiptVerificator
 @see RMStoreTransactionReceiptVerificator
 */
@property (nonatomic, weak) id<RMStoreReceiptVerificator> receiptVerificator;

/**
 The transaction persistor. It is recommended to provide your own obfuscator if piracy is a concern. The store will use weak obfuscation via `NSKeyedArchiver` by default.
 @see RMStoreKeychainPersistence
 @see RMStoreUserDefaultsPersistence
 */
@property (nonatomic, weak) id<RMStoreTransactionPersistor> transactionPersistor;


#pragma mark Product management
///---------------------------------------------
/// @name Managing Products
///---------------------------------------------

- (SKProduct*)productForIdentifier:(NSString*)productIdentifier;

+ (NSString*)localizedPriceOfProduct:(SKProduct*)product;

#pragma mark Notifications
///---------------------------------------------
/// @name Managing Observers
///---------------------------------------------

/** Adds an observer to the store.
 Unlike `SKPaymentQueue`, it is not necessary to set an observer.
 @param observer The observer to add.
 */
- (void)addStoreObserver:(id<RMStoreObserver>)observer;

/** Removes an observer from the store.
 @param observer The observer to remove.
 */
- (void)removeStoreObserver:(id<RMStoreObserver>)observer;

@end

@protocol RMStoreContentDownloader <NSObject>

/**
 Downloads the self-hosted content associated to the given transaction and calls the given success or failure block accordingly. Can also call the given progress block to notify progress.
 @param transaction The transaction whose associated content will be downloaded.
 @param successBlock Called if the download was successful. Must be called in the main queue.
 @param progressBlock Called to notify progress. Provides a number between 0.0 and 1.0, inclusive, where 0.0 means no data has been downloaded and 1.0 means all the data has been downloaded. Must be called in the main queue.
 @param failureBlock Called if the download failed. Must be called in the main queue.
 @discussion Hosted content from Apple’s server (@c SKDownload) is handled automatically by RMStore.
 */
- (void)downloadContentForTransaction:(SKPaymentTransaction*)transaction
                              success:(void (^)())successBlock
                             progress:(void (^)(float progress))progressBlock
                              failure:(void (^)(NSError *error))failureBlock;

@end

@protocol RMStoreTransactionPersistor<NSObject>

- (void)persistTransaction:(SKPaymentTransaction*)transaction;

@end

@protocol RMStoreReceiptVerificator <NSObject>

/** Verifies the given transaction and calls the given success or failure block accordingly.
 @param transaction The transaction to be verified.
 @param successBlock Called if the transaction passed verification. Must be called in the main queu.
 @param failureBlock Called if the transaction failed verification. If verification could not be completed (e.g., due to connection issues), then error must be of code RMStoreErrorCodeUnableToCompleteVerification to prevent RMStore to finish the transaction. Must be called in the main queu.
 */
// - (void)verifyTransaction:(SKPaymentTransaction*)transaction
//                   success:(void (^)())successBlock
//                   failure:(void (^)(NSError *error))failureBlock;

@end

@protocol RMStoreObserver<NSObject>
@optional

/**
 Tells the observer that a download has been canceled.
 @discussion Only for Apple-hosted downloads.
 */
- (void)storeDownloadCanceled:(NSNotification*)notification __attribute__((availability(ios,introduced=6.0)));

/**
 Tells the observer that a download has failed. Use @c storeError to get the cause.
 */
- (void)storeDownloadFailed:(NSNotification*)notification __attribute__((availability(ios,introduced=6.0)));

/**
 Tells the observer that a download has finished.
 */
- (void)storeDownloadFinished:(NSNotification*)notification __attribute__((availability(ios,introduced=6.0)));

/**
 Tells the observer that a download has been paused.
 @discussion Only for Apple-hosted downloads.
 */
- (void)storeDownloadPaused:(NSNotification*)notification __attribute__((availability(ios,introduced=6.0)));

/**
 Tells the observer that a download has been updated. Use @c downloadProgress to get the progress.
 */
- (void)storeDownloadUpdated:(NSNotification*)notification __attribute__((availability(ios,introduced=6.0)));

- (void)storePaymentTransactionDeferred:(NSNotification*)notification __attribute__((availability(ios,introduced=8.0)));
- (void)storePaymentTransactionFailed:(NSNotification*)notification;
- (void)storePaymentTransactionFinished:(NSNotification*)notification;
- (void)storeProductsRequestFailed:(NSNotification*)notification;
- (void)storeProductsRequestFinished:(NSNotification*)notification;
- (void)storeRefreshReceiptFailed:(NSNotification*)notification __attribute__((availability(ios,introduced=7.0)));
- (void)storeRefreshReceiptFinished:(NSNotification*)notification __attribute__((availability(ios,introduced=7.0)));
- (void)storeRestoreTransactionsFailed:(NSNotification*)notification;
- (void)storeRestoreTransactionsFinished:(NSNotification*)notification;

@end

/**
 Category on NSNotification to recover store data from userInfo without requiring to know the keys.
 */
@interface NSNotification(RMStore)

/**
 A value that indicates how much of the file has been downloaded.
 The value of this property is a floating point number between 0.0 and 1.0, inclusive, where 0.0 means no data has been downloaded and 1.0 means all the data has been downloaded. Typically, your app uses the value of this property to update a user interface element, such as a progress bar, that displays how much of the file has been downloaded.
 @discussion Corresponds to [SKDownload progress].
 @discussion Used in @c storeDownloadUpdated:.
 */
@property (nonatomic, readonly) float rm_downloadProgress;

/** Array of product identifiers that were not recognized by the App Store. Used in @c storeProductsRequestFinished:.
 */
@property (nonatomic, readonly) NSArray *rm_invalidProductIdentifiers;

/** Used in @c storeDownload*:, @c storePaymentTransactionFinished: and @c storePaymentTransactionFailed:.
 */
@property (nonatomic, readonly) NSString *rm_productIdentifier;

/** Array of SKProducts, one product for each valid product identifier provided in the corresponding request. Used in @c storeProductsRequestFinished:.
 */
@property (nonatomic, readonly) NSArray *rm_products;

/** Used in @c storeDownload*:.
 */
@property (nonatomic, readonly) SKDownload *rm_storeDownload __attribute__((availability(ios,introduced=6.0)));

/** Used in @c storeDownloadFailed:, @c storePaymentTransactionFailed:, @c storeProductsRequestFailed:, @c storeRefreshReceiptFailed: and @c storeRestoreTransactionsFailed:.
 */
@property (nonatomic, readonly) NSError *rm_storeError;

/** Used in @c storeDownload*:, @c storePaymentTransactionFinished: and in @c storePaymentTransactionFailed:.
 */
@property (nonatomic, readonly) SKPaymentTransaction *rm_transaction;

/** Used in @c storeRestoreTransactionsFinished:.
 */
@property (nonatomic, readonly) NSArray *rm_transactions;

@end
