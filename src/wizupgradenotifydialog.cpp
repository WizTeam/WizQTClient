#include "wizupgradenotifydialog.h"
#include "ui_wizupgradenotifydialog.h"

WizUpgradeNotifyDialog::WizUpgradeNotifyDialog(const QString& changelogUrl, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizUpgradeNotifyDialog)
{
    ui->setupUi(this);

    ui->webView->load(QUrl(changelogUrl));

    connect(ui->buttonWait, SIGNAL(clicked()), SLOT(reject()));
    connect(ui->buttonNow, SIGNAL(clicked()), SLOT(accept()));
}

WizUpgradeNotifyDialog::~WizUpgradeNotifyDialog()
{
    delete ui;
}
