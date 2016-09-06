//
//  RMStoreTransactionReceiptVerificator.m
//  RMStore
//
//  Created by Hermes Pique on 7/31/13.
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

#import "RMStoreTransactionReceiptVerificator.h"

#ifdef DEBUG
#define RMStoreLog(...) NSLog(@"RMStore: %@", [NSString stringWithFormat:__VA_ARGS__]);
#else
#define RMStoreLog(...)
#endif

@interface NSData(rm_base64)

- (NSString *)rm_stringByBase64Encoding;

@end

@implementation NSData(rm_base64)

static const char _base64EncodingTable[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

- (NSString *)rm_stringByBase64Encoding
{ // From: http://stackoverflow.com/a/4727124/143378
    const unsigned char * objRawData = self.bytes;
    char * objPointer;
    char * strResult;
    
    // Get the Raw Data length and ensure we actually have data
    NSInteger intLength = self.length;
    if (intLength == 0) return nil;
    
    // Setup the String-based Result placeholder and pointer within that placeholder
    strResult = (char *)calloc((((intLength + 2) / 3) * 4) + 1, sizeof(char));
    objPointer = strResult;
    
    // Iterate through everything
    while (intLength > 2) { // keep going until we have less than 24 bits
        *objPointer++ = _base64EncodingTable[objRawData[0] >> 2];
        *objPointer++ = _base64EncodingTable[((objRawData[0] & 0x03) << 4) + (objRawData[1] >> 4)];
        *objPointer++ = _base64EncodingTable[((objRawData[1] & 0x0f) << 2) + (objRawData[2] >> 6)];
        *objPointer++ = _base64EncodingTable[objRawData[2] & 0x3f];
        
        // we just handled 3 octets (24 bits) of data
        objRawData += 3;
        intLength -= 3;
    }
    
    // now deal with the tail end of things
    if (intLength != 0) {
        *objPointer++ = _base64EncodingTable[objRawData[0] >> 2];
        if (intLength > 1) {
            *objPointer++ = _base64EncodingTable[((objRawData[0] & 0x03) << 4) + (objRawData[1] >> 4)];
            *objPointer++ = _base64EncodingTable[(objRawData[1] & 0x0f) << 2];
            *objPointer++ = '=';
        } else {
            *objPointer++ = _base64EncodingTable[(objRawData[0] & 0x03) << 4];
            *objPointer++ = '=';
            *objPointer++ = '=';
        }
    }
    
    // Terminate the string-based result
    *objPointer = '\0';
    
    // Create result NSString object
    NSString *base64String = @(strResult);
    
    // Free memory
    free(strResult);
    
    return base64String;
}

@end


@implementation RMStoreTransactionReceiptVerificator

- (void)verifyTransaction:(SKPaymentTransaction*)transaction
                           success:(void (^)())successBlock
                           failure:(void (^)(NSError *error))failureBlock
{    
    NSString *receipt = [transaction.transactionReceipt rm_stringByBase64Encoding];
    if (receipt == nil)
    {
        if (failureBlock != nil)
        {
            NSError *error = [NSError errorWithDomain:RMStoreErrorDomain code:0 userInfo:nil];
            failureBlock(error);
        }
        return;
    }
    static NSString *receiptDataKey = @"receipt-data";
    NSDictionary *jsonReceipt = @{receiptDataKey : receipt};
    
    NSError *error;
    NSData *requestData = [NSJSONSerialization dataWithJSONObject:jsonReceipt options:0 error:&error];
    if (!requestData)
    {
        RMStoreLog(@"Failed to serialize receipt into JSON");
        if (failureBlock != nil)
        {
            failureBlock(error);
        }
        return;
    }
    
    static NSString *productionURL = @"https://buy.itunes.apple.com/verifyReceipt";
    
    [self verifyRequestData:requestData url:productionURL success:successBlock failure:failureBlock];
}

- (void)verifyRequestData:(NSData*)requestData
                      url:(NSString*)urlString
                  success:(void (^)())successBlock
                  failure:(void (^)(NSError *error))failureBlock
{
    NSURL *url = [NSURL URLWithString:urlString];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
    request.HTTPBody = requestData;
    static NSString *requestMethod = @"POST";
    request.HTTPMethod = requestMethod;

    dispatch_async(dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSError *error;
        NSData *data = [NSURLConnection sendSynchronousRequest:request returningResponse:nil error:&error];
        dispatch_async(dispatch_get_main_queue(), ^{
            if (!data)
            {
                RMStoreLog(@"Server Connection Failed");
                NSError *wrapperError = [NSError errorWithDomain:RMStoreErrorDomain code:RMStoreErrorCodeUnableToCompleteVerification userInfo:@{NSUnderlyingErrorKey : error, NSLocalizedDescriptionKey : NSLocalizedStringFromTable(@"Connection to Apple failed. Check the underlying error for more info.", @"RMStore", @"Error description")}];
                if (failureBlock != nil)
                {
                    failureBlock(wrapperError);
                }
                return;
            }
            NSError *jsonError;
            NSDictionary *responseJSON = [NSJSONSerialization JSONObjectWithData:data options:0 error:&jsonError];
            if (!responseJSON)
            {
                RMStoreLog(@"Failed To Parse Server Response");
                if (failureBlock != nil)
                {
                    failureBlock(jsonError);
                }
            }
            
            static NSString *statusKey = @"status";
            NSInteger statusCode = [responseJSON[statusKey] integerValue];
            
            static NSInteger successCode = 0;
            static NSInteger sandboxCode = 21007;
            if (statusCode == successCode)
            {
                if (successBlock != nil)
                {
                    successBlock();
                }
            }
            else if (statusCode == sandboxCode)
            {
                RMStoreLog(@"Verifying Sandbox Receipt");
                // From: https://developer.apple.com/library/ios/#technotes/tn2259/_index.html
                // See also: http://stackoverflow.com/questions/9677193/ios-storekit-can-i-detect-when-im-in-the-sandbox
                // Always verify your receipt first with the production URL; proceed to verify with the sandbox URL if you receive a 21007 status code. Following this approach ensures that you do not have to switch between URLs while your application is being tested or reviewed in the sandbox or is live in the App Store.
                
                static NSString *sandboxURL = @"https://sandbox.itunes.apple.com/verifyReceipt";
                [self verifyRequestData:requestData url:sandboxURL success:successBlock failure:failureBlock];
            }
            else
            {
                RMStoreLog(@"Verification Failed With Code %ld", (long)statusCode);
                NSError *serverError = [NSError errorWithDomain:RMStoreErrorDomain code:statusCode userInfo:nil];
                if (failureBlock != nil)
                {
                    failureBlock(serverError);
                }
            }
        });
    });
}

@end
