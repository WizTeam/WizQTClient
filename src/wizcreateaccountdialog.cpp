#include "wizcreateaccountdialog.h"
#include "ui_wizcreateaccountdialog.h"

#include <QMessageBox>

#include "sync/asyncapi.h"

using namespace WizService;


CreateAccountDialog::CreateAccountDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateAccountDialog)
{
    ui->setupUi(this);
    //connect(this, SIGNAL(registerAccount(const QString&, const QString&, const QString&)),
    //        CWizCloudPool::instance(), SLOT(registerAccount(const QString&, const QString&, const QString&)));
    //connect(CWizCloudPool::instance(), SIGNAL(accountRegistered(bool)), SLOT(accountRegistered_done(bool)));
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
    if (userId().isEmpty()) {
        QMessageBox::critical(this, "", tr("Please enter user id"));
        return;
    }

    if (password().isEmpty()) {
        QMessageBox::critical(this, "", tr("Please enter user password"));
        return;
    }

    if (password() != password2()) {
        QMessageBox::critical(this, "", tr("password is not equal, please try again."));
        return;
    }

    enableControls(false);

//#if defined Q_OS_MAC
//    QString strCode = "129ce11c";
//#elif defined Q_OS_LINUX
//    QString strCode = "7abd8f4a";
//#else
//    QString strCode = "8480c6d7";
//#endif

    AsyncApi* api = new AsyncApi(this);
    connect(api, SIGNAL(registerAccountFinished(bool)), SLOT(onRegisterAccountFinished(bool)));
    api->registerAccount(userId(), password(), "");
}

void CreateAccountDialog::onRegisterAccountFinished(bool bOk)
{
    AsyncApi* api = dynamic_cast<AsyncApi*>(sender());
    enableControls(true);

    if (bOk) {
        QDialog::accept();
    } else {
        QMessageBox::critical(this, "", api->lastErrorMessage());
    }

    api->deleteLater();
}

//void CreateAccountDialog::accountRegistered_done(bool bOk)
//{
//    enableControls(true);
//
//    if (bOk) {
//        QDialog::accept();
//    } else {
//        QMessageBox::critical(this, "", CWizCloudPool::instance()->lastErrorMessage());
//    }
//}

void CreateAccountDialog::enableControls(bool b)
{
    ui->editUserId->setEnabled(b);
    ui->editPassword->setEnabled(b);
    ui->editPassword2->setEnabled(b);
    ui->buttonBox->setEnabled(b);
}
