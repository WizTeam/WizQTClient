#ifndef WIZWELCOMEDIALOG_H
#define WIZWELCOMEDIALOG_H

#include <QHash>
#include <QDialog>
#include "share/wizverifyaccount.h"
#include "share/wizcreateaccount.h"
#include "share/wizsettings.h"

class QAbstractButton;
class QUrl;

namespace Ui {
    class WelcomeDialog;
}

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(const QString& strDefaultUserId, const QString& strLocale, QWidget* parent = 0);

    void setUsers();
    void setPassword(const QString& strPassword);

    QString userId() const;
    QString password() const;

    ~WelcomeDialog();

private:
    Ui::WelcomeDialog* ui;
    QString m_strDefaultUserId;
    QHash<QString, QString> m_users;
    CWizVerifyAccount m_verifyAccount;

    void getUserPasswordPairs();
    void updateUserSettings();
    void enableControls(bool bEnable);
    void setUser(const QString &userId);

public Q_SLOTS:
    virtual void accept();

    void verifyAccountDone(bool succeeded, int errorCode, const QString& errorMessage);
    void on_webView_linkClicked(const QUrl& url);

    void on_labelForgotPassword_linkActivated(const QString&);
    void on_labelRegisterNew_linkActivated(const QString&);
    void on_labelProxySettings_linkActivated(const QString& link);

    void on_comboUsers_activated(const QString& userId);
    void on_comboUsers_editTextChanged(const QString& strText);
    void on_checkAutoLogin_stateChanged(int state);
};

#endif // WIZWELCOMEDIALOG_H
