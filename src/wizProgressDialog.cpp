#include "wizProgressDialog.h"
#include "ui_wizProgressDialog.h"
#include<QDebug>

CWizProgressDialog::CWizProgressDialog(QWidget *parent, bool showStop) :
    QDialog(parent),
    ui(new Ui::CWizProgressDialog)
{
    ui->setupUi(this);
    setFixedSize(size());

    ui->btn_stop->setVisible(showStop);
    ui->btn_hide->setVisible(showStop);
}

CWizProgressDialog::~CWizProgressDialog()
{
    delete ui;
}

void CWizProgressDialog::setActionString(const QString& strAction)
{
//    QString elideText = fontMetrics().elidedText(strAction, Qt::ElideRight, ui->progressBar->width());
    ui->labelAction->setText(strAction);
    update();
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
    update();
}

void CWizProgressDialog::on_btn_stop_clicked()
{
    emit stopRequest();
    accept();
}

void CWizProgressDialog::on_btn_hide_clicked()
{
    accept();
}
