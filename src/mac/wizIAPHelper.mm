#include "wizIAPHelper.h"
#include "wizmachelper_mm.h"


#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

#define kProductsLoadedNotification         @"ProductsLoaded"
#define kProductPurchasedNotification       @"ProductPurchased"
#define kProductPurchaseFailedNotification  @"ProductPurchaseFailed"

@interface IAPHelper : NSObject <SKProductsRequestDelegate, SKPaymentTransactionObserver> {
    NSSet * _productIdentifiers;
    NSArray * _products;
    NSMutableSet * _purchasedProducts;
    SKProductsRequest * _request;
    CWizIAPHelperPrivate *_container;
}

@property (retain) NSSet *productIdentifiers;
@property (retain) NSArray * products;
@property (retain) NSMutableSet *purchasedProducts;
@property (retain) SKProductsRequest *request;
@property CWizIAPHelperPrivate *container;

- (void)requestProducts;
- (id)initWithProductIdentifiers:(NSSet *)productIdentifiers;
- (void)buyProduct:(SKProduct *)product;
- (void)buyProductIdentifier:(NSString *)productIdentifier;
- (NSDictionary *) getStoreReceipt:(BOOL)sandbox;
- (NSDictionary *) getJsonDictionaryWithPostFromUrlString:(NSString *)urlString andDataString:(NSString *)dataString;
- (NSDictionary *) getDictionaryFromJsonString:(NSString *)jsonstring;
- (NSString *) getStringWithPostFromUrlString:(NSString *)urlString andDataString:(NSString *)dataString;
- (NSString*)base64forData:(NSData*)theData;
@end


    class CWizIAPHelperPrivate
    {
    public:
        CWizIAPHelperPrivate(CWizIAPHelper* container);
        ~CWizIAPHelperPrivate();

        void requestProducts();
        void onProductsLoaded(const QList<CWizIAPProduct>& productList);
        void onPurchaseFinished(bool ok, const QString& receipt);
        void purchaseProduct(const QString& strID);

    private:
        IAPHelper* m_helper;
        CWizIAPHelper* m_container;
    };



@implementation IAPHelper
@synthesize productIdentifiers = _productIdentifiers;
@synthesize products = _products;
@synthesize purchasedProducts = _purchasedProducts;
@synthesize request = _request;

- (id)initWithProductIdentifiers:(NSSet *)productIdentifiers {
    if ((self = [super init])) {

        // Store product identifiers
        _productIdentifiers = [productIdentifiers retain];

        // Check for previously purchased products
//        NSMutableSet * purchasedProducts = [NSMutableSet set];
//        for (NSString * productIdentifier in _productIdentifiers) {
//            BOOL productPurchased = [[NSUserDefaults standardUserDefaults] boolForKey:productIdentifier];
//            if (productPurchased) {
//                [purchasedProducts addObject:productIdentifier];
//                NSLog(@"Previously purchased: %@", productIdentifier);
//            }
//            NSLog(@"Not purchased: %@", productIdentifier);
//        }
//        self.purchasedProducts = purchasedProducts;

    }

    return self;
}

- (void)requestProducts {

    self.request = [[[SKProductsRequest alloc] initWithProductIdentifiers:_productIdentifiers] autorelease];
     [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
    _request.delegate = self;
    [_request start];

}

- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response {

    NSLog(@"Received products results...");
    self.products = response.products;
    self.request = nil;

    //[[NSNotificationCenter defaultCenter] postNotificationName:kProductsLoadedNotification object:_products];
    NSLog(@"products count : %lu", [self.products count]);

    QList<CWizIAPProduct> productList;
    for (SKProduct* product in self.products)
    {
        NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
        [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
        [numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
        [numberFormatter setLocale:product.priceLocale];
        NSString *formattedPrice = [numberFormatter stringFromNumber:product.price];
        NSLog(@"get a product , price : %@ , title : %@ , description : %@ , id : %@",
              formattedPrice, product.localizedTitle, product.localizedDescription, product.productIdentifier);

//        if ([product.productIdentifier isEqual : @"cn.wiz.wiznote.mac.pro.monthly"])
//        {
//            [self buyProductIdentifier:product];
//        }
        CWizIAPProduct iapProduct;
        iapProduct.id = WizToQString(product.productIdentifier);
        iapProduct.localizedTitle = WizToQString(product.localizedTitle);
        iapProduct.localizedPrice = WizToQString(formattedPrice);
        iapProduct.localizedDescription = WizToQString(product.localizedDescription);
        productList.append(iapProduct);
    }

    self.container->onProductsLoaded(productList);
}

- (void)recordTransaction:(SKPaymentTransaction *)transaction {
    // TODO: Record the transaction on the server side...
}

- (void)provideContent:(NSString *)productIdentifier {

    NSLog(@"Toggling flag for: %@", productIdentifier);
    [[NSUserDefaults standardUserDefaults] setBool:TRUE forKey:productIdentifier];
    [[NSUserDefaults standardUserDefaults] synchronize];
    [_purchasedProducts addObject:productIdentifier];

    [[NSNotificationCenter defaultCenter] postNotificationName:kProductPurchasedNotification object:productIdentifier];

}

    - (NSDictionary *) getStoreReceipt:(BOOL)sandbox {

        NSArray *objects;
        NSArray *keys;
        NSDictionary *dictionary;

        BOOL gotreceipt = false;

        @try {

            NSURL *receiptUrl = [[NSBundle mainBundle] appStoreReceiptURL];

            if ([[NSFileManager defaultManager] fileExistsAtPath:[receiptUrl path]]) {

                NSData *receiptData = [NSData dataWithContentsOfURL:receiptUrl];

                NSString *receiptString = [self base64forData:receiptData];

                if (receiptString != nil) {

                    objects = [[NSArray alloc] initWithObjects:receiptString, nil];
                    keys = [[NSArray alloc] initWithObjects:@"receipt-data", nil];
                    dictionary = [[NSDictionary alloc] initWithObjects:objects forKeys:keys];

//                    NSString *postData = [self getJsonStringFromDictionary:dictionary];
                    NSError *error = nil;
                    NSData *postData = [NSJSONSerialization dataWithJSONObject:dictionary options:NSJSONWritingPrettyPrinted error:&error];
                    NSString *postString = @"";
                    if (! postData)
                    {
                        NSLog(@"Got an error: %@", error);
                    }
                    else
                    {
                        postString = [[NSString alloc] initWithData:postData encoding:NSUTF8StringEncoding];
                    }

                    NSString *urlSting = @"https://buy.itunes.apple.com/verifyReceipt";
                    if (sandbox) urlSting = @"https://sandbox.itunes.apple.com/verifyReceipt";

                    dictionary = [self getJsonDictionaryWithPostFromUrlString:urlSting andDataString:postString];

                    if ([dictionary objectForKey:@"status"] != nil) {

                        if ([[dictionary objectForKey:@"status"] intValue] == 0) {

                            gotreceipt = true;

                        }
                    }

                }

            }

        } @catch (NSException * e) {
            gotreceipt = false;
        }

        if (!gotreceipt) {
            objects = [[NSArray alloc] initWithObjects:@"-1", nil];
            keys = [[NSArray alloc] initWithObjects:@"status", nil];
            dictionary = [[NSDictionary alloc] initWithObjects:objects forKeys:keys];
        }

        return dictionary;
    }



    - (NSDictionary *) getJsonDictionaryWithPostFromUrlString:(NSString *)urlString andDataString:(NSString *)dataString {
        NSString *jsonString = [self getStringWithPostFromUrlString:urlString andDataString:dataString];
        NSLog(@"%@", jsonString); // see what the response looks like
        return [self getDictionaryFromJsonString:jsonString];
    }


    - (NSDictionary *) getDictionaryFromJsonString:(NSString *)jsonstring {
        NSError *jsonError;
        NSDictionary *dictionary = (NSDictionary *) [NSJSONSerialization JSONObjectWithData:[jsonstring dataUsingEncoding:NSUTF8StringEncoding] options:0 error:&jsonError];
        if (jsonError) {
           dictionary = [[NSDictionary alloc] init];
        }
        return dictionary;
    }


    - (NSString *) getStringWithPostFromUrlString:(NSString *)urlString andDataString:(NSString *)dataString {
        NSString *s = @"";
        @try {
            NSData *postdata = [dataString dataUsingEncoding:NSASCIIStringEncoding allowLossyConversion:YES];
            NSString *postlength = [NSString stringWithFormat:@"%d", [postdata length]];
            NSMutableURLRequest *request = [[NSMutableURLRequest alloc] init];
            [request setURL:[NSURL URLWithString:urlString]];
                [request setTimeoutInterval:60];
            [request setHTTPMethod:@"POST"];
            [request setValue:postlength forHTTPHeaderField:@"Content-Length"];
            [request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
            [request setHTTPBody:postdata];
            NSData *data = [NSURLConnection sendSynchronousRequest:request returningResponse:nil error:nil];
            if (data != nil) {
                s = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            }
        }
        @catch (NSException *exception) {
            s = @"";
        }
        return s;
    }


    // from http://stackoverflow.com/questions/2197362/converting-nsdata-to-base64
    - (NSString*)base64forData:(NSData*)theData {
        const uint8_t* input = (const uint8_t*)[theData bytes];
        NSInteger length = [theData length];
        static char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
        NSMutableData* data = [NSMutableData dataWithLength:((length + 2) / 3) * 4];
        uint8_t* output = (uint8_t*)data.mutableBytes;
        NSInteger i;
        for (i=0; i < length; i += 3) {
            NSInteger value = 0;
            NSInteger j;
            for (j = i; j < (i + 3); j++) {
                value <<= 8;

                if (j < length) {
                    value |= (0xFF & input[j]);
                }
            }
            NSInteger theIndex = (i / 3) * 4;
            output[theIndex + 0] =                    table[(value >> 18) & 0x3F];
            output[theIndex + 1] =                    table[(value >> 12) & 0x3F];
            output[theIndex + 2] = (i + 1) < length ? table[(value >> 6)  & 0x3F] : '=';
            output[theIndex + 3] = (i + 2) < length ? table[(value >> 0)  & 0x3F] : '=';
        }
        return [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
    }

- (void)completeTransaction:(SKPaymentTransaction *)transaction {

    NSLog(@"completeTransaction...");

    [self recordTransaction: transaction];
    [self provideContent: transaction.payment.productIdentifier];
    [[SKPaymentQueue defaultQueue] finishTransaction: transaction];


//    NSURL *receiptURL = [[NSBundle mainBundle] appStoreReceiptURL];
//    NSData *receiptData = [NSData dataWithContentsOfURL:receiptURL];
//    NSString *encodeStr = [receiptData base64EncodedStringWithOptions:NSDataBase64EncodingEndLineWithLineFeed];
//    NSString *aString = [transaction.transactionReceipt base64EncodedStringWithOptions:NSDataBase64EncodingEndLineWithLineFeed];

//    NSError *error = nil;
    NSDictionary *receiptDict = [self getStoreReceipt:true];//[receipt dictionaryFromPlistData:&error];
    NSLog(@"receiptDict Info: %@", receiptDict);
    NSString *aString= [receiptDict description];
//    NSString *transactionPurchaseInfo = [receiptDict objectForKey:@"purchase-info"];
//    NSLog(@"Purchase Info: %@", transactionPurchaseInfo);
//    NSData* decodedPurchaseData = [NSData dataFromBase64String:transactionPurchaseInfo];
//    NSString *decodedPurchaseInfo =[[NSString alloc] initWithData:decodedPurchaseData encoding:NSUTF8StringEncoding];
//    NSDictionary *purchaseInfoDict = [[decodedPurchaseInfo dataUsingEncoding:NSUTF8StringEncoding] dictionaryFromPlistData:&error];

//    NSString *transactionID = [purchaseInfoDict objectForKey:@"transaction-id"];
//    NSString *purchaseDateString = [purchaseInfoDict objectForKey:@"purchase-date"];
//    NSString *signature = [receiptDict objectForKey:@"signature"];
//    NSString *signatureDecoded = [NSString stringWithUTF8String:[[NSData dataFromBase64String:signature] bytes]];

    // Convert the string into a date
//    NSDateFormatter *dateFormat = [[NSDateFormatter alloc] init];
//    [dateFormat setDateFormat:@"yyyy-MM-dd HH:mm:ss z"];

//    NSDate *purchaseDate = [dateFormat dateFromString:[purchaseDateString stringByReplacingOccurrencesOfString:@"Etc/" withString:@""]];

//    NSLog(@"Raw receipt content: \n%@", [NSString stringWithUTF8String:[receipt bytes]]);
//    NSLog(@"Purchase Info: %@", purchaseInfoDict);
//    NSLog(@"Transaction ID: %@", transactionID);
//    NSLog(@"Purchase Date: %@", purchaseDate);
//    NSLog(@"Signature: %@", signatureDecoded);


//    NSData* receiptData = [NSData dataWithContentsOfURL:[[NSBundle mainBundle] appStoreReceiptURL]];
//    NSString *aString = [receiptData base64EncodedStringWithOptions:NSDataBase64EncodingEndLineWithLineFeed];
    NSLog(@"recipt : %@", aString);
    QString strReceipt = WizToQString(aString);
    self.container->onPurchaseFinished(true, strReceipt);
}

- (void)restoreTransaction:(SKPaymentTransaction *)transaction {

    NSLog(@"restoreTransaction...");

    [self recordTransaction: transaction];
    [self provideContent: transaction.originalTransaction.payment.productIdentifier];
    [[SKPaymentQueue defaultQueue] finishTransaction: transaction];

}

- (void)failedTransaction:(SKPaymentTransaction *)transaction {

    if (transaction.error.code != SKErrorPaymentCancelled)
    {
        NSLog(@"Transaction error: %@", transaction.error.localizedDescription);
    }

    [[NSNotificationCenter defaultCenter] postNotificationName:kProductPurchaseFailedNotification object:transaction];

    [[SKPaymentQueue defaultQueue] finishTransaction: transaction];

    self.container->onPurchaseFinished(false, "");
}

- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions
{
    NSLog(@"paymentQueue called ");
    for (SKPaymentTransaction *transaction in transactions)
    {
        switch (transaction.transactionState)
        {
            case SKPaymentTransactionStatePurchased:
                [self completeTransaction:transaction];
                break;
            case SKPaymentTransactionStateFailed:
                [self failedTransaction:transaction];
                break;
            case SKPaymentTransactionStateRestored:
                [self restoreTransaction:transaction];
            default:
                break;
        }
    }
}

- (void)buyProduct:(SKProduct *)product {

    NSLog(@"Buying %@...", product);

    SKPayment *payment = [SKPayment paymentWithProduct:product];
    [[SKPaymentQueue defaultQueue] addPayment:payment];

}

- (void)buyProductIdentifier:(NSString *)productIdentifier {
        for (SKProduct* product in self.products)
        {
            if ([product.productIdentifier isEqual : productIdentifier])
            {
                [self buyProduct:product];
                break;
            }
        }
}


- (void)dealloc
{
    [_productIdentifiers release];
    _productIdentifiers = nil;
    [_products release];
    _products = nil;
    [_purchasedProducts release];
    _purchasedProducts = nil;
    [_request release];
    _request = nil;
    [super dealloc];
}

@end

CWizIAPHelperPrivate::CWizIAPHelperPrivate(CWizIAPHelper* container)
{
    NSSet *idSet = [NSSet setWithObjects : @"cn.wiz.wiznote.mac.pro.monthly", @"cn.wiz.wiznote.mac.pro.yearly", nil];
    m_helper = [[IAPHelper alloc] initWithProductIdentifiers : idSet];
    m_helper.container = this;
    m_container = container;
}

CWizIAPHelperPrivate::~CWizIAPHelperPrivate()
{
    [m_helper dealloc];
}

void CWizIAPHelperPrivate::requestProducts()
{
    [m_helper requestProducts];
}

void CWizIAPHelperPrivate::onProductsLoaded(const QList<CWizIAPProduct>& productList)
{
    m_container->onProductsLoaded(productList);
}

void CWizIAPHelperPrivate::onPurchaseFinished(bool ok, const QString& receipt)
{
    m_container->onPurchaseFinished(ok, receipt);
}

void CWizIAPHelperPrivate::purchaseProduct(const QString& strID)
{
    NSString* productIdentifier = WizToNSString(strID);
    [m_helper buyProductIdentifier : productIdentifier];
}

CWizIAPHelper::CWizIAPHelper(CWizIAPCaller* caller)
    : m_helper(new CWizIAPHelperPrivate(this))
    , m_caller(caller)
{
}

CWizIAPHelper::~CWizIAPHelper()
{
    delete m_helper;
}

void CWizIAPHelper::purchaseProduct(const QString& strID)
{
    m_helper->purchaseProduct(strID);
}

void CWizIAPHelper::requestProducts()
{
    m_helper->requestProducts();
}

void CWizIAPHelper::onProductsLoaded(const QList<CWizIAPProduct>& productList)
{
    m_caller->onProductsLoaded(productList);
}

void CWizIAPHelper::onPurchaseFinished(bool ok, const QString& receipt)
{
    m_caller->onPurchaseFinished(ok, receipt);
}
