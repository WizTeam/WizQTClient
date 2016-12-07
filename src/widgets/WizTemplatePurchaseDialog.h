#ifndef WIZTEMPLATEPURCHASEDIALOG_H
#define WIZTEMPLATEPURCHASEDIALOG_H

#include <QDialog>
#include <QTimer>

#ifdef Q_OS_MAC

#include "mac/WizIAPHelper.h"

class QNetworkReply;
class QNetworkAccessManager;

namespace Ui {
class WizTemplatePurchaseDialog;
}

class WizTemplatePurchaseDialog : public QDialog, public WizIAPCaller
{
    Q_OBJECT

public:
    explicit WizTemplatePurchaseDialog(QWidget *parent = 0);
    ~WizTemplatePurchaseDialog();

    void showTemplateInfo(int tmplId, const QString& tmplName, const QString& thumbUrl);

    virtual void onProductsLoaded(const QList<CWizIAPProduct>& productList);
    virtual void onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID);

    static QStringList getUnfinishedTransations();
    void processUnfinishedTransation();

signals:
    void purchaseSuccess();

private slots:
    void imageDownloadFinished(QNetworkReply* reply);

    void on_btn_purchase_clicked();

    void on_btn_cancel_clicked();

    void on_btn_quit_clicked();

    void checkReciptFinished(QNetworkReply* reply);

    void processIAPPurchaseResult(bool ok, const QByteArray& receipt, const QString& strTransationID);

    void onWaitingTimerOut();

private:
    void showStatusMeesage(const QString& text);
    void changeToStatusPage();
    void changeToTemplatePage();

    void checkRecipt(const QByteArray& receipt, const QString& strTransationID, int templateId);
    void parseCheckResult(const QString& strResult, const QString& strTransationID);
    //
    void saveUnfinishedTransation(const QString& strTransationID, int templateId);
    void removeTransationFromUnfinishedList(const QString& strTransationID);
    //
    void on_purchase_failed(const QString& error);
    void on_purchase_successed();

private:
    Ui::WizTemplatePurchaseDialog *ui;
    QNetworkAccessManager* m_net;
    int m_tmplId;
    WizIAPHelper* m_iapHelper;
    QString m_transationID;
    QTimer m_waitingTimer;
};

#endif

#endif // WIZTEMPLATEPURCHASEDIALOG_H
