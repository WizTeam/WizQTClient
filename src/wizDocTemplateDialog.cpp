#include "wizDocTemplateDialog.h"
#include "ui_wizDocTemplateDialog.h"

CWizDocTemplateDialog::CWizDocTemplateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizDocTemplateDialog)
{
    ui->setupUi(this);
    ui->btn_useLocal->setVisible(false);
}

CWizDocTemplateDialog::~CWizDocTemplateDialog()
{
    delete ui;
}

void CWizDocTemplateDialog::on_btn_downloadNew_clicked()
{
    shiftStackIndex(StackIndex_downloadNew);
}

void CWizDocTemplateDialog::on_btn_useLocal_clicked()
{
    shiftStackIndex(StackIndex_useLocal);
}

void CWizDocTemplateDialog::shiftStackIndex(CWizDocTemplateDialog::StackIndex index)
{
    switch (index) {
    case StackIndex_downloadNew:
        ui->btn_downloadNew->setVisible(true);
        ui->btn_useLocal->setVisible(false);
        ui->stackedWidget->setCurrentIndex(0);
        break;
    case StackIndex_useLocal:
        ui->btn_downloadNew->setVisible(false);
        ui->btn_useLocal->setVisible(true);
        ui->stackedWidget->setCurrentIndex(1);
        break;
    }
}
