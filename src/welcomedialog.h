#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>
#include "share/wizverifyaccount.h"

class QAbstractButton;

namespace Ui {
    class WelcomeDialog;
}

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget *parent = 0);
    ~WelcomeDialog();

private:
    Ui::WelcomeDialog *ui;
    //
    CWizVerifyAccount m_verifyAccount;
    //
    void enableControls(bool b);
public:
    void setUserId(const QString& strUserId);
    QString userId() const;
    QString password() const;
    //
public Q_SLOTS:
    virtual void accept();

private slots:
    void verifyAccountDone(bool error, const CString& errorMessage);
};

#endif // WELCOMEDIALOG_H
