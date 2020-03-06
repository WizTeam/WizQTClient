#include "WizLoginDialog.h"
#include "ui_WizLoginDialog.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QBitmap>
#include <QToolButton>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMovie>
#include <QDebug>
#include <QDateTime>
#include <QMetaObject>
#include <QMessageBox>
#include <QState>
#include <QHistoryState>
#include <QAbstractState>
#include <QStateMachine>
#include <QWebEngineView>
#include <QNetworkReply>

#include "share/jsoncpp/json/json.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizPathResolve.h"
#include "utils/WizLogger.h"
#include "share/WizUdpClient.h"
#include "share/WizMessageBox.h"
#include "share/WizMisc.h"
#include "share/WizSettings.h"
#include "share/WizAnalyzer.h"
#include "share/WizObjectDataDownloader.h"
#include "share/WizUI.h"
#include "share/WizThreads.h"
#include "share/WizGlobal.h"
#include "sync/WizKMServer.h"
#include "sync/WizAsyncApi.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"
#include "widgets/WizVerificationCodeDialog.h"
#include "WizWebSettingsDialog.h"
#include "WizProxyDialog.h"
#include "WizNoteStyle.h"
#include "WizOEMSettings.h"
#include "share/WizUIBase.h"


#define WIZ_SERVERACTION_CONNECT_WIZSERVER     "CONNECT_TO_WIZSERVER"
#define WIZ_SERVERACTION_CONNECT_BIZSERVER        "CONNECT_TO_BIZSERVER"
#define WIZ_SERVERACTION_SEARCH_SERVER          "SEARCH_WIZBOXSERVER"
#define WIZ_SERVERACTION_HELP               "SERVERHELP"


#define WIZ_ERROR_REGISTRATION_COUNT  366

#define WIZBOX_PROT     9269

#define WIZLOGO_WIDTH_IMAGE    180
#define WIZLOGO_WIDTH_AREA      190

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

WizLoginDialog::WizLoginDialog(const QString &strLocale, const QList<WizLocalUser>& localUsers, QWidget *parent)
#ifdef Q_OS_MAC
    : QDialog(parent)
#else
    : WizShadowWindow<QDialog>(parent, false)
#endif
    , ui(new Ui::wizLoginWidget)
    , m_menuUsers(new QMenu(this))
    , m_menuServers(new QMenu(this))
    , m_udpClient(0)
    , m_serverType(WizServer)
    , m_animationWaitingDialog(nullptr)
    , m_oemDownloader(nullptr)
    , m_userList(localUsers)
    , m_newRegisterAccount(false)
{
#ifdef Q_OS_MAC
    setWindowFlags(Qt::CustomizeWindowHint);
    ui->setupUi(this);
#else
    layoutTitleBar();
    //
    QWidget* uiWidget = new QWidget(clientWidget());
    clientLayout()->addWidget(uiWidget);
    ui->setupUi(uiWidget);
    //
    //  init style for wizbox
    ui->btn_selectServer->setMaximumHeight(::WizSmartScaleUI(20));
    ui->layout_titleBar->setContentsMargins(0, 0, 0, 0);
    ui->widget_titleBar->layout()->setContentsMargins(0, 0, 0, 0);
    ui->widget_titleBar->layout()->setSpacing(0);
    ui->label_logo->setMinimumHeight(WizSmartScaleUI(80));
    ui->btn_max->setVisible(false);
    ui->btn_min->setVisible(false);
    ui->btn_close->setVisible(false);
    //
    WizWindowTitleBar* title = titleBar();
    if (isDarkMode()) {
        title->setPalette(QPalette(QColor("#363636")));
    } else {
        title->setPalette(QPalette(QColor("#448aff")));
    }
    title->setContentsMargins(QMargins(0, 2, 2 ,0));
#endif

    m_lineEditUserName = ui->wgt_usercontainer->edit();
    m_lineEditPassword = ui->wgt_passwordcontainer->edit();
    m_lineEditServer = ui->wgt_serveroptioncontainer->edit();
    m_buttonLogin = ui->btn_login;
    m_lineEditNewUserName = ui->wgt_newUser->edit();
    m_lineEditNewPassword = ui->wgt_newPassword->edit();
    m_lineEditRepeatPassword = ui->wgt_passwordRepeat->edit();
    m_buttonSignUp = ui->btn_singUp;

    ui->wgt_newUser->setAutoClearRightIcon(true);
    ui->wgt_newPassword->setAutoClearRightIcon(true);
    ui->wgt_passwordRepeat->setAutoClearRightIcon(true);


//    connect(m_menuUsers, SIGNAL(triggered(QAction*)), SLOT(userListMenuClicked(QAction*)));
    connect(m_menuServers, SIGNAL(triggered(QAction*)), SLOT(serverListMenuClicked(QAction*)));

    connect(m_lineEditNewPassword, SIGNAL(textChanged(QString)), SLOT(onSignUpInputDataChanged()));
    connect(m_lineEditNewUserName, SIGNAL(textChanged(QString)), SLOT(onSignUpInputDataChanged()));
    connect(m_lineEditRepeatPassword, SIGNAL(textChanged(QString)), SLOT(onSignUpInputDataChanged()));
    connect(m_lineEditRepeatPassword, SIGNAL(returnPressed()), SLOT(on_btn_singUp_clicked()));
    connect(m_lineEditPassword, SIGNAL(textChanged(QString)), SLOT(onLoginInputChanged()));
    connect(m_lineEditUserName, SIGNAL(textChanged(QString)), SLOT(onLoginInputChanged()));
    connect(ui->wgt_usercontainer, SIGNAL(rightIconClicked()), SLOT(showUserListMenu()));
    connect(ui->wgt_serveroptioncontainer, SIGNAL(rightIconClicked()), SLOT(searchWizBoxServer()));
    connect(ui->btn_selectServer, SIGNAL(clicked()), SLOT(showServerListMenu()));
    connect(m_lineEditUserName, SIGNAL(textEdited(QString)), SLOT(onUserNameEdited(QString)));
    //
    if (isDarkMode()) {
#ifdef Q_OS_MAC
        QString style = QString("background-color:%1").arg(WizColorLineEditorBackground.name());
#else
        QString style = QString("background-color:%1;color:#ffffff").arg(WizColorLineEditorBackground.name());
#endif
        m_lineEditNewPassword->setStyleSheet(style);
        m_lineEditNewUserName->setStyleSheet(style);
        m_lineEditRepeatPassword->setStyleSheet(style);
        m_lineEditPassword->setStyleSheet(style);
        m_lineEditUserName->setStyleSheet(style);
        m_lineEditServer->setStyleSheet(style);
        //
    }
    //
    connect(&m_wizBoxSearchingTimer, SIGNAL(timeout()), SLOT(onWizBoxSearchingTimeOut()));
#ifndef Q_OS_MAC
    connect(m_buttonLogin, SIGNAL(clicked()), SLOT(on_btn_login_clicked()));
    connect(ui->btn_changeToLogin, SIGNAL(clicked()), SLOT(on_btn_changeToLogin_clicked()));
    connect(ui->btn_changeToSignin, SIGNAL(clicked()), SLOT(on_btn_changeToSignin_clicked()));
    connect(ui->btn_fogetpass, SIGNAL(clicked()), SLOT(on_btn_fogetpass_clicked()));
    connect(ui->btn_singUp, SIGNAL(clicked()), SLOT(on_btn_singUp_clicked()));
    connect(ui->btn_proxysetting, SIGNAL(clicked()), SLOT(on_btn_proxysetting_clicked()));
    connect(ui->cbx_autologin, SIGNAL(toggled(bool)), SLOT(on_cbx_autologin_toggled(bool)));
    connect(ui->cbx_remberPassword, SIGNAL(toggled(bool)), SLOT(on_cbx_remberPassword_toggled(bool)));
#endif

    QAction* actionWizServer = m_menuServers->addAction(tr("Sign in to WizNote Server"));
    actionWizServer->setData(WIZ_SERVERACTION_CONNECT_WIZSERVER);
    actionWizServer->setCheckable(true);
    QAction* actionWizBoxServer = m_menuServers->addAction(tr("Sign in to Private Server/WizBox"));
    actionWizBoxServer->setData(WIZ_SERVERACTION_CONNECT_BIZSERVER);
    actionWizBoxServer->setCheckable(true);
    //m_menuServers->addAction(tr("Find WizBox..."))->setData(WIZ_SERVERACTION_SEARCH_SERVER);
    m_menuServers->addAction(tr("About private deployment ..."))->setData(WIZ_SERVERACTION_HELP);
    m_menuServers->setDefaultAction(actionWizServer);

    //
    applyElementStyles(strLocale);

    loadDefaultUser();
    //
#ifdef Q_OS_MAC
    if (isDarkMode()) {
        ui->widget->setStyleSheet("background-color:transparent;");
        ui->widget_titleBar->setStyleSheet("background-color:transparent;");
    }
    //
#else
    //
    QSize totalSizeHint = layout()->totalSizeHint();
    //
    QSize minSize = QSize(totalSizeHint.width(), totalSizeHint.height() + ::WizSmartScaleUI(10));
    //
    if (minSize.height() > minSize.width() * 1.4)
    {
        minSize.setWidth(int(minSize.height() / 1.4));
        //
        int buttonMinimumWidth = minSize.width() - 2 * ::WizSmartScaleUI(40);
        ui->btn_login->setMinimumWidth(buttonMinimumWidth);
    }
    setMinimumSize(minSize);
    //
    if (isDarkMode()) {
        uiWidget->setStyleSheet("background-color:#363636;");
        ui->widget->setStyleSheet("background-color:#363636;");
        ui->widget_titleBar->setStyleSheet("background-color:#363636;");
    }
    //
    ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
        //
        QSize sz = ui->btn_login->size();
        ui->btn_singUp->setMinimumSize(sz);

    }, 300, 30000, [=]{});
#endif
    //
    initSateMachine();
}

WizLoginDialog::~WizLoginDialog()
{
    delete ui;
    if (m_animationWaitingDialog)
    {
        m_animationWaitingDialog->deleteLater();
    }
    if (m_udpClient)
    {
        m_udpClient->deleteLater();
    }
}



QString WizLoginDialog::userId() const
{
    return m_lineEditUserName->text();
}

QString WizLoginDialog::loginUserGuid() const
{
    return m_loginUserGuid;
}

QString WizLoginDialog::password() const
{
    return m_lineEditPassword->text();
}

QString WizLoginDialog::serverIp() const
{
    return m_lineEditServer->text();
}

WizServerType WizLoginDialog::serverType() const
{
    return m_serverType;
}

void WizLoginDialog::resetUserList()
{
    m_menuUsers->clear();

    for (WizLocalUser user : m_userList)
    {
        if (user.nUserType == m_serverType || (WizServer == m_serverType && user.nUserType == 0))
        {
//            QAction* action = m_menuUsers->addAction(user.strUserId);
            WizUserItemAction* userItem = new WizUserItemAction(user, m_menuUsers);
            connect(userItem, SIGNAL(userDeleteRequest(WizLocalUser)), SLOT(onDeleteUserRequest(WizLocalUser)));
            connect(userItem, SIGNAL(userSelected(WizLocalUser)), SLOT(onUserSelected(WizLocalUser)));
            userItem->setData(user.strGuid);
            m_menuUsers->addAction(userItem);
        }
    }
    //
    QSettings* settings = WizGlobal::globalSettings();
    QString strDefault = (WizServer == m_serverType) ? settings->value("Users/DefaultWizUserGuid").toString()
                                                     : settings->value("Users/DefaultWizBoxUserGuid").toString();

    if (strDefault.isEmpty() && m_menuUsers->actions().count() > 0)
    {
        strDefault = m_menuUsers->actions().first()->data().toString();
    }

    // set default user as default login entry.
    setUser(strDefault);
    QAction* action = findActionInMenu(strDefault);
    if (action)
    {
        m_menuUsers->setDefaultAction(action);
        WizUserItemAction* userAction = dynamic_cast<WizUserItemAction*>(action);
        if (userAction)
        {
            userAction->setSelected(true);
        }
    }
}

void WizLoginDialog::setUser(const QString &strUserGuid)
{
    m_newRegisterAccount = false;

    if (strUserGuid.isEmpty())
        return;

    QString strAccountFolder;
    QString strUserId;
    for (WizLocalUser user : m_userList)
    {
        if (user.strGuid == strUserGuid)
        {
            strAccountFolder = user.strDataFolderName;
            strUserId = user.strUserId;
            break;
        }
    }

    if (strAccountFolder.isEmpty())
    {
        qWarning() << "can not found user guid in local users : " << strUserGuid;
        return;
    }

    WizUserSettings userSettings(strAccountFolder);
    QString strPassword = userSettings.password();
    QString strUserName = userSettings.get("ACCOUNT", "USERNAME");
    strUserName = strUserName.isEmpty() ? strUserId : strUserName;

    m_loginUserGuid = strUserGuid;
    m_lineEditUserName->setText(strUserName);
    if (strPassword.isEmpty())
    {
        m_lineEditPassword->clear();
        ui->cbx_remberPassword->setCheckState(Qt::Unchecked);
    }
    else
    {
        m_lineEditPassword->setText(strPassword);
        ui->cbx_remberPassword->setCheckState(Qt::Checked);
    }

    //
    m_currentUserServerType = userSettings.serverType();
    qDebug() << "set user , user type : " << m_currentUserServerType;
    if (m_currentUserServerType == EnterpriseServer)
    {
        m_lineEditServer->setText(userSettings.enterpriseServerIP());
        WizCommonApiEntry::setEnterpriseServerIP(userSettings.enterpriseServerIP());
        // update oem settings
        downloadOEMSettingsFromWizBox();
    }
    else if ((m_currentUserServerType == NoServer && !userSettings.myWizMail().isEmpty()) ||
             m_currentUserServerType == WizServer)
    {
        m_currentUserServerType = WizServer;
        WizCommonApiEntry::setEnterpriseServerIP(WIZNOTE_API_SERVER);
    }
}

void WizLoginDialog::doAccountVerify()
{
    QString strUserGUID;
    QString strAccountFolder;
    for (WizLocalUser user : m_userList)
    {
        if (user.strUserId == userId())
        {
            strAccountFolder = user.strDataFolderName;
            strUserGUID = user.strGuid;
            break;
        }
    }

    if (strAccountFolder.isEmpty())
    {
        emit accountCheckStart();
        doOnlineVerify();
        return;
    }

    checkLocalUser(strAccountFolder, strUserGUID);
}

void WizLoginDialog::doOnlineVerify()
{
    WizToken::setUserId(userId());
    WizToken::setPasswd(password());

    connect(WizToken::instance(), SIGNAL(tokenAcquired(QString)), SLOT(onTokenAcquired(QString)), Qt::QueuedConnection);
    WizToken::requestToken();
    showAnimationWaitingDialog(tr("Connecting..."));
}

bool WizLoginDialog::updateGlobalProfile()
{
    QSettings* settings = WizGlobal::globalSettings();
    settings->setValue("Users/DefaultUserGuid", m_loginUserGuid);
    return true;
}

bool WizLoginDialog::updateUserProfile(bool bLogined)
{
    QString localUserId = WizGetLocalUserId(m_userList, m_loginUserGuid);
    QString strUserAccount = WizGetLocalFolderName(m_userList, m_loginUserGuid);
    qDebug() << "udate user profile , userid  " << localUserId;
    strUserAccount.isEmpty() ? (strUserAccount = userId()) : 0;
    WizUserSettings userSettings(strUserAccount);

    if(ui->cbx_autologin->checkState() == Qt::Checked) {
        userSettings.setAutoLogin(true);
    } else {
        userSettings.setAutoLogin(false);
    }

    if(ui->cbx_remberPassword->checkState() != Qt::Checked) {
        userSettings.setPassword();
    }

    WizDatabase db;
    if (!db.open(strUserAccount)) {
        ui->label_passwordError->setText(tr("Can not open database while update user profile"));
        return false;
    }

    if (bLogined)
    {
        if (ui->cbx_remberPassword->checkState() == Qt::Checked)
            userSettings.setPassword(::WizEncryptPassword(password()));

        db.setUserInfo(WizToken::userInfo());
    }
    db.setMeta("ACCOUNT", "USERID", userId());
    db.close();

    userSettings.setServerType(m_serverType);
    if (EnterpriseServer == m_serverType)
    {
        userSettings.setEnterpriseServerIP(serverIp());
        userSettings.setServerLicence(m_serverLicence);

        //
        QString logoFile = m_oemLogoMap.value(serverIp());
        qDebug() << "update oem logo path : " << logoFile;
        QString strAccountPath = Utils::WizPathResolve::dataStorePath() + m_lineEditNewUserName->text() + "/";
        if (logoFile.isEmpty() || !WizOEMSettings::settingFileExists(strAccountPath))
            return true;

        //
        WizOEMSettings settings(strAccountPath);
        QString strOldPath = settings.logoPath();
        if (!strOldPath.isEmpty())
        {
            QDir dir;
            dir.remove(strOldPath);
            QFile::copy(logoFile, strOldPath);
        }
        else
        {
            QString strNewPath = Utils::WizPathResolve::cachePath() + "logo/";
            Utils::WizPathResolve::ensurePathExists(strNewPath);
            strNewPath = strNewPath + WizGenGUIDLowerCaseLetterOnly() + ".png";
            QFile::copy(logoFile, strNewPath);
            settings.setLogoPath(strNewPath);
        }
    }

    return true;
}

void WizLoginDialog::enableLoginControls(bool bEnable)
{
    ui->wgt_usercontainer->setEnabled(bEnable);
    m_lineEditUserName->setEnabled(bEnable);
    m_lineEditPassword->setEnabled(bEnable);
    ui->cbx_autologin->setEnabled(bEnable);
    ui->cbx_remberPassword->setEnabled(bEnable);
    m_buttonLogin->setEnabled(bEnable);
    ui->btn_changeToSignin->setEnabled(bEnable);
    ui->btn_selectServer->setEnabled(bEnable);
}

void WizLoginDialog::enableSignUpControls(bool bEnable)
{
    //NOTE: 很奇怪的问题，修改编辑框的状态会导致sns登录按钮被触发
//    m_lineEditNewUserName->setEnabled(bEnable);
//    m_lineEditNewPassword->setEnabled(bEnable);
//    m_lineEditRepeatPassword->setEnabled(bEnable);
    ui->btn_singUp->setEnabled(bEnable);
    ui->btn_changeToLogin->setEnabled(bEnable);
}

bool WizLoginDialog::isNewRegisterAccount()
{
    return m_newRegisterAccount;
}

#ifdef Q_OS_MAC
void WizLoginDialog::mousePressEvent(QMouseEvent *event)
{
    m_mousePoint = event->globalPos();
}

void WizLoginDialog::mouseMoveEvent(QMouseEvent *event)
{
    if(m_mousePoint != QPoint(0, 0))
    {
        move(geometry().x() + event->globalPos().x() - m_mousePoint.x(), geometry().y() + event->globalPos().y() - m_mousePoint.y());
        m_mousePoint = event->globalPos();
    }
}

void WizLoginDialog::mouseReleaseEvent(QMouseEvent *)
{
    m_mousePoint = QPoint(0, 0);
}

#endif


void WizLoginDialog::on_btn_close_clicked()
{
    qApp->quit();
}

void WizLoginDialog::applyElementStyles(const QString &strLocal)
{
    ui->stackedWidget->setCurrentIndex(0);
    //setFixedWidth(::WizSmartScaleUI(354));
    ui->widget_titleBar->setFixedHeight(::WizSmartScaleUI(40));

    QString strThemeName = Utils::WizStyleHelper::themeName();

    // setup locale for welcome dialog
    if (isDarkMode()) {
        if (strLocal.startsWith("zh_")) {
            m_wizLogoPath= ::WizGetSkinResourceFileName(strThemeName, "dark_loginLogoCn");
        } else {
            m_wizLogoPath= ::WizGetSkinResourceFileName(strThemeName, "dark_loginLogoUs");
        }
    } else {
        if (strLocal.startsWith("zh_")) {
            m_wizLogoPath= ::WizGetSkinResourceFileName(strThemeName, "loginLogoCn");
        } else {
            m_wizLogoPath= ::WizGetSkinResourceFileName(strThemeName, "loginLogoUs");
        }
    }
    //
    ui->label_logo->setMinimumWidth(190);   // use fixed logo size for oem
    if (isDarkMode()) {
        ui->label_logo->setStyleSheet(QString("QLabel {border: none; image: url(%1);"
                                            "background-position: center; background-repeat: no-repeat; background-color:transparent}").arg(m_wizLogoPath));
        ui->label_placehold->setStyleSheet(QString("QLabel {border: none;background-color:transparent}"));
    } else {
        ui->label_logo->setStyleSheet(QString("QLabel {border: none; image: url(%1);"
                                            "background-position: center; background-repeat: no-repeat; background-color:#448aff}").arg(m_wizLogoPath));
        ui->label_placehold->setStyleSheet(QString("QLabel {border: none;background-color:#448aff}"));
    }

    //
#ifdef Q_OS_MAC
    QString strBtnCloseNormal = ::WizGetSkinResourceFileName(strThemeName, "loginCloseButton_normal");
    QString strBtnCloseHot = ::WizGetSkinResourceFileName(strThemeName, "loginCloseButton_hot");
    ui->btn_close->setStyleSheet(QString("QToolButton{ border:none; image:url(%1); padding:0px; height: 13px; width: 13px;}"
                                         "QToolButton:hover{ image:url(%2);}"
                                           "QToolButton:pressed { image:url(%3);}")
                                 .arg(strBtnCloseNormal).arg(strBtnCloseHot).arg(strBtnCloseHot));
    QString strGrayButton = ::WizGetSkinResourceFileName(strThemeName, "loginGrayButton");
    ui->btn_min->setStyleSheet(QString("QToolButton{ border:none; image:url(%1); padding:0px; height: 13px; width: 13px;}").arg(strGrayButton));
    ui->btn_max->setStyleSheet(QString("QToolButton{ border:none; image:url(%1); padding:0px; height: 13px; width: 13px;}").arg(strGrayButton));
#else
    WizWindowTitleBar* m_titleBar = titleBar();
    if (m_titleBar)
    {
        QString strBtnCloseNormal = ::WizGetSkinResourceFileName(strThemeName, "linuxlogindialoclose");
        QString strBtnCloseHover = ::WizGetSkinResourceFileName(strThemeName, "linuxwindowclose_on");
        QString strBtnCloseDown = ::WizGetSkinResourceFileName(strThemeName, "linuxwindowclose_selected");
        m_titleBar->minButton()->setVisible(false);
        m_titleBar->maxButton()->setVisible(false);
        m_titleBar->closeButton()->setIcon(QIcon());
        m_titleBar->closeButton()->setStyleSheet(QString("QToolButton{ border-image:url(%1); height: 16px; width: 16px;}"
                                                         "QToolButton:hover{ border-image:url(%2); height: 16px; width: 16px;}"
                                                         "QToolButton:pressed{ border-image:url(%3); height: 16px; width: 16px;}")
                                                 .arg(strBtnCloseNormal).arg(strBtnCloseHover).arg(strBtnCloseDown));
        m_titleBar->closeButton()->setFixedSize(::WizSmartScaleUI(16), ::WizSmartScaleUI(16));
        //
        if (isDarkMode()) {
            //m_titleBar->setStyleSheet("background-color:#363636");
            //m_titleBar->setStyleSheet("background-color:red");
        }
    }
#endif

    //
    ui->cbx_autologin->setStyleSheet(QString("QCheckBox{background:none;border:none;}"
                                             "QCheckBox:focus{background:none;border:none;}"
                                             "QCheckBox::pressed{background:none;border:none;}"));
    ui->cbx_remberPassword->setStyleSheet(QString("QCheckBox{background:none;border:none;}"
                                             "QCheckBox:focus{background:none;border:none;}"
                                             "QCheckBox::pressed{background:none;border:none;}"));

#ifndef Q_OS_MAC
    if (isDarkMode()) {
        //indicator is not visible
        //QString style = QString("QCheckBox:indicator{color:#ffffff;background-color:%1;}").arg(WizColorLineEditorBackground.name());
        //ui->cbx_autologin->setStyleSheet(style);
        //ui->cbx_remberPassword->setStyleSheet(style);
        ui->cbx_autologin->setStyleSheet(QString("QCheckBox{background:none;border:none;color:#a6a6a6;}"
                                                 "QCheckBox:focus{background:none;border:none;}"
                                                 "QCheckBox::pressed{background:none;border:none;}"));
        ui->cbx_remberPassword->setStyleSheet(QString("QCheckBox{background:none;border:none;color:#a6a6a6;}"
                                                 "QCheckBox:focus{background:none;border:none;}"
                                                 "QCheckBox::pressed{background:none;border:none;}"));
    }
#endif
    //
    QString strLoginTopLineEditor = WizGetSkinResourceFileName(strThemeName, "loginTopLineEditor");
    QString strLoginMidLineEditor = WizGetSkinResourceFileName(strThemeName, "loginMidLineEditor");
    QString strLoginBottomLineEditor = WizGetSkinResourceFileName(strThemeName, "loginBottomLineEditor");
    bool isHighPix = ::WizIsHighPixel();
    QString strIconPerson = WizGetSkinResourceFileName(strThemeName, "loginIconPerson" + (isHighPix ? "@2x" : QString()));
    QString strIconKey = WizGetSkinResourceFileName(strThemeName, "loginIconKey" + (isHighPix ? "@2x" : QString()));
    QString strIconServer = WizGetSkinResourceFileName(strThemeName, "loginIconServer" + (isHighPix ? "@2x" : QString()));
    ui->wgt_usercontainer->setBackgroundImage(strLoginTopLineEditor, QPoint(8, 8), WizColorLineEditorBackground);
    ui->wgt_usercontainer->setLeftIcon(strIconPerson);
    ui->wgt_usercontainer->setRightIcon(WizGetSkinResourceFileName(strThemeName, "loginLineEditorDownArrow" + (isHighPix ? "@2x" : QString())));
    m_lineEditUserName->setPlaceholderText("example@mail.com");

    ui->wgt_passwordcontainer->setBackgroundImage(strLoginMidLineEditor, QPoint(8, 8), WizColorLineEditorBackground);
    ui->wgt_passwordcontainer->setLeftIcon(strIconKey);
    m_lineEditPassword->setEchoMode(QLineEdit::Password);
    m_lineEditPassword->setPlaceholderText(tr("Password"));

    ui->wgt_serveroptioncontainer->setBackgroundImage(strLoginBottomLineEditor, QPoint(8, 8), WizColorLineEditorBackground);
    ui->wgt_serveroptioncontainer->setLeftIcon(strIconServer);
    //QString strIconSearch = WizGetSkinResourceFileName(strThemeName, "empty_search" + (isHighPix ? "@2x" : QString()));
    //ui->wgt_serveroptioncontainer->setRightIcon(strIconSearch);
    ui->wgt_serveroptioncontainer->setRightIcon(WizLoadSkinIcon(strThemeName, "empty_search"));
    m_lineEditServer->setPlaceholderText(tr("Please search or input your server IP"));

    ui->wgt_newUser->setBackgroundImage(strLoginTopLineEditor, QPoint(8, 8), WizColorLineEditorBackground);
    ui->wgt_newUser->setLeftIcon(strIconPerson);
    m_lineEditNewUserName->setPlaceholderText(tr("Please input email as your account"));

    ui->wgt_newPassword->setBackgroundImage(strLoginMidLineEditor, QPoint(8, 8), WizColorLineEditorBackground);
    ui->wgt_newPassword->setLeftIcon(strIconKey);
    m_lineEditNewPassword->setPlaceholderText(tr("Please enter your password"));
    m_lineEditNewPassword->setEchoMode(QLineEdit::Password);

    ui->wgt_passwordRepeat->setBackgroundImage(strLoginBottomLineEditor, QPoint(8, 8), WizColorLineEditorBackground);
    ui->wgt_passwordRepeat->setLeftIcon(strIconKey);
    m_lineEditRepeatPassword->setPlaceholderText(tr("Please repeat your password"));
    m_lineEditRepeatPassword->setEchoMode(QLineEdit::Password);
    //

    QString strBtnNormal = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_active");
    QString strBtnHover = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_hover");
    QString strBtnDown = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_down");
    QString strBtnDisable = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_normal");
    //
    m_buttonLogin->setButtonStyle(strBtnNormal, strBtnHover, strBtnDown, strBtnDisable, QColor("#ffffff"),
                              QColor("#ffffff"), QColor("#b1b1b1"));
    m_buttonLogin->setText(tr("Login"));
    m_buttonLogin->setEnabled(false);

    m_buttonSignUp->setButtonStyle(strBtnNormal, strBtnHover, strBtnDown, strBtnDisable, QColor("#ffffff"),
                                   QColor("#ffffff"), QColor("#b1b1b1"));
    m_buttonSignUp->setText(tr("Create Account"));
    m_buttonSignUp->setEnabled(false);
    //
    QString strSeparator = ::WizGetSkinResourceFileName(strThemeName, "loginSeparator");
    ui->label_separator2->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                               "background-position: center; background-repeat: no-repeat}").arg(strSeparator));
   //
    ui->label_noaccount->setStyleSheet(QString("QLabel {border: none; color: #5f5f5f;}"));
    //
#ifndef Q_OS_MAC
    if (isDarkMode()) {
        m_buttonLogin->setStyleSheet("QPushButton{color:#ffffff}QPushButton:disabled{color:#b1b1b1}");
    }
#endif
    //
#ifdef Q_OS_MAC
    ui->btn_changeToSignin->setStyleSheet(QString("QPushButton { border: 1px; background: none; "
                                                 "color: #448aff;  padding-left: 10px; padding-bottom: 0px}"));
    ui->btn_changeToLogin->setStyleSheet(QString("QPushButton { border: 1px; background: none; "
                                                 "color: #448aff;  padding-left: 10px; padding-bottom: 0px}"));
#else
    ui->btn_changeToSignin->setStyleSheet(QString("QPushButton { border: 1px; background: none; "
                                                 "color: #448aff;  padding-left: 10px; padding-bottom: 0px}"));
    ui->btn_changeToLogin->setStyleSheet(QString("QPushButton { border: 1px; background: none; "
                                                 "color: #448aff; padding-left: 10px; padding-bottom: 0px}"));
#endif

    QString bg_switchserver_menu = ::WizGetSkinResourceFileName(strThemeName, "bg_switchserver_menu");
    ui->btn_selectServer->setStyleSheet(QString("QPushButton { border: none; background-image: url(%1);"
                                                "background-position: right center; background-repeat: no-repeat;"
                                                     "color: rgba(255, 255, 255, 153); padding:0px; padding-right:15px; margin-right: 20px; margin-top: 5px}").arg(bg_switchserver_menu));

    ui->btn_proxysetting->setStyleSheet(QString("QPushButton { border: none; background: none; "
                                                "color: #b1b1b1; margin:0px; margin-left:10px; margin-right:10px;  padding:0px; padding-bottom: 5px}"));
    ui->btn_fogetpass->setStyleSheet(QString("QPushButton { border: none; background: none; "
                                                 "color: #b1b1b1; margin:0px; margin-left: 10px; margin-right:10px; padding:0px; padding-bottom: 5px}"));
    ui->btn_snsLogin->setStyleSheet(QString("QPushButton { border: none; background: none; "
                                            "color: #b1b1b1; margin:0px; margin-left: 10px; margin-right:10px; padding:0px; padding-bottom: 5px}"));

    QString strLineSeparator = ::WizGetSkinResourceFileName(strThemeName, "loginLineSeparator");
    ui->label_separator3->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                                "background-position: center; background-repeat: no-repeat}").arg(strLineSeparator));
    ui->label_separator4->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                                "background-position: center; background-repeat: no-repeat}").arg(strLineSeparator));

    setStyleSheet("QLineEdit { padding:0px; border:0px; }");

    //
    ui->btn_changeToLogin->setVisible(false);
    ui->label_passwordError->setStyleSheet(QString("QLabel {border: none; padding-left: 25px; color: red;}"));
    ui->label_passwordError->setText("");

    m_menuUsers->setFixedWidth(ui->wgt_usercontainer->width());

    QString status_switchserver_selected = ::WizGetSkinResourceFileName(strThemeName, "status_switchserver_selected");
    //
    if (isDarkMode()) {
        //
        QString menuStyle = QString("QMenu {background-color: #272727; border-style: solid; border-color: #3399ff; border-width: 1px; padding: 0px 0px 0px 0px;  menu-scrollable: 1;}"
                                    "QMenu::item {padding: 4px 10px 4px 25px; color: #a6a6a6;  background-color: #272727;}"
                                    "QMenu::item:selected {background-color: #0058d1; color:#ffffff }"
                                    "QMenu::item:disabled {color: #5c5c5c; }"
                                    "QMenu::indicator { width: 16px; height: 16px; margin-left: 5px;} "
                                    "QMenu::indicator:non-exclusive:checked { image: url(%1); }").arg(status_switchserver_selected);
        //
        m_menuUsers->setStyleSheet(menuStyle);
        m_menuServers->setStyleSheet(menuStyle);

    } else {
        m_menuUsers->setStyleSheet("QMenu {background-color: #ffffff; border-style: solid; border-color: #448aff; border-width: 1px; color: #5F5F5F; padding: 0px 0px 0px 0px; menu-scrollable: 1;}");
        m_menuServers->setStyleSheet(QString("QMenu {background-color: #ffffff; border-style: solid; border-color: #3399ff; border-width: 1px; padding: 0px 0px 0px 0px;  menu-scrollable: 1;}"
                                     "QMenu::item {padding: 4px 10px 4px 25px; color: #000000;  background-color: #ffffff;}"
                                     "QMenu::item:selected {background-color: #E7F5FF; }"
                                     "QMenu::item:disabled {color: #999999; }"
                                     "QMenu::indicator { width: 16px; height: 16px; margin-left: 5px;} "
                                     "QMenu::indicator:non-exclusive:checked { image: url(%1); }").arg(status_switchserver_selected));
    }
    //
#ifndef Q_OS_MAC
    m_buttonLogin->setMinimumHeight(WizSmartScaleUI(28));
#endif
}

bool WizLoginDialog::checkSignMessage()
{
    QRegExp mailRex("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
    mailRex.setCaseSensitivity(Qt::CaseInsensitive);
    QString strThemeName = Utils::WizStyleHelper::themeName();
    QString strErrorIcon = ::WizGetSkinResourceFileName(strThemeName, "loginErrorInput");
    if (!mailRex.exactMatch(m_lineEditNewUserName->text()))
    {
        ui->wgt_newUser->setRightIcon(strErrorIcon);
        ui->label_passwordError->setText(tr("Invalid email address."));
        return false;
    }

    if (m_lineEditNewPassword->text().isEmpty())
    {
        ui->wgt_newPassword->setRightIcon(strErrorIcon);
        ui->label_passwordError->setText(tr("Password is Empty"));
        return false;
    }

    if (m_lineEditNewPassword->text() != m_lineEditRepeatPassword->text())
    {
        ui->wgt_passwordRepeat->setRightIcon(strErrorIcon);
        ui->label_passwordError->setText(tr("Passwords don't match"));
        return false;
    }

    return true;
}

QAction *WizLoginDialog::findActionInMenu(const QString &strActData)
{
    QList<QAction*> actionList = m_menuUsers->actions();
    for (int i = 0; i < actionList.count(); i++)
    {
        if (actionList.at(i)->data().toString() == strActData)
            return actionList.at(i);
    }
    return 0;
}

bool WizLoginDialog::doVerificationCodeCheck(QString& strCaptchaID, QString& strCaptcha)
{
    strCaptchaID = QString::number(QDateTime::currentMSecsSinceEpoch()).right(8);
    strCaptchaID += WizGenGUIDLowerCaseLetterOnly().right(6);

    WizVerificationCodeDialog dlg(this);
    if (dlg.verificationRequest(strCaptchaID) == QDialog::Accepted)
    {
        strCaptcha = dlg.getVerificationCode();
        return true;
    }
    return false;
}

void WizLoginDialog::searchWizBoxServer()
{
    qDebug() << "start findWizBoxServer ";
    startWizBoxUdpClient();

    emit wizBoxSearchRequest(WIZBOX_PROT, "find wizbox");
    m_wizBoxSearchingTimer.start(10 * 1000);
    showSearchingDialog();
}

void WizLoginDialog::showSearchingDialog()
{
    int result = showAnimationWaitingDialog(tr("Finding Service...."));
    if (QDialog::Rejected == result)
    {
        m_wizBoxSearchingTimer.stop();
        qDebug() << "Searching cancel";
        closeWizBoxUdpClient();

        if (m_currentUserServerType != WizServer)
        {
            m_lineEditServer->clear();
            m_serverType = EnterpriseServer;
        }
    }
}

void WizLoginDialog::initAnimationWaitingDialog(const QString& text)
{
    m_animationWaitingDialog = new QDialog();
    m_animationWaitingDialog->setWindowFlags(Qt::FramelessWindowHint);
    m_animationWaitingDialog->setFixedSize(WizSmartScaleUI(150), WizSmartScaleUI(100));
    QPalette pl = m_animationWaitingDialog->palette();
    m_animationWaitingDialog->setStyleSheet("QDialog{background-color:#000000;} ");
    pl.setColor(QPalette::Window, QColor(0, 0, 0, 200));
    m_animationWaitingDialog->setPalette(pl);
    m_animationWaitingDialog->setAutoFillBackground(true);
    m_animationWaitingDialog->setWindowOpacity(0.7);

    QHBoxLayout* closeLayout = new QHBoxLayout();
    QToolButton* closeButton = new QToolButton(m_animationWaitingDialog);
    QString strBtnCloseNormal = Utils::WizStyleHelper::createTempPixmap("linuxlogindialoclose_white");
    QString strBtnCloseHover = Utils::WizStyleHelper::createTempPixmap("linuxwindowclose_on");
    QString strBtnCloseDown = Utils::WizStyleHelper::createTempPixmap("linuxwindowclose_selected");
    closeButton->setStyleSheet(QString("QToolButton{ image:url(%1); background:transparent; padding:0px; border:0px; height: 16px; width: 16px;}"
                                                     "QToolButton:hover{ image:url(%2); height: %4px; width: %4px;}"
                                                     "QToolButton:pressed{ image:url(%3); height: %4px; width: %4px;}")
                               .arg(strBtnCloseNormal)
                               .arg(strBtnCloseHover)
                               .arg(strBtnCloseDown)
                               .arg(WizSmartScaleUI(16))
                               );
    closeLayout->setContentsMargins(0, 0, 0, 0);
    closeLayout->addStretch();
    closeLayout->addWidget(closeButton);
    connect(closeButton, SIGNAL(clicked()), m_animationWaitingDialog, SLOT(reject()));
    //
    QLabel* labelSearching = new QLabel(m_animationWaitingDialog);
    labelSearching->setFixedSize(WizSmartScaleUI(32), WizSmartScaleUI(32));
    QMovie* movie =new QMovie(m_animationWaitingDialog);
    movie->setFileName(":/searching.gif");
    labelSearching->setMovie(movie);
    QHBoxLayout* labelLayout = new QHBoxLayout();
    labelLayout->setContentsMargins(0, 0, 0, 0);
    labelLayout->addStretch();
    labelLayout->addWidget(labelSearching);
    labelLayout->addStretch();
    QHBoxLayout* textLayout = new QHBoxLayout();
    textLayout->setContentsMargins(0, WizSmartScaleUI(5), 0, 0);
    textLayout->addStretch();
    QLabel* labelText = new QLabel(m_animationWaitingDialog);
    labelText->setText(text);
    QPalette plText = labelText->palette();
    plText.setColor(QPalette::WindowText, QColor(255, 255, 255, 200));
    labelText->setPalette(plText);
    textLayout->addWidget(labelText);
    textLayout->addStretch();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, WizSmartScaleUI(15));
    layout->addLayout(closeLayout);
    layout->addLayout(labelLayout);
    layout->addLayout(textLayout);
   m_animationWaitingDialog->setLayout(layout);

   movie->start();
}

int WizLoginDialog::showAnimationWaitingDialog(const QString& text)
{
    if (m_animationWaitingDialog)
    {
        //NOTE:修复按钮样式问题，每次重新创建
        delete m_animationWaitingDialog;
    }
    initAnimationWaitingDialog(text);

    //
    QPoint leftTop = geometry().topLeft();
    leftTop.setX(leftTop.x() + (width() - m_animationWaitingDialog->width()) / 2);
    leftTop.setY(leftTop.y() + (height() - m_animationWaitingDialog->height()) / 2);
    m_animationWaitingDialog->move(leftTop);

    return m_animationWaitingDialog->exec();
}

void WizLoginDialog::closeAnimationWaitingDialog()
{
    if (m_animationWaitingDialog && m_animationWaitingDialog->isVisible())
    {
        m_animationWaitingDialog->accept();
    }
}

void WizLoginDialog::startWizBoxUdpClient()
{
    if (!m_udpClient)
    {
        m_udpClient = new WizUdpClient();
        connect(this, SIGNAL(wizBoxSearchRequest(int,QString)),
                m_udpClient, SLOT(boardcast(int,QString)), Qt::QueuedConnection);

        m_udpThread = new QThread(this);
//        qDebug() << "create thread : " << m_udpThread;
        m_udpClient->moveToThread(m_udpThread);
    }

    connect(m_udpClient, SIGNAL(udpResponse(QString,QString,QString)),
            SLOT(onWizBoxResponse(QString,QString,QString)));

    if (!m_udpThread->isRunning())
    {
        m_udpThread->start();
    }
}

void WizLoginDialog::closeWizBoxUdpClient()
{
    disconnect(m_udpClient, SIGNAL(udpResponse(QString,QString,QString)),
               this, SLOT(onWizBoxResponse(QString,QString,QString)));
    QMetaObject::invokeMethod(m_udpClient, "closeUdpConnections", Qt::QueuedConnection);
    m_udpThread->quit();
}

void WizLoginDialog::checkServerLicence()
{
    qDebug() << "current server ip : " << serverIp();
    if (serverIp().isEmpty())
    {
        WizMessageBox::warning(this, tr("Info"), tr("There is no server address, please input it."));
        return;
    }

    if (!m_oemDownloader)
    {
        initOEMDownloader();
    }
    m_oemDownloader->setServerIp(serverIp());
    WizCommonApiEntry::setEnterpriseServerIP(serverIp());

    WizUserSettings userSettings(userId());
    QString strOldLicence = userSettings.serverLicence();

    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        m_oemDownloader->checkServerLicence(strOldLicence);
    });

    showAnimationWaitingDialog(tr("Connecting...."));
}

void WizLoginDialog::setSwicthServerSelectedAction(const QString& strActionData)
{
    QList<QAction*> actionsList = m_menuServers->actions();
    std::for_each(std::begin(actionsList), std::end(actionsList), [&](QAction* const item) {
        item->setChecked(false);
        if (item->data().toString() == strActionData)
        {
            item->setChecked(true);
        }
    });
}

void WizLoginDialog::setSwicthServerActionEnable(const QString& strActionData, bool bEnable)
{
    QList<QAction*> actionsList = m_menuServers->actions();
    for (QAction* actionItem : actionsList)
    {
        if (actionItem && actionItem->data().toString() == strActionData)
        {
            actionItem->setEnabled(bEnable);
        }
    }
}

void WizLoginDialog::downloadLogoFromWizBox(const QString& strUrl)
{
    if (!m_oemDownloader)
    {
        initOEMDownloader();
    }

    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        m_oemDownloader->downloadOEMLogo(strUrl);
    });
}

void WizLoginDialog::downloadOEMSettingsFromWizBox()
{
    if (serverIp().isEmpty())
        return;
    //
    if (!m_oemDownloader)
    {
        initOEMDownloader();
    }
    m_oemDownloader->setServerIp(serverIp());

    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
       m_oemDownloader->downloadOEMSettings();
    });
}

void WizLoginDialog::setLogo(const QString& logoPath)
{
    if (isDarkMode()) {
        ui->label_logo->setStyleSheet(QString("QLabel {border: none; image: url(%1);"
                                            "background-position: center; background-repeat: no-repeat; background-color:transparent}").
                                      arg(logoPath.isEmpty() ? m_wizLogoPath : logoPath));
    } else {
        ui->label_logo->setStyleSheet(QString("QLabel {border: none; image: url(%1);"
                                            "background-position: center; background-repeat: no-repeat; background-color:#448aff}").
                                      arg(logoPath.isEmpty() ? m_wizLogoPath : logoPath));
    }
}

void WizLoginDialog::checkLocalUser(const QString& strAccountFolder, const QString& strUserGUID)
{
    qDebug() << "do local account verify , folder path : " << strAccountFolder;
    WizUserSettings userSettings(strAccountFolder);

    //  首先判断用户的服务器类型，如果是之前使用过但是没有记录服务器类型，则使用wiz服务器
    //  如果登录过企业服务则需要登录到企业服务器
    if (EnterpriseServer == m_serverType)
    {
        if (serverIp().isEmpty())
        {
            WizMessageBox::warning(this, tr("Info"), tr("There is no server address, please input it."));
            return;
        }

        if (userSettings.enterpriseServerIP().isEmpty() && !userSettings.myWizMail().isEmpty())
        {
            WizMessageBox::warning(this, tr("Info"), tr("The user name can't switch to enterprise server, it was signed in to WizNote."));
            return;
        }
        // clear proxy for app
//        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    }
    else if (WizServer == m_serverType && !userSettings.enterpriseServerIP().isEmpty())
    {
        WizMessageBox::warning(this, tr("Info"), tr("The user name can't switch to WizNote, it was signed in to enterprise server. "));
        return;
    }

    qDebug() << "do account verify , server type : " << m_serverType << userId() << password().isEmpty();
    // FIXME: should verify password if network is available to avoid attack?
    if (password() != userSettings.password())
    {
        // check server licence and update oem settings
        if (EnterpriseServer == m_serverType)
        {
            checkServerLicence();
            return;
        }
        //
        emit accountCheckStart();
        doOnlineVerify();
        return;
    }
    else if (EnterpriseServer == m_serverType)
    {
        if (userSettings.enterpriseServerIP() != serverIp())
        {
            checkServerLicence();
            return;
        }
        else
        {
            WizCommonApiEntry::setEnterpriseServerIP(serverIp());
        }
    }

    //
    m_loginUserGuid = strUserGUID;
    if (updateUserProfile(false) && updateGlobalProfile())
    {
        QDialog::accept();
    }
}


void WizLoginDialog::on_btn_changeToSignin_clicked()
{

}

void WizLoginDialog::on_btn_changeToLogin_clicked()
{
    ui->btn_changeToLogin->setVisible(false);
    ui->btn_changeToSignin->setVisible(true);
    ui->label_noaccount->setText(tr("No account yet,"));
    ui->label_passwordError->clear();
    ui->stackedWidget->setCurrentIndex(0);
    ui->wgt_usercontainer->setFocus();
}

void WizLoginDialog::on_btn_proxysetting_clicked()
{
    WizProxyDialog dlg;
    dlg.exec();
}

void WizLoginDialog::on_btn_fogetpass_clicked()
{
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("forgot_password");
    QDesktopServices::openUrl(QUrl(strUrl));
}

void WizLoginDialog::on_btn_login_clicked()
{
    if (m_lineEditUserName->text().isEmpty()) {
        ui->label_passwordError->setText(tr("Please enter user id"));
        return;
    }

    if (password().isEmpty()) {
        ui->label_passwordError->setText(tr("Please enter user password"));
        return;
    }

    if (EnterpriseServer == m_serverType)
    {
        if (m_lineEditServer->text().isEmpty())
        {
            ui->label_passwordError->setText(tr("Please enter server address"));
        }
        else
        {
            WizCommonApiEntry::setEnterpriseServerIP(m_lineEditServer->text());
        }
    }

    doAccountVerify();
}

void WizLoginDialog::on_btn_singUp_clicked()
{
    if (checkSignMessage())
    {
        WizAsyncApi* api = new WizAsyncApi(this);
        connect(api, SIGNAL(registerAccountFinished(bool)), SLOT(onRegisterAccountFinished(bool)));
        api->registerAccount(m_lineEditNewUserName->text(), m_lineEditNewPassword->text(), "");
        emit accountCheckStart();
    }
}

void WizLoginDialog::onLoginInputChanged()
{
    ui->label_passwordError->clear();
    bool bInputFinished =  !m_lineEditUserName->text().isEmpty() && !m_lineEditPassword->text().isEmpty();
    ui->btn_login->setEnabled(bInputFinished);
    m_currentUserServerType = NoServer;
}

void WizLoginDialog::onTokenAcquired(const QString &strToken)
{
    closeAnimationWaitingDialog();
    //
    WizToken::instance()->disconnect(this);

    qDebug() << "on tonken acquired : " << strToken;


    emit accountCheckFinished();
    if (strToken.isEmpty())
    {
        int nErrorCode = WizToken::lastErrorCode();
        // network unavailable
        if (QNetworkReply::ProtocolUnknownError == nErrorCode)
        {
            WizUserSettings userSettings(userId());
            if (password() != userSettings.password())
            {
                ui->label_passwordError->setText(tr("Network connection is unavailable."));
                return;
            }
            else
            {
                // login use local data
                QDialog::accept();
                return;
            }
        }
        else
        {
            if (WIZKM_XMLRPC_ERROR_INVALID_TOKEN == nErrorCode)
            {
                ui->label_passwordError->setText(tr("User name or password is not correct!"));
            }
            else if (WIZKM_XMLRPC_ERROR_INVALID_USER == nErrorCode)
            {
                ui->label_passwordError->setText(tr("User not exists!"));
            }
            else if (WIZKM_XMLRPC_ERROR_INVALID_PASSWORD == nErrorCode)
            {
                ui->label_passwordError->setText(tr("Password error!"));
            }
            else if (WIZKM_XMLRPC_ERROR_TOO_MANY_LOGINS == nErrorCode)
            {
                ui->label_passwordError->setText(tr("Log in too many times in a short time, please try again later."));
            }
            else if (WIZKM_XMLRPC_ERROR_SYSTEM_ERROR == nErrorCode)
            {
                ui->label_passwordError->setText(WizToken::lastErrorMessage());
            }
            else
            {
                ui->label_passwordError->setText(WizToken::lastErrorMessage());
            }
            return;
        }
    }

    m_loginUserGuid = WizToken::userInfo().strUserGUID;
    if (updateUserProfile(true) && updateGlobalProfile())
        QDialog::accept();
}

void WizLoginDialog::onSignUpInputDataChanged()
{
    ui->label_passwordError->clear();
    bool bInputFinished = !m_lineEditNewUserName->text().isEmpty() && !m_lineEditNewPassword->text().isEmpty()
            && !m_lineEditRepeatPassword->text().isEmpty();
    ui->btn_singUp->setEnabled(bInputFinished);
}

//void CWizLoginDialog::userListMenuClicked(QAction *action)
//{
//    if (action)
//    {
//        QAction* currentDefault = m_menuUsers->defaultAction();
//        CWizUserItemAction* userAction = dynamic_cast<CWizUserItemAction*>(currentDefault);
//        if (userAction)
//        {
//            userAction->setSelected(false);
//        }

//        m_menuUsers->setDefaultAction(action);
//        userAction = dynamic_cast<CWizUserItemAction*>(action);
//        if (userAction)
//        {
//            userAction->setSelected(true);
//        }
//        setUser(action->data().toString());
//    }
//}

void WizLoginDialog::serverListMenuClicked(QAction* action)
{
    if (action)
    {
        QString strActionData = action->data().toString();
        if (strActionData == WIZ_SERVERACTION_CONNECT_WIZSERVER)
        {
            m_serverType = WizServer;
            emit wizServerSelected();
            action->setChecked(true);
        }
        else if (strActionData == WIZ_SERVERACTION_CONNECT_BIZSERVER)
        {
            m_serverType = EnterpriseServer;
            emit wizBoxServerSelected();
            //searchWizBoxServer();
            action->setChecked(true);
        }
        else if (strActionData == WIZ_SERVERACTION_SEARCH_SERVER)
        {
            m_serverType = EnterpriseServer;
            emit wizBoxServerSelected();
            searchWizBoxServer();

            /*
            QList<QAction*> actionList = m_menuServers->actions();
            for (QAction* act : actionList)
            {
                if (act->data().toString() == WIZ_SERVERACTION_CONNECT_BIZSERVER)
                {
                    act->setChecked(true);
                    break;
                }
            }
            */
        }
        else if (strActionData == WIZ_SERVERACTION_HELP)
        {
            QString strUrl = WizOfficialApiEntry::standardCommandUrl("link");
            strUrl += "&name=docker";
            QDesktopServices::openUrl(strUrl);
        }

        resetUserList();
    }
}

void WizLoginDialog::showUserListMenu()
{
    QPoint point = ui->wgt_usercontainer->mapToGlobal(QPoint(0, ui->wgt_usercontainer->height()));

    m_menuUsers->popup(point);
}

void WizLoginDialog::showServerListMenu()
{
    QPoint point = ui->btn_selectServer->mapToGlobal(QPoint(0, ui->btn_selectServer->height()));

    m_menuServers->popup(point);
}

void WizLoginDialog::onRegisterAccountFinished(bool bFinish)
{

    WizAsyncApi* api = dynamic_cast<WizAsyncApi*>(sender());
    emit accountCheckFinished();
    if (bFinish) {
        m_newRegisterAccount = true;
        m_lineEditUserName->setText(m_lineEditNewUserName->text());
        m_lineEditPassword->setText(m_lineEditNewPassword->text());
        ui->cbx_remberPassword->setChecked(false);
        doAccountVerify();
    } else {
        static bool readVerificationCodeError = false;
        ui->label_passwordError->setText(api->lastErrorMessage());
        if (WIZ_ERROR_REGISTRATION_COUNT == api->lastErrorCode()) {

            if (readVerificationCodeError)
            {
                ui->label_passwordError->setText(tr("Verification code error"));
            }
            else
            {
                ui->label_passwordError->clear();
            }
            readVerificationCodeError = false;

            QString strCaptchaID, strCaptcha;
            if (doVerificationCodeCheck(strCaptchaID, strCaptcha))
            {
                readVerificationCodeError = true;
                WizAsyncApi* api = new WizAsyncApi(this);
                connect(api, SIGNAL(registerAccountFinished(bool)), SLOT(onRegisterAccountFinished(bool)));
                api->registerAccount(m_lineEditNewUserName->text(), m_lineEditNewPassword->text(), "", strCaptchaID, strCaptcha);
                emit accountCheckStart();
            }
        }
    }

    api->deleteLater();
}

void WizLoginDialog::on_cbx_remberPassword_toggled(bool checked)
{
    if (!checked)
        ui->cbx_autologin->setChecked(false);
}

void WizLoginDialog::on_cbx_autologin_toggled(bool checked)
{
    if (checked)
        ui->cbx_remberPassword->setChecked(true);
}

void WizLoginDialog::onUserNameEdited(const QString& arg1)
{
    Q_UNUSED(arg1);
    m_lineEditPassword->setText("");
    m_newRegisterAccount = false;
}

void WizLoginDialog::onDeleteUserRequest(const WizLocalUser& user)
{
    QAction* action = findActionInMenu(user.strGuid);
    if (action)
    {
        if (WizMessageBox::question(this, tr("Info"), tr("Remove user will delete local cache notes, are you sure to remove"
                                                      " user %1 ?").arg(user.strUserId)) == QMessageBox::Yes)
        {
            m_menuUsers->removeAction(action);
            QString folderPath = Utils::WizPathResolve::dataStorePath() + user.strDataFolderName;
            qDebug() << "remove folder path : " << folderPath;
            ::WizDeleteFolder(folderPath);
            m_lineEditUserName->clear();
            m_lineEditPassword->clear();
        }
    }
}

void WizLoginDialog::onUserSelected(const WizLocalUser& user)
{
    // clear old selected
    QAction* currentDefault = m_menuUsers->defaultAction();
    WizUserItemAction* userAction = dynamic_cast<WizUserItemAction*>(currentDefault);
    if (userAction)
    {
        userAction->setSelected(false);
    }

    //
    QAction* action = findActionInMenu(user.strGuid);
    if (action)
    {
        m_menuUsers->setDefaultAction(action);
        userAction = dynamic_cast<WizUserItemAction*>(action);
        if (userAction)
        {
            userAction->setSelected(true);
        }
        setUser(user.strGuid);
    }
}

void WizLoginDialog::onSNSPageUrlChanged(const QUrl& url)
{
    QString strUrl = url.toString();

    if (strUrl.contains("user_id") && strUrl.contains("user_guid") && strUrl.contains("access_token"))
    {
        emit snsLoginSuccess(strUrl);
        onSNSLoginSuccess(strUrl);
    }
}

void WizLoginDialog::onSNSLoginSuccess(const QString& strUrl)
{
    QStringList strList = strUrl.split("&");
    QString userIdString = "";
    QString userGuidString = "";
    QString encryptedPassword = "";

    QString keyOfUserId = "user_id=";
    QString keyOfUserGuid = "user_guid=";
    QString keyOfAccessToken = "access_token=";
    foreach (QString str, strList) {
        if (str.startsWith(keyOfUserId)) {
            userIdString = str.remove(0, keyOfUserId.length());
        } else if (str.startsWith(keyOfUserGuid)) {
            userGuidString = str.remove(0, keyOfUserGuid.length());
        } else if (str.startsWith(keyOfAccessToken)) {
            encryptedPassword = str.remove(0, keyOfAccessToken.length());
        }
    }
    userIdString = QByteArray::fromPercentEncoding(userIdString.toUtf8());
    m_lineEditUserName->setText(userIdString);
    QString strPassword(QByteArray::fromBase64(encryptedPassword.toUtf8()));
    m_lineEditPassword->setText(strPassword);
    m_loginUserGuid = userGuidString;

    accept();
}

void WizLoginDialog::onWizBoxResponse(const QString& boardAddress, const QString& serverAddress,
                                       const QString& responseMessage)
{
    qDebug() << "response from wizbox : " << responseMessage;

    if (responseMessage.isEmpty())
        return;

    m_wizBoxSearchingTimer.stop();
    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(responseMessage.toUtf8().constData(), d))
        return;

    if (!d.isMember("ip"))
    {
        TOLOG("no ip field");
        return;
    }
    //
    if (d["ip"].isNull())
        return;

    QString ip = QString::fromStdString(d["ip"].asString());
    QString iptype = QString::fromStdString(d["iptype"].asString());
    if (ip.isEmpty())
    {
        TOLOG(CString(responseMessage));
        TOLOG("no ip field");
        return;
    }

    if (iptype.isEmpty())
    {
        TOLOG(CString(responseMessage));
        TOLOG("no iptype field");
        return;
    }

    closeWizBoxUdpClient();
    //
//    if (iptype != "static")
//    {
//        CWizMessageBox::warning(this, tr("Info"), tr("Server ip should set to be static"));
//        m_searchingDialog->reject();
//        return;
//    }
    m_animationWaitingDialog->accept();
    m_lineEditServer->setText(ip);
    m_serverType = EnterpriseServer;
    WizCommonApiEntry::setEnterpriseServerIP(ip);

//    downloadLogoFromWizBox();
    downloadOEMSettingsFromWizBox();
}

void WizLoginDialog::onWizBoxSearchingTimeOut()
{
    m_wizBoxSearchingTimer.stop();
    m_animationWaitingDialog->reject();
    closeWizBoxUdpClient();
    //
    ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
        WizMessageBox::information(this, tr("Info"), tr("There is no server address, please input it."));
    });
}

bool WizLoginDialog::onOEMSettingsDownloaded(const QString& settings)
{
    if (settings.isEmpty())
        return false;
    //
    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(settings.toUtf8().constData(), d))
        return false;

    if (d.isMember("LogoConfig") && d["LogoConfig"].isMember("enable")
            && d["LogoConfig"]["enable"].asBool())
    {
        QString strUrl = QString::fromUtf8(d["LogoConfig"].isMember("common") ?
                    d["LogoConfig"]["common"].asString().c_str() : "");
        //
        if (strUrl.isEmpty())
        {
            qDebug() << "Can not found logo path in oem settings";
            return false;
        }

        if (!strUrl.startsWith("http"))
        {
            strUrl = "http://" + serverIp() + strUrl;
        }

        downloadLogoFromWizBox(strUrl);
    }

    return true;
}

void WizLoginDialog::onOEMLogoDownloaded(const QString& logoFile)
{
    if (!QFile::exists(logoFile))
        return;
    //
    setLogo(logoFile);
    m_oemLogoMap.insert(serverIp(), logoFile);
}

void WizLoginDialog::showOEMErrorMessage(const QString& stterror)
{
    if (EnterpriseServer == m_serverType && m_oemDownloader
            && m_oemDownloader->serverIp() == serverIp())
    {
        closeAnimationWaitingDialog();
        ui->label_passwordError->setText(stterror);
    }
}

void WizLoginDialog::onCheckServerLicenceFinished(bool result, const QString& settings)
{
    closeAnimationWaitingDialog();
    //
    if (result)
    {
        // update oem setttings
        onOEMSettingsDownloaded(settings);
        QString strAccountPath = Utils::WizPathResolve::dataStorePath() + userId() + "/";
        WizOEMSettings::updateOEMSettings(strAccountPath, settings);
        //
        emit accountCheckStart();
        doOnlineVerify();
    }
    else
    {
        WizMessageBox::warning(this, tr("Info"), tr("The user can't sigin in to the server, it had been signed in to other servers."));
    }
}

void WizLoginDialog::onWizLogInStateEntered()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->label_noaccount->setVisible(true);
    ui->btn_changeToLogin->setVisible(false);
    ui->btn_changeToSignin->setVisible(true);
    ui->wgt_serveroptioncontainer->setVisible(false);
    ui->btn_selectServer->setVisible(true);
    //
    ui->label_noaccount->setText(tr("No account yet,"));
    ui->label_passwordError->clear();
    ui->wgt_usercontainer->setFocus();
    //
    ui->label_separator4->setVisible(true);
    ui->btn_proxysetting->setVisible(true);

    //
    QString strThemeName = Utils::WizStyleHelper::themeName();
    QString strLoginBottomLineEditor = WizGetSkinResourceFileName(strThemeName, "loginBottomLineEditor");
    ui->wgt_passwordcontainer->setBackgroundImage(strLoginBottomLineEditor, QPoint(8, 8), WizColorLineEditorBackground);

    //
    m_serverType = WizServer;
    WizCommonApiEntry::setEnterpriseServerIP(WIZNOTE_API_SERVER);
    setSwicthServerSelectedAction(WIZ_SERVERACTION_CONNECT_WIZSERVER);
    setSwicthServerActionEnable(WIZ_SERVERACTION_SEARCH_SERVER, false);

    setLogo(m_wizLogoPath);
}

void WizLoginDialog::onWizBoxLogInStateEntered()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->label_noaccount->setVisible(false);
    ui->btn_changeToSignin->setVisible(false);
    ui->wgt_serveroptioncontainer->setVisible(true);
    //
    ui->label_passwordError->clear();
    ui->wgt_usercontainer->setFocus();
    //
//    ui->btn_snsLogin->setVisible(false);
//    ui->btn_fogetpass->setVisible(false);
    ui->btn_proxysetting->setVisible(false);
    ui->label_separator4->setVisible(false);
    //
    //
    QString strThemeName = Utils::WizStyleHelper::themeName();
    QString strLoginMidLineEditor = WizGetSkinResourceFileName(strThemeName, "loginMidLineEditor");
    ui->wgt_passwordcontainer->setBackgroundImage(strLoginMidLineEditor, QPoint(8, 8), WizColorLineEditorBackground);

    m_serverType = EnterpriseServer;
    setSwicthServerSelectedAction(WIZ_SERVERACTION_CONNECT_BIZSERVER);
    setSwicthServerActionEnable(WIZ_SERVERACTION_SEARCH_SERVER, true);

    // update logo if user server is oem server
    if (m_currentUserServerType == EnterpriseServer)
    {
        QString strAccountPath = Utils::WizPathResolve::dataStorePath() + m_lineEditNewUserName->text() + "/";
        if (WizOEMSettings::settingFileExists(strAccountPath))
        {
            WizOEMSettings settings(strAccountPath);
            QString strLogoPath = settings.logoPath();
            setLogo(strLogoPath);
        }
    }
}

void WizLoginDialog::onWizSignUpStateEntered()
{
    ui->btn_selectServer->setVisible(false);
    ui->stackedWidget->setCurrentIndex(1);
    ui->btn_changeToLogin->setVisible(true);
    ui->btn_changeToSignin->setVisible(false);
    ui->label_noaccount->setVisible(true);
    //
    ui->label_noaccount->setText(tr("Already got account,"));
    ui->label_passwordError->clear();
    ui->wgt_newUser->setFocus();

    m_serverType = WizServer;
}

void WizLoginDialog::onLogInCheckStart()
{
    enableLoginControls(false);
}

void WizLoginDialog::onLogInCheckEnd()
{
    enableLoginControls(true);
}

void WizLoginDialog::onSignUpCheckStart()
{
    enableSignUpControls(false);
}

void WizLoginDialog::onSignUpCheckEnd()
{
    enableSignUpControls(true);
}

void WizLoginDialog::loadDefaultUser()
{
    QSettings* settings = WizGlobal::globalSettings();
    QString strDefaultGuid = settings->value("Users/DefaultUserGuid").toString();
    if (!strDefaultGuid.isEmpty())
    {
        QString strUserAccount = WizGetLocalFolderName(m_userList, strDefaultGuid);
        if (!strUserAccount.isEmpty())
        {
            WizDatabase db;
            if (db.open(strUserAccount))
            {
                int serverType = db.meta("QT_WIZNOTE", "SERVERTYPE").toInt();
                if (EnterpriseServer == serverType)
                {
                    m_serverType = EnterpriseServer;
                    settings->setValue("Users/DefaultWizBoxUserGuid", strDefaultGuid);
                }
                else
                {
                    m_serverType = WizServer;
                    settings->setValue("Users/DefaultWizUserGuid", strDefaultGuid);
                }
            }
        }
    }
    else
    {
        m_serverType = WizServer;
        emit wizServerSelected();
    }

    resetUserList();
}

void WizLoginDialog::initSateMachine()
{
    QState* st = new QState();
    m_stateWizLogIn = new QState(st);
    m_stateWizBoxLogIn = new QState(st);
    m_stateWizSignUp = new QState(st);
    m_stateWizLogInCheck = new QState(st);
    m_stateWizBoxLogInCheck = new QState(st);
    m_stateSignUpCheck = new QState(st);
    if (EnterpriseServer == m_currentUserServerType)
    {
        st->setInitialState(m_stateWizBoxLogIn);
    }
    else
    {
        st->setInitialState(m_stateWizLogIn);
    }

    connect(m_stateWizLogIn, SIGNAL(entered()), SLOT(onWizLogInStateEntered()));
    connect(m_stateWizBoxLogIn, SIGNAL(entered()), SLOT(onWizBoxLogInStateEntered()));
    connect(m_stateWizSignUp, SIGNAL(entered()), SLOT(onWizSignUpStateEntered()));
    connect(m_stateWizLogInCheck, SIGNAL(entered()), SLOT(onLogInCheckStart()));
    connect(m_stateWizLogInCheck, SIGNAL(exited()), SLOT(onLogInCheckEnd()));
    connect(m_stateWizBoxLogInCheck, SIGNAL(entered()), SLOT(onLogInCheckStart()));
    connect(m_stateWizBoxLogInCheck, SIGNAL(exited()), SLOT(onLogInCheckEnd()));
    connect(m_stateSignUpCheck, SIGNAL(entered()), SLOT(onSignUpCheckStart()));
    connect(m_stateSignUpCheck, SIGNAL(exited()), SLOT(onSignUpCheckEnd()));
    // sign up / log in
    m_stateWizLogIn->addTransition(this, SIGNAL(wizBoxServerSelected()), m_stateWizBoxLogIn);
    m_stateWizBoxLogIn->addTransition(this, SIGNAL(wizServerSelected()), m_stateWizLogIn);
//    m_stateWizLogIn->addTransition(ui->btn_wizBoxLogIn, SIGNAL(clicked()), m_stateWizBoxLogIn);
//    m_stateWizBoxLogIn->addTransition(ui->btn_wizLogIn, SIGNAL(clicked()), m_stateWizLogIn);
    m_stateWizLogIn->addTransition(ui->btn_changeToSignin, SIGNAL(clicked()), m_stateWizSignUp);
    m_stateWizSignUp->addTransition(ui->btn_changeToLogin, SIGNAL(clicked()), m_stateWizLogIn);

    // start check
    m_stateWizLogIn->addTransition(this, SIGNAL(accountCheckStart()), m_stateWizLogInCheck);
    m_stateWizBoxLogIn->addTransition(this, SIGNAL(accountCheckStart()), m_stateWizBoxLogInCheck);
    m_stateWizSignUp->addTransition(this, SIGNAL(accountCheckStart()), m_stateSignUpCheck);
    // check finished
    m_stateWizLogInCheck->addTransition(this, SIGNAL(accountCheckFinished()), m_stateWizLogIn);
    m_stateWizBoxLogInCheck->addTransition(this, SIGNAL(accountCheckFinished()), m_stateWizBoxLogIn);
    m_stateSignUpCheck->addTransition(this, SIGNAL(accountCheckFinished()), m_stateWizSignUp);

    QStateMachine* stMachine = new QStateMachine(this);
    stMachine->addState(st);
    stMachine->setInitialState(st);
    stMachine->start();
}

void WizLoginDialog::initOEMDownloader()
{
    m_oemDownloader = new WizOEMDownloader(nullptr);
    connect(m_oemDownloader, SIGNAL(oemSettingsDownloaded(QString)),
            SLOT(onOEMSettingsDownloaded(QString)));
    connect(m_oemDownloader, SIGNAL(logoDownloaded(QString)),
            SLOT(onOEMLogoDownloaded(QString)));
    connect(m_oemDownloader, SIGNAL(errorMessage(QString)),
            SLOT(showOEMErrorMessage(QString)));
    connect(m_oemDownloader, SIGNAL(checkLicenceFinished(bool, QString)),
            SLOT(onCheckServerLicenceFinished(bool, QString)));
}

void WizLoginDialog::on_btn_snsLogin_clicked()
{
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("snspage");
    WizWebSettingsDialog dlg(strUrl, QSize(800, 480), 0);
    connect(dlg.web(), SIGNAL(urlChanged(QUrl)), SLOT(onSNSPageUrlChanged(QUrl)));
    connect(this, SIGNAL(snsLoginSuccess(QString)), &dlg, SLOT(accept()));
    dlg.exec();
}


QString WizOEMDownloader::_downloadOEMSettings()
{
    // get oem settings from server
    QNetworkAccessManager net;
    WizCommonApiEntry::setEnterpriseServerIP(m_server);
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("oem");
    QNetworkRequest req(strUrl);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply* reply = net.get(req);
    qDebug() << "get oem from server : " << strUrl;

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Download oem data failed!";
        emit errorMessage(tr("Can not find server %1").arg(m_server));
        reply->deleteLater();
        return "";
    }

    QString strResult = reply->readAll();
    reply->deleteLater();
    return strResult;
}

WizOEMDownloader::WizOEMDownloader(QObject* parent)
    : QObject(parent)
    , m_server(QString())
{
}

QString WizOEMDownloader::serverIp() const
{
    return m_server;
}

void WizOEMDownloader::setServerIp(const QString& ip)
{
    m_server = ip;
}

void WizOEMDownloader::downloadOEMLogo(const QString& strUrl)
{
    if (strUrl.isEmpty())
        return;

    QString strFileName = WizGenGUIDLowerCaseLetterOnly() + ".png";
    WizFileDownloader* downloader = new WizFileDownloader(strUrl, strFileName, "", true);
    QEventLoop loop;
    loop.connect(downloader, SIGNAL(downloadDone(QString,bool)), &loop, SLOT(quit()));
    //  just wait for 15 seconds
    QTimer::singleShot(15 * 1000, &loop, SLOT(quit()));
    downloader->startDownload();
    loop.exec();

    strFileName = Utils::WizPathResolve::tempPath() + strFileName;
    if (!QFile::exists(strFileName))
    {
        qDebug() << "Download logo image failed";
        return;
    }

    emit logoDownloaded(strFileName);
}

void WizOEMDownloader::downloadOEMSettings()
{
    QString settings = _downloadOEMSettings();
    qDebug() << "oem settings downloaded : " << settings;
    emit oemSettingsDownloaded(settings);
}

void WizOEMDownloader::checkServerLicence(const QString& licence)
{
    QString settings = _downloadOEMSettings();
    if (settings.isEmpty())
    {
        emit errorMessage(tr("Licence not found : %1").arg(settings.left(100)));
        qDebug() << "Can not find licence from oem settings";
        return;
    }
    //
    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(settings.toUtf8().constData(), d))
    {
        emit errorMessage(tr("Licence not found : %1").arg(settings.left(100)));
        qDebug() << "Can not find licence from oem settings";
        return;
    }

    if (d.isMember("licence"))
    {
        QString newLicence = QString::fromStdString(d["licence"].asString());

        QString strOldLicence = licence;
        // check wheather licence changed
        qDebug() << "compare licence : " << newLicence << "  with old licence : " << strOldLicence;
        if (newLicence.isEmpty() || (!strOldLicence.isEmpty() && strOldLicence != newLicence))
        {
            qDebug() << "compare licence failed";
            emit checkLicenceFinished(false, settings);
        }
        else
        {
            emit checkLicenceFinished(true, settings);
        }
    }
    else
    {
        emit errorMessage(tr("Licence not found : %1").arg(settings.left(100)));
        qDebug() << "Can not find licence from oem settings";
    }
}


WizUserItemAction::WizUserItemAction(const WizLocalUser& localUser, QMenu* parent)
    : QWidgetAction(parent)
    , m_userData(localUser)
    , m_menu(parent)
{
    m_widget = new WizActionWidget(m_userData.strUserId, parent);
    setDefaultWidget(m_widget);
    connect(m_widget, SIGNAL(delButtonClicked()), SLOT(on_delButtonClicked()));
    connect(m_widget, SIGNAL(widgetClicked()), SLOT(on_widgetClicked()));
}

WizLocalUser WizUserItemAction::getUserData()
{
    return m_userData;
}

void WizUserItemAction::setSelected(bool selected)
{
    m_widget->setSelected(selected);
}

void WizUserItemAction::on_delButtonClicked()
{
    emit userDeleteRequest(m_userData);
}

void WizUserItemAction::on_widgetClicked()
{
    m_menu->hide();
    emit userSelected(m_userData);
}


WizActionWidget::WizActionWidget(const QString& text, QWidget* parent)
    : QWidget(parent)
    , m_mousePress(false)
    , m_selected(false)
    , m_text(text)
{
    setMouseTracking(true);
    m_deleteButton = new QPushButton();
    QIcon deleteIcon = Utils::WizStyleHelper::loadIcon("loginCloseButton_hot");
    m_deleteButton->setIcon(deleteIcon);
    m_deleteButton->setStyleSheet("background:transparent;");
    connect(m_deleteButton, SIGNAL(clicked()), this, SIGNAL(delButtonClicked()));

    QHBoxLayout *main_layout = new QHBoxLayout();
    main_layout->addStretch();
    main_layout->addWidget(m_deleteButton);
    main_layout->setContentsMargins(5, 5, 10, 5);
    main_layout->setSpacing(5);
    setLayout(main_layout);
    setMinimumHeight(40);
    m_deleteButton->setVisible(false);
}

void WizActionWidget::setSelected(bool selected)
{
    m_selected = selected;
}

void WizActionWidget::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_mousePress = true;
    }
    update();
    QWidget::mousePressEvent(event);
}

void WizActionWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QPoint delLeftTop = m_deleteButton->mapToParent(QPoint(0, 0));
    if(m_mousePress && (rect().contains(event->pos()))
            && (event->pos().x() < delLeftTop.x()))
    {
        emit widgetClicked();
    }
    m_mousePress = false;
    update();
    QWidget::mouseReleaseEvent(event);
}

void WizActionWidget::enterEvent(QEvent* event)
{
    m_deleteButton->setVisible(true);
    QWidget::enterEvent(event);
}

void WizActionWidget::leaveEvent(QEvent* event)
{
    m_deleteButton->setVisible(false);
    QWidget::leaveEvent(event);
}

void WizActionWidget::paintEvent(QPaintEvent * event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);

    QRect rcText = opt.rect;
    rcText.setRight(rcText.right() - 40);
    rcText.setLeft(rcText.left() + 45);
    //
    if (isDarkMode()) {
        //
        if (opt.state & QStyle::State_MouseOver || m_selected)
        {
            p.fillRect(opt.rect, QBrush(QColor("#0058d1")));
            p.setPen(QColor("#ffffff"));
        }
        else
        {
            p.fillRect(opt.rect, QBrush("#272727"));
            p.setPen(QColor("#a6a6a6"));
        }
        //
    } else {
        if (opt.state & QStyle::State_MouseOver || m_selected)
        {
            p.fillRect(opt.rect, QBrush(QColor("#E7F5FF")));
        }
        else
        {
            p.fillRect(opt.rect, QBrush(Qt::white));
        }
        p.setPen(QColor("#5F5F5F"));
    }
    p.drawText(rcText, Qt::AlignVCenter | Qt::AlignLeft, m_text);
}
