#include "WizEditorInsertTableForm.h"
#include "ui_WizEditorInsertTableForm.h"

#include <QtGui>

WizEditorInsertTableForm::WizEditorInsertTableForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WizEditorInsertTableForm)
{
    ui->setupUi(this);

    setWindowModality(Qt::ApplicationModal);

    QIntValidator *validator = new QIntValidator(1, 30, this);
    ui->editRows->setValidator(validator);
    ui->editCols->setValidator(validator);
}

WizEditorInsertTableForm::~WizEditorInsertTableForm()
{
    delete ui;
}

int WizEditorInsertTableForm::getRows()
{
    return ui->editRows->text().toInt();
}

int WizEditorInsertTableForm::getCols()
{
    return ui->editCols->text().toInt();
}

void WizEditorInsertTableForm::clear()
{
    ui->editRows->setFocus();
    ui->editRows->clear();
    ui->editCols->clear();
}

void WizEditorInsertTableForm::on_pushButton_cancel_clicked()
{
    reject();
}

void WizEditorInsertTableForm::on_pushButton_ok_clicked()
{
    accept();
}
