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
    virtual void onPurchaseFinished(bool ok, const QString& receipt) = 0;
};

class CWizIAPHelper
{
public:
    CWizIAPHelper(CWizIAPCaller* caller);
    ~CWizIAPHelper();

    void purchaseProduct(const QString& strID);
    void requestProducts();
    void onProductsLoaded(const QList<CWizIAPProduct>& productList);
    void onPurchaseFinished(bool ok, const QString& receipt);

private:
    CWizIAPHelperPrivate* m_helper;
    CWizIAPCaller* m_caller;
};

#endif // CWIZIPAMANAGER_H
