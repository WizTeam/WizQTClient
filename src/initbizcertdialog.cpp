#include "initbizcertdialog.h"
#include "ui_initbizcertdialog.h"

InitBizCertDialog::InitBizCertDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InitBizCertDialog)
{
    ui->setupUi(this);
}

InitBizCertDialog::~InitBizCertDialog()
{
    delete ui;
}

void InitBizCertDialog::VerifyCert()
{

}

void InitBizCertDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
    {
        VerifyCert();
    }
}
