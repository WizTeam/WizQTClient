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

class CWizIAPCaller
{
public:
    virtual void onProductsLoaded(const QList<CWizIAPProduct>& productList) = 0;
    virtual void onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID) = 0;
};

class CWizIAPHelper
{
public:
    CWizIAPHelper();
    CWizIAPHelper(CWizIAPCaller* caller);
    ~CWizIAPHelper();

    void purchaseProduct(const QString& strID);
    void requestProducts();
    void loadLocalReceipt(QByteArray& receipt);
    void onProductsLoaded(const QList<CWizIAPProduct>& productList);
    void onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID);
    void validteReceiptOnLauch();

protected:
    CWizIAPHelperPrivate* m_helper;
    CWizIAPCaller* m_caller;
};

#endif // CWIZIPAMANAGER_H
