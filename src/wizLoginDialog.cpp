#include "wizLoginDialog.h"
#include "ui_wizLoginDialog.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QBitmap>
#include <QToolButton>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QWebView>
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

#include "utils/stylehelper.h"
#include "utils/pathresolve.h"
#include "utils/logger.h"
#include "sync/apientry.h"
#include "share/wizmisc.h"
#include "share/wizsettings.h"
#include "share/wizAnalyzer.h"
#include "share/wizObjectDataDownloader.h"
#include "sync/wizKMServer.h"
#include "sync/asyncapi.h"
#include "sync/token.h"
#include "wizproxydialog.h"
#include <extensionsystem/pluginmanager.h>
#include "widgets/wizVerificationCodeDialog.h"
#include "wizWebSettingsDialog.h"
#include "share/wizui.h"
#include "wiznotestyle.h"
#include "share/wizUDPClient.h"
#include "rapidjson/document.h"
#include "share/wizMessageBox.h"
#include "wizOEMSettings.h"

using namespace WizService;

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


CWizLoginDialog::CWizLoginDialog(const QString &strLocale, const QList<WizLocalUser>& localUsers, QWidget *parent)
#ifdef Q_OS_MAC
    : QDialog(parent)
#else
    : CWizShadowWindow<QDialog>(parent)
#endif
    , ui(new Ui::wizLoginWidget)
    , m_menuUsers(new QMenu(this))
    , m_menuServers(new QMenu(this))
    , m_udpClient(0)
    , m_serverType(WizServer)
    , m_animationWaitingDialog(nullptr)
    , m_oemDownloader(nullptr)
    , m_oemThread(nullptr)
    , m_userList(localUsers)
    , m_newRegisterAccount(false)
{

#ifdef Q_OS_MAC
    setWindowFlags(Qt::CustomizeWindowHint);
    ui->setupUi(this);
#else
    layoutTitleBar();
    //
    setCanResize(false);
    //
    QWidget* uiWidget = new QWidget(clientWidget());
    clientLayout()->addWidget(uiWidget);
    ui->setupUi(uiWidget);
    QRect rcUI = uiWidget->geometry();
    setMinimumSize(rcUI.width() + 20, rcUI.height() + 20);
    //
    //  init style for wizbox
    ui->btn_selectServer->setMaximumHeight(20);
    ui->layout_titleBar->setContentsMargins(0, 0, 0, 0);
    ui->widget_titleBar->layout()->setContentsMargins(0, 0, 0, 0);
    ui->widget_titleBar->layout()->setSpacing(0);
    ui->label_logo->setMinimumHeight(80);
    ui->btn_max->setVisible(false);
    ui->btn_min->setVisible(false);
    ui->btn_close->setVisible(false);
    //
    CWizTitleBar* title = titleBar();
    title->setPalette(QPalette(QColor::fromRgb(0x43, 0xA6, 0xE8)));
    title->setContentsMargins(QMargins(0, 2, 2 ,0));
    rootWidget()->setContentsMargins(10, 0, 10, 10);
    //

#endif
    QPainterPath path;
    QRectF rect = geometry();
    path.addRoundRect(rect, 4, 1);
    QPolygon polygon= path.toFillPolygon().toPolygon();
    QRegion region(polygon);
    setMask(region);

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
    connect(ui->btn_selectServer, SIGNAL(clicked()), SLOT(showServerListMenu()));
    connect(m_lineEditUserName, SIGNAL(textEdited(QString)), SLOT(onUserNameEdited(QString)));
    //

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
    QAction* actionWizBoxServer = m_menuServers->addAction(tr("Sign in to Enterprise Private Server (WizBox)"));
    actionWizBoxServer->setData(WIZ_SERVERACTION_CONNECT_BIZSERVER);
    actionWizBoxServer->setCheckable(true);
    m_menuServers->addAction(tr("Find WizBox..."))->setData(WIZ_SERVERACTION_SEARCH_SERVER);
    m_menuServers->addAction(tr("About WizBox..."))->setData(WIZ_SERVERACTION_HELP);
    m_menuServers->setDefaultAction(actionWizServer);

    //
    applyElementStyles(strLocale);

    loadDefaultUser();
    //
    initSateMachine();
}

CWizLoginDialog::~CWizLoginDialog()
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
    if (m_oemDownloader)
    {
        QObject::disconnect(m_oemDownloader, 0, 0, 0);
        m_oemDownloader->deleteLater();
        connect(m_oemThread, SIGNAL(finished()), m_oemThread, SLOT(deleteLater()));
        m_oemThread->quit();
    }
}

QString CWizLoginDialog::userId() const
{
    return m_lineEditUserName->text();
}

QString CWizLoginDialog::loginUserGuid() const
{
    return m_loginUserGuid;
}

QString CWizLoginDialog::password() const
{
    return m_lineEditPassword->text();
}

QString CWizLoginDialog::serverIp() const
{
    return m_lineEditServer->text();
}

WizServerType CWizLoginDialog::serverType() const
{
    return m_serverType;
}

void CWizLoginDialog::resetUserList()
{
    m_menuUsers->clear();

    for (WizLocalUser user : m_userList)
    {
        if (user.nUserType == m_serverType || (WizServer == m_serverType && user.nUserType == 0))
        {
//            QAction* action = m_menuUsers->addAction(user.strUserId);
            CWizUserItemAction* userItem = new CWizUserItemAction(user, m_menuUsers);
            connect(userItem, SIGNAL(userDeleteRequest(WizLocalUser)), SLOT(onDeleteUserRequest(WizLocalUser)));
            connect(userItem, SIGNAL(userSelected(WizLocalUser)), SLOT(onUserSelected(WizLocalUser)));
            userItem->setData(user.strGuid);
            m_menuUsers->addAction(userItem);
        }
    }
    //
    QSettings* settings = ExtensionSystem::PluginManager::globalSettings();
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
        CWizUserItemAction* userAction = dynamic_cast<CWizUserItemAction*>(action);
        if (userAction)
        {
            userAction->setSelected(true);
        }
    }
}

void CWizLoginDialog::setUser(const QString &strUserGuid)
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

    CWizUserSettings userSettings(strAccountFolder);
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
        CommonApiEntry::setEnterpriseServerIP(userSettings.enterpriseServerIP());
        // update oem settings
        downloadOEMSettingsFromWizBox();
    }
    else if ((m_currentUserServerType == NoServer && !userSettings.myWizMail().isEmpty()) ||
             m_currentUserServerType == WizServer)
    {
        m_currentUserServerType = WizServer;
        CommonApiEntry::setEnterpriseServerIP(WIZNOTE_API_SERVER);
    }
}

void CWizLoginDialog::doAccountVerify()
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

void CWizLoginDialog::doOnlineVerify()
{
    Token::setUserId(userId());
    Token::setPasswd(password());

    connect(Token::instance(), SIGNAL(tokenAcquired(QString)), SLOT(onTokenAcquired(QString)), Qt::QueuedConnection);
    Token::requestToken();
    showAnimationWaitingDialog(tr("Connecting..."));
}

bool CWizLoginDialog::updateGlobalProfile()
{
    QSettings* settings = ExtensionSystem::PluginManager::globalSettings();
    settings->setValue("Users/DefaultUserGuid", m_loginUserGuid);
    return true;
}

bool CWizLoginDialog::updateUserProfile(bool bLogined)
{
    QString localUserId = WizGetLocalUserId(m_userList, m_loginUserGuid);
    QString strUserAccount = WizGetLocalFolderName(m_userList, m_loginUserGuid);
    qDebug() << "udate user profile , userid  " << localUserId;
    strUserAccount.isEmpty() ? (strUserAccount = userId()) : 0;
    CWizUserSettings userSettings(strUserAccount);

    if(ui->cbx_autologin->checkState() == Qt::Checked) {
        userSettings.setAutoLogin(true);
    } else {
        userSettings.setAutoLogin(false);
    }

    if(ui->cbx_remberPassword->checkState() != Qt::Checked) {
        userSettings.setPassword();
    }

    CWizDatabase db;
    if (!db.Open(strUserAccount)) {
        ui->label_passwordError->setText(tr("Can not open database while update user profile"));
        return false;
    }

    if (bLogined)
    {
        if (ui->cbx_remberPassword->checkState() == Qt::Checked)
            userSettings.setPassword(::WizEncryptPassword(password()));

        db.SetUserInfo(Token::info());
    }
    db.SetMeta("ACCOUNT", "USERID", userId());
    db.Close();

    userSettings.setServerType(m_serverType);
    if (EnterpriseServer == m_serverType)
    {
        userSettings.setEnterpriseServerIP(serverIp());
        userSettings.setServerLicence(m_serverLicence);

        //
        QString logoFile = m_oemLogoMap.value(serverIp());
        qDebug() << "update oem logo path : " << logoFile;
        QString strAccountPath = Utils::PathResolve::dataStorePath() + m_lineEditNewUserName->text() + "/";
        if (logoFile.isEmpty() || !CWizOEMSettings::settingFileExists(strAccountPath))
            return true;

        //
        CWizOEMSettings settings(strAccountPath);
        QString strOldPath = settings.logoPath();
        if (!strOldPath.isEmpty())
        {
            QDir dir;
            dir.remove(strOldPath);
            QFile::copy(logoFile, strOldPath);
        }
        else
        {
            QString strNewPath = Utils::PathResolve::cachePath() + "logo/";
            Utils::PathResolve::ensurePathExists(strNewPath);
            strNewPath = strNewPath + WizGenGUIDLowerCaseLetterOnly() + ".png";
            QFile::copy(logoFile, strNewPath);
            settings.setLogoPath(strNewPath);
        }
    }

    return true;
}

void CWizLoginDialog::enableLoginControls(bool bEnable)
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

void CWizLoginDialog::enableSignUpControls(bool bEnable)
{
    //NOTE: 很奇怪的问题，修改编辑框的状态会导致sns登录按钮被触发
//    m_lineEditNewUserName->setEnabled(bEnable);
//    m_lineEditNewPassword->setEnabled(bEnable);
//    m_lineEditRepeatPassword->setEnabled(bEnable);
    ui->btn_singUp->setEnabled(bEnable);
    ui->btn_changeToLogin->setEnabled(bEnable);
}

bool CWizLoginDialog::isNewRegisterAccount()
{
    return m_newRegisterAccount;
}

#ifdef Q_OS_MAC
void CWizLoginDialog::mousePressEvent(QMouseEvent *event)
{
    m_mousePoint = event->globalPos();
}

void CWizLoginDialog::mouseMoveEvent(QMouseEvent *event)
{
    if(m_mousePoint != QPoint(0, 0))
    {
        move(geometry().x() + event->globalPos().x() - m_mousePoint.x(), geometry().y() + event->globalPos().y() - m_mousePoint.y());
        m_mousePoint = event->globalPos();
    }
}

void CWizLoginDialog::mouseReleaseEvent(QMouseEvent *)
{
    m_mousePoint = QPoint(0, 0);
}

#endif


void CWizLoginDialog::on_btn_close_clicked()
{
    qApp->quit();
}

void CWizLoginDialog::applyElementStyles(const QString &strLocal)
{
    ui->stackedWidget->setCurrentIndex(0);
    setFixedWidth(354);
    ui->widget_titleBar->setFixedHeight(40);

    QString strThemeName = Utils::StyleHelper::themeName();

    // setup locale for welcome dialog
    if (strLocal != WizGetDefaultTranslatedLocal()) {
        m_wizLogoPath= ::WizGetSkinResourceFileName(strThemeName, "loginLogoCn");
    } else {
        m_wizLogoPath= ::WizGetSkinResourceFileName(strThemeName, "loginLogoUS");
    }
    ui->label_logo->setMinimumWidth(190);   // use fixed logo size for oem
    ui->label_logo->setStyleSheet(QString("QLabel {border: none; image: url(%1);"
                                        "background-position: center; background-repeat: no-repeat; background-color:#43A6E8}").arg(m_wizLogoPath));
    ui->label_placehold->setStyleSheet(QString("QLabel {border: none;background-color:#43A6E8}"));

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
    CWizTitleBar* m_titleBar = qobject_cast<CWizTitleBar*>(titleBar());
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
        m_titleBar->closeButton()->setFixedSize(16, 16);
    }
#endif

    //
    ui->cbx_autologin->setStyleSheet(QString("QCheckBox{background:none;border:none;}"
                                             "QCheckBox:focus{background:none;border:none;}"
                                             "QCheckBox::pressed{background:none;border:none;}"));
    ui->cbx_remberPassword->setStyleSheet(QString("QCheckBox{background:none;border:none;}"
                                             "QCheckBox:focus{background:none;border:none;}"
                                             "QCheckBox::pressed{background:none;border:none;}"));
    //
    QString strLoginTopLineEditor = WizGetSkinResourceFileName(strThemeName, "loginTopLineEditor");
    QString strLoginMidLineEditor = WizGetSkinResourceFileName(strThemeName, "loginMidLineEditor");
    QString strLoginBottomLineEditor = WizGetSkinResourceFileName(strThemeName, "loginBottomLineEditor");
    bool isHighPix = ::WizIsHighPixel();
    QString strIconPerson = WizGetSkinResourceFileName(strThemeName, "loginIconPerson" + (isHighPix ? "@2x" : QString()));
    QString strIconKey = WizGetSkinResourceFileName(strThemeName, "loginIconKey" + (isHighPix ? "@2x" : QString()));
    QString strIconServer = WizGetSkinResourceFileName(strThemeName, "loginIconServer" + (isHighPix ? "@2x" : QString()));
    ui->wgt_usercontainer->setBackgroundImage(strLoginTopLineEditor, QPoint(8, 8));
    ui->wgt_usercontainer->setLeftIcon(strIconPerson);
    ui->wgt_usercontainer->setRightIcon(WizGetSkinResourceFileName(strThemeName, "loginLineEditorDownArrow" + (isHighPix ? "@2x" : QString())));
    m_lineEditUserName->setPlaceholderText("example@mail.com");

    ui->wgt_passwordcontainer->setBackgroundImage(strLoginMidLineEditor, QPoint(8, 8));
    ui->wgt_passwordcontainer->setLeftIcon(strIconKey);
    m_lineEditPassword->setEchoMode(QLineEdit::Password);
    m_lineEditPassword->setPlaceholderText(tr("Password"));

    ui->wgt_serveroptioncontainer->setBackgroundImage(strLoginBottomLineEditor, QPoint(8, 8));
    ui->wgt_serveroptioncontainer->setLeftIcon(strIconServer);
    m_lineEditServer->setPlaceholderText(tr("Please search or input your server IP"));

    ui->wgt_newUser->setBackgroundImage(strLoginTopLineEditor, QPoint(8, 8));
    ui->wgt_newUser->setLeftIcon(strIconPerson);
    m_lineEditNewUserName->setPlaceholderText(tr("Please input email as your account"));

    ui->wgt_newPassword->setBackgroundImage(strLoginMidLineEditor, QPoint(8, 8));
    ui->wgt_newPassword->setLeftIcon(strIconKey);
    m_lineEditNewPassword->setPlaceholderText(tr("Please enter your password"));
    m_lineEditNewPassword->setEchoMode(QLineEdit::Password);

    ui->wgt_passwordRepeat->setBackgroundImage(strLoginBottomLineEditor, QPoint(8, 8));
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
                              QColor("#ffffff"), QColor("b1b1b1"));
    m_buttonLogin->setText(tr("Login"));
    m_buttonLogin->setEnabled(false);

    m_buttonSignUp->setButtonStyle(strBtnNormal, strBtnHover, strBtnDown, strBtnDisable, QColor("#ffffff"),
                                   QColor("#ffffff"), QColor("b1b1b1"));
    m_buttonSignUp->setText(tr("Create Account"));
    m_buttonSignUp->setEnabled(false);
    //
    QString strSeparator = ::WizGetSkinResourceFileName(strThemeName, "loginSeparator");
    ui->label_separator2->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                               "background-position: center; background-repeat: no-repeat}").arg(strSeparator));
   //
    ui->label_noaccount->setStyleSheet(QString("QLabel {border: none; color: #5f5f5f;}"));
#ifdef Q_OS_MAC
    ui->btn_changeToSignin->setStyleSheet(QString("QPushButton { border: 1px; background: none; "
                                                 "color: #43a6e8;  padding-left: 10px; padding-bottom: 3px}"));
    ui->btn_changeToLogin->setStyleSheet(QString("QPushButton { border: 1px; background: none; "
                                                 "color: #43a6e8;  padding-left: 10px; padding-bottom: 3px}"));
#else
    ui->btn_changeToSignin->setStyleSheet(QString("QPushButton { border: 1px; background: none; "
                                                 "color: #43a6e8;  padding-left: 10px; padding-bottom: 0px}"));
    ui->btn_changeToLogin->setStyleSheet(QString("QPushButton { border: 1px; background: none; "
                                                 "color: #43a6e8; padding-left: 10px; padding-bottom: 0px}"));
#endif

    QString bg_switchserver_menu = ::WizGetSkinResourceFileName(strThemeName, "bg_switchserver_menu");
    ui->btn_selectServer->setStyleSheet(QString("QPushButton { border: none; background-image: url(%1);"
                                                "background-position: right center; background-repeat: no-repeat;"
                                                     "color: rgba(255, 255, 255, 153); padding:0px; padding-right:15px; margin-right: 20px; margin-top: 5px}").arg(bg_switchserver_menu));
//    QString strWizBoxLogInOn = ::WizGetSkinResourceFileName(strThemeName, "action_logInWizBox_on");
//    ui->btn_wizBoxLogIn->setStyleSheet(QString("QPushButton{ border-image:url(%1); height: 16px; width: 16px;  margin-right: 25px; margin-top:5px;}"
//                                               "QPushButton:pressed{border-image:url(%2);}").arg(strWizBoxLogIn).arg(strWizBoxLogInOn));

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
    m_menuUsers->setStyleSheet("QMenu {background-color: #ffffff; border-style: solid; border-color: #43A6E8; border-width: 1px; color: #5F5F5F; padding: 0px 0px 0px 0px; menu-scrollable: 1;}");
//                          "QMenu::item {padding: 10px 0px 10px 40px; background-color: #ffffff;}"
//                          "QMenu::item:selected {background-color: #E7F5FF; }"
//                          "QMenu::item:default {background-color: #E7F5FF; }");

    QString status_switchserver_selected = ::WizGetSkinResourceFileName(strThemeName, "status_switchserver_selected");
    m_menuServers->setStyleSheet(QString("QMenu {background-color: #ffffff; border-style: solid; border-color: #3399ff; border-width: 1px; padding: 0px 0px 0px 0px;  menu-scrollable: 1;}"
                                 "QMenu::item {padding: 4px 10px 4px 25px; color: #000000;  background-color: #ffffff;}"
                                 "QMenu::item:selected {background-color: #E7F5FF; }"
                                 "QMenu::item:disabled {color: #999999; }"
                                 "QMenu::indicator { width: 16px; height: 16px; margin-left: 5px;} "
                                 "QMenu::indicator:non-exclusive:checked { image: url(%1); }").arg(status_switchserver_selected));
}

bool CWizLoginDialog::checkSignMessage()
{
    QRegExp mailRex("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
    mailRex.setCaseSensitivity(Qt::CaseInsensitive);
    QString strThemeName = Utils::StyleHelper::themeName();
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

QAction *CWizLoginDialog::findActionInMenu(const QString &strActData)
{
    QList<QAction*> actionList = m_menuUsers->actions();
    for (int i = 0; i < actionList.count(); i++)
    {
        if (actionList.at(i)->data().toString() == strActData)
            return actionList.at(i);
    }
    return 0;
}

bool CWizLoginDialog::doVerificationCodeCheck(QString& strCaptchaID, QString& strCaptcha)
{
    strCaptchaID = QString::number(QDateTime::currentMSecsSinceEpoch()).right(8);
    strCaptchaID += WizGenGUIDLowerCaseLetterOnly().Right(6);    

    CWizVerificationCodeDialog dlg(this);
    if (dlg.verificationRequest(strCaptchaID) == QDialog::Accepted)
    {
        strCaptcha = dlg.getVerificationCode();
        return true;
    }
    return false;
}

void CWizLoginDialog::searchWizBoxServer()
{
    qDebug() << "start findWizBoxServer ";
    startWizBoxUdpClient();

    emit wizBoxSearchRequest(WIZBOX_PROT, "find wizbox");
    m_wizBoxSearchingTimer.start(10 * 1000);
    showSearchingDialog();
}

void CWizLoginDialog::showSearchingDialog()
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

void CWizLoginDialog::initAnimationWaitingDialog(const QString& text)
{
    m_animationWaitingDialog = new QDialog();
    m_animationWaitingDialog->setWindowFlags(Qt::FramelessWindowHint);
    m_animationWaitingDialog->setFixedSize(150, 100);
    QPalette pl = m_animationWaitingDialog->palette();
    m_animationWaitingDialog->setStyleSheet("QDialog{background-color:#000000;} ");
    pl.setColor(QPalette::Window, QColor(0, 0, 0, 200));
    m_animationWaitingDialog->setPalette(pl);
    m_animationWaitingDialog->setAutoFillBackground(true);
    m_animationWaitingDialog->setWindowOpacity(0.7);

    QHBoxLayout* closeLayout = new QHBoxLayout();
    QToolButton* closeButton = new QToolButton(m_animationWaitingDialog);
    QString strBtnCloseNormal = Utils::StyleHelper::skinResourceFileName("linuxlogindialoclose_white");
    QString strBtnCloseHover = Utils::StyleHelper::skinResourceFileName("linuxwindowclose_on");
    QString strBtnCloseDown = Utils::StyleHelper::skinResourceFileName("linuxwindowclose_selected"); // ::WizGetSkinResourceFileName(strThemeName, "linuxwindowclose_selected");
    closeButton->setStyleSheet(QString("QToolButton{ image:url(%1); background:transparent; padding:0px; border:0px; height: 16px; width: 16px;}"
                                                     "QToolButton:hover{ image:url(%2); height: 16px; width: 16px;}"
                                                     "QToolButton:pressed{ image:url(%3); height: 16px; width: 16px;}")
                                             .arg(strBtnCloseNormal).arg(strBtnCloseHover).arg(strBtnCloseDown));
    closeLayout->setContentsMargins(0, 0, 0, 0);
    closeLayout->addStretch();
    closeLayout->addWidget(closeButton);
    connect(closeButton, SIGNAL(clicked()), m_animationWaitingDialog, SLOT(reject()));
    //
    QLabel* labelSearching = new QLabel(m_animationWaitingDialog);
    labelSearching->setFixedSize(32, 32);
    QMovie* movie =new QMovie(m_animationWaitingDialog);
    movie->setFileName(":/searching.gif");
    labelSearching->setMovie(movie);
    QHBoxLayout* labelLayout = new QHBoxLayout();
    labelLayout->setContentsMargins(0, 0, 0, 0);
    labelLayout->addStretch();
    labelLayout->addWidget(labelSearching);
    labelLayout->addStretch();
    QHBoxLayout* textLayout = new QHBoxLayout();
    textLayout->setContentsMargins(0, 5, 0, 0);
    textLayout->addStretch();
    QLabel* labelText = new QLabel(m_animationWaitingDialog);
    labelText->setText(text);
    QPalette plText = labelText->palette();
    plText.setColor(QPalette::WindowText, QColor(255, 255, 255, 200));
    labelText->setPalette(plText);
    textLayout->addWidget(labelText);
    textLayout->addStretch();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 15);
    layout->addLayout(closeLayout);
    layout->addLayout(labelLayout);
    layout->addLayout(textLayout);
   m_animationWaitingDialog->setLayout(layout);

   movie->start();
}

int CWizLoginDialog::showAnimationWaitingDialog(const QString& text)
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

void CWizLoginDialog::closeAnimationWaitingDialog()
{
    if (m_animationWaitingDialog && m_animationWaitingDialog->isVisible())
    {
        m_animationWaitingDialog->accept();
    }
}

void CWizLoginDialog::startWizBoxUdpClient()
{
    if (!m_udpClient)
    {
        m_udpClient = new CWizUdpClient();
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

void CWizLoginDialog::closeWizBoxUdpClient()
{
    disconnect(m_udpClient, SIGNAL(udpResponse(QString,QString,QString)),
               this, SLOT(onWizBoxResponse(QString,QString,QString)));
    QMetaObject::invokeMethod(m_udpClient, "closeUdpConnections", Qt::QueuedConnection);
    m_udpThread->quit();
}

void CWizLoginDialog::checkServerLicence()
{
    qDebug() << "current server ip : " << serverIp();
    if (serverIp().isEmpty())
    {
        CWizMessageBox::warning(this, tr("Info"), tr("There is no server address, please input it."));
        return;
    }

    if (!m_oemDownloader)
    {
        initOEMDownloader();
    }
    m_oemDownloader->setServerIp(serverIp());
    WizService::CommonApiEntry::setEnterpriseServerIP(serverIp());

//    downloadOEMSettingsFromWizBox();
    CWizUserSettings userSettings(userId());
    QString strOldLicence = userSettings.serverLicence();
    emit checkServerLicenceRequest(strOldLicence);

    showAnimationWaitingDialog(tr("Connecting...."));
}

void CWizLoginDialog::setSwicthServerSelectedAction(const QString& strActionData)
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

void CWizLoginDialog::setSwicthServerActionEnable(const QString& strActionData, bool bEnable)
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

void CWizLoginDialog::downloadLogoFromWizBox(const QString& strUrl)
{
    if (!m_oemDownloader)
    {
        initOEMDownloader();
    }
    qDebug() << "download logo request in main thread : " << QThread::currentThreadId();
//    QTimer::singleShot(0, m_oemDownloader, SLOT(downloadOEMLogo()));
    emit logoDownloadRequest(strUrl);
}

void CWizLoginDialog::downloadOEMSettingsFromWizBox()
{
    if (serverIp().isEmpty())
        return;
    //
    if (!m_oemDownloader)
    {
        initOEMDownloader();
    }
    m_oemDownloader->setServerIp(serverIp());
    qDebug() << "main thread : " << QThread::currentThreadId();
    QTimer::singleShot(0, m_oemDownloader, SLOT(downloadOEMSettings()));
}

void CWizLoginDialog::setLogo(const QString& logoPath)
{
    ui->label_logo->setStyleSheet(QString("QLabel {border: none; image: url(%1);"
                                        "background-position: center; background-repeat: no-repeat; background-color:#43A6E8}").
                                  arg(logoPath.isEmpty() ? m_wizLogoPath : logoPath));
}

void CWizLoginDialog::checkLocalUser(const QString& strAccountFolder, const QString& strUserGUID)
{
    qDebug() << "do local account verify , folder path : " << strAccountFolder;
    CWizUserSettings userSettings(strAccountFolder);

    //  首先判断用户的服务器类型，如果是之前使用过但是没有记录服务器类型，则使用wiz服务器
    //  如果登录过企业服务则需要登录到企业服务器
    if (EnterpriseServer == m_serverType)
    {
        if (serverIp().isEmpty())
        {
            CWizMessageBox::warning(this, tr("Info"), tr("There is no server address, please input it."));
            return;
        }

        if (userSettings.enterpriseServerIP().isEmpty() && !userSettings.myWizMail().isEmpty())
        {
            CWizMessageBox::warning(this, tr("Info"), tr("The user name can't switch to enterprise server, it was signed in to WizNote."));
            return;
        }
        // clear proxy for app
//        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    }
    else if (WizServer == m_serverType && !userSettings.enterpriseServerIP().isEmpty())
    {
        CWizMessageBox::warning(this, tr("Info"), tr("The user name can't switch to WizNote, it was signed in to enterprise server. "));
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
            WizService::CommonApiEntry::setEnterpriseServerIP(serverIp());
        }
    }

    //
    m_loginUserGuid = strUserGUID;
    if (updateUserProfile(false) && updateGlobalProfile())
    {
        QDialog::accept();
    }
}


void CWizLoginDialog::on_btn_changeToSignin_clicked()
{

}

void CWizLoginDialog::on_btn_changeToLogin_clicked()
{
    ui->btn_changeToLogin->setVisible(false);
    ui->btn_changeToSignin->setVisible(true);
    ui->label_noaccount->setText(tr("No account yet,"));
    ui->label_passwordError->clear();
    ui->stackedWidget->setCurrentIndex(0);
    ui->wgt_usercontainer->setFocus();
}

void CWizLoginDialog::on_btn_proxysetting_clicked()
{
    ProxyDialog dlg;
    dlg.exec();
}

void CWizLoginDialog::on_btn_fogetpass_clicked()
{
    QString strUrl = WizService::CommonApiEntry::makeUpUrlFromCommand("forgot_password");
    QDesktopServices::openUrl(QUrl(strUrl));
}

void CWizLoginDialog::on_btn_login_clicked()
{
    if (m_lineEditUserName->text().isEmpty()) {
        ui->label_passwordError->setText(tr("Please enter user id"));
        return;
    }

    if (password().isEmpty()) {
        ui->label_passwordError->setText(tr("Please enter user password"));
        return;
    }

    doAccountVerify();
}

void CWizLoginDialog::on_btn_singUp_clicked()
{
    if (checkSignMessage())
    {        
        AsyncApi* api = new AsyncApi(this);
        connect(api, SIGNAL(registerAccountFinished(bool)), SLOT(onRegisterAccountFinished(bool)));
        api->registerAccount(m_lineEditNewUserName->text(), m_lineEditNewPassword->text(), "");
        emit accountCheckStart();
    }
}

void CWizLoginDialog::onLoginInputChanged()
{
    ui->label_passwordError->clear();
    bool bInputFinished =  !m_lineEditUserName->text().isEmpty() && !m_lineEditPassword->text().isEmpty();
    ui->btn_login->setEnabled(bInputFinished);
    m_currentUserServerType = NoServer;
}

void CWizLoginDialog::onTokenAcquired(const QString &strToken)
{
    closeAnimationWaitingDialog();
    //
    Token::instance()->disconnect(this);

    qDebug() << "on tonken acquired : " << strToken;


    emit accountCheckFinished();
    if (strToken.isEmpty())
    {
        int nErrorCode = Token::lastErrorCode();
        // network unavailable
        if (QNetworkReply::ProtocolUnknownError == nErrorCode)
        {
            CWizUserSettings userSettings(userId());
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
            else
            {
                ui->label_passwordError->setText(Token::lastErrorMessage());
            }
            return;
        }
    }

    m_loginUserGuid = Token::info().strUserGUID;
    if (updateUserProfile(true) && updateGlobalProfile())
        QDialog::accept();
}

void CWizLoginDialog::onSignUpInputDataChanged()
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

void CWizLoginDialog::serverListMenuClicked(QAction* action)
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
            searchWizBoxServer();
            action->setChecked(true);
        }
        else if (strActionData == WIZ_SERVERACTION_SEARCH_SERVER)
        {
            m_serverType = EnterpriseServer;
            emit wizBoxServerSelected();
            searchWizBoxServer();

            QList<QAction*> actionList = m_menuServers->actions();
            for (QAction* act : actionList)
            {
                if (act->data().toString() == WIZ_SERVERACTION_CONNECT_BIZSERVER)
                {
                    act->setChecked(true);
                    break;
                }
            }
        }
        else if (strActionData == WIZ_SERVERACTION_HELP)
        {
            QString strUrl = WizService::WizApiEntry::standardCommandUrl("link");
            strUrl += "&name=wiz-box-search-help.html";
            QDesktopServices::openUrl(strUrl);
        }

        resetUserList();
    }
}

void CWizLoginDialog::showUserListMenu()
{
    QPoint point = ui->wgt_usercontainer->mapToGlobal(QPoint(0, ui->wgt_usercontainer->height()));

    m_menuUsers->popup(point);
}

void CWizLoginDialog::showServerListMenu()
{
    QPoint point = ui->btn_selectServer->mapToGlobal(QPoint(0, ui->btn_selectServer->height()));

    m_menuServers->popup(point);
}

void CWizLoginDialog::onRegisterAccountFinished(bool bFinish)
{

    AsyncApi* api = dynamic_cast<AsyncApi*>(sender());
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
                AsyncApi* api = new AsyncApi(this);
                connect(api, SIGNAL(registerAccountFinished(bool)), SLOT(onRegisterAccountFinished(bool)));
                api->registerAccount(m_lineEditNewUserName->text(), m_lineEditNewPassword->text(), "", strCaptchaID, strCaptcha);
                emit accountCheckStart();
            }
        }
    }

    api->deleteLater();
}

void CWizLoginDialog::on_cbx_remberPassword_toggled(bool checked)
{
    if (!checked)
        ui->cbx_autologin->setChecked(false);
}

void CWizLoginDialog::on_cbx_autologin_toggled(bool checked)
{
    if (checked)
        ui->cbx_remberPassword->setChecked(true);
}

void CWizLoginDialog::onUserNameEdited(const QString& arg1)
{
    Q_UNUSED(arg1);
    m_lineEditPassword->setText("");
    m_newRegisterAccount = false;
}

void CWizLoginDialog::onDeleteUserRequest(const WizLocalUser& user)
{
    QAction* action = findActionInMenu(user.strGuid);
    if (action)
    {
        if (CWizMessageBox::question(this, tr("Info"), tr("Remove user will delete local cache notes, are you sure to remove"
                                                      " user %1 ?").arg(user.strUserId)) == QMessageBox::Yes)
        {
            m_menuUsers->removeAction(action);
            QString folderPath = Utils::PathResolve::dataStorePath() + user.strDataFolderName;
            qDebug() << "remove folder path : " << folderPath;
            ::WizDeleteFolder(folderPath);
            m_lineEditUserName->clear();
            m_lineEditPassword->clear();
        }
    }
}

void CWizLoginDialog::onUserSelected(const WizLocalUser& user)
{
    // clear old selected
    QAction* currentDefault = m_menuUsers->defaultAction();
    CWizUserItemAction* userAction = dynamic_cast<CWizUserItemAction*>(currentDefault);
    if (userAction)
    {
        userAction->setSelected(false);
    }

    //
    QAction* action = findActionInMenu(user.strGuid);
    if (action)
    {
        m_menuUsers->setDefaultAction(action);
        userAction = dynamic_cast<CWizUserItemAction*>(action);
        if (userAction)
        {
            userAction->setSelected(true);
        }
        setUser(user.strGuid);
    }
}

void CWizLoginDialog::onSNSPageUrlChanged(const QUrl& url)
{
    QString strUrl = url.toString();

    if (strUrl.contains("user_id") && strUrl.contains("user_guid") && strUrl.contains("access_token"))
    {
        emit snsLoginSuccess(strUrl);
        onSNSLoginSuccess(strUrl);
    }
}

void CWizLoginDialog::onSNSLoginSuccess(const QString& strUrl)
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

void CWizLoginDialog::onWizBoxResponse(const QString& boardAddress, const QString& serverAddress,
                                       const QString& responseMessage)
{
    qDebug() << "response from wizbox : " << responseMessage;

    m_wizBoxSearchingTimer.stop();
    rapidjson::Document d;
    d.Parse<0>(responseMessage.toUtf8().constData());

    QString ip = QString::fromUtf8(d.FindMember("ip")->value.GetString());
    QString iptype = QString::fromUtf8(d.FindMember("iptype")->value.GetString());
    if (ip.isEmpty())
    {
        TOLOG(CString(responseMessage));
        TOLOG(_T("no ip field"));
        return;
    }

    if (iptype.isEmpty())
    {
        TOLOG(CString(responseMessage));
        TOLOG(_T("no iptype field"));
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
    CommonApiEntry::setEnterpriseServerIP(ip);

//    downloadLogoFromWizBox();
    downloadOEMSettingsFromWizBox();
}

void CWizLoginDialog::onWizBoxSearchingTimeOut()
{
    m_wizBoxSearchingTimer.stop();
    m_animationWaitingDialog->reject();
    closeWizBoxUdpClient();
    CWizMessageBox::information(this, tr("Info"), tr("There is no server address, please input it."));
}

bool CWizLoginDialog::onOEMSettingsDownloaded(const QString& settings)
{
    if (settings.isEmpty())
        return false;
    //
    rapidjson::Document d;
    d.Parse<0>(settings.toUtf8().constData());

    if (d.FindMember("LogoConfig") && d.FindMember("LogoConfig")->value.FindMember("enable")
            && d.FindMember("LogoConfig")->value.FindMember("enable")->value.GetBool())
    {
        QString strUrl = d.FindMember("LogoConfig")->value.FindMember("common") ?
                    d.FindMember("LogoConfig")->value.FindMember("common")->value.GetString() : "";
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

void CWizLoginDialog::onOEMLogoDownloaded(const QString& logoFile)
{
    if (!QFile::exists(logoFile))
        return;
    //
    setLogo(logoFile);
    m_oemLogoMap.insert(serverIp(), logoFile);
}

void CWizLoginDialog::showOEMErrorMessage(const QString& stterror)
{
    if (EnterpriseServer == m_serverType && m_oemDownloader
            && m_oemDownloader->serverIp() == serverIp())
    {
        closeAnimationWaitingDialog();
        ui->label_passwordError->setText(stterror);
    }
}

void CWizLoginDialog::onCheckServerLicenceFinished(bool result, const QString& settings)
{
    closeAnimationWaitingDialog();
    //
    if (result)
    {
        // update oem setttings
        onOEMSettingsDownloaded(settings);
        QString strAccountPath = Utils::PathResolve::dataStorePath() + userId() + "/";
        CWizOEMSettings::updateOEMSettings(strAccountPath, settings);
        //
        emit accountCheckStart();
        doOnlineVerify();
    }
    else
    {
        CWizMessageBox::warning(this, tr("Info"), tr("The user can't sigin in to the server, it had been signed in to other servers."));
    }
}

void CWizLoginDialog::onWizLogInStateEntered()
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
    QString strThemeName = Utils::StyleHelper::themeName();
    QString strLoginBottomLineEditor = WizGetSkinResourceFileName(strThemeName, "loginBottomLineEditor");
    ui->wgt_passwordcontainer->setBackgroundImage(strLoginBottomLineEditor, QPoint(8, 8));

    //
    m_serverType = WizServer;
    CommonApiEntry::setEnterpriseServerIP(WIZNOTE_API_SERVER);
    setSwicthServerSelectedAction(WIZ_SERVERACTION_CONNECT_WIZSERVER);
    setSwicthServerActionEnable(WIZ_SERVERACTION_SEARCH_SERVER, false);

    setLogo(m_wizLogoPath);    
}

void CWizLoginDialog::onWizBoxLogInStateEntered()
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
    QString strThemeName = Utils::StyleHelper::themeName();
    QString strLoginMidLineEditor = WizGetSkinResourceFileName(strThemeName, "loginMidLineEditor");
    ui->wgt_passwordcontainer->setBackgroundImage(strLoginMidLineEditor, QPoint(8, 8));

    m_serverType = EnterpriseServer;
    setSwicthServerSelectedAction(WIZ_SERVERACTION_CONNECT_BIZSERVER);
    setSwicthServerActionEnable(WIZ_SERVERACTION_SEARCH_SERVER, true);

    // update logo if user server is oem server
    if (m_currentUserServerType == EnterpriseServer)
    {
        QString strAccountPath = Utils::PathResolve::dataStorePath() + m_lineEditNewUserName->text() + "/";
        if (CWizOEMSettings::settingFileExists(strAccountPath))
        {
            CWizOEMSettings settings(strAccountPath);
            QString strLogoPath = settings.logoPath();
            setLogo(strLogoPath);
        }
    }    
}

void CWizLoginDialog::onWizSignUpStateEntered()
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

void CWizLoginDialog::onLogInCheckStart()
{
    enableLoginControls(false);
}

void CWizLoginDialog::onLogInCheckEnd()
{
    enableLoginControls(true);
}

void CWizLoginDialog::onSignUpCheckStart()
{
    enableSignUpControls(false);
}

void CWizLoginDialog::onSignUpCheckEnd()
{
    enableSignUpControls(true);
}

void CWizLoginDialog::loadDefaultUser()
{
    QSettings* settings = ExtensionSystem::PluginManager::globalSettings();
    QString strDefaultGuid = settings->value("Users/DefaultUserGuid").toString();
    if (!strDefaultGuid.isEmpty())
    {
        QString strUserAccount = WizGetLocalFolderName(m_userList, strDefaultGuid);
        if (!strUserAccount.isEmpty())
        {
            CWizDatabase db;
            if (db.Open(strUserAccount))
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

void CWizLoginDialog::initSateMachine()
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

void CWizLoginDialog::initOEMDownloader()
{
    m_oemDownloader = new CWizOEMDownloader(nullptr);
    connect(m_oemDownloader, SIGNAL(oemSettingsDownloaded(QString)),
            SLOT(onOEMSettingsDownloaded(QString)));
    connect(m_oemDownloader, SIGNAL(logoDownloaded(QString)),
            SLOT(onOEMLogoDownloaded(QString)));
    connect(m_oemDownloader, SIGNAL(errorMessage(QString)),
            SLOT(showOEMErrorMessage(QString)));
    connect(m_oemDownloader, SIGNAL(checkLicenceFinished(bool, QString)),
            SLOT(onCheckServerLicenceFinished(bool, QString)));
    connect(this, SIGNAL(logoDownloadRequest(QString)),
            m_oemDownloader, SLOT(downloadOEMLogo(QString)));
    connect(this, SIGNAL(checkServerLicenceRequest(QString)),
            m_oemDownloader, SLOT(onCheckServerLicenceRequest(QString)));

    m_oemThread = new QThread();
    m_oemThread->start();
    m_oemDownloader->moveToThread(m_oemThread);
}

void CWizLoginDialog::on_btn_snsLogin_clicked()
{
    QString strUrl = WizService::CommonApiEntry::makeUpUrlFromCommand("snspage");
    CWizWebSettingsDialog dlg(strUrl, QSize(800, 480), 0);
    connect(dlg.webVew(), SIGNAL(urlChanged(QUrl)), SLOT(onSNSPageUrlChanged(QUrl)));
    connect(this, SIGNAL(snsLoginSuccess(QString)), &dlg, SLOT(accept()));
    dlg.exec();
}


QString CWizOEMDownloader::_downloadOEMSettings()
{
    // get oem settings from server
    QNetworkAccessManager net;
    CommonApiEntry::setEnterpriseServerIP(m_server);
    QString strUrl = CommonApiEntry::makeUpUrlFromCommand("oem");
    QNetworkReply* reply = net.get(QNetworkRequest(strUrl));
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

CWizOEMDownloader::CWizOEMDownloader(QObject* parent)
    : QObject(parent)
    , m_server(QString())
{
}

QString CWizOEMDownloader::serverIp() const
{
    return m_server;
}

void CWizOEMDownloader::setServerIp(const QString& ip)
{
    m_server = ip;
}

void CWizOEMDownloader::downloadOEMLogo(const QString& strUrl)
{
    if (strUrl.isEmpty())
        return;

    QString strFileName = WizGenGUIDLowerCaseLetterOnly() + ".png";
    CWizFileDownloader* downloader = new CWizFileDownloader(strUrl, strFileName, "", true);
    QEventLoop loop;
    loop.connect(downloader, SIGNAL(downloadDone(QString,bool)), &loop, SLOT(quit()));
    //  just wait for 15 seconds
    QTimer::singleShot(15 * 1000, &loop, SLOT(quit()));
    downloader->startDownload();
    loop.exec();

    strFileName = Utils::PathResolve::tempPath() + strFileName;
    if (!QFile::exists(strFileName))
    {
        qDebug() << "Download logo image failed";
        return;
    }

    emit logoDownloaded(strFileName);
}

void CWizOEMDownloader::downloadOEMSettings()
{
    QString settings = _downloadOEMSettings();
    qDebug() << "oem settings downloaded : " << settings;
    emit oemSettingsDownloaded(settings);
}

void CWizOEMDownloader::onCheckServerLicenceRequest(const QString& licence)
{
    QString settings = _downloadOEMSettings();
    if (settings.isEmpty())
    {
        return;
    }    
    //
    rapidjson::Document d;
    d.Parse<0>(settings.toUtf8().constData());

    if (d.FindMember("licence"))
    {
        QString newLicence = QString::fromUtf8(d.FindMember("licence")->value.GetString());

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


CWizUserItemAction::CWizUserItemAction(const WizLocalUser& localUser, QMenu* parent)
    : QWidgetAction(parent)
    , m_userData(localUser)
    , m_menu(parent)
{
    m_widget = new CWizActionWidget(m_userData.strUserId, parent);
    setDefaultWidget(m_widget);
    connect(m_widget, SIGNAL(delButtonClicked()), SLOT(on_delButtonClicked()));
    connect(m_widget, SIGNAL(widgetClicked()), SLOT(on_widgetClicked()));
}

WizLocalUser CWizUserItemAction::getUserData()
{
    return m_userData;
}

void CWizUserItemAction::setSelected(bool selected)
{
    m_widget->setSelected(selected);
}

void CWizUserItemAction::on_delButtonClicked()
{
    emit userDeleteRequest(m_userData);
}

void CWizUserItemAction::on_widgetClicked()
{
    m_menu->hide();
    emit userSelected(m_userData);
}


CWizActionWidget::CWizActionWidget(const QString& text, QWidget* parent)
    : QWidget(parent)
    , m_mousePress(false)
    , m_selected(false)
    , m_text(text)
{
    setMouseTracking(true);
    m_deleteButton = new QPushButton();
    QIcon deleteIcon = Utils::StyleHelper::loadIcon("loginCloseButton_hot");
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

void CWizActionWidget::setSelected(bool selected)
{
    m_selected = selected;
}

void CWizActionWidget::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_mousePress = true;
    }
    update();
    QWidget::mousePressEvent(event);
}

void CWizActionWidget::mouseReleaseEvent(QMouseEvent* event)
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

void CWizActionWidget::enterEvent(QEvent* event)
{
    m_deleteButton->setVisible(true);
    QWidget::enterEvent(event);
}

void CWizActionWidget::leaveEvent(QEvent* event)
{
    m_deleteButton->setVisible(false);
    QWidget::leaveEvent(event);
}

void CWizActionWidget::paintEvent(QPaintEvent * event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);    

    QRect rcText = opt.rect;
    rcText.setRight(rcText.right() - 40);
    rcText.setLeft(rcText.left() + 45);
    if (opt.state & QStyle::State_MouseOver || m_selected)
    {
        p.fillRect(opt.rect, QBrush(QColor("#E7F5FF")));
    }
    else
    {
        p.fillRect(opt.rect, QBrush(Qt::white));
    }
    p.setPen(QColor("#5F5F5F"));
    p.drawText(rcText, Qt::AlignVCenter | Qt::AlignLeft, m_text);
}
