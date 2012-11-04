#include "wizproxydialog.h"
#include "ui_wizproxydialog.h"

#include "share/wizsettings.h"

#include "share/wizmisc.h"

ProxyDialog::ProxyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProxyDialog)
{
    ui->setupUi(this);

    CWizSettings settings(WizGetSettingsFileName());
    ui->editAddress->setText(settings.GetProxyHost());
    ui->editPort->setText(WizIntToStr(settings.GetProxyPort()));
    ui->editUserName->setText(settings.GetProxyUserName());
    ui->editPassword->setText(settings.GetProxyPassword());

    bool proxyStatus = settings.GetProxyStatus();
    if (!proxyStatus) {
        ui->checkProxyStatus->setChecked(true);
    }

    enableControl(!proxyStatus);

    connect(ui->checkProxyStatus, SIGNAL(stateChanged(int)), SLOT(proxyStatusChanged(int)));
}

ProxyDialog::~ProxyDialog()
{
    delete ui;
}

void ProxyDialog::proxyStatusChanged(int state)
{
    enableControl(state);
}

void ProxyDialog::enableControl(bool b)
{
    ui->editAddress->setDisabled(b);
    ui->editPort->setDisabled(b);
    ui->editUserName->setDisabled(b);
    ui->editPassword->setDisabled(b);
}

void ProxyDialog::accept()
{
    CWizSettings settings(WizGetSettingsFileName());
    settings.SetProxyHost(ui->editAddress->text());
    settings.SetProxyPort(ui->editPort->text().toInt());
    settings.SetProxyUserName(ui->editUserName->text());
    settings.SetProxyPassword(ui->editPassword->text());
    settings.SetProxyStatus(!ui->checkProxyStatus->isChecked());

    QDialog::accept();
}
