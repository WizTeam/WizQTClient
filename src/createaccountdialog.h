#ifndef CREATEACCOUNTDIALOG_H
#define CREATEACCOUNTDIALOG_H

#include <QDialog>
#include "share/wizcreateaccount.h"

namespace Ui {
    class CreateAccountDialog;
}

class CreateAccountDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CreateAccountDialog(QWidget *parent = 0);
    ~CreateAccountDialog();

private:
    Ui::CreateAccountDialog *ui;
    //
    CWizCreateAccount m_createAccount;
    //
    void enableControls(bool b);
    QString password2() const;
public:
    QString userId() const;
    QString password() const;
public Q_SLOTS:
    virtual void accept();

private slots:
    void createAccountDone(bool succeeded, const CString& errorMessage);
};

#endif // CREATEACCOUNTDIALOG_H
