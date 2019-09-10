#include "WizUserServiceExprDialog.h"
#include "ui_WizUserServiceExprDialog.h"
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>

#include "../share/WizThreads.h"
#include "../sync/WizToken.h"
#include "../sync/WizApiEntry.h"

WizUserServiceExprDialog::WizUserServiceExprDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizUserServiceExprDialog)
{
    ui->setupUi(this);
    //
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(okClicked()));
    connect(ui->buttonBox, SIGNAL(helpRequested()), this, SLOT(helpClicked()));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(QObject::tr("Cancel"));
    ui->buttonBox->button(QDialogButtonBox::Help)->setText(QObject::tr("Help"));
}

void WizUserServiceExprDialog::setUserInfo(bool free, bool isBizUser, WIZGROUPDATA group)
{
    QString text;
    if (group.isGroup())
    {
        if (group.isBiz())
        {
            text = tr("Team service of [%1] has expired, temporarily unable to sync the new and edited notes and attachments, please renew on time.").arg(group.strGroupName);
        }
        else
        {
            text = tr("The personal group [%1] has expired and can not be uploaded. Please contact the group creator to upgrade to the VIP or upgrade to the team service!").arg(group.strGroupName);
        }
    }
    else if (isBizUser)
    {
        if (free)
        {
            text = tr("The free trial period has expired, please upgrade to VIP for uploading new personal notes and modification of personal notes, which make sure your data can be accessed on any device. (This has no influence on the business service.)");
        }
        else
        {
            text = tr("The period of VIP service has expired, please renew for uploading new personal notes and modification of personal notes, which make sure your data can be accessed on any device. (This has no influence on the business service.)");
        }
    }
    else
    {
        if (free)
        {
            text = tr("The free trial period has expired, please upgrade to VIP for uploading new personal notes and modification of personal notes, which make sure your data can be accessed on any device.");
        }
        else
        {
            text = tr("The period of VIP service has expired, please renew for uploading new personal notes and modification of personal notes, which make sure your data can be accessed on any device.");
        }
    }
    //
    if (!group.isGroup())
    {
        if (free)
        {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Upgrade to VIP"));
        }
        else
        {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Renew VIP"));
        }
    }
    //
    ui->labelMessage->setText(text);
}

void WizUserServiceExprDialog::helpClicked()
{
    QString url = WizOfficialApiEntry::standardCommandUrl("link");
    url += "&site=blog&name=sync_service.html";
    QDesktopServices::openUrl(QUrl(url));
}

void WizUserServiceExprDialog::okClicked()
{
    setResult(1);
    accept();
}

