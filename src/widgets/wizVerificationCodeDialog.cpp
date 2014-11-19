#include "wizVerificationCodeDialog.h"
#include "ui_wizVerificationCodeDialog.h"
#include "share/wizmisc.h"
#include "utils/pathresolve.h"
#include <QIcon>
#include <QPixmap>
#include <QEventLoop>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

CWizVerificationCodeDialog::CWizVerificationCodeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizVerificationCodeDialog)
{
    ui->setupUi(this);
    ui->btn_image->setToolTip(tr("Click to refresh verification code"));
    connect(ui->lineEdit, SIGNAL(returnPressed()), SLOT(inputFinished()));
}

CWizVerificationCodeDialog::~CWizVerificationCodeDialog()
{
    delete ui;
}

int CWizVerificationCodeDialog::verificationRequest(const QString& strUrl)
{
    m_strUrl = strUrl;
    QPixmap pix;
    if (downloadImage(pix))
    {
        QIcon icon(pix);
        ui->btn_image->setFixedSize(pix.size());
        ui->btn_image->setMinimumSize(pix.size());
        ui->btn_image->setIcon(icon);

        return exec();
    }

    return QDialog::Rejected;
}

QString CWizVerificationCodeDialog::getVerificationCode() const
{
    return ui->lineEdit->text();
}

bool CWizVerificationCodeDialog::downloadImage(QPixmap& pix)
{
    QNetworkAccessManager m_WebCtrl;
    QNetworkRequest request(m_strUrl);
    QEventLoop loop;
    loop.connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
    QNetworkReply* reply = m_WebCtrl.get(request);
    loop.exec();

    QByteArray byData = reply->readAll();
    pix.loadFromData(byData);
    if (pix.isNull() && !byData.isEmpty())
    {
        QMessageBox::information(this->parentWidget(), tr("Inof"), tr("Too many request, please wait for one minute."));
        return false;
    }
    return true;
}

void CWizVerificationCodeDialog::on_btn_image_clicked()
{
    QPixmap pix;
    if (downloadImage(pix))
    {
        QIcon icon(pix);
        ui->btn_image->setIcon(icon);
    }
    else
    {
        reject();
    }
}

void CWizVerificationCodeDialog::on_btn_OK_clicked()
{
    QString strCode = ui->lineEdit->text();
    accept();

    emit verificationCodeInputed(strCode);
}

void CWizVerificationCodeDialog::inputFinished()
{
    on_btn_OK_clicked();
}
