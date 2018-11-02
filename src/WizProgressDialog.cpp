#include "WizProgressDialog.h"
#include "ui_WizProgressDialog.h"
#include<QDebug>
#include "share/WizUIBase.h"

WizProgressDialog::WizProgressDialog(QWidget *parent, bool showStop) :
    QDialog(parent),
    ui(new Ui::WizProgressDialog)
{
    ui->setupUi(this);
    setFixedSize(size());

    ui->btn_stop->setVisible(showStop);
    ui->btn_hide->setVisible(showStop);
    //
#ifndef Q_OS_MAC
    if (isDarkMode()) {
        setStyleSheet("color:#e9e9e9");
    }
#endif
}

WizProgressDialog::~WizProgressDialog()
{
    delete ui;
}

void WizProgressDialog::setActionString(const QString& strAction)
{
//    QString elideText = fontMetrics().elidedText(strAction, Qt::ElideRight, ui->progressBar->width());
    ui->labelAction->setText(strAction);
    update();
}

void WizProgressDialog::setProgress(int nMax, int nCurrent)
{
    ui->progressBar->setMaximum(nMax);
    ui->progressBar->setValue(nCurrent);
    update();
}

void WizProgressDialog::setProgress(QString strObjGUID, int nMax, int nCurrent)
{
    ui->progressBar->setMaximum(nMax);
    ui->progressBar->setValue(nCurrent);
    update();
}

void WizProgressDialog::on_btn_stop_clicked()
{
    emit stopRequest();
    accept();
}

void WizProgressDialog::on_btn_hide_clicked()
{
    accept();
}
