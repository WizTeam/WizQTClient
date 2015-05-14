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
#include "sync/apientry.h"
#include "share/wizmisc.h"
#include "share/wizsettings.h"
#include "sync/wizkmxmlrpc.h"
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


CWizLoginDialog::CWizLoginDialog(const QString &strDefaultUserId, const QString &strLocale, QWidget *parent)
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
    ui->widget_titleBar->setVisible(false);
    //
    ui->layout_titleBar->removeWidget(ui->widget_titleBar);
    //
    QWidget* title = titleBar();
    title->setPalette(QPalette(QColor::fromRgb(0x43, 0xA6, 0xE8)));
    //

#endif
    QPainterPath path;
    QRectF rect = geometry();
    path.addRoundRect(rect, 5, 2);
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


    connect(m_menuUsers, SIGNAL(triggered(QAction*)), SLOT(userListMenuClicked(QAction*)));
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
    //
    setUsers(strDefaultUserId);
    //
    initSateMachine();
}

CWizLoginDialog::~CWizLoginDialog()
{
    delete ui;
    if (m_searchingDialog)
    {
        m_searchingDialog->deleteLater();
    }
    if (m_udpClient)
    {
        m_udpClient->deleteLater();
    }
}

QString CWizLoginDialog::userId() const
{
    return m_lineEditUserName->text();
}

QString CWizLoginDialog::password() const
{
    return m_lineEditPassword->text();
}

WizServerType CWizLoginDialog::serverType() const
{
    return m_serverType;
}

void CWizLoginDialog::setUsers(const QString &strDefault)
{
    CWizStdStringArray usersFolder;
    ::WizEnumFolders(Utils::PathResolve::dataStorePath(), usersFolder, 0);

    for(CWizStdStringArray::const_iterator it = usersFolder.begin();
        it != usersFolder.end(); it++)
    {
        QString strPath = *it;
        QString strUserId = ::WizFolderNameByPath(strPath);

        QRegExp mailRex("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
        mailRex.setCaseSensitivity(Qt::CaseInsensitive);

        if (!mailRex.exactMatch(strUserId))
            continue;

        if (!QFile::exists(strPath + "data/index.db"))
            continue;

        m_menuUsers->addAction(strUserId);
    }

    // set default user as default login entry.
    setUser(strDefault);
    QAction* action = findActionInMenu(strDefault);
    if (action)
    {
        m_menuUsers->setDefaultAction(action);
    }
}

void CWizLoginDialog::setUser(const QString &strUserId)
{
    CWizUserSettings userSettings(strUserId);
    QString strPassword = userSettings.password();

    m_lineEditUserName->setText(strUserId);
    if (strPassword.isEmpty())
    {
        m_lineEditPassword->clear();
        ui->cbx_remberPassword->setCheckState(Qt::Unchecked);
    } else
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
        ApiEntry::setEnterpriseServerIP(userSettings.enterpriseServerIP());
        emit wizBoxServerSelected();
    }
    else if ((m_currentUserServerType == NoServer && !userSettings.myWizMail().isEmpty()) ||
             m_currentUserServerType == WizServer)
    {
        m_currentUserServerType = WizServer;
        ApiEntry::setEnterpriseServerIP("");
        emit wizServerSelected();
    }
}

void CWizLoginDialog::doAccountVerify()
{
    CWizUserSettings userSettings(userId());

//    ControlWidgetsLocker locker;
//    locker.lockWidget(m_lineEditUserName);
//    locker.lockWidget(m_lineEditPassword);
//    locker.lockWidget(ui->cbx_autologin);
//    locker.lockWidget(ui->cbx_remberPassword);
//    locker.lockWidget(m_buttonLogin);
//    locker.lockWidget(ui->btn_changeToSignin);

    //  首先判断用户的服务器类型，如果是之前使用过但是没有记录服务器类型，则使用wiz服务器
    //  如果登录过企业服务则需要登录到企业服务器
    if (EnterpriseServer == m_serverType)
    {
        if (m_lineEditServer->text().isEmpty())
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

    qDebug() << "do account verify , server type : " << m_serverType;
    // FIXME: should verify password if network is available to avoid attack?
    if (password() != userSettings.password()) {
        Token::setUserId(userId());
        Token::setPasswd(password());
//        locker.releaseWidgets();
        // check server licence and update oem settings
        if (EnterpriseServer == m_serverType && !checkServerLicence(userSettings.serverLicence()))
            return;
        //
        emit accountCheckStart();
        doOnlineVerify();
        return;
    }

    if (updateUserProfile(false) && updateGlobalProfile()) {
//        locker.releaseWidgets();
        QDialog::accept();
    }
}

void CWizLoginDialog::doOnlineVerify()
{
    connect(Token::instance(), SIGNAL(tokenAcquired(QString)), SLOT(onTokenAcquired(QString)), Qt::QueuedConnection);
    Token::requestToken();
}

bool CWizLoginDialog::updateGlobalProfile()
{
    QSettings* settings = ExtensionSystem::PluginManager::globalSettings();
    settings->setValue("Users/DefaultUser", userId());
    return true;
}

bool CWizLoginDialog::updateUserProfile(bool bLogined)
{
    CWizUserSettings userSettings(userId());

    if(ui->cbx_autologin->checkState() == Qt::Checked) {
        userSettings.setAutoLogin(true);
    } else {
        userSettings.setAutoLogin(false);
    }

    if(ui->cbx_remberPassword->checkState() != Qt::Checked) {
        userSettings.setPassword();
    }

    if (bLogined) {
        if (ui->cbx_remberPassword->checkState() == Qt::Checked)
            userSettings.setPassword(::WizEncryptPassword(password()));

        CWizDatabase db;
        if (!db.Open(userId())) {
            //QMessageBox::critical(0, tr("Update user profile"), QObject::tr("Can not open database while update user profile"));
            ui->label_passwordError->setText(tr("Can not open database while update user profile"));
            return false;
        }

        db.SetUserInfo(Token::info());
        db.Close();
    }

    userSettings.setServerType(m_serverType);
    if (EnterpriseServer == m_serverType)
    {
        userSettings.setEnterpriseServerIP(m_lineEditServer->text());
        userSettings.setServerLicence(m_serverLicence);
    }

    return true;
}

void CWizLoginDialog::enableLoginControls(bool bEnable)
{
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
    m_lineEditNewUserName->setEnabled(bEnable);
    m_lineEditNewPassword->setEnabled(bEnable);
    m_lineEditRepeatPassword->setEnabled(bEnable);
    ui->btn_singUp->setEnabled(bEnable);
    ui->btn_changeToLogin->setEnabled(bEnable);
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

    QString strThemeName = Utils::StyleHelper::themeName();

    QString strlogo;
    // setup locale for welcome dialog
    if (strLocal != WizGetDefaultTranslatedLocal()) {
        strlogo= ::WizGetSkinResourceFileName(strThemeName, "loginLogoCn");
    } else {
        strlogo= ::WizGetSkinResourceFileName(strThemeName, "loginLogoUS");
    }
    ui->label_logo->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                        "background-position: center; background-repeat: no-repeat; background-color:#43A6E8}").arg(strlogo));
    ui->label_placehold->setStyleSheet(QString("QLabel {border: none;background-color:#43A6E8}"));

    //
#ifdef Q_OS_MAC
    QString strBtnCloseNormal = ::WizGetSkinResourceFileName(strThemeName, "loginCloseButton_normal");
    QString strBtnCloseHot = ::WizGetSkinResourceFileName(strThemeName, "loginCloseButton_hot");
    ui->btn_close->setStyleSheet(QString("QToolButton{ border-image:url(%1); height: 13px; width: 13px;}"
                                         "QToolButton:hover{ border-image:url(%2);}"
                                           "QToolButton:pressed { border-image:url(%3);}")
                                 .arg(strBtnCloseNormal).arg(strBtnCloseHot).arg(strBtnCloseHot));
    QString strGrayButton = ::WizGetSkinResourceFileName(strThemeName, "loginGrayButton");
    ui->btn_min->setStyleSheet(QString("QToolButton{ border-image:url(%1); height: 13px; width: 13px;}").arg(strGrayButton));
    ui->btn_max->setStyleSheet(QString("QToolButton{ border-image:url(%1); height: 13px; width: 13px;}").arg(strGrayButton));
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
    QString strIconPerson = WizGetSkinResourceFileName(strThemeName, "loginIconPerson");
    QString strIconKey = WizGetSkinResourceFileName(strThemeName, "loginIconKey");
    ui->wgt_usercontainer->setBackgroundImage(strLoginTopLineEditor, QPoint(8, 8));
    ui->wgt_usercontainer->setLeftIcon(strIconPerson);
    ui->wgt_usercontainer->setRightIcon(WizGetSkinResourceFileName(strThemeName, "loginLineEditorDownArrow"));
    m_lineEditUserName->setPlaceholderText("example@mail.com");

    ui->wgt_passwordcontainer->setBackgroundImage(strLoginMidLineEditor, QPoint(8, 8));
    ui->wgt_passwordcontainer->setLeftIcon(strIconKey);
    m_lineEditPassword->setEchoMode(QLineEdit::Password);
    m_lineEditPassword->setPlaceholderText(tr("Password"));

    ui->wgt_serveroptioncontainer->setBackgroundImage(strLoginBottomLineEditor, QPoint(8, 8));
    ui->wgt_serveroptioncontainer->setLeftIcon(strIconKey);
//    ui->wgt_serveroptioncontainer->setRightIcon(WizGetSkinResourceFileName(strThemeName, "loginLineEditorDownArrow"));
    m_lineEditServer->setText(tr("Please search or input your server ip"));
    m_lineEditServer->setPlaceholderText(tr("Please search or input your server ip"));

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
                                                     "color: rgba(255, 255, 255, 153); padding-right:15px; margin-right: 20px; margin-top: 5px}").arg(bg_switchserver_menu));
//    QString strWizBoxLogInOn = ::WizGetSkinResourceFileName(strThemeName, "action_logInWizBox_on");
//    ui->btn_wizBoxLogIn->setStyleSheet(QString("QPushButton{ border-image:url(%1); height: 16px; width: 16px;  margin-right: 25px; margin-top:5px;}"
//                                               "QPushButton:pressed{border-image:url(%2);}").arg(strWizBoxLogIn).arg(strWizBoxLogInOn));

    ui->btn_proxysetting->setStyleSheet(QString("QPushButton { border: none; background: none; "
                                                "color: #b1b1b1;margin-left:10px; margin-right:10px;  padding-bottom: 5px}"));
    ui->btn_fogetpass->setStyleSheet(QString("QPushButton { border: none; background: none; "
                                                 "color: #b1b1b1; margin-left: 10px; margin-right:10px; padding-bottom: 5px}"));
    ui->btn_snsLogin->setStyleSheet(QString("QPushButton { border: none; background: none; "
                                            "color: #b1b1b1; margin-left: 10px; margin-right:10px; padding-bottom: 5px}"));

    QString strLineSeparator = ::WizGetSkinResourceFileName(strThemeName, "loginLineSeparator");
    ui->label_separator3->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                                "background-position: center; background-repeat: no-repeat}").arg(strLineSeparator));
    ui->label_separator4->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                                "background-position: center; background-repeat: no-repeat}").arg(strLineSeparator));

    //
    ui->btn_changeToLogin->setVisible(false);
    ui->label_passwordError->setStyleSheet(QString("QLabel {border: none; padding-left: 25px; color: red;}"));
    ui->label_passwordError->setText("");

    m_menuUsers->setFixedWidth(ui->wgt_usercontainer->width());
    m_menuUsers->setStyleSheet("QMenu {background-color: #ffffff; border-style: solid; border-color: #43A6E8; border-width: 1px; color: #5F5F5F; padding: 0px 0px 0px 0px; menu-scrollable: 1;}"
                          "QMenu::item {padding: 10px 0px 10px 40px; background-color: #ffffff;}"
                          "QMenu::item:selected {background-color: #E7F5FF; }"
                          "QMenu::item:default {background-color: #E7F5FF; }");

    QString status_switchserver_selected = ::WizGetSkinResourceFileName(strThemeName, "status_switchserver_selected");
    //  font-family:'黑体','Microsoft YaHei UI', 'Microsoft YaHei UI' ;
    m_menuServers->setStyleSheet(QString("QMenu {background-color: #ffffff; border-style: solid; border-color: #3399ff; border-width: 1px; padding: 0px 0px 0px 0px;  menu-scrollable: 1;}"
                                 "QMenu::item {padding: 4px 10px 4px 25px; color: #000000;  background-color: #ffffff;}"
                                 "QMenu::item:selected {background-color: #E7F5FF; }"
                                 "QMenu::item:disabled {color: #999999; }"
                                 "QMenu::indicator { width: 16px; height: 16px; margin-left: 5px;} "
                                 "QMenu::indicator:non-exclusive:checked { image: url(%1); }").arg(status_switchserver_selected));
}

bool CWizLoginDialog::checkSingMessage()
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

QAction *CWizLoginDialog::findActionInMenu(const QString &strActName)
{
    QList<QAction*> actionList = m_menuUsers->actions();
    for (int i = 0; i < actionList.count(); i++)
    {
        if (actionList.at(i)->text() == strActName)
            return actionList.at(i);
    }
    return 0;
}

bool CWizLoginDialog::doVerificationCodeCheck(QString& strCaptchaID, QString& strCaptcha)
{
    strCaptchaID = QString::number(QDateTime::currentMSecsSinceEpoch()).right(8);
    strCaptchaID += WizGenGUIDLowerCaseLetterOnly().Right(6);
    QString strUrl = WizService::ApiEntry::captchaUrl(strCaptchaID);

    CWizVerificationCodeDialog dlg(this);
    if (dlg.verificationRequest(strUrl) == QDialog::Accepted)
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
    qDebug() << "call func form " << QThread::currentThread();
    m_wizBoxSearchingTimer.start(10 * 1000);
    showSearchingDialog();
}

void CWizLoginDialog::showSearchingDialog()
{
    if (!m_searchingDialog) {
        initSearchingDialog();
    }

    //
#ifdef Q_OS_MAC
    QPoint leftTop = geometry().topLeft();
    leftTop.setX(leftTop.x() + (width() - m_searchingDialog->width()) / 2);
    leftTop.setY(leftTop.y() + (height() - m_searchingDialog->height()) / 2);
    m_searchingDialog->move(leftTop);
#endif
    //
    if (m_searchingDialog->exec() == QDialog::Rejected)
    {
        m_wizBoxSearchingTimer.stop();
        qDebug() << "Searching cancel";
        closeWizBoxUdpClient();

        if (m_currentUserServerType != WizServer)
        {
            m_lineEditServer->clear();
            m_lineEditServer->setPlaceholderText(tr("There is no server address, please input it."));
            m_serverType = EnterpriseServer;
        }
    }
}

void CWizLoginDialog::initSearchingDialog()
{
    m_searchingDialog = new QDialog();
    m_searchingDialog->setWindowFlags(Qt::FramelessWindowHint);
    m_searchingDialog->setFixedSize(150, 100);
    QPalette pl = m_searchingDialog->palette();
    pl.setColor(QPalette::Window, QColor(0, 0, 0, 200));
    m_searchingDialog->setPalette(pl);
    m_searchingDialog->setAutoFillBackground(true);
    m_searchingDialog->setWindowOpacity(0.7);

    QHBoxLayout* closeLayout = new QHBoxLayout();
    QToolButton* closeButton = new QToolButton(m_searchingDialog);
    QString strBtnCloseNormal = Utils::StyleHelper::skinResourceFileName("linuxlogindialoclose_white");
    QString strBtnCloseHover = Utils::StyleHelper::skinResourceFileName("linuxwindowclose_on");
    QString strBtnCloseDown = Utils::StyleHelper::skinResourceFileName("linuxwindowclose_selected"); // ::WizGetSkinResourceFileName(strThemeName, "linuxwindowclose_selected");
    closeButton->setStyleSheet(QString("QToolButton{ border-image:url(%1); height: 16px; width: 16px;}"
                                                     "QToolButton:hover{ border-image:url(%2); height: 16px; width: 16px;}"
                                                     "QToolButton:pressed{ border-image:url(%3); height: 16px; width: 16px;}")
                                             .arg(strBtnCloseNormal).arg(strBtnCloseHover).arg(strBtnCloseDown));
    closeLayout->setContentsMargins(0, 0, 0, 0);
    closeLayout->addStretch();
    closeLayout->addWidget(closeButton);
    connect(closeButton, SIGNAL(clicked()), m_searchingDialog, SLOT(reject()));
    //
    QLabel* labelSearching = new QLabel(m_searchingDialog);
    labelSearching->setFixedSize(32, 32);
    QMovie* movie =new QMovie(m_searchingDialog);
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
    QLabel* labelText = new QLabel(m_searchingDialog);
    labelText->setText(tr("Finding Service...."));
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
   m_searchingDialog->setLayout(layout);

    movie->start();
}

void CWizLoginDialog::startWizBoxUdpClient()
{
    if (!m_udpClient)
    {
        m_udpClient = new CWizUdpClient();        
        connect(this, SIGNAL(wizBoxSearchRequest(int,QString)),
                m_udpClient, SLOT(boardcast(int,QString)), Qt::QueuedConnection);

        m_udpThread = new QThread(this);
        qDebug() << "create thread : " << m_udpThread;
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

bool CWizLoginDialog::checkServerLicence(const QString& strOldLicence)
{
    if (m_lineEditServer->text().isEmpty())
    {
        CWizMessageBox::warning(this, tr("Info"), tr("There is no server address, please input it."));
        return false;
    }
    ApiEntry::setEnterpriseServerIP(m_lineEditServer->text());

    // get licence from server    
    QNetworkAccessManager net;
    QString strUrl = ApiEntry::standardCommandUrl("oem", false);    
    QNetworkReply* reply = net.get(QNetworkRequest(strUrl));
    qDebug() << "get oem from server : " << strUrl;

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Download oem data failed!";
        ui->label_passwordError->setText(tr("Can not find server."));
        reply->deleteLater();
        return false;
    }

    QString strResult = reply->readAll();
    reply->deleteLater();
    rapidjson::Document d;
    d.Parse<0>(strResult.toUtf8().constData());

    qDebug() << "oem settings : " << strResult;

    if (!d.FindMember("licence"))
    {
        ui->label_passwordError->setText(tr("Can not get licence from server."));
        qDebug() << "Can not find licence from oem";
        return false;
    }
    m_serverLicence = QString::fromUtf8(d.FindMember("licence")->value.GetString());

    // check wheather licence changed
    qDebug() << "compare licence : " << m_serverLicence << "  with old licence : " << strOldLicence;
    if (m_serverLicence.isEmpty() || (!strOldLicence.isEmpty() && strOldLicence != m_serverLicence))
    {
        CWizMessageBox::warning(this, tr("Info"), tr("The user can't sigin in to the server, it had been signed in to other servers."));
        return false;
    } else
    {
        // update oem setttings
        CWizOEMSettings::updateOEMSettings(userId(), strResult);
    }

    return true;
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
    QString strUrl = WizService::ApiEntry::standardCommandUrl("forgot_password", false);
    QDesktopServices::openUrl(QUrl(strUrl));
}

void CWizLoginDialog::on_btn_login_clicked()
{
    if (userId().isEmpty()) {
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
    if (checkSingMessage())
    {

//    #if defined Q_OS_MAC
//        QString strCode = "129ce11c";
//    #elif defined Q_OS_LINUX
//        QString strCode = "7abd8f4a";
//    #else
//        QString strCode = "8480c6d7";
//    #endif

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
    Token::instance()->disconnect(this);

    qDebug() << " check user online : " << m_currentUserServerType << m_serverType << " api " << ApiEntry::syncUrl();

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
                ui->label_passwordError->setText(tr("Connection is not available, please check your network connection."));
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
            //QMessageBox::critical(0, tr("Verify account failed"), );
            if (errorTokenInvalid == nErrorCode)
            {
                ui->label_passwordError->setText(tr("User name or password is not correct!"));
            }
            else
            {
                ui->label_passwordError->setText(Token::lastErrorMessage());
            }
            return;
        }
    }

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

void CWizLoginDialog::userListMenuClicked(QAction *action)
{
    if (action)
    {
        m_menuUsers->setDefaultAction(action);
        setUser(action->text());
    }
}

void CWizLoginDialog::serverListMenuClicked(QAction* action)
{
    if (action)
    {
        QString strActionData = action->data().toString();
        if (strActionData == WIZ_SERVERACTION_CONNECT_WIZSERVER)
        {
//            if (EnterpriseServer == m_currentUserServerType)
//            {
//                QMessageBox::warning(0, tr("Info"), tr("The user name can't switch to WizNote,"
//                                                       " it was signed in to enterprise server."));
//                return;
//            }
            m_serverType = WizServer;
            emit wizServerSelected();
        }
        else if (strActionData == WIZ_SERVERACTION_CONNECT_BIZSERVER)
        {
//            if (WizServer == m_currentUserServerType)
//            {
//                QMessageBox::warning(0, tr("Info"), tr("The user name can't switch to enterprise server,"
//                                                       " it was signed in to WizNote."));
//                return;
//            }
//            CWizUserSettings userSettings(userId());
//            qDebug() << "server type : " << userSettings.serverType();
//            qDebug() << "my wiz ; " << userSettings.myWizMail();
//            if (EnterpriseServer != userSettings.serverType() && !userSettings.myWizMail().isEmpty())
//            {
//                QMessageBox::warning(0, tr("Info"), tr("The user name can't switch to enterprise server,"
//                                                       " it was signed in to WizNote."));
//                return;
//            }
            m_serverType = EnterpriseServer;
            emit wizBoxServerSelected();
            searchWizBoxServer();
        }
        else if (strActionData == WIZ_SERVERACTION_SEARCH_SERVER)
        {
            m_serverType = EnterpriseServer;
            emit wizBoxServerSelected();
            searchWizBoxServer();
        }
        else if (strActionData == WIZ_SERVERACTION_HELP)
        {
            QString strUrl = WizService::ApiEntry::standardCommandUrl("link", true);
            strUrl += "&name=wiz-box-search-help.html";
            QDesktopServices::openUrl(strUrl);
        }
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
        m_lineEditUserName->setText(m_lineEditNewUserName->text());
        m_lineEditPassword->setText(m_lineEditNewPassword->text());
        ui->cbx_remberPassword->setChecked(false);
        doAccountVerify();
    } else {
        ui->label_passwordError->setText(api->lastErrorMessage());
        if (WIZ_ERROR_REGISTRATION_COUNT == api->lastErrorCode()) {
            QString strCaptchaID, strCaptcha;
            if (doVerificationCodeCheck(strCaptchaID, strCaptcha))
            {
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
    m_lineEditUserName->setText(userIdString);
    QString strPassword(QByteArray::fromBase64(encryptedPassword.toUtf8()));
    m_lineEditPassword->setText(strPassword);

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
    if (iptype != "static")
    {
        CWizMessageBox::warning(this, tr("Info"), tr("Server ip should set to be static"));
        return;
    }
    m_searchingDialog->accept();
    m_lineEditServer->setText(ip);
    m_serverType = EnterpriseServer;
    ApiEntry::setEnterpriseServerIP(ip);
}

void CWizLoginDialog::onWizBoxSearchingTimeOut()
{
    m_wizBoxSearchingTimer.stop();
    m_searchingDialog->reject();
    closeWizBoxUdpClient();
    CWizMessageBox::information(this, tr("Info"), tr("There is no server address, please input it."));
}

void CWizLoginDialog::onWizLogInStateEntered()
{
    qDebug() << "CWizLoginDialog::onWizLogInStateEntered()";
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
    ApiEntry::setEnterpriseServerIP("");
    setSwicthServerSelectedAction(WIZ_SERVERACTION_CONNECT_WIZSERVER);
    setSwicthServerActionEnable(WIZ_SERVERACTION_SEARCH_SERVER, false);
}

void CWizLoginDialog::onWizBoxLogInStateEntered()
{
    qDebug() << "CWizLoginDialog::onWizBoxLogInStateEntered()";
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
}

void CWizLoginDialog::onWizSignUpStateEntered()
{
    qDebug() << "CWizLoginDialog::onWizSignUpStateEntered()";
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
    qDebug() << "CWizLoginDialog::onLogInCheckStart()";
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
//    QHistoryState* stHistory = new QHistoryState(st);
//    stHistory->setDefaultState(m_stateWizLogIn);

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

void CWizLoginDialog::on_btn_snsLogin_clicked()
{
    QString strUrl = WizService::ApiEntry::standardCommandUrl("snspage", false);
    CWizWebSettingsDialog dlg(strUrl, QSize(800, 480), 0);
    connect(dlg.webVew(), SIGNAL(urlChanged(QUrl)), SLOT(onSNSPageUrlChanged(QUrl)));
    connect(this, SIGNAL(snsLoginSuccess(QString)), &dlg, SLOT(accept()));
    dlg.exec();
}
