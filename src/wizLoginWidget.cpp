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
#include <QPainterPath>
#include <QMouseEvent>
#include <QMenu>
#include <QFocusEvent>
#include <QBitmap>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>

#include "share/wizui.h"
#include "wiznotestyle.h"

using namespace WizService;




CWizIconLineEditContainer::CWizIconLineEditContainer(QWidget* parent)
    : QWidget(parent)
    , m_background(NULL)
    , m_layout(NULL)
    , m_edit(NULL)
    , m_leftIcon(NULL)
    , m_dropdownIcon(NULL)
{
    m_layout = new QHBoxLayout(this);
    m_edit = new QLineEdit(this);
    m_edit->setAttribute(Qt::WA_MacShowFocusRect, false);
    m_edit->setStyleSheet(QString("QLineEdit{ border:none; color:#2F2F2F; "
                          "selection-background-color: #8ECAF1;}"));
    m_leftIcon = new QLabel(this);
    m_dropdownIcon = new QLabel(this);
    //
    m_layout->setSpacing(8);
    m_layout->setContentsMargins(8, 8, 8, 8);
    //
    m_layout->addWidget(m_leftIcon);
    m_layout->addWidget(m_edit);
    m_layout->addWidget(m_dropdownIcon);

}
void CWizIconLineEditContainer::setBackgroundImage(QString fileName, QPoint pt)
{
    m_background = new CWizSkin9GridImage();
    m_background->SetImage(fileName, pt);
}

void CWizIconLineEditContainer::setLeftIcon(QString fileName)
{
    m_leftIcon->setPixmap(QPixmap(fileName));
}
void CWizIconLineEditContainer::setRightIcon(QString fileName)
{
    m_dropdownIcon->setPixmap(QPixmap(fileName));
}

void CWizIconLineEditContainer::setPlaceholderText(const QString &strText)
{
    m_edit->setPlaceholderText(strText);
}

void CWizIconLineEditContainer::paintEvent(QPaintEvent *event)
{
    if (m_background && m_background->Valid())
    {
        QPainter paint(this);
        m_background->Draw(&paint, rect(), 0);
    }
    else
    {
        QWidget::paintEvent(event);
    }
}


void CWizIconLineEditContainer::mousePressEvent(QMouseEvent *event)
{
    if (m_dropdownIcon->geometry().contains(m_dropdownIcon->mapFromGlobal(event->pos())))
    {
        emit rightIconClicked();
    }
}


CWizImageButton::CWizImageButton(QWidget *parent)
    :QPushButton(parent)
{
}

void CWizImageButton::setButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
                                     const QString& downBackgroundFileName, const QString& disabledBackgroundFileName,
                                     const QColor& normalTextColor, const QColor& activeTextColor, const QColor& disableTextColor)
{
    QStyle* style = WizGetImageButtonStyle(normalBackgroundFileName, hotBackgroundFileName,
                                           downBackgroundFileName, disabledBackgroundFileName, normalTextColor,
                                           activeTextColor, disableTextColor);
    setStyle(style);
}


LoginLineEdit::LoginLineEdit(QWidget *parent) : QLineEdit(parent)
{
    connect(this, SIGNAL(textChanged(QString)), SLOT(on_containt_changed(QString)));
}

void LoginLineEdit::setElementStyle(const QString &strBgFile, EchoMode mode, const QString &strPlaceHoldTxt)
{
    setStyleSheet(QString("QLineEdit{ border-image:url(%1);font:16px; color:#2F2F2F; "
                          "selection-background-color: #8ECAF1;}").arg(strBgFile));
    setTextMargins(40, 0, 0, 0);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setEchoMode(mode);
    setPlaceholderText(strPlaceHoldTxt);
}

void LoginLineEdit::setErrorStatus(bool bErrorStatus)
{
    if (bErrorStatus)
    {
        QString strThemeName = Utils::StyleHelper::themeName();
        QString strError = ::WizGetSkinResourceFileName(strThemeName, "loginErrorInput");
        m_extraIcon = QPixmap(strError);
    }
    else
    {
        m_extraIcon = QPixmap();
    }
}

void LoginLineEdit::on_containt_changed(const QString &strText)
{
    Q_UNUSED(strText);
    setErrorStatus(false);
    update();
}

void LoginLineEdit::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);

    if (!m_extraIcon.isNull())
    {
        QPainter painter(this);
        QRect rect = getExtraIconBorder();
        painter.drawPixmap(rect, m_extraIcon);
    }
}

QRect LoginLineEdit::getExtraIconBorder()
{
    if (m_extraIcon.isNull())
        return QRect();

    QStyleOptionFrameV3 option;
    initStyleOption(&option);

    QRect rect(option.rect.right() - m_extraIcon.width() - 5, option.rect.top() + (option.rect.height() - m_extraIcon.height()) / 2,
               m_extraIcon.width(), m_extraIcon.height());

    return rect;
}

LoginButton::LoginButton(QWidget *parent) : QPushButton(parent)
{
    setFixedSize(302, 46);
}

void LoginButton::setElementStyle()
{
    QString strThemeName = Utils::StyleHelper::themeName();
    //
    QString strBtnNormal = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_normal");
    QString strBtnHover = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_hover");
    QString strBtnDown = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_down");
    setStyleSheet(QString("QPushButton{ background-image:url(%1); border: none; font:12px}"
                        "QPushButton:hover{ background-image:url(%2); }"
                          "QPushButton:pressed { background-image:url(%3); font:12px}")
                  .arg(strBtnNormal).arg(strBtnHover).arg(strBtnDown));
}

void LoginButton::setEnabled(bool bEnable)
{
    QString strThemeName = Utils::StyleHelper::themeName();
    if (!bEnable)
    {
        QString strBtnNormal = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_normal");
        setStyleSheet(QString("QPushButton{ background-image:url(%1); border: none; font:12px; color: black;}").arg(strBtnNormal));
    }
    else
    {
        QString strBtnActive = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_active");
        QString strBtnHover = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_hover");
        QString strBtnDown = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_down");
        setStyleSheet(QString("QPushButton{ background-image:url(%1); border: none; font:12px; color: white;}"
                            "QPushButton:hover{ background-image:url(%2); }"
                              "QPushButton:pressed { background-image:url(%3); font:12px}")
                      .arg(strBtnActive).arg(strBtnHover).arg(strBtnDown));
    }
    QPushButton::setEnabled(bEnable);
}

CWizLoginWidget::CWizLoginWidget(const QString &strDefaultUserId, const QString &strLocale, QWidget *parent)
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
    //setContentsMargins(10, 10, 10, 10);
//    setFixedSize(352, 503);
//    QPalette plt(palette());
//    QPixmap pix(::WizGetSkinResourceFileName(Utils::StyleHelper::themeName(), "loginBackground"));
//    plt.setBrush(QPalette::Window, QBrush(pix));
//    setPalette(plt);
#else
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

    ui->wgt_usercontainer->setBackgroundImage(WizGetSkinResourceFileName(Utils::StyleHelper::themeName(), "loginTopLineEditor"), QPoint(8, 8));
    ui->wgt_usercontainer->setLeftIcon(WizGetSkinResourceFileName(Utils::StyleHelper::themeName(), "loginIconPerson"));
    ui->wgt_usercontainer->setRightIcon(WizGetSkinResourceFileName(Utils::StyleHelper::themeName(), "loginLineEditorDownArrow"));
    m_lineEditUserName = ui->wgt_usercontainer->edit();

    ui->wgt_passwordcontainer->setBackgroundImage(WizGetSkinResourceFileName(Utils::StyleHelper::themeName(), "loginBottomLineEditor"), QPoint(8, 8));
    ui->wgt_passwordcontainer->setLeftIcon(WizGetSkinResourceFileName(Utils::StyleHelper::themeName(), "loginIconKey"));
    ui->wgt_passwordcontainer->edit()->setEchoMode(QLineEdit::Password);
    m_lineEditPassword = ui->wgt_passwordcontainer->edit();
    //

    QString strThemeName = Utils::StyleHelper::themeName();
    QString strBtnNormal = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_active");
    QString strBtnHover = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_hover");
    QString strBtnDown = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_down");
    QString strBtnDisable = ::WizGetSkinResourceFileName(strThemeName, "loginOKButton_normal");
    //
    m_buttonLogin = ui->btn_login;
    m_buttonLogin->setButtonStyle(strBtnNormal, strBtnHover, strBtnDown, strBtnDisable, QColor("#ffffff"),
                              QColor("#ffffff"), QColor("b1b1b1"));
    m_buttonLogin->setText("Login");
    m_buttonLogin->setContentsMargins(10, 20, 10, 20);

    setElementStyles();



    connect(m_menu, SIGNAL(triggered(QAction*)), SLOT(userListMenuClicked(QAction*)));

    connect(ui->lineEdit_newPassword, SIGNAL(textChanged(QString)), SLOT(onSignUpInputDataChanged()));
    connect(ui->lineEdit_newUserName, SIGNAL(textChanged(QString)), SLOT(onSignUpInputDataChanged()));
    connect(ui->lineEdit_repaetPassword, SIGNAL(textChanged(QString)), SLOT(onSignUpInputDataChanged()));
    connect(m_lineEditPassword, SIGNAL(textChanged(QString)), SLOT(onLoginInputChanged()));
    connect(m_lineEditUserName, SIGNAL(textChanged(QString)), SLOT(onLoginInputChanged()));
    //connect(ui->lineEdit_userName, SIGNAL(showMenuRequest(QPoint)), SLOT(showUserListMenu(QPoint)));
    //

#ifndef Q_OS_MAC
    connect(m_buttonLogin, SIGNAL(clicked()), SLOT(on_btn_login_clicked()));
#endif

    setUsers(strDefaultUserId);
}

CWizLoginWidget::~CWizLoginWidget()
{
    delete ui;
}

QString CWizLoginWidget::userId() const
{
    return m_lineEditUserName->text();
}

QString CWizLoginWidget::password() const
{
    return m_lineEditPassword->text();
}

void CWizLoginWidget::setUsers(const QString &strDefault)
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

void CWizLoginWidget::setUser(const QString &strUserId)
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

void CWizLoginWidget::doAccountVerify()
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

void CWizLoginWidget::doOnlineVerify()
{
    connect(Token::instance(), SIGNAL(tokenAcquired(QString)), SLOT(onTokenAcquired(QString)), Qt::QueuedConnection);
    Token::requestToken();
}

bool CWizLoginWidget::updateGlobalProfile()
{
    QSettings* settings = ExtensionSystem::PluginManager::globalSettings();
    settings->setValue("Users/DefaultUser", userId());
    return true;
}

bool CWizLoginWidget::updateUserProfile(bool bLogined)
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

void CWizLoginWidget::enableLoginControls(bool bEnable)
{
    m_lineEditUserName->setEnabled(bEnable);
    m_lineEditPassword->setEnabled(bEnable);
    ui->cbx_autologin->setEnabled(bEnable);
    ui->cbx_remberPassword->setEnabled(bEnable);
    m_buttonLogin->setEnabled(bEnable);
    ui->btn_changeToSignin->setEnabled(bEnable);
}

void CWizLoginWidget::enableSignInControls(bool bEnable)
{
    ui->lineEdit_newUserName->setEnabled(bEnable);
    ui->lineEdit_newPassword->setEnabled(bEnable);
    ui->lineEdit_repaetPassword->setEnabled(bEnable);
    ui->btn_singin->setEnabled(bEnable);
    ui->btn_changeToLogin->setEnabled(bEnable);
}

#ifdef Q_OS_MAC
void CWizLoginWidget::mousePressEvent(QMouseEvent *event)
{
    m_mousePoint = event->globalPos();
}

void CWizLoginWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_mousePoint != QPoint(0, 0))
    {
        move(geometry().x() + event->globalPos().x() - m_mousePoint.x(), geometry().y() + event->globalPos().y() - m_mousePoint.y());
        m_mousePoint = event->globalPos();
    }
}

void CWizLoginWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_mousePoint = QPoint(0, 0);
}
#endif


void CWizLoginWidget::on_btn_close_clicked()
{
    qApp->quit();
}

void CWizLoginWidget::setElementStyles()
{
    ui->stackedWidget->setCurrentIndex(0);

    QString strThemeName = Utils::StyleHelper::themeName();
    QString strlogo = ::WizGetSkinResourceFileName(strThemeName, "loginLogoCn");
    ui->label_logo->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                        "background-position: center; background-repeat: no-repeat; background-color:#43A6E8}").arg(strlogo));
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
    QString strLineEditName = ::WizGetSkinResourceFileName(strThemeName, "loginTopLineEditor");
    QString strLineEditMidPassword = ::WizGetSkinResourceFileName(strThemeName, "loginMidLineEditor");
    QString strLineEditBottomPassword = ::WizGetSkinResourceFileName(strThemeName, "loginBottomLineEditor");
    ui->lineEdit_newUserName->setElementStyle(strLineEditName, QLineEdit::Normal, "Please enter email as your account");
    ui->lineEdit_newPassword->setElementStyle(strLineEditMidPassword, QLineEdit::Password, tr("Please enter password"));
    ui->lineEdit_repaetPassword->setElementStyle(strLineEditBottomPassword, QLineEdit::Password, tr("Please enter password again"));
    //
    //ui->btn_login->setElementStyle();
    ui->btn_singin->setElementStyle();
    ui->btn_singin->setEnabled(false);
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

    m_menu->setFixedWidth(302);
    m_menu->setStyleSheet("QMenu {background-color: #ffffff; border-style: solid; border-color: #43A6E8; border-width: 1px; font: 16px; color: #5F5F5F; menu-scrollable: 1;}"
                          "QMenu::item {padding: 10px 0px 10px 40px; }"
                          "QMenu::item:selected {background-color: #E7F5FF; }"
                          "QMenu::item:default {background-color: #E7F5FF; }");
}

bool CWizLoginWidget::checkSingMessage()
{
    QRegExp mailRex("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
    mailRex.setCaseSensitivity(Qt::CaseInsensitive);
    if (!mailRex.exactMatch(ui->lineEdit_newUserName->text()))
    {
        ui->lineEdit_newUserName->setErrorStatus(true);
        ui->lineEdit_newUserName->update();
        ui->label_passwordError->setText(tr("Invalid email address."));
        return false;
    }

    if (ui->lineEdit_newPassword->text().isEmpty())
    {
        ui->lineEdit_newPassword->setErrorStatus(true);
        ui->lineEdit_newPassword->update();
        ui->label_passwordError->setText(tr("Password is Empty"));
        return false;
    }

    if (ui->lineEdit_newPassword->text() != ui->lineEdit_repaetPassword->text())
    {
        ui->lineEdit_repaetPassword->setErrorStatus(true);
        ui->lineEdit_repaetPassword->update();
        ui->label_passwordError->setText(tr("Passwords don't match"));
        return false;
    }

    return true;
}

QAction *CWizLoginWidget::findActionInMenu(const QString &strActName)
{
    QList<QAction*> actionList = m_menu->actions();
    for (int i = 0; i < actionList.count(); i++)
    {
        if (actionList.at(i)->text() == strActName)
            return actionList.at(i);
    }
    return 0;
}




void CWizLoginWidget::on_btn_changeToSignin_clicked()
{
    ui->btn_changeToSignin->setVisible(false);
    ui->btn_changeToLogin->setVisible(true);
    ui->label_noaccount->setText(tr("Already got account,"));
    ui->label_passwordError->clear();
    ui->stackedWidget->setCurrentIndex(1);
}

void CWizLoginWidget::on_btn_changeToLogin_clicked()
{
    ui->btn_changeToLogin->setVisible(false);
    ui->btn_changeToSignin->setVisible(true);
    ui->label_noaccount->setText(tr("No account,"));
    ui->label_passwordError->clear();
    ui->stackedWidget->setCurrentIndex(0);
}

void CWizLoginWidget::on_btn_thridpart_clicked()
{
    QString strUrl = WizService::ApiEntry::standardCommandUrl("snspage");
    QDesktopServices::openUrl(QUrl(strUrl));
}

void CWizLoginWidget::on_btn_fogetpass_clicked()
{
    QString strUrl = WizService::ApiEntry::standardCommandUrl("forgot_password");
    QDesktopServices::openUrl(QUrl(strUrl));
}


LoginMenuLineEdit::LoginMenuLineEdit(QWidget *parent) : LoginLineEdit(parent)
{
    m_extraIcon = QPixmap(::WizGetSkinResourceFileName(Utils::StyleHelper::themeName(), "loginLineEditorDownArrow"));
}

void LoginMenuLineEdit::on_containt_changed(const QString &strText)
{
    Q_UNUSED(strText);
}


void LoginMenuLineEdit::mousePressEvent(QMouseEvent *event)
{
    QRect rect = getExtraIconBorder();
    if (rect.contains(event->pos()))
    {
        emit showMenuRequest(mapToGlobal(QPoint(0, height())));
    }
    else
    {
        LoginLineEdit::mousePressEvent(event);
    }
}

void CWizLoginWidget::on_btn_login_clicked()
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

void CWizLoginWidget::on_btn_singin_clicked()
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
        api->registerAccount(ui->lineEdit_newUserName->text(), ui->lineEdit_newPassword->text(), strCode);
        enableLoginControls(false);
    }
}

void CWizLoginWidget::onLoginInputChanged()
{
    ui->label_passwordError->clear();
    bool bInputFinished =  !m_lineEditUserName->text().isEmpty() && !m_lineEditPassword->text().isEmpty();
    ui->btn_login->setEnabled(bInputFinished);
}

void CWizLoginWidget::onTokenAcquired(const QString &strToken)
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

void CWizLoginWidget::onSignUpInputDataChanged()
{
    ui->label_passwordError->clear();
    bool bInputFinished = !ui->lineEdit_newUserName->text().isEmpty() && !ui->lineEdit_newPassword->text().isEmpty()
            && !ui->lineEdit_repaetPassword->text().isEmpty();
    ui->btn_singin->setEnabled(bInputFinished);
}

void CWizLoginWidget::userListMenuClicked(QAction *action)
{
    if (action)
    {
        m_menu->setDefaultAction(action);
        setUser(action->text());
    }
}

void CWizLoginWidget::showUserListMenu(QPoint point)
{
    m_menu->popup(point);
}

void CWizLoginWidget::onRegisterAccountFinished(bool bFinish)
{
    AsyncApi* api = dynamic_cast<AsyncApi*>(sender());
    enableSignInControls(true);
    if (bFinish) {
        ui->stackedWidget->setCurrentIndex(0);
        enableLoginControls(false);
        m_lineEditUserName->setText(ui->lineEdit_newUserName->text());
        m_lineEditPassword->setText(ui->lineEdit_newPassword->text());
        doAccountVerify();
    } else {
        ui->label_passwordError->setText(api->lastErrorMessage());
    }

    api->deleteLater();
}

void CWizLoginWidget::on_cbx_remberPassword_toggled(bool checked)
{
    if (!checked)
        ui->cbx_autologin->setChecked(false);
}

void CWizLoginWidget::on_cbx_autologin_toggled(bool checked)
{
    if (checked)
        ui->cbx_remberPassword->setChecked(true);
}

void CWizLoginWidget::on_lineEdit_userName_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);
    m_lineEditPassword->setText("");
}
