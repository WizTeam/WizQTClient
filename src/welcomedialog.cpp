#include "welcomedialog.h"
#include "ui_welcomedialog.h"

#include <QAbstractButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QWebFrame>
#include "createaccountdialog.h"
#include "proxydialog.h"

WelcomeDialog::WelcomeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WelcomeDialog),
    m_verifyAccount(WIZ_API_URL)
{
    ui->setupUi(this);
    //
    connect(&m_verifyAccount, SIGNAL(done(bool, const CString&)), this, SLOT(verifyAccountDone(bool, const CString&)));
    //
    QString localName = QLocale::system().name();
    CString strDefaultFileName = ::WizGetResourcesPath() + "languages/welcome.htm";
    CString strLocalFileName = ::WizGetResourcesPath() + "languages/welcome_" + localName + ".htm";
    CString strFileName = strLocalFileName;
    if (!PathFileExists(strFileName))
    {
        strFileName = strDefaultFileName;
    }
    //
    QPalette pal = palette();
    QColor color = pal.color(QPalette::Background);
    //
    CString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    strHtml.replace("ButtonFace", "#" + ::WizColorToString(color));
    //
    ui->webView->setHtml(strHtml, QUrl::fromLocalFile(strFileName));
    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(ui->webView, SIGNAL(linkClicked(const QUrl&)), this, SLOT(on_web_linkClicked(const QUrl&)));
    //
    ui->labelProxySettings->setText(WizFormatString1("<a href=\"proxy_settings\">%1</a>", tr("Proxy settings")));
    connect(ui->labelProxySettings, SIGNAL(linkActivated(QString)), this, SLOT(on_labelProxy_linkActivated(QString)));
    //
    setFixedSize(size());
}

WelcomeDialog::~WelcomeDialog()
{
    delete ui;
}
void WelcomeDialog::setUserId(const QString& strUserId)
{
    ui->editUserId->setText(strUserId);
}

QString WelcomeDialog::userId() const
{
    return ui->editUserId->text();
}
QString WelcomeDialog::password() const
{
    return ui->editPassword->text();
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
    //
    enableControls(false);
    //
    m_verifyAccount.verifyAccount(userId(), password());
}

void WelcomeDialog::verifyAccountDone(bool succeeded, const CString& errorMessage)
{
    enableControls(true);
    //
    if (succeeded)
    {
        QDialog::accept();
    }
    else
    {
        QMessageBox::critical(this, "", errorMessage);
    }
}

void WelcomeDialog::enableControls(bool b)
{
    ui->editUserId->setEnabled(b);
    ui->editPassword->setEnabled(b);
    ui->buttonBox->setEnabled(b);
}
void WelcomeDialog::on_web_linkClicked(const QUrl & url)
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
            ui->editUserId->setText(strUserId);
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

void WelcomeDialog::on_labelProxy_linkActivated(const QString & link)
{
    Q_UNUSED(link);
    //
    ProxyDialog dlg;
    if (QDialog::Accepted != dlg.exec())
        return;
    //
    m_verifyAccount.resetProxy();
}


