#include "wizNewDialog.h"
#include "ui_wizNewDialog.h"

CWizNewDialog::CWizNewDialog(const QString& hint, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizNewDialog)
{
    ui->setupUi(this);
    ui->labelHint->setText(hint);
    setFixedSize(size());
    setWindowModality(Qt::ApplicationModal);
}

CWizNewDialog::~CWizNewDialog()
{
    delete ui;
}

QString CWizNewDialog::input()
{
    return ui->editInput->text();
}

void CWizNewDialog::clear()
{
    ui->editInput->clear();
}
