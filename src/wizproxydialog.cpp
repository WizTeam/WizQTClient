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
}

ProxyDialog::~ProxyDialog()
{
    delete ui;
}

void ProxyDialog::accept()
{
    CWizSettings settings(WizGetSettingsFileName());
    settings.SetProxyHost(ui->editAddress->text());
    settings.SetProxyPort(ui->editPort->text().toInt());
    settings.SetProxyUserName(ui->editUserName->text());
    settings.SetProxyPassword(ui->editPassword->text());

    QDialog::accept();
}
