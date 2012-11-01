#include "wizcreateaccountdialog.h"
#include "ui_wizcreateaccountdialog.h"

#include <QMessageBox>

CreateAccountDialog::CreateAccountDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateAccountDialog),
    m_createAccount(WIZ_API_URL)
{
    ui->setupUi(this);

    connect(&m_createAccount, SIGNAL(done(bool, const CString&)), \
            SLOT(createAccountDone(bool, const CString&)));
}

CreateAccountDialog::~CreateAccountDialog()
{
    delete ui;
}

QString CreateAccountDialog::userId() const
{
    return ui->editUserId->text();
}
QString CreateAccountDialog::password() const
{
    return ui->editPassword->text();
}

QString CreateAccountDialog::password2() const
{
    return ui->editPassword2->text();
}

void CreateAccountDialog::accept()
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
    m_createAccount.createAccount(userId(), password(), "");
}

void CreateAccountDialog::createAccountDone(bool succeeded, const CString& errorMessage)
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

void CreateAccountDialog::enableControls(bool b)
{
    ui->editUserId->setEnabled(b);
    ui->editPassword->setEnabled(b);
    ui->editPassword2->setEnabled(b);
    ui->buttonBox->setEnabled(b);
}
