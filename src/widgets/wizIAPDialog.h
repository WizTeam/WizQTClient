#ifndef WIZIAPDIALOG_H
#define WIZIAPDIALOG_H

#ifdef Q_OS_MAC

#include <QDialog>
#include <QTimer>
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

private slots:
    void on_btn_goBack_clicked();

    void on_btn_month_clicked();

    void on_btn_year_clicked();

    void loadProducts();

    //
    void onWaitingTimeOut();
    void onEditorPopulateJavaScriptWindowObject();

private:
    void initStyles();
    void createIAPHelper();
    void setPurchaseAvailable(bool b);
    void hideInfoLabel();
    void checkReceiptInfo(const QByteArray& receipt, const QString& strTransationID);

private:
    Ui::CWizIAPDialog *ui;
    CWizIAPHelper* m_iAPhelper;
    QMessageBox* m_waitingMsgBox;
    QTimer m_timer;
};
#endif

#endif // WIZIAPDIALOG_H
