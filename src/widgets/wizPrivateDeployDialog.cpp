#include "wizPrivateDeployDialog.h"
#include "ui_wizPrivateDeployDialog.h"
#include "../utils/pathresolve.h"
#include "../share/wizsettings.h"

#include <QMessageBox>

CWizPrivateDeployDialog::CWizPrivateDeployDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizPrivateDeployDialog)
{
    ui->setupUi(this);

    CWizSettings settings(Utils::PathResolve::globalSettingsFile());
    bool useCustomSettings = settings.GetBool("PrivateDeploy", "CustomSetting", false);
    ui->checkBox_useCustomSettings->setChecked(useCustomSettings);
    ui->lineEdit_apiServer->setEnabled(useCustomSettings);
    QString strAPIServer = settings.GetString("PrivateDeploy", "ApiServer");
    ui->lineEdit_apiServer->setText(strAPIServer);
}

CWizPrivateDeployDialog::~CWizPrivateDeployDialog()
{
    delete ui;
}

void CWizPrivateDeployDialog::on_checkBox_useCustomSettings_toggled(bool checked)
{
    ui->lineEdit_apiServer->setEnabled(checked);
}

void CWizPrivateDeployDialog::on_pushButton_cancel_clicked()
{
    reject();
}

void CWizPrivateDeployDialog::on_pushButton_ok_clicked()
{
    CWizSettings settings(Utils::PathResolve::globalSettingsFile());
    bool useCustomSettings = settings.GetBool("PrivateDeploy", "CustomSetting", false);
    QString strAPIServer = settings.GetString("PrivateDeploy", "ApiServer");

    bool checked = ui->checkBox_useCustomSettings->checkState() == Qt::Checked;
    QString strNewApiServer = ui->lineEdit_apiServer->text();
    if (checked != useCustomSettings || strAPIServer != strNewApiServer)
    {
        settings.SetBool("PrivateDeploy", "CustomSetting", checked);
        settings.SetString("PrivateDeploy", "ApiServer", strNewApiServer);

        QMessageBox::information(0, tr("Info"), tr("Restart WizNote to use the new server."));
    }

    accept();
}
