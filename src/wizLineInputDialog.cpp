#include "wizLineInputDialog.h"
#include "ui_wizLineInputDialog.h"

CWizLineInputDialog::CWizLineInputDialog(const QString& strTitle,
                                         const QString& strHint,
                                         QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CWizLineInputDialog)
{
    ui->setupUi(this);
    setFixedSize(size());

    setWindowTitle(strTitle);
    ui->labelHint->setText(strHint);
}

CWizLineInputDialog::~CWizLineInputDialog()
{
    delete ui;
}

QString CWizLineInputDialog::input()
{
    return ui->editInput->text();
}
