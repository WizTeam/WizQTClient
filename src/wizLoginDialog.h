#ifndef WIZLOGINWIDGET_H
#define WIZLOGINWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

#ifndef Q_OS_MAC
#include "share/wizshadowwindow.h"
#endif

class QLabel;
class CWizSkin9GridImage;
class CWizImageButton;



namespace Ui {
class wizLoginWidget;
}

class CWizLoginDialog
#ifdef Q_OS_MAC
        : public QDialog
#else
        : public CWizShadowWindow<QDialog>
#endif
{
    Q_OBJECT

public:
    explicit CWizLoginDialog(const QString& strDefaultUserId, const QString& strLocale, QWidget *parent = 0);
    ~CWizLoginDialog();

    QString userId() const;
    QString password() const;

    void setUsers(const QString& strDefault);
    void setUser(const QString& strUserId);

    void doAccountVerify();
    void doOnlineVerify();
    bool updateGlobalProfile();
    bool updateUserProfile(bool bLogined);
    void enableLoginControls(bool bEnable);
    void enableSignInControls(bool bEnable);

#ifdef Q_OS_MAC
private:
    QPoint m_mousePoint;
protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
#endif
private slots:
    void on_btn_close_clicked();
    void on_btn_changeToSignin_clicked();
    void on_btn_changeToLogin_clicked();
    void on_btn_proxysetting_clicked();
    void on_btn_fogetpass_clicked();
    void on_btn_login_clicked();
    void on_btn_singin_clicked();


    void onLoginInputChanged();
    void onSignUpInputDataChanged();
    void userListMenuClicked(QAction* action);
    void showUserListMenu();

    void onTokenAcquired(const QString& strToken);
    void onRegisterAccountFinished(bool bFinish);

    void on_cbx_remberPassword_toggled(bool checked);
    void on_cbx_autologin_toggled(bool checked);

    void onUserNameEdited(const QString& arg1);

private:
    void applyElementStyles();
    bool checkSingMessage();
    QAction* findActionInMenu(const QString& strActName);

private:
    Ui::wizLoginWidget *ui;
    QMenu* m_menu;

    QLineEdit* m_lineEditUserName;
    QLineEdit* m_lineEditPassword;
    CWizImageButton* m_buttonLogin;
    QLineEdit* m_lineEditNewUserName;
    QLineEdit* m_lineEditNewPassword;
    QLineEdit* m_lineEditRepeatPassword;
    CWizImageButton* m_buttonSignIn;
};

#endif // WIZLOGINWIDGET_H
