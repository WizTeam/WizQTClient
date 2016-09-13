#include "WizCreateAccountDialog.h"
#include "ui_WizCreateAccountDialog.h"

#include <QMessageBox>

#include "sync/WizAsyncApi.h"



WizCreateAccountDialog::WizCreateAccountDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizCreateAccountDialog)
{
    ui->setupUi(this);
    //connect(this, SIGNAL(registerAccount(const QString&, const QString&, const QString&)),
    //        CWizCloudPool::instance(), SLOT(registerAccount(const QString&, const QString&, const QString&)));
    //connect(CWizCloudPool::instance(), SIGNAL(accountRegistered(bool)), SLOT(accountRegistered_done(bool)));
}

WizCreateAccountDialog::~WizCreateAccountDialog()
{
    delete ui;
}

QString WizCreateAccountDialog::userId() const
{
    return ui->editUserId->text();
}
QString WizCreateAccountDialog::password() const
{
    return ui->editPassword->text();
}

QString WizCreateAccountDialog::password2() const
{
    return ui->editPassword2->text();
}

void WizCreateAccountDialog::accept()
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

    WizAsyncApi* api = new WizAsyncApi(this);
    connect(api, SIGNAL(registerAccountFinished(bool)), SLOT(onRegisterAccountFinished(bool)));
    api->registerAccount(userId(), password(), "");
}

void WizCreateAccountDialog::onRegisterAccountFinished(bool bOk)
{
    WizAsyncApi* api = dynamic_cast<WizAsyncApi*>(sender());
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

void WizCreateAccountDialog::enableControls(bool b)
{
    ui->editUserId->setEnabled(b);
    ui->editPassword->setEnabled(b);
    ui->editPassword2->setEnabled(b);
    ui->buttonBox->setEnabled(b);
}
