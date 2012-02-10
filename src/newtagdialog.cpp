#include "newtagdialog.h"
#include "ui_newtagdialog.h"

NewTagDialog::NewTagDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewTagDialog)
{
    ui->setupUi(this);
    //
    setFixedSize(size());
}

NewTagDialog::~NewTagDialog()
{
    delete ui;
}

QString NewTagDialog::tagName() const
{
    return ui->editTagName->text();
}
