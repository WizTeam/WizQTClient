#include "WizLineInputDialog.h"
#include "ui_WizLineInputDialog.h"
#include <QPushButton>
#include "share/WizUIBase.h"

WizLineInputDialog::WizLineInputDialog(const QString& strTitle,
                                         const QString& strHint,
                                         const QString& strDefault /* = "" */,
                                         QWidget *parent /* = 0 */, QLineEdit::EchoMode echo)
    : QDialog(parent)
    , ui(new Ui::WizLineInputDialog)
{
    ui->setupUi(this);
    //setFixedSize(size());

    setWindowTitle(strTitle);
    ui->labelHint->setText(strHint);

    ui->editInput->setText(strDefault);
    ui->editInput->selectAll();
    ui->editInput->setEchoMode(echo);
    m_strDefault = strDefault;

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connect(ui->editInput, SIGNAL(textChanged(QString)), SIGNAL(textChanged(QString)));
    //
    if (isDarkMode()) {
        ui->editInput->setStyleSheet(QString("background-color:%1").arg(WizColorLineEditorBackground.name()));
        WizApplyDarkModeStyles(this);
    }
}

WizLineInputDialog::~WizLineInputDialog()
{
    delete ui;
}

QString WizLineInputDialog::input()
{
    QString strText = ui->editInput->text();

    // only trigger value change if user modified default value
    if (strText == m_strDefault) {
        return "";
    }


    return strText;
}

void WizLineInputDialog::setOKButtonEnable(bool enable)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void WizLineInputDialog::setErrorMessage(QString message)
{
    ui->labelError->setText(message);
}

void WizLineInputDialog::accept()
{
    if (m_okHandler)
    {
        if (!m_okHandler(ui->editInput->text()))
            return;
    }
    //
    QDialog::accept();
}

