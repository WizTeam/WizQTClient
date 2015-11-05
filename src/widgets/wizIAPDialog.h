#ifndef WIZIAPDIALOG_H
#define WIZIAPDIALOG_H


#include <QDialog>
#include <QTimer>
#if defined Q_OS_MAC
#include "mac/wizIAPHelper.h"

namespace Ui {
class CWizIAPDialog;
}

class QMessageBox;
class CWizIAPHelper;
class CWizIAPDialog : public QDialog, public CWizIAPCaller
{
    Q_OBJECT

public:
    explicit CWizIAPDialog(QWidget *parent = 0);
    ~CWizIAPDialog();

    virtual void onProductsLoaded(const QList<CWizIAPProduct>& productList);
    virtual void onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID);

    void loadUserInfo();
    void loadIAPPage();

public slots:
    virtual int exec();

signals:
    void checkReceiptRequest(const QByteArray receipt, const QString strTransationID);

private slots:
    void on_btn_goBack_clicked();

    void on_btn_month_clicked();

    void on_btn_year_clicked();

    void loadProducts();

    void stopWaitTimer();

    //
    void onWaitingTimeOut();
    void onEditorPopulateJavaScriptWindowObject();

    void onCheckReceiptRequest(const QByteArray& receipt, const QString& strTransationID);

    void checkUnfinishedTransation();

    void on_purchase_successed();
    void on_purchase_failed(const QString& errorMsg);

private:
    void initStyles();
    void createIAPHelper();
    void setPurchaseAvailable(bool b);
    void hideInfoLabel();
    void checkReceiptInfo(const QByteArray& receipt, const QString& strTransationID);
    void parseCheckResult(const QString& strResult, const QString& strTransationID);

    //
    QStringList getUnfinishedTransations();
    void saveUnfinishedTransation(const QString& strTransationID);
    void removeTransationFromUnfinishedList(const QString& strTransationID);

private:
    Ui::CWizIAPDialog *ui;
    CWizIAPHelper* m_iAPhelper;
    QMessageBox* m_waitingMsgBox;
    QTimer m_timer;
};
#endif

#endif // WIZIAPDIALOG_H
