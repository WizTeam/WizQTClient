#ifndef WIZIAPDIALOG_H
#define WIZIAPDIALOG_H

#include <QDialog>
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
    virtual void onPurchaseFinished(bool ok, const QString& receipt);

private slots:
    void on_btn_goBack_clicked();

    void on_btn_month_clicked();

    void on_btn_year_clicked();

    void loadProducts();


private:
    void initStyles();
    void createIAPHelper();
    void setPurchaseAvailable(bool b);
    void hideInfoLabel();
    void checkReceiptInfo(const QString& receipt);

private:
    Ui::CWizIAPDialog *ui;
    CWizIAPHelper* m_iAPhelper;
    QMessageBox* m_waitingMsgBox;
};

#endif // WIZIAPDIALOG_H
