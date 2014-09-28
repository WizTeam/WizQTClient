#include "wizLineInputDialog.h"
#include "ui_wizLineInputDialog.h"
#include <QPushButton>

CWizLineInputDialog::CWizLineInputDialog(const QString& strTitle,
                                         const QString& strHint,
                                         const QString& strDefault /* = "" */,
                                         QWidget *parent /* = 0 */, QLineEdit::EchoMode echo)
    : QDialog(parent)
    , ui(new Ui::CWizLineInputDialog)
{
    ui->setupUi(this);
    setFixedSize(size());

    setWindowTitle(strTitle);
    ui->labelHint->setText(strHint);

    ui->editInput->setText(strDefault);
    ui->editInput->selectAll();
    ui->editInput->setEchoMode(echo);
    m_strDefault = strDefault;

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

}

CWizLineInputDialog::~CWizLineInputDialog()
{
    delete ui;
}

QString CWizLineInputDialog::input()
{
    QString strText = ui->editInput->text();

    // only trigger value change if user modified default value
    if (strText == m_strDefault) {
        return "";
    }


    return strText;
}
