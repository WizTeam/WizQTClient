#ifndef WIZLOGINDIALOG_H
#define WIZLOGINDIALOG_H

#include <QDialog>

#include "share/wizsettings.h"

class QLabel;
class QComboBox;
class QLineEdit;
class QCheckBox;
//class CWizAvatarWidget;


class CWizLoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizLoginDialog(const QString& strDefaultUserId, const QString& strLocale, QWidget* parent = 0);

    QString userId() const;
    QString password() const;

private:
    QComboBox* m_comboUsers;
    QLineEdit* m_editPassword;
    QCheckBox* m_checkSavePassword;
    QCheckBox* m_checkAutoLogin;
    QPushButton* m_btnLogin;
    //CWizAvatarWidget* m_avatar;

    //void resetAvatar(const QString& strUserId);

    void setUsers(const QString& strDefault);
    void setUser(const QString& userId);

    void doAccountVerify();
    void doOnlineVerify();
    bool updateGlobalProfile();
    bool updateUserProfile(bool bLogined);
    void enableControls(bool bEnable);

public Q_SLOTS:
    virtual void accept();

    void onTokenAcquired(const QString& strToken);

    void on_labelRegister_linkActivated(const QString&);
    void on_labelForgot_linkActivated(const QString&);
    void on_labelNetwork_linkActivated(const QString& link);

    void on_comboUsers_indexChanged(const QString& strUserId);
    void on_comboUsers_editTextChanged(const QString& strText);
    void on_checkAutoLogin_stateChanged(int state);
};

#endif // WIZLOGINDIALOG_H
