#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

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
    explicit WelcomeDialog(const QString &strDefaultUserId, QWidget *parent = 0);

    void setUsers();
    void setPassword(const QString &strPassword);

    QString userId() const;
    QString password() const;

    ~WelcomeDialog();

private:
    Ui::WelcomeDialog *ui;
    QString m_strDefaultUserId;
    QHash<QString, QString> m_users;
    CWizVerifyAccount m_verifyAccount;

    void getUserPasswordPairs();
    void updateConfig();
    void enableControls(bool bEnable);

public Q_SLOTS:
    virtual void accept();

    void verifyAccountDone(bool succeeded, const CString &errorMessage);
    void on_web_linkClicked(const QUrl &url);
    void on_labelProxy_linkActivated(const QString &link);

    void on_comboUsers_indexChanged(const QString &userId);
    void on_autoLogin_stateChanged(int state);
};

#endif // WELCOMEDIALOG_H
