#include "newfolderdialog.h"
#include "ui_newfolderdialog.h"

NewFolderDialog::NewFolderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewFolderDialog)
{
    ui->setupUi(this);
    //
    setFixedSize(size());
}

NewFolderDialog::~NewFolderDialog()
{
    delete ui;
}
QString NewFolderDialog::folderName()
{
    return ui->editFolderName->text();
}
