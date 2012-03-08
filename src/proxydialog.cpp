#include "proxydialog.h"
#include "ui_proxydialog.h"

#include "share/wizsettings.h"

#include "share/wizmisc.h"

ProxyDialog::ProxyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProxyDialog)
{
    ui->setupUi(this);
    //
    ui->editAddress->setText(WizGetProxyHost());
    ui->editPort->setText(WizIntToStr(WizGetProxyPort()));
    ui->editUserName->setText(WizGetProxyUserName());
    ui->editPassword->setText(WizGetProxyPassword());
}

ProxyDialog::~ProxyDialog()
{
    delete ui;
}

void ProxyDialog::accept()
{
    WizSetProxyHost(ui->editAddress->text());
    WizSetProxyPort(ui->editPort->text().toInt());
    WizSetProxyUserName(ui->editUserName->text());
    WizSetProxyPassword(ui->editPassword->text());
    //
    QDialog::accept();
}
