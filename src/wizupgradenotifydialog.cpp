#include "wizupgradenotifydialog.h"
#include "ui_wizupgradenotifydialog.h"

CWizUpgradeNotifyDialog::CWizUpgradeNotifyDialog(const QString& changelogUrl, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CWizUpgradeNotifyDialog)
{
    ui->setupUi(this);

    ui->webView->load(QUrl(changelogUrl));

    connect(ui->buttonWait, SIGNAL(clicked()), SLOT(reject()));
    connect(ui->buttonNow, SIGNAL(clicked()), SLOT(accept()));
}

CWizUpgradeNotifyDialog::~CWizUpgradeNotifyDialog()
{
    delete ui;
}
