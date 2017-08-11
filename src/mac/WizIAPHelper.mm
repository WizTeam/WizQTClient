#include "WizIAPHelper.h"
#include "WizMacHelper_mm.h"
#include <QByteArray>

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>
#import "rmstore/RMStoreAppReceiptVerificator.h"

#define kProductsLoadedNotification         @"ProductsLoaded"
#define kProductPurchasedNotification       @"ProductPurchased"
#define kProductPurchaseFailedNotification  @"ProductPurchaseFailed"




@interface IAPHelper : NSObject <SKProductsRequestDelegate, SKPaymentTransactionObserver> {
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

- (void)requestProducts:(NSMutableSet *)productIdentifiers;
- (void)buyProduct:(SKProduct *)product;
- (void)buyProductIdentifier:(NSString *)productIdentifier;
- (NSData *)loadLocalReceipt;
- (void)validteReceiptOnLauch;
@end


    class CWizIAPHelperPrivate
    {
    public:
        CWizIAPHelperPrivate(WizIAPHelper* container);
        ~CWizIAPHelperPrivate();

        void requestProducts(const QList<QString>& productIdList);
        void loadLocalReceipt(QByteArray& receipt);
        void onProductsLoaded(const QList<CWizIAPProduct>& productList);
        void onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID);
        void purchaseProduct(const QString& strID);
        void validteReceiptOnLauch();

    private:
        IAPHelper* m_helper;
        WizIAPHelper* m_container;
    };





@implementation IAPHelper
@synthesize products = _products;
@synthesize purchasedProducts = _purchasedProducts;
@synthesize request = _request;

- (void)requestProducts:(NSMutableSet *)productIdentifiers {
    NSLog(@"identifiers : %@", productIdentifiers);
    self.request = [[[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifiers] autorelease];
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


- (NSData *)loadLocalReceipt {
        NSURL *receiptURL = [[NSBundle mainBundle] appStoreReceiptURL];
        NSData *receipt = [NSData dataWithContentsOfURL:receiptURL];
        if (!receipt) {
            NSLog(@"can not load receipt!");
//            exit(173);
            return NULL;
        }

        return receipt;
}


- (void)completeTransaction:(SKPaymentTransaction *)transaction {

    NSLog(@"completeTransaction...");

    [self recordTransaction: transaction];
    [self provideContent: transaction.payment.productIdentifier];
    [[SKPaymentQueue defaultQueue] finishTransaction: transaction];


    NSURL *receiptURL = [[NSBundle mainBundle] appStoreReceiptURL];
    NSData *receipt = [NSData dataWithContentsOfURL:receiptURL];
    if (!receipt) {
        NSLog(@"can not load receipt!");
        return;
    }

    NSString* nsID = [transaction transactionIdentifier];
    QString strTransationID = WizToQString(nsID);
    QByteArray ba = QByteArray::fromNSData(receipt);
    self.container->onPurchaseFinished(true, ba, strTransationID);
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

    self.container->onPurchaseFinished(false, "", "");
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


//    NSURL *receiptURL = [[NSBundle mainBundle] appStoreReceiptURL];
//    NSData *receipt = [NSData dataWithContentsOfURL:receiptURL];
//    if (!receipt) {
//        NSLog(@"can not load receipt!");
//        return;
//    }

//    NSLog(@"orign data : %@", receipt);

//    NSString *aString = [[NSString alloc] initWithData:receipt encoding:NSUTF8StringEncoding];
//    NSLog(@"local receipt NSUTF8StringEncoding : %@", aString);

//    aString = [[NSString alloc] initWithData:receipt encoding:NSASCIIStringEncoding];
//    NSLog(@"local receipt NSASCIIStringEncoding : %@", aString);

//    aString = [[NSString alloc] initWithData:receipt encoding:NSNEXTSTEPStringEncoding];
//    NSLog(@"local receipt NSNEXTSTEPStringEncoding : %@", aString);

//    aString = [[NSString alloc] initWithData:receipt encoding:NSISOLatin1StringEncoding];
//    NSLog(@"local receipt NSISOLatin1StringEncoding : %@", aString);

//    aString = [[NSString alloc] initWithData:receipt encoding:NSUnicodeStringEncoding];
//    NSLog(@"local receipt NSUnicodeStringEncoding : %@", aString);

//    aString = [[NSString alloc] initWithData:receipt encoding:NSSymbolStringEncoding];
//    NSLog(@"local receipt NSSymbolStringEncoding : %@", aString);

////    QString strTransationID = WizToQString(nsID);
//    QByteArray ba = QByteArray::fromNSData(receipt);
//    self.container->onPurchaseFinished(true, ba, strTransationID);
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


- (void)validteReceiptOnLauch
{
#ifdef BUILD4APPSTORE
    RMStoreAppReceiptVerificator* verctor = [[[RMStoreAppReceiptVerificator alloc] init] autorelease];
    if (![verctor verifyAppReceipt])
    {
        NSLog(@"Valide receipt failed");
        //exit(173);
    }
#endif
}


- (void)dealloc
{
    [_products release];
    _products = nil;
    [_purchasedProducts release];
    _purchasedProducts = nil;
    [_request release];
    _request = nil;
    [super dealloc];
}

@end

CWizIAPHelperPrivate::CWizIAPHelperPrivate(WizIAPHelper* container)
{    
    m_helper = [[IAPHelper alloc] init];
    m_helper.container = this;
    m_container = container;
}

CWizIAPHelperPrivate::~CWizIAPHelperPrivate()
{
    [m_helper dealloc];
}

void CWizIAPHelperPrivate::requestProducts(const QList<QString>& productIdList)
{
    NSMutableSet *idSet = [[[NSMutableSet alloc] init] autorelease];
    for (QString product : productIdList)
    {
        NSString* nsProduct = WizToNSString(product);
        [idSet addObject:nsProduct];
    }
    [m_helper requestProducts:idSet];
}

void CWizIAPHelperPrivate::loadLocalReceipt(QByteArray& receipt)
{
    NSData* nsReceipt = [m_helper loadLocalReceipt];
    receipt = QByteArray::fromNSData(nsReceipt);
}

void CWizIAPHelperPrivate::onProductsLoaded(const QList<CWizIAPProduct>& productList)
{
    m_container->onProductsLoaded(productList);
}

void CWizIAPHelperPrivate::onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID)
{
    m_container->onPurchaseFinished(ok, receipt, strTransationID);
}

void CWizIAPHelperPrivate::purchaseProduct(const QString& strID)
{
    NSString* productIdentifier = WizToNSString(strID);
    [m_helper buyProductIdentifier : productIdentifier];
}

void CWizIAPHelperPrivate::validteReceiptOnLauch()
{
    [m_helper validteReceiptOnLauch];
}

WizIAPHelper::WizIAPHelper()
    : m_helper(new CWizIAPHelperPrivate(this))
    , m_caller(0)
{

}

WizIAPHelper::WizIAPHelper(WizIAPCaller* caller)
    : m_helper(new CWizIAPHelperPrivate(this))
    , m_caller(caller)
{
}

WizIAPHelper::~WizIAPHelper()
{
    delete m_helper;
}

void WizIAPHelper::purchaseProduct(const QString& strID)
{
    m_helper->purchaseProduct(strID);
}

void WizIAPHelper::requestProducts(const QList<QString>& productIdList)
{
    m_helper->requestProducts(productIdList);
}

void WizIAPHelper::loadLocalReceipt(QByteArray& receipt)
{
    m_helper->loadLocalReceipt(receipt);
}

void WizIAPHelper::onProductsLoaded(const QList<CWizIAPProduct>& productList)
{
    m_caller->onProductsLoaded(productList);
}

void WizIAPHelper::onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID)
{
    m_caller->onPurchaseFinished(ok, receipt, strTransationID);
}

void WizIAPHelper::validteReceiptOnLauch()
{
    m_helper->validteReceiptOnLauch();
}
