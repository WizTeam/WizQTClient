#include "wizLoginDialog.h"

#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>

#include <extensionsystem/pluginmanager.h>

#include "sync/wizkmxmlrpc.h"
#include "wizcreateaccountdialog.h"
#include "wizproxydialog.h"

#include "sync/apientry.h"
#include "sync/token.h"

using namespace WizService;


//class CWizAvatarWidget : public QLabel
//{
//public:
//    CWizAvatarWidget(QWidget* parent = 0) : QLabel(parent) {}
//
//    virtual void paintEvent(QPaintEvent* event)
//    {
//        Q_UNUSED(event);
//
//        const QPixmap* avatar = pixmap();
//        if (!avatar || avatar->isNull()) {
//            return;
//        }
//
//        int nMargin = 8;
//
//        QPainter p(this);
//        QRegion reg(nMargin, nMargin, 102, 102, QRegion::Ellipse);
//        p.setClipRegion(reg);
//        p.drawPixmap(nMargin, nMargin, *avatar);
//    }
//
//    void setAvatar(const QString& strFileName)
//    {
//        setPixmap(strFileName);
//    }
//};


CWizLoginDialog::CWizLoginDialog(const QString& strDefaultUserId, const QString& strLocale, QWidget *parent)
    : QDialog(parent)
{
    QLabel* labelInput = new QLabel(tr("Account Login"), this);

    m_comboUsers = new QComboBox(this);
    m_comboUsers->setEditable(true);
    connect(m_comboUsers, SIGNAL(currentIndexChanged(const QString&)),
            SLOT(on_comboUsers_indexChanged(const QString&)));
    connect(m_comboUsers, SIGNAL(editTextChanged(const QString&)),
            SLOT(on_comboUsers_editTextChanged(const QString&)));

    m_editPassword = new QLineEdit(this);
    m_editPassword->setEchoMode(QLineEdit::Password);
    m_editPassword->setPlaceholderText(tr("Password"));
    m_checkSavePassword = new QCheckBox(tr("Save password"));

    m_checkAutoLogin = new QCheckBox(tr("Auto login"));
    connect(m_checkAutoLogin, SIGNAL(stateChanged(int)),
            SLOT(on_checkAutoLogin_stateChanged(int)));

    QHBoxLayout* layoutState = new QHBoxLayout();
    layoutState->setContentsMargins(0, 0, 0, 0);
    layoutState->addWidget(m_checkSavePassword);
    layoutState->addSpacing(10);
    layoutState->addWidget(m_checkAutoLogin);
    layoutState->setAlignment(Qt::AlignRight);

    QVBoxLayout* layoutInput = new QVBoxLayout();
    layoutInput->setContentsMargins(10, 10, 10, 0);
    layoutInput->setSpacing(10);
    layoutInput->addWidget(labelInput);
    layoutInput->addWidget(m_comboUsers);
    layoutInput->addWidget(m_editPassword);
    layoutInput->addSpacing(5);
    layoutInput->addLayout(layoutState);

    QLabel* m_avatar = new QLabel(this);
    //m_avatar = new CWizAvatarWidget(this);
    m_avatar->setFixedSize(120, 120);
    m_avatar->setPixmap(QPixmap(":/avatar_login.png"));
    //resetAvatar(strDefaultUserId);

    QHBoxLayout* layoutUser = new QHBoxLayout();
    layoutUser->addSpacing(20);
    layoutUser->addWidget(m_avatar);
    layoutUser->addLayout(layoutInput);

    QWidget* line1 = new QWidget(this);
    line1->setFixedWidth(1);
    line1->setStyleSheet("border-left-width:1;border-left-style:solid;border-left-color:#d9dcdd");

    QWidget* line2 = new QWidget(this);
    line2->setFixedWidth(1);
    line2->setStyleSheet("border-left-width:1;border-left-style:solid;border-left-color:#d9dcdd");

    QLabel* m_labelRegister = new QLabel(this);
    m_labelRegister->setText(tr("<a href=\"javascript:void(0)\" style=\"color:#363636; text-decoration:none;\">New user</a>"));
    connect(m_labelRegister, SIGNAL(linkActivated(const QString&)),
            SLOT(on_labelRegister_linkActivated(const QString&)));

    QLabel* m_labelForgot = new QLabel(this);
    m_labelForgot->setText(tr("<a href=\"javascript:void(0)\" style=\"color:#363636; text-decoration:none;\">Forget password</a>"));
    connect(m_labelForgot, SIGNAL(linkActivated(const QString&)),
            SLOT(on_labelForgot_linkActivated(const QString&)));

    //QLabel* m_labelNetwork = new QLabel(this);
    //m_labelNetwork->setText(tr("<a href=\"javascript:void(0)\" style=\"color:#363636; text-decoration:none;\">Network</a>"));
    //connect(m_labelNetwork, SIGNAL(linkActivated(const QString&)),
    //        SLOT(on_labelNetwork_linkActivated(const QString&)));

    m_btnLogin = new QPushButton(tr("Login"), this);
    connect(m_btnLogin, SIGNAL(clicked()), SLOT(accept()));

    QHBoxLayout* layoutCommon = new QHBoxLayout();
    layoutCommon->setContentsMargins(10, 0, 10, 0);
    layoutCommon->setSpacing(5);
    layoutCommon->addWidget(m_labelRegister);
    layoutCommon->addWidget(line1);
    layoutCommon->addWidget(m_labelForgot);
    //layoutCommon->addWidget(line2);
    //layoutCommon->addWidget(m_labelNetwork);
    layoutCommon->addStretch();
    layoutCommon->addWidget(m_btnLogin);

    QWidget* line3 = new QWidget(this);
    line3->setFixedHeight(1);
    line3->setStyleSheet("border-top-width:1;border-top-style:solid;border-top-color:#d9dcdd");

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(8);
    setLayout(mainLayout);
    mainLayout->addLayout(layoutUser);
    mainLayout->addWidget(line3);
    mainLayout->addLayout(layoutCommon);

    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    setFixedSize(sizeHint());

    setUsers(strDefaultUserId);
}

//void CWizLoginDialog::resetAvatar(const QString& strUserId)
//{
//    QString strAvatarFile = ::WizGetDataStorePath() + strUserId + "/avatar/" + strUserId + ".png";
//    if (!QFile::exists(strAvatarFile)) {
//        strAvatarFile = ::WizGetResourcesPath() + "skins/default/avatar.png";
//    }
//
//    QPixmap avatar(strAvatarFile);
//    m_avatar->setPixmap(avatar.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
//}

QString CWizLoginDialog::userId() const
{
    return m_comboUsers->currentText();
}

QString CWizLoginDialog::password() const
{
    return m_editPassword->text();
}

void CWizLoginDialog::accept()
{
    if (userId().isEmpty()) {
        QMessageBox::critical(this, "", tr("Please enter user id"));
        return;
    }

    if (password().isEmpty()) {
        QMessageBox::critical(this, "", tr("Please enter user password"));
        return;
    }

    enableControls(false);
    doAccountVerify();
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

    enableControls(true);
}

void CWizLoginDialog::doOnlineVerify()
{
    connect(Token::instance(), SIGNAL(tokenAcquired(QString)), SLOT(onTokenAcquired(QString)), Qt::QueuedConnection);
    Token::requestToken();
}

void CWizLoginDialog::onTokenAcquired(const QString& strToken)
{
    Token::instance()->disconnect(this);

    enableControls(true);

    if (strToken.isEmpty()) {
        QMessageBox::critical(0, tr("Verify account failed"), Token::lastErrorMessage());
        return;
    }

    if (updateUserProfile(true) && updateGlobalProfile())
        QDialog::accept();
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

    if(m_checkAutoLogin->checkState() == Qt::Checked) {
        userSettings.setAutoLogin(true);
    } else {
        userSettings.setAutoLogin(false);
    }

    if(m_checkSavePassword->checkState() != Qt::Checked) {
        userSettings.setPassword();
    }

    if (bLogined) {
        if (m_checkSavePassword->checkState() == Qt::Checked)
            userSettings.setPassword(::WizEncryptPassword(password()));

        CWizDatabase db;
        if (!db.Open(userId())) {
            QMessageBox::critical(0, tr("Update user profile"), QObject::tr("Can not open database while update user profile"));
            return false;
        }

        db.SetUserInfo(Token::info());
        db.Close();
    }

    return true;
}

void CWizLoginDialog::setUsers(const QString& strDefault)
{
    CWizStdStringArray usersFolder;
    ::WizEnumFolders(::WizGetDataStorePath(), usersFolder, 0);

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

        m_comboUsers->addItem(strUserId);
    }

    // set default user as default login entry.
    int i = m_comboUsers->findText(strDefault);
    if (-1 == i) {
        m_comboUsers->insertItem(0, NULL, strDefault);
        m_comboUsers->setCurrentIndex(0);
    } else {
        m_comboUsers->setCurrentIndex(i);
    }
}

void CWizLoginDialog::setUser(const QString& strUserId)
{
    //resetAvatar(strUserId);

    CWizUserSettings userSettings(strUserId);
    QString strPassword = userSettings.password();

    if (strPassword.isEmpty()) {
        m_editPassword->clear();
        m_checkSavePassword->setCheckState(Qt::Unchecked);
    } else {
        m_editPassword->setText(strPassword);
        m_checkSavePassword->setCheckState(Qt::Checked);
    }
}

void CWizLoginDialog::enableControls(bool b)
{
    m_comboUsers->setEnabled(b);
    m_editPassword->setEnabled(b);
    m_checkSavePassword->setEnabled(b);
    m_checkAutoLogin->setEnabled(b);
    m_btnLogin->setEnabled(b);
}

void CWizLoginDialog::on_labelForgot_linkActivated(const QString& strUrl)
{
    Q_UNUSED(strUrl);

    // FIXME: use different locale page
    QUrl url("http://as.wiz.cn/wizas/htmlpages/reset_password_zh_CN.html");
    QDesktopServices::openUrl(url);
}

void CWizLoginDialog::on_labelRegister_linkActivated(const QString& strUrl)
{
    Q_UNUSED(strUrl);

    CreateAccountDialog dlg(this);
    if (QDialog::Accepted != dlg.exec())
        return;

    QString strUserId = dlg.userId();
    QString strPassword = dlg.password();

    m_comboUsers->insertItem(0, strUserId);
    m_comboUsers->setCurrentIndex(0);
    m_editPassword->setText(strPassword);
    enableControls(false);
    doAccountVerify();
}

void CWizLoginDialog::on_labelNetwork_linkActivated(const QString & link)
{
    Q_UNUSED(link);

    //ProxyDialog dlg(this);
    //if (QDialog::Accepted != dlg.exec())
    //    return;
}

void CWizLoginDialog::on_comboUsers_indexChanged(const QString &userId)
{
    setUser(userId);
}

void CWizLoginDialog::on_comboUsers_editTextChanged(const QString& strText)
{
    Q_UNUSED(strText);

    m_editPassword->clear();
    m_checkSavePassword->setCheckState(Qt::Unchecked);
}

void CWizLoginDialog::on_checkAutoLogin_stateChanged(int state)
{
    if (state == Qt::Checked)
        m_checkSavePassword->setCheckState(Qt::Checked);
}
