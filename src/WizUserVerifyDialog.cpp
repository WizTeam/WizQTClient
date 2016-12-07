#include "WizUserVerifyDialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>

#include <sync/WizToken.h>

WizUserVerifyDialog::WizUserVerifyDialog(const QString& strUser,
                                           const QString& strHint,
                                           QWidget *parent)
    : QDialog(parent)
{
    m_labelHint = new QLabel(strHint, this);

    QLabel* labelUser = new QLabel(tr("User Name:"), this);
    QLabel* labelPasswd = new QLabel(tr("Password:"), this);

    m_editUser = new QLineEdit(strUser, this);
    m_editUser->setDisabled(true);

    m_editPasswd = new QLineEdit(this);
    m_editPasswd->setEchoMode(QLineEdit::Password);

    QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, SIGNAL(accepted()), this, SLOT(on_btnAccept_clicked()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));

    QGridLayout* layout = new QGridLayout();
    layout->addWidget(labelUser, 0, 0);
    layout->addWidget(m_editUser, 0, 1);
    layout->addWidget(labelPasswd, 1, 0);
    layout->addWidget(m_editPasswd, 1, 1);

    QVBoxLayout* layoutMain = new QVBoxLayout();
    setLayout(layoutMain);
    layoutMain->addWidget(m_labelHint);
    layoutMain->addSpacing(10);
    layoutMain->addLayout(layout);
    layoutMain->addWidget(btnBox);
    layoutMain->setAlignment(layout, Qt::AlignHCenter);
    layoutMain->setAlignment(btnBox, Qt::AlignRight);

    setFixedSize(sizeHint());
}

QString WizUserVerifyDialog::password()
{
    return m_editPasswd->text();
}

void WizUserVerifyDialog::on_btnAccept_clicked()
{
    if (!m_editPasswd->text().isEmpty()) {
        WizToken::setPasswd(m_editPasswd->text());
        accept();
    }
}
