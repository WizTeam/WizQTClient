#include "wizwelcomedialog.h"
#include "ui_wizwelcomedialog.h"

#include <QAbstractButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QWebFrame>
#include "wizcreateaccountdialog.h"
#include "wizproxydialog.h"


WelcomeDialog::WelcomeDialog(const QString &strDefaultUserId, const QString& strLocale, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WelcomeDialog)
    , m_strDefaultUserId(strDefaultUserId)
{
    ui->setupUi(this);

    // load webview content
    QString strLocalFileName = ::WizGetResourcesPath() + "languages/welcome_" + strLocale + ".htm";
    QString strFileName = ::WizGetResourcesPath() + "languages/welcome.htm";
    if (PathFileExists(strLocalFileName)) {
        strFileName = strLocalFileName;
    }

    QPalette pal = palette();
    QColor color = pal.color(QPalette::Window);

    CString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    strHtml.replace("ButtonFace", "#" + ::WizColorToString(color));

    ui->webView->setHtml(strHtml, QUrl::fromLocalFile(strFileName));
    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(ui->webView, SIGNAL(linkClicked(const QUrl&)), \
            SLOT(on_webView_linkClicked(const QUrl&)), Qt::UniqueConnection);

    connect(ui->labelRegisterNew, SIGNAL(linkActivated(const QString&)),
            SLOT(on_labelRegisterNew_linkActivated(const QString&)),
            Qt::UniqueConnection);

    connect(ui->labelForgotPassword, SIGNAL(linkActivated(const QString&)),
            SLOT(on_labelForgotPassword_linkActivated(const QString&)),
            Qt::UniqueConnection);

    connect(ui->labelProxySettings, SIGNAL(linkActivated(const QString&)), \
            SLOT(on_labelProxySettings_linkActivated(const QString&)), \
            Qt::UniqueConnection);

    connect(ui->comboUsers, SIGNAL(activated(const QString&)), \
            SLOT(on_comboUsers_activated(const QString&)));

    connect(ui->comboUsers, SIGNAL(editTextChanged(const QString&)), \
            SLOT(on_comboUsers_editTextChanged(const QString&)));

    connect(ui->checkAutoLogin, SIGNAL(stateChanged(int)), \
            SLOT(on_checkAutoLogin_stateChanged(int)));

    connect(ui->buttonLogin, SIGNAL(clicked()), SLOT(accept()));

//    connect(&m_verifyAccount, SIGNAL(done(bool, int, const QString&)), \
//            SLOT(verifyAccountDone(bool, int, const QString&)));

    setUsers();

    setFixedSize(size());
}

WelcomeDialog::~WelcomeDialog()
{
    delete ui;
}

void WelcomeDialog::setUsers()
{
    getUserPasswordPairs();
    ui->comboUsers->addItems(m_users.keys());

    // set default user as default login entry.
    for (int i = 0; i < ui->comboUsers->count(); i++) {
        if (!m_strDefaultUserId.compare(ui->comboUsers->itemText(i))) {
            ui->comboUsers->setCurrentIndex(i);
            setUser(m_strDefaultUserId);
            break;
        }
    }
}

void WelcomeDialog::setPassword(const QString &strPassword)
{
    // only set encrypted password here, avoid attack risk!
    ui->editPassword->setText(strPassword);
}

QString WelcomeDialog::userId() const
{
    return ui->comboUsers->currentText();
}

QString WelcomeDialog::password() const
{
    QString strPassword = ui->editPassword->text();
    if (!m_users.value(userId()).compare(strPassword))
        return ::WizDecryptPassword(strPassword);
    else
        return strPassword;
}

void WelcomeDialog::accept()
{
    if (userId().isEmpty())
    {
        QMessageBox::critical(this, "", tr("Please enter user id"));
        return;
    }
    if (password().isEmpty())
    {
        QMessageBox::critical(this, "", tr("Please enter user password"));
        return;
    }

    enableControls(false);

    verifyAccountStart();

    //m_verifyAccount.resetProxy();
    //m_verifyAccount.verifyAccount(userId(), password());
}

void WelcomeDialog::verifyAccountStart()
{
    CWizKMAccountsServer server(WizKMGetAccountsServerURL(true));
    if (server.Login(userId(), password(), "normal")) {
        m_info = server.GetUserInfo();
        updateUserSettings();
        QDialog::accept();
    } else {
        QMessageBox::critical(this, tr("Verify account failed"), server.GetLastErrorMessage());
    }

    enableControls(true);
}

//void WelcomeDialog::verifyAccountDone(bool succeeded, int errorCode, const QString& errorMessage)
//{
//    if (succeeded) {
//        updateUserSettings();
//        QDialog::accept();
//    } else {
//        QString msg;
//        if (errorCode == 3) {
//            msg = tr("Network Error, please check your network connection!\n\nErrorCode: ") + QString::number(errorCode);
//        } else {
//            msg = errorMessage + tr("\n\nErrorCode: ") + QString::number(errorCode);
//        }
//
//        QMessageBox::critical(this, tr("Verify account failed"), msg);
//    }
//
//    enableControls(true);
//}

void WelcomeDialog::updateUserSettings()
{
    CWizSettings settings(::WizGetDataStorePath() + "wiznote.ini");

    // set current login user as default user.
    settings.SetString("Users", "DefaultUser", userId());

    CWizUserSettings userSettings(userId());

    if(ui->checkAutoLogin->checkState() == Qt::Checked) {
        userSettings.setAutoLogin(true);
    } else {
        userSettings.setAutoLogin(false);
    }

    if(ui->checkRememberMe->checkState() == Qt::Checked) {
        userSettings.setPassword(::WizEncryptPassword(password()));
    } else {
        userSettings.setPassword();
    }
}

void WelcomeDialog::getUserPasswordPairs()
{
    CWizStdStringArray usersFolder;
    ::WizEnumFolders(::WizGetDataStorePath(), usersFolder, 0);

    for(CWizStdStringArray::const_iterator iter = usersFolder.begin();
        iter != usersFolder.end();
        iter++)
    {
        QString strPath = *iter;
        QString strUserId = ::WizFolderNameByPath(strPath);

        if (strUserId.indexOf("@") == -1) {
            continue;
        }

        CWizUserSettings userSettings(strUserId);
        m_users.insert(strUserId, userSettings.password());
    }
}

void WelcomeDialog::enableControls(bool b)
{
    ui->comboUsers->setEnabled(b);
    ui->editPassword->setEnabled(b);
    ui->checkRememberMe->setEnabled(b);
    ui->checkAutoLogin->setEnabled(b);
    ui->buttonLogin->setEnabled(b);
}

void WelcomeDialog::on_webView_linkClicked(const QUrl& url)
{
    CString strUrl = url.toString();
    strUrl.MakeLower();

    CString strStart("http://wiz.cn/cmd/");
    if (strUrl.startsWith(strStart))
    {
        strUrl.remove(0, strStart.length());
        if (strUrl == "view_guide")
        {
            QDesktopServices::openUrl(QUrl("http://www.wiz.cn/wiznote-maclinux.html"));
        }
        else if (strUrl == "create_account")
        {
            CreateAccountDialog dlg(this);
            if (QDialog::Accepted != dlg.exec())
                return;

            QString strUserId = dlg.userId();
            QString strPassword = dlg.password();

            ui->comboUsers->insertItem(0, strUserId);
            ui->comboUsers->setCurrentIndex(0);
            ui->editPassword->setText(strPassword);
        }
    }
    else
    {
        QDesktopServices::openUrl(url);
    }
}

void WelcomeDialog::on_labelForgotPassword_linkActivated(const QString& strUrl)
{
    Q_UNUSED(strUrl);

    // FIXME: use different locale page
    QUrl url("http://as.wiz.cn/wizas/htmlpages/reset_password_zh_CN.html");
    QDesktopServices::openUrl(url);
}

void WelcomeDialog::on_labelRegisterNew_linkActivated(const QString& strUrl)
{
    CreateAccountDialog dlg(this);
    if (QDialog::Accepted != dlg.exec())
        return;

    QString strUserId = dlg.userId();
    QString strPassword = dlg.password();

    ui->comboUsers->insertItem(0, strUserId);
    ui->comboUsers->setCurrentIndex(0);
    ui->editPassword->setText(strPassword);
}

void WelcomeDialog::on_labelProxySettings_linkActivated(const QString & link)
{
    Q_UNUSED(link);

    ProxyDialog dlg(this);
    if (QDialog::Accepted != dlg.exec())
        return;

    //m_verifyAccount.resetProxy();
}

void WelcomeDialog::on_comboUsers_activated(const QString &userId)
{
    setUser(userId);
}

void WelcomeDialog::setUser(const QString &userId)
{
    QString strPassword = m_users.value(userId);

    setPassword(strPassword);

    if (strPassword.isEmpty())
        ui->checkRememberMe->setCheckState(Qt::Unchecked);
    else
        ui->checkRememberMe->setCheckState(Qt::Checked);
}

void WelcomeDialog::on_comboUsers_editTextChanged(const QString& strText)
{
    Q_UNUSED(strText);

    ui->editPassword->clear();
    ui->checkRememberMe->setCheckState(Qt::Unchecked);
}

void WelcomeDialog::on_checkAutoLogin_stateChanged(int state)
{
    if (state == Qt::Checked)
        ui->checkRememberMe->setCheckState(Qt::Checked);
}
