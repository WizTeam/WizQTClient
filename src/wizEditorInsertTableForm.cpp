#include "wizEditorInsertTableForm.h"
#include "ui_wizEditorInsertTableForm.h"

#include <QtGui>

CWizEditorInsertTableForm::CWizEditorInsertTableForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizEditorInsertTableForm)
{
    ui->setupUi(this);

    setWindowModality(Qt::ApplicationModal);

    QIntValidator *validator = new QIntValidator(1, 30, this);
    ui->editRows->setValidator(validator);
    ui->editCols->setValidator(validator);
}

CWizEditorInsertTableForm::~CWizEditorInsertTableForm()
{
    delete ui;
}

int CWizEditorInsertTableForm::getRows()
{
    return ui->editRows->text().toInt();
}

int CWizEditorInsertTableForm::getCols()
{
    return ui->editCols->text().toInt();
}

void CWizEditorInsertTableForm::clear()
{
    ui->editRows->setFocus();
    ui->editRows->clear();
    ui->editCols->clear();
}

void CWizEditorInsertTableForm::on_pushButton_cancel_clicked()
{
    reject();
}

void CWizEditorInsertTableForm::on_pushButton_ok_clicked()
{
    accept();
}
