#include "wizproxydialog.h"
#include "ui_wizproxydialog.h"

#include "share/wizsettings.h"
#include "share/wizmisc.h"

#include "utils/pathresolve.h"

#include <QNetworkProxy>

ProxyDialog::ProxyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProxyDialog)
{
    ui->setupUi(this);

    CWizSettings settings(Utils::PathResolve::globalSettingsFile());
    ui->editAddress->setText(settings.GetString("Sync", "ProxyHost"));
    ui->editPort->setText(WizIntToStr(settings.GetInt("Sync", "ProxyPort", 0)));
    ui->editUserName->setText(settings.GetString("Sync", "ProxyUserName"));
    ui->editPassword->setText(settings.GetString("Sync", "ProxyPassword"));
    ui->editPassword->setEchoMode(QLineEdit::Password);
    ui->comboBox->setCurrentIndex(settings.GetInt("Sync", "ProxyType", 1));

    bool proxyStatus = settings.GetBool("Sync", "ProxyStatus", false);
    ui->checkProxyStatus->setChecked(proxyStatus);

    enableControl(proxyStatus);

    connect(ui->checkProxyStatus, SIGNAL(stateChanged(int)), SLOT(proxyStatusChanged(int)));
}

ProxyDialog::~ProxyDialog()
{
    delete ui;
}

void ProxyDialog::proxyStatusChanged(int state)
{
    enableControl(Qt::Checked == state);
}

void ProxyDialog::enableControl(bool b)
{
    ui->editAddress->setEnabled(b);
    ui->editPort->setEnabled(b);
    ui->editUserName->setEnabled(b);
    ui->editPassword->setEnabled(b);
    ui->comboBox->setEnabled(b);
}

void ProxyDialog::setApplicationProxy()
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

QNetworkProxy::ProxyType ProxyDialog::getProxyType()
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

void ProxyDialog::accept()
{
    CWizSettings settings(Utils::PathResolve::globalSettingsFile());
    WizProxyType type = (WizProxyType)ui->comboBox->currentIndex();
    settings.SetInt("Sync", "ProxyType", type);
    settings.SetString("Sync", "ProxyHost", ui->editAddress->text());
    settings.SetInt("Sync", "ProxyPort", ui->editPort->text().toInt());
    settings.SetString("Sync", "ProxyUserName", ui->editUserName->text());
    settings.SetString("Sync", "ProxyPassword", ui->editPassword->text());
    settings.SetBool("Sync", "ProxyStatus", ui->checkProxyStatus->isChecked());

    setApplicationProxy();

    QDialog::accept();
}

