#include "wizProgressDialog.h"
#include "ui_wizProgressDialog.h"
#include<QDebug>

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
    qDebug() << "dialog set action ; " << strAction;
    QString elideText = fontMetrics().elidedText(strAction, Qt::ElideRight, ui->labelAction->width());
    ui->labelAction->setText(elideText);
    update();
}

void CWizProgressDialog::setProgress(int nMax, int nCurrent)
{
    qDebug() << "dialog set progress : " << nMax << "  current ; " << nCurrent;
    ui->progressBar->setMaximum(nMax);
    ui->progressBar->setValue(nCurrent);
    update();
}

void CWizProgressDialog::setProgress(QString strObjGUID, int nMax, int nCurrent)
{
    qDebug() << "dialog set progress : " << nMax << "  current ; " << nCurrent;
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
