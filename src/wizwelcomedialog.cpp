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
    , m_verifyAccount(WIZ_API_URL)
{
    ui->setupUi(this);

    ui->labelProxySettings->setText(WizFormatString1("<a href=\"proxy_settings\">%1</a>", tr("Proxy settings")));

    // load webview content
    QString strLocalFileName = ::WizGetResourcesPath() + "languages/welcome_" + strLocale + ".htm";
    QString strFileName = ::WizGetResourcesPath() + "languages/welcome.htm";
    if (PathFileExists(strLocalFileName))
    {
        strFileName = strLocalFileName;
    }

    QPalette pal = palette();
    QColor color = pal.color(QPalette::Background);

    CString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    strHtml.replace("ButtonFace", "#" + ::WizColorToString(color));

    ui->webView->setHtml(strHtml, QUrl::fromLocalFile(strFileName));
    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(ui->webView, SIGNAL(linkClicked(const QUrl&)), this, SLOT(on_webView_linkClicked(const QUrl&)));

    // callbacks
    connect(ui->labelProxySettings, SIGNAL(linkActivated(const QString&)), SLOT(on_labelProxySettings_linkActivated(const QString&)));
    connect(ui->comboUsers, SIGNAL(activated(QString)), SLOT(on_comboUsers_activated(QString)));
    connect(ui->comboUsers, SIGNAL(editTextChanged(QString)), SLOT(on_comboUsers_editTextChanged(QString)));
    connect(ui->checkAutoLogin, SIGNAL(stateChanged(int)), SLOT(on_checkAutoLogin_stateChanged(int)));
    connect(ui->buttonLogin, SIGNAL(clicked()), SLOT(accept()));

    connect(&m_verifyAccount, SIGNAL(done(bool, const CString&)), SLOT(verifyAccountDone(bool, const CString&)));

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

    m_verifyAccount.verifyAccount(userId(), password());
}

void WelcomeDialog::verifyAccountDone(bool succeeded, const CString& errorMessage)
{
    if (succeeded) {
        updateUserSettings();
        QDialog::accept();
    } else {
        QMessageBox::critical(this, "", errorMessage);
    }

    enableControls(true);
}

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
    //
    CString strStart("http://wiz.cn/cmd/");
    if (strUrl.startsWith(strStart))
    {
        strUrl.remove(0, strStart.length());
        if (strUrl == "view_guide")
        {
            QDesktopServices::openUrl(QUrl("http://www.wiz.cn/"));
        }
        else if (strUrl == "create_account")
        {
            CreateAccountDialog dlg(this);
            if (QDialog::Accepted != dlg.exec())
                return;
            //
            CString strUserId = dlg.userId();
            CString strPassword = dlg.password();
            //
            //ui->editUserId->setText(strUserId);
            ui->editPassword->setText(strPassword);
            //
            QDialog::accept();
        }
    }
    else
    {
        QDesktopServices::openUrl(url);
    }
}

void WelcomeDialog::on_labelProxySettings_linkActivated(const QString & link)
{
    Q_UNUSED(link);

    ProxyDialog dlg;
    if (QDialog::Accepted != dlg.exec())
        return;

    m_verifyAccount.resetProxy();
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
