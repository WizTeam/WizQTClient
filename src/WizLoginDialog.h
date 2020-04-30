#ifndef WIZLOGINWIDGET_H
#define WIZLOGINWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QWidgetAction>
#include <QTimer>
#include "share/WizSettings.h"

#ifndef Q_OS_MAC
#include "share/WizShadowWindow.h"
#endif

class QLabel;
class QState;
class QHistoryState;
class WizSkin9GridImage;
class WizStyleButton;
class WizUdpClient;

class WizOEMDownloader : public QObject
{
    Q_OBJECT
public:
    WizOEMDownloader(QObject* parent);

    QString serverIp() const;
public slots:
    void setServerIp(const QString& ip);
    void downloadOEMLogo(const QString& strUrl);
    void downloadOEMSettings();
    void checkServerLicence(const QString& licence);

signals:
    void oemSettingsDownloaded(const QString& setting);
    void logoDownloaded(const QString& logoFile);
    void errorMessage(const QString& error);
    void checkLicenceFinished(bool result, const QString& settings);

private:
    QString _downloadOEMSettings();

    QString m_server;
};

class WizActionWidget : public QWidget
{
    Q_OBJECT
public:
    WizActionWidget(const QString& text, QWidget* parent);
    void setSelected(bool selected);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent * event);
    void leaveEvent(QEvent * event);
    void paintEvent(QPaintEvent*);

signals:
    void delButtonClicked();
    void widgetClicked();

private:
    bool m_mousePress;
    bool m_selected;
    QString m_text;
    QPushButton *m_deleteButton;
};

class WizUserItemAction : public QWidgetAction
{
    Q_OBJECT
public:
    explicit WizUserItemAction(const WizLocalUser& localUser, QMenu *parent);
    WizLocalUser getUserData();
    void setSelected(bool selected);

signals:
    void userDeleteRequest(const WizLocalUser& WizLocalUser);
    void userSelected(const WizLocalUser& WizLocalUser);

private slots:
    void on_delButtonClicked();
    void on_widgetClicked();

private:
    WizLocalUser m_userData;
    QMenu* m_menu;
    WizActionWidget* m_widget;
};


namespace Ui {
class wizLoginWidget;
}

class WizLoginDialog
#ifdef Q_OS_MAC
        : public QDialog
#else
        : public WizShadowWindow<QDialog>
#endif
{
    Q_OBJECT

public:
    explicit WizLoginDialog(const QString& strLocale, const QList<WizLocalUser>& localUsers, QWidget *parent = 0);
    ~WizLoginDialog();

    QString userId() const;
    QString loginUserGuid() const;
    QString password() const;
    QString serverIp() const;
    WizServerType serverType() const;

    void setUser(const QString& strUserGuid);

    void doAccountVerify();
    void doOnlineVerify();
    bool updateGlobalProfile();
    bool updateUserProfile(bool bLogined);
    void enableLoginControls(bool bEnable);
    void enableSignUpControls(bool bEnable);

    bool isNewRegisterAccount();

signals:
    void snsLoginSuccess(const QString& strUrl);
    void wizBoxSearchRequest(int port, QString message);
    void wizServerSelected();
    void wizBoxServerSelected();
    void accountCheckStart();
    void accountCheckFinished();
    void logoDownloadRequest(const QString& strUrl);
    void checkServerLicenceRequest(const QString& licence);

#ifdef Q_OS_MAC
protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
private:
    QPoint m_mousePoint;
//#else
//    void layoutTitleBar();
#endif
private slots:
    void on_btn_close_clicked();
    void on_btn_changeToSignin_clicked();
    void on_btn_changeToLogin_clicked();
    void on_btn_proxysetting_clicked();
    void on_btn_snsLogin_clicked();
    void on_btn_fogetpass_clicked();
    void on_btn_login_clicked();
    void on_btn_singUp_clicked();


    void onLoginInputChanged();
    void onSignUpInputDataChanged();
//    void userListMenuClicked(QAction* action);
    void serverListMenuClicked(QAction* action);
    void showUserListMenu();
    void showServerListMenu();

    void onTokenAcquired(const QString& strToken);
    void onRegisterAccountFinished(bool bFinish);

    void on_cbx_remberPassword_toggled(bool checked);
    void on_cbx_autologin_toggled(bool checked);

    void onUserNameEdited(const QString& arg1);
    void onDeleteUserRequest(const WizLocalUser& user);
    void onUserSelected(const WizLocalUser& user);

    void onSNSPageUrlChanged(const QUrl& url);
    void onSNSLoginSuccess(const QString& strUrl);

    void onWizBoxResponse(const QString& boardAddress, const QString& serverAddress,
                          const QString& responseMessage);
    void onWizBoxSearchingTimeOut();

    // download oem
    bool onOEMSettingsDownloaded(const QString& settings);
    void onOEMLogoDownloaded(const QString& logoFile);
    void showOEMErrorMessage(const QString& stterror);
    void onCheckServerLicenceFinished(bool result, const QString& settings);

    // state machine
    void onWizLogInStateEntered();
    void onWizBoxLogInStateEntered();
    void onWizSignUpStateEntered();
    void onLogInCheckStart();
    void onLogInCheckEnd();
    void onSignUpCheckStart();
    void onSignUpCheckEnd();
    void searchWizBoxServer();

    void resetUserList();    

private:
    void loadDefaultUser();    

    void initSateMachine();
    void initOEMDownloader();
    //
    void applyElementStyles(const QString& strLocal);
    bool checkSignMessage();
    QAction* findActionInMenu(const QString& strActData);
    bool doVerificationCodeCheck(QString& strCaptchaID, QString& strCaptcha);
    //
    void initAnimationWaitingDialog(const QString& text);
    int showAnimationWaitingDialog(const QString& text);
    void closeAnimationWaitingDialog();
    void showSearchingDialog();
    void startWizBoxUdpClient();
    void closeWizBoxUdpClient();
    void checkServerLicence();
    void setSwicthServerSelectedAction(const QString& strActionData);
    void setSwicthServerActionEnable(const QString &strActionData, bool bEnable);
    void downloadLogoFromWizBox(const QString& strUrl);
    void downloadOEMSettingsFromWizBox();
    void setLogo(const QString& logoPath);    

    void checkLocalUser(const QString& strAccountFolder, const QString& strUserGUID);
private:
    Ui::wizLoginWidget *ui;
    QMenu* m_menuUsers;
    QMenu* m_menuServers;
    QDialog* m_animationWaitingDialog;
    WizUdpClient* m_udpClient;
    QThread* m_udpThread;
    WizServerType m_serverType;
    WizServerType m_currentUserServerType;
    QString m_serverLicence;
    QTimer m_wizBoxSearchingTimer;
    QString m_wizLogoPath;

    QLineEdit* m_lineEditUserName;
    QLineEdit* m_lineEditPassword;
    QLineEdit* m_lineEditServer;
    WizStyleButton* m_buttonLogin;
    QLineEdit* m_lineEditNewUserName;
    QLineEdit* m_lineEditNewPassword;
    QLineEdit* m_lineEditRepeatPassword;
    WizStyleButton* m_buttonSignUp;

    //
    QState* m_stateWizLogIn;
    QState* m_stateWizBoxLogIn;
    QState* m_stateWizSignUp;
    QState* m_stateWizLogInCheck;
    QState* m_stateWizBoxLogInCheck;
    QState* m_stateSignUpCheck;

    //
    QMap<QString, QString> m_oemLogoMap;
    WizOEMDownloader* m_oemDownloader;

    QList<WizLocalUser> m_userList;
    QString m_loginUserGuid;

    bool m_newRegisterAccount;
};

#endif // WIZLOGINWIDGET_H
