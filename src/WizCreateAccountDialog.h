#ifndef WIZCREATEACCOUNTDIALOG_H
#define WIZCREATEACCOUNTDIALOG_H

#include <QDialog>

namespace Ui {
    class WizCreateAccountDialog;
}

class WizCreateAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizCreateAccountDialog(QWidget *parent = 0);
    ~WizCreateAccountDialog();

private:
    Ui::WizCreateAccountDialog *ui;

    void enableControls(bool b);
    QString password2() const;

public:
    QString userId() const;
    QString password() const;

public Q_SLOTS:
    virtual void accept();

    void onRegisterAccountFinished(bool bOk);
    //void accountRegistered_done(bool bOk);

Q_SIGNALS:
    void registerAccount(const QString& strUser, const QString& strPassword, const QString& strInviteCode);
};

#endif // WIZCREATEACCOUNTDIALOG_H
