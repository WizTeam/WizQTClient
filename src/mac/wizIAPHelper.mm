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



- (void)completeTransaction:(SKPaymentTransaction *)transaction {

    NSLog(@"completeTransaction...");

    [self recordTransaction: transaction];
    [self provideContent: transaction.payment.productIdentifier];
    [[SKPaymentQueue defaultQueue] finishTransaction: transaction];

    NSData* reciptData = [NSData dataWithContentsOfURL:[[NSBundle mainBundle] appStoreReceiptURL]];
    NSString *aString = [[NSString alloc] initWithData:reciptData encoding:NSUTF8StringEncoding];
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
    NSLog(@"on product loaded");
    m_caller->onProductsLoaded(productList);
}

void CWizIAPHelper::onPurchaseFinished(bool ok, const QString& receipt)
{
//    NSLog(@"on product loaded");
    m_caller->onPurchaseFinished(ok, receipt);
}
