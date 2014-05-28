#include "wizLoginWidget.h"
#include "ui_wizLoginWidget.h"
#include "utils/stylehelper.h"
#include "utils/pathresolve.h"
#include "sync/apientry.h"
#include "share/wizmisc.h"
#include "share/wizsettings.h"
#include "sync/wizkmxmlrpc.h"
#include "sync/asyncapi.h"
#include "sync/token.h"
#include <extensionsystem/pluginmanager.h>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QBitmap>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>


#include "share/wizui.h"
#include "wiznotestyle.h"

using namespace WizService;




CWizLoginDialog::CWizLoginDialog(const QString &strDefaultUserId, const QString &strLocale, QWidget *parent)
#ifdef Q_OS_MAC
    : QDialog(parent)
#else
    : CWizShadowWindow(parent)
#endif
    , ui(new Ui::wizLoginWidget)
    , m_menu(new QMenu(this))
{
#ifdef Q_OS_MAC
    setWindowFlags(Qt::CustomizeWindowHint);
    ui->setupUi(this);
#else
    setCanResize(false);
    //
    QWidget* uiWidget = new QWidget(clientWidget());
    clientLayout()->addWidget(uiWidget);
    ui->setupUi(uiWidget);
    //
    ui->widget_titleBar->setVisible(false);
    //
    ui->layout_titleBar->removeWidget(ui->widget_titleBar);
    //
    QWidget* title = titleBar();
    title->setPalette(QPalette(QColor::fromRgb(0x43, 0xA6, 0xE8)));
    //
#endif
    m_lineEditUserName = ui->wgt_usercontainer->edit();
    m_lineEditPassword = ui->wgt_passwordcontainer->edit();
    m_buttonLogin = ui->btn_login;
    m_lineEditNewUserName = ui->wgt_newUser->edit();
    m_lineEditNewPassword = ui->wgt_newPassword->edit();
    m_lineEditRepeatPassword = ui->wgt_passwordRepeat->edit();
    m_buttonSignIn = ui->btn_singin;

    ui->wgt_newUser->setAutoClearRightIcon(true);
    ui->wgt_newPassword->setAutoClearRightIcon(true);
    ui->wgt_passwordRepeat->setAutoClearRightIcon(true);

    setElementStyles();


    connect(m_menu, SIGNAL(triggered(QAction*)), SLOT(userListMenuClicked(QAction*)));

    connect(m_lineEditNewPassword, SIGNAL(textChanged(QString)), SLOT(onSignUpInputDataChanged()));
    connect(m_lineEditNewUserName, SIGNAL(textChanged(QString)), SLOT(onSignUpInputDataChanged()));
    connect(m_lineEditRepeatPassword, SIGNAL(textChanged(QString)), SLOT(onSignUpInputDataChanged()));
    connect(m_lineEditPassword, SIGNAL(textChanged(QString)), SLOT(onLoginInputChanged()));
    connect(m_lineEditUserName, SIGNAL(textChanged(QString)), SLOT(onLoginInputChanged()));
    connect(ui->wgt_usercontainer, SIGNAL(rightIconClicked()), SLOT(showUserListMenu()));
    connect(m_lineEditUserName, SIGNAL(textEdited(QString)), SLOT(onUserNameEdited(QString)));
    //

#ifndef Q_OS_MAC
    connect(m_buttonLogin, SIGNAL(clicked()), SLOT(on_btn_login_clicked()));
    connect(ui->btn_changeToLogin, SIGNAL(clicked()), SLOT(on_btn_changeToLogin_clicked()));
    connect(ui->btn_changeToSignin, SIGNAL(clicked()), SLOT(on_btn_changeToSignin_clicked()));
    connect(ui->btn_fogetpass, SIGNAL(clicked()), SLOT(on_btn_fogetpass_clicked()));
    connect(ui->btn_singin, SIGNAL(clicked()), SLOT(on_btn_singin_clicked()));
    connect(ui->btn_thridpart, SIGNAL(clicked()), SLOT(on_btn_thridpart_clicked()));
    connect(ui->cbx_autologin, SIGNAL(toggled(bool)), SLOT(on_cbx_autologin_toggled(bool)));
    connect(ui->cbx_remberPassword, SIGNAL(toggled(bool)), SLOT(on_cbx_remberPassword_toggled(bool)));
#endif

    setUsers(strDefaultUserId);
}

CWizLoginDialog::~CWizLoginDialog()
{
    delete ui;
}

QString CWizLoginDialog::userId() const
{
    return m_lineEditUserName->text();
}

QString CWizLoginDialog::password() const
{
    return m_lineEditPassword->text();
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

        m_menu->addAction(strUserId);
    }

    // set default user as default login entry.
    setUser(strDefault);
    QAction* action = findActionInMenu(strDefault);
    if (action)
    {
        m_menu->setDefaultAction(action);
    }
}

void CWizLoginDialog::setUser(const QString &strUserId)
{
    CWizUserSettings userSettings(strUserId);
    QString strPassword = userSettings.password();

    m_lineEditUserName->setText(strUserId);
    if (strPassword.isEmpty()) {
        m_lineEditPassword->clear();
        ui->cbx_remberPassword->setCheckState(Qt::Unchecked);
    } else {
        m_lineEditPassword->setText(strPassword);
        ui->cbx_remberPassword->setCheckState(Qt::Checked);
    }
}

void CWizLoginDialog::doAccountVerify()
{
    CWizUserSettings userSettings(userId());

    // FIXME: should verify password if network is available to avoid attack?
    if (password() != userSettings.password()) {
        Token::setUserId(userId());
        Token::setPasswd(password());
        doOnlineVerify();
        return;
    }

    if (updateUserProfile(false) && updateGlobalProfile()) {
        QDialog::accept();
    }
    enableLoginControls(true);
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
}

void CWizLoginDialog::enableSignInControls(bool bEnable)
{
    m_lineEditNewUserName->setEnabled(bEnable);
    m_lineEditNewPassword->setEnabled(bEnable);
    m_lineEditRepeatPassword->setEnabled(bEnable);
    ui->btn_singin->setEnabled(bEnable);
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

void CWizLoginDialog::setElementStyles()
{
    ui->stackedWidget->setCurrentIndex(0);

    QString strThemeName = Utils::StyleHelper::themeName();
    QString strlogo = ::WizGetSkinResourceFileName(strThemeName, "loginLogoCn");
    ui->label_logo->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                        "background-position: center; background-repeat: no-repeat; background-color:#43A6E8}").arg(strlogo));
    ui->label_placehold->setStyleSheet(QString("QLabel {border: none;background-color:#43A6E8}"));
    //
    QString strBtnCloseNormal = ::WizGetSkinResourceFileName(strThemeName, "loginCloseButton_normal");
    QString strBtnCloseHot = ::WizGetSkinResourceFileName(strThemeName, "loginCloseButton_hot");
    ui->btn_close->setStyleSheet(QString("QToolButton{ border-image:url(%1); height: 13px; width: 13px;}"
                                         "QToolButton:hover{ border-image:url(%2);}"
                                           "QToolButton:pressed { border-image:url(%3);}")
                                 .arg(strBtnCloseNormal).arg(strBtnCloseHot).arg(strBtnCloseHot));
    QString strGrayButton = ::WizGetSkinResourceFileName(strThemeName, "loginGrayButton");
    ui->btn_min->setStyleSheet(QString("QToolButton{ border-image:url(%1); height: 13px; width: 13px;}").arg(strGrayButton));
    ui->btn_max->setStyleSheet(QString("QToolButton{ border-image:url(%1); height: 13px; width: 13px;}").arg(strGrayButton));
    //

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

    ui->wgt_passwordcontainer->setBackgroundImage(strLoginBottomLineEditor, QPoint(8, 8));
    ui->wgt_passwordcontainer->setLeftIcon(strIconKey);
    m_lineEditPassword->setEchoMode(QLineEdit::Password);
    m_lineEditPassword->setPlaceholderText(tr("Password"));

    ui->wgt_newUser->setBackgroundImage(strLoginTopLineEditor, QPoint(8, 8));
    ui->wgt_newUser->setLeftIcon(strIconPerson);
    m_lineEditNewUserName->setPlaceholderText(tr("Please input eamil as your account"));

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
    m_buttonLogin->setText("Login");
    m_buttonLogin->setEnabled(false);
//    m_buttonLogin->setContentsMargins(10, 10, 10, 10);
//    m_buttonLogin->setStyleSheet("QPushButton{margin:30px;}");

    m_buttonSignIn->setButtonStyle(strBtnNormal, strBtnHover, strBtnDown, strBtnDisable, QColor("#ffffff"),
                                   QColor("#ffffff"), QColor("b1b1b1"));
    m_buttonSignIn->setText("Sign Up");
    m_buttonSignIn->setEnabled(false);
    //
    QString strSeparator = ::WizGetSkinResourceFileName(strThemeName, "loginSeparator");
    ui->label_separator2->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                               "background-position: center; background-repeat: no-repeat}").arg(strSeparator));
   //
    ui->label_noaccount->setStyleSheet(QString("QLabel {border: none; font: 15px; color: #5f5f5f;}"));
    ui->btn_changeToSignin->setStyleSheet(QString("QPushButton { border: 1px; background: none; "
                                                 "color: #43a6e8; font: 15px; padding-left: 10px; padding-bottom: 3px}"));
    ui->btn_thridpart->setStyleSheet(QString("QPushButton { border: none; background: none; "
                                                 "color: #b1b1b1; font: 13px; padding-right: 15px; padding-bottom: 5px}"));
    ui->btn_fogetpass->setStyleSheet(QString("QPushButton { border: none; background: none; "
                                                 "color: #b1b1b1; font: 13px; padding-left: 15px; padding-bottom: 5px}"));

    QString strLineSeparator = ::WizGetSkinResourceFileName(strThemeName, "loginLineSeparator");
    ui->label_separator3->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                                "background-position: center; background-repeat: no-repeat}").arg(strLineSeparator));

    ui->btn_changeToLogin->setStyleSheet(QString("QPushButton { border: 1px; background: none; "
                                                 "color: #43a6e8; font: 15px; padding-left: 10px; padding-bottom: 3px}"));
    ui->btn_changeToLogin->setVisible(false);
    ui->label_passwordError->setStyleSheet(QString("QLabel {border: none; padding-left: 25px; color: red;}"));

    m_menu->setFixedWidth(ui->wgt_usercontainer->width());
    m_menu->setStyleSheet("QMenu {background-color: #ffffff; border-style: solid; border-color: #43A6E8; border-width: 1px; font: 16px; color: #5F5F5F; menu-scrollable: 1;}"
                          "QMenu::item {padding: 10px 0px 10px 40px; }"
                          "QMenu::item:selected {background-color: #E7F5FF; }"
                          "QMenu::item:default {background-color: #E7F5FF; }");
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
    QList<QAction*> actionList = m_menu->actions();
    for (int i = 0; i < actionList.count(); i++)
    {
        if (actionList.at(i)->text() == strActName)
            return actionList.at(i);
    }
    return 0;
}




void CWizLoginDialog::on_btn_changeToSignin_clicked()
{
    ui->btn_changeToSignin->setVisible(false);
    ui->btn_changeToLogin->setVisible(true);
    ui->label_noaccount->setText(tr("Already got account,"));
    ui->label_passwordError->clear();
    ui->stackedWidget->setCurrentIndex(1);
    ui->wgt_newUser->setFocus();
}

void CWizLoginDialog::on_btn_changeToLogin_clicked()
{
    ui->btn_changeToLogin->setVisible(false);
    ui->btn_changeToSignin->setVisible(true);
    ui->label_noaccount->setText(tr("No account,"));
    ui->label_passwordError->clear();
    ui->stackedWidget->setCurrentIndex(0);
    ui->wgt_usercontainer->setFocus();
}

void CWizLoginDialog::on_btn_thridpart_clicked()
{
    QString strUrl = WizService::ApiEntry::standardCommandUrl("snspage");
    QDesktopServices::openUrl(QUrl(strUrl));
}

void CWizLoginDialog::on_btn_fogetpass_clicked()
{
    QString strUrl = WizService::ApiEntry::standardCommandUrl("forgot_password");
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

    enableLoginControls(false);
    doAccountVerify();
}

void CWizLoginDialog::on_btn_singin_clicked()
{
    if (checkSingMessage())
    {

    #if defined Q_OS_MAC
        QString strCode = "129ce11c";
    #elif defined Q_OS_LINUX
        QString strCode = "7abd8f4a";
    #else
        QString strCode = "8480c6d7";
    #endif

        AsyncApi* api = new AsyncApi(this);
        connect(api, SIGNAL(registerAccountFinished(bool)), SLOT(onRegisterAccountFinished(bool)));
        api->registerAccount(m_lineEditNewUserName->text(), m_lineEditNewPassword->text(), strCode);
        enableLoginControls(false);
    }
}

void CWizLoginDialog::onLoginInputChanged()
{
    ui->label_passwordError->clear();
    bool bInputFinished =  !m_lineEditUserName->text().isEmpty() && !m_lineEditPassword->text().isEmpty();
    ui->btn_login->setEnabled(bInputFinished);
}

void CWizLoginDialog::onTokenAcquired(const QString &strToken)
{
    Token::instance()->disconnect(this);

    enableLoginControls(true);
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
            ui->label_passwordError->setText(Token::lastErrorMessage());
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
    ui->btn_singin->setEnabled(bInputFinished);
}

void CWizLoginDialog::userListMenuClicked(QAction *action)
{
    if (action)
    {
        m_menu->setDefaultAction(action);
        setUser(action->text());
    }
}

void CWizLoginDialog::showUserListMenu()
{
    QPoint point = ui->wgt_usercontainer->mapToGlobal(QPoint(0, ui->wgt_usercontainer->height()));

    m_menu->popup(point);
}

void CWizLoginDialog::onRegisterAccountFinished(bool bFinish)
{
    AsyncApi* api = dynamic_cast<AsyncApi*>(sender());
    enableSignInControls(true);
    if (bFinish) {
        ui->stackedWidget->setCurrentIndex(0);
        enableLoginControls(false);
        m_lineEditUserName->setText(m_lineEditNewUserName->text());
        m_lineEditPassword->setText(m_lineEditNewPassword->text());
        doAccountVerify();
    } else {
        ui->label_passwordError->setText(api->lastErrorMessage());
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


