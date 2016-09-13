#include "WizProxyDialog.h"
#include "ui_WizProxyDialog.h"

#include "share/WizSettings.h"
#include "share/WizMisc.h"

#include "utils/WizPathResolve.h"

#include <QNetworkProxy>

WizProxyDialog::WizProxyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WizProxyDialog)
{
    ui->setupUi(this);

    WizSettings settings(Utils::WizPathResolve::globalSettingsFile());
    ui->editAddress->setText(settings.getProxyHost());
    ui->editPort->setText(WizIntToStr(settings.getProxyPort()));
    ui->editUserName->setText(settings.getProxyUserName());
    ui->editPassword->setText(settings.getProxyPassword());
    ui->editPassword->setEchoMode(QLineEdit::Password);
    ui->comboBox->setCurrentIndex((int)settings.getProxyType());

    bool proxyStatus = settings.getProxyStatus();
    ui->checkProxyStatus->setChecked(proxyStatus);

    enableControl(proxyStatus);

    connect(ui->checkProxyStatus, SIGNAL(stateChanged(int)), SLOT(proxyStatusChanged(int)));
}

WizProxyDialog::~WizProxyDialog()
{
    delete ui;
}

void WizProxyDialog::proxyStatusChanged(int state)
{
    enableControl(Qt::Checked == state);
}

void WizProxyDialog::enableControl(bool b)
{
    ui->editAddress->setEnabled(b);
    ui->editPort->setEnabled(b);
    ui->editUserName->setEnabled(b);
    ui->editPassword->setEnabled(b);
    ui->comboBox->setEnabled(b);
}

void WizProxyDialog::setApplicationProxy()
{
    if (ui->checkProxyStatus->checkState() == Qt::Checked)
    {
        QNetworkProxy proxy;
        proxy.setType(getProxyType());
        proxy.setHostName(ui->editAddress->text());
        proxy.setPort(ui->editPort->text().toInt());
        proxy.setUser(ui->editUserName->text());
        proxy.setPassword(ui->editPassword->text());
        QNetworkProxy::setApplicationProxy(proxy);
    }
    else
    {
        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    }
}

QNetworkProxy::ProxyType WizProxyDialog::getProxyType()
{
    if (ui->comboBox->currentIndex() == WizProxy_HttpProxy)
    {
        return QNetworkProxy::HttpProxy;
    }
    else if (ui->comboBox->currentIndex() == WizProxy_Socks5Proxy)
    {
        return QNetworkProxy::Socks5Proxy;
    }

    return QNetworkProxy::NoProxy;
}

void WizProxyDialog::accept()
{
    WizSettings settings(Utils::WizPathResolve::globalSettingsFile());
    settings.setProxyType((WizProxyType)ui->comboBox->currentIndex());
    settings.setProxyHost(ui->editAddress->text());
    settings.setProxyPort(ui->editPort->text().toInt());
    settings.setProxyUserName(ui->editUserName->text());
    settings.setProxyPassword(ui->editPassword->text());
    settings.setProxyStatus(ui->checkProxyStatus->isChecked());

    setApplicationProxy();

    QDialog::accept();
}

