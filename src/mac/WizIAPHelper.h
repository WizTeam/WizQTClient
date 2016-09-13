#ifndef CWIZIPAMANAGER_H
#define CWIZIPAMANAGER_H

#include <QList>
#include <QString>

class CWizIAPHelperPrivate;

struct CWizIAPProduct
{
    QString id;
    QString localizedTitle;
    QString localizedPrice;
    QString localizedDescription;
};

class WizIAPCaller
{
public:
    virtual void onProductsLoaded(const QList<CWizIAPProduct>& productList) = 0;
    virtual void onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID) = 0;
};

class WizIAPHelper
{
public:
    WizIAPHelper();
    WizIAPHelper(WizIAPCaller* caller);
    ~WizIAPHelper();

    void purchaseProduct(const QString& strID);
    void requestProducts(const QList<QString>& productIdList);
    void loadLocalReceipt(QByteArray& receipt);
    void onProductsLoaded(const QList<CWizIAPProduct>& productList);
    void onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID);
    void validteReceiptOnLauch();

protected:
    CWizIAPHelperPrivate* m_helper;
    WizIAPCaller* m_caller;
};

#endif // CWIZIPAMANAGER_H
