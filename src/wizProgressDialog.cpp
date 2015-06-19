#include "wizProgressDialog.h"
#include "ui_wizProgressDialog.h"

CWizProgressDialog::CWizProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizProgressDialog)
{
    ui->setupUi(this);
    setFixedSize(size());
}

CWizProgressDialog::~CWizProgressDialog()
{
    delete ui;
}

void CWizProgressDialog::setActionString(const QString& strAction)
{
    ui->labelAction->setText(strAction);
}

void CWizProgressDialog::setNotifyString(const QString& strNotify)
{
    QString elideText = fontMetrics().elidedText(strNotify, Qt::ElideRight, ui->labelDetails->width());
    ui->labelDetails->setText(elideText);
}

void CWizProgressDialog::setProgress(int nMax, int nCurrent)
{
    ui->progressBar->setMaximum(nMax);
    ui->progressBar->setValue(nCurrent);
    update();
}

void CWizProgressDialog::setProgress(QString strObjGUID, int nMax, int nCurrent)
{
    ui->progressBar->setMaximum(nMax);
    ui->progressBar->setValue(nCurrent);
}
