#include "wizPrivateDeployDialog.h"
#include "ui_wizPrivateDeployDialog.h"
#include "utils/pathresolve.h"
#include "share/wizsettings.h"
#include "wizproxydialog.h"
#include "sync/apientry.h"

#include <QMessageBox>

CWizPrivateDeployDialog::CWizPrivateDeployDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizPrivateDeployDialog)
{
    ui->setupUi(this);

    CWizSettings settings(Utils::PathResolve::globalSettingsFile());

    bool useCustomSettings = settings.GetBool("PrivateDeploy", "CustomSetting", false);
    ui->checkBox_useCustomSettings->setChecked(useCustomSettings);
    ui->groupBox->setEnabled(useCustomSettings);

    QString strAPIServer = settings.GetString("PrivateDeploy", "ApiServer");
    if (strAPIServer.isEmpty()) {
        strAPIServer = WIZNOTE_API_SERVER;
    }
    ui->lineEdit_apiServer->setText(strAPIServer);

    bool useHttps = settings.GetBool("PrivateDeploy", "UseHttpsConnection", false);
    ui->checkBox_useHttps->setChecked(useHttps);

    bool useMD5 = settings.GetBool("PrivateDeploy", "UseMD5Password", false);
    ui->checkBox_useMD5->setChecked(useMD5);
}

CWizPrivateDeployDialog::~CWizPrivateDeployDialog()
{
    delete ui;
}

void CWizPrivateDeployDialog::on_checkBox_useCustomSettings_toggled(bool checked)
{
//    ui->lineEdit_apiServer->setEnabled(checked);
    ui->groupBox->setEnabled(checked);
}

void CWizPrivateDeployDialog::on_pushButton_cancel_clicked()
{
    reject();
}

void CWizPrivateDeployDialog::on_pushButton_ok_clicked()
{
    CWizSettings settings(Utils::PathResolve::globalSettingsFile());

    bool newUseCustomSettings = ui->checkBox_useCustomSettings->checkState() == Qt::Checked;
    settings.SetBool("PrivateDeploy", "CustomSetting", newUseCustomSettings);

    QString newApiServer = ui->lineEdit_apiServer->text();
    settings.SetString("PrivateDeploy", "ApiServer", newApiServer);

    bool newUseHttps = ui->checkBox_useHttps->checkState() == Qt::Checked;
    settings.SetBool("PrivateDeploy", "UseHttpsConnection", newUseHttps);

    bool newUseMD5 = ui->checkBox_useMD5->checkState() == Qt::Checked;
    settings.SetBool("PrivateDeploy", "UseMD5Password", newUseMD5);

    WizService::ApiEntry::reloadPrivateDeploySettings();

    accept();
}

void CWizPrivateDeployDialog::on_label_proxyLink_linkActivated(const QString &link)
{
    if (link == "proxysettings")
    {
        ProxyDialog dlg(this);
        dlg.exec();
    }
}
