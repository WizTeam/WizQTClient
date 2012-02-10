#include "welcomedialog.h"
#include "ui_welcomedialog.h"

#include <QAbstractButton>
#include <QMessageBox>

WelcomeDialog::WelcomeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WelcomeDialog),
    m_verifyAccount(WIZ_API_URL)

{
    ui->setupUi(this);
    //
    connect(&m_verifyAccount, SIGNAL(done(bool, const CString&)), this, SLOT(verifyAccountDone(bool, const CString&)));
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

void WelcomeDialog::verifyAccountDone(bool error, const CString& errorMessage)
{
    enableControls(true);
    //
    if (error)
    {
        QMessageBox::critical(this, "", errorMessage);
    }
    else
    {
        QDialog::accept();
    }
}

void WelcomeDialog::enableControls(bool b)
{
    ui->editUserId->setEnabled(b);
    ui->editPassword->setEnabled(b);
    ui->buttonBox->setEnabled(b);
}


