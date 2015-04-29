#ifndef WIZLOGINWIDGET_H
#define WIZLOGINWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include "share/wizsettings.h"

#ifndef Q_OS_MAC
#include "share/wizshadowwindow.h"
#endif

class QLabel;
class CWizSkin9GridImage;
class CWizImageButton;
class CWizUdpClient;


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
    WizServerType serverType() const;

    void setUsers(const QString& strDefault);
    void setUser(const QString& strUserId);

    void doAccountVerify();
    void doOnlineVerify();
    bool updateGlobalProfile();
    bool updateUserProfile(bool bLogined);
    void enableLoginControls(bool bEnable);
    void enableSignInControls(bool bEnable);

signals:
    void snsLoginSuccess(const QString& strUrl);
    void wizBoxSearchRequest(int port, QString message);

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
    void on_btn_snsLogin_clicked();
    void on_btn_singUp_clicked();


    void onLoginInputChanged();
    void onSignUpInputDataChanged();
    void userListMenuClicked(QAction* action);
    void serverListMenuClicked(QAction* action);
    void showUserListMenu();
    void showServerListMenu();

    void onTokenAcquired(const QString& strToken);
    void onRegisterAccountFinished(bool bFinish);

    void on_cbx_remberPassword_toggled(bool checked);
    void on_cbx_autologin_toggled(bool checked);

    void onUserNameEdited(const QString& arg1);

    void onSNSPageUrlChanged(const QUrl& url);
    void onSNSLoginSuccess(const QString& strUrl);

    void onWizBoxResponse(const QString& boardAddress, const QString& serverAddress,
                          const QString& responseMessage);
    void onWizBoxSearchingTimeOut();

private:
    void applyElementStyles(const QString& strLocal);
    bool checkSingMessage();
    QAction* findActionInMenu(const QString& strActName);
    bool doVerificationCodeCheck(QString& strCaptchaID, QString& strCaptcha);
    //
    void findWizBoxServer();
    void initSearchingDialog();
    void showSearchingDialog();
    void startWizBoxUdpClient();
    void closeWizBoxUdpClient();
    bool checkServerLicence(const QString& strOldLicence);

    class ControlWidgetsLocker
    {
    public:
        ControlWidgetsLocker(){}
        ~ControlWidgetsLocker() {
            releaseWidgets();
        }

        void releaseWidgets() {
            foreach (QWidget* wgt, m_widgetList) {
                wgt->setEnabled(true);
            }
            m_widgetList.clear();
        }

        void lockWidget(QWidget* wgt) {
            m_widgetList.append(wgt);
            wgt->setEnabled(false);
        }

    private:
        QList<QWidget*> m_widgetList;
    };

private:
    Ui::wizLoginWidget *ui;
    QMenu* m_menuUsers;
    QMenu* m_menuServers;
    QDialog* m_searchingDialog;
    CWizUdpClient* m_udpClient;
    QThread* m_udpThread;
    WizServerType m_serverType;
    WizServerType m_currentUserServerType;
    QString m_serverLicence;
    QTimer m_wizBoxSearchingTimer;

    QLineEdit* m_lineEditUserName;
    QLineEdit* m_lineEditPassword;
    QLineEdit* m_lineEditServer;
    CWizImageButton* m_buttonLogin;
    QLineEdit* m_lineEditNewUserName;
    QLineEdit* m_lineEditNewPassword;
    QLineEdit* m_lineEditRepeatPassword;
    CWizImageButton* m_buttonSignUp;
};

#endif // WIZLOGINWIDGET_H
