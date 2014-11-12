#include "wizVerificationCodeDialog.h"
#include "ui_wizVerificationCodeDialog.h"
#include "share/wizmisc.h"
#include "utils/pathresolve.h"
#include <QIcon>
#include <QPixmap>
#include <QEventLoop>
#include <QDebug>
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
}

CWizVerificationCodeDialog::~CWizVerificationCodeDialog()
{
    delete ui;
}

int CWizVerificationCodeDialog::verificationRequest(const QString& strUrl)
{
    m_strUrl = strUrl;
    qDebug() << "try to download from url : " << m_strUrl;
    QPixmap pix;
    downloadImage(pix);

    QIcon icon(pix);
    qDebug() << "pix to icon : " << icon.isNull();
    ui->btn_image->setFixedSize(pix.size());
    ui->btn_image->setMinimumSize(pix.size());
    ui->btn_image->setIcon(icon);

    return exec();
}

QString CWizVerificationCodeDialog::getVerificationCode() const
{
    return ui->lineEdit->text();
}

void CWizVerificationCodeDialog::downloadImage(QPixmap& pix)
{
    QNetworkAccessManager m_WebCtrl;
    QNetworkRequest request(m_strUrl);
    QEventLoop loop;
    loop.connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
    QNetworkReply* reply = m_WebCtrl.get(request);
    loop.exec();

    QByteArray byData = reply->readAll();
    qDebug() << "load image from : " << m_strUrl << " image empty : " << byData.isEmpty() << " reply error : " << reply->error();;
    qDebug() << "imaged data : " << byData;
    pix.loadFromData(byData);
    qDebug() << "pix load from data , pix empty : " << pix.isNull();
}

void CWizVerificationCodeDialog::on_btn_image_clicked()
{
    QPixmap pix;
    downloadImage(pix);
    QIcon icon(pix);
    ui->btn_image->setIcon(icon);
    if (pix.isNull())
    {
        QMessageBox::information(this, tr("Inof"), tr("Too many request, please wait for one minute."));
        reject();
    }
}

void CWizVerificationCodeDialog::on_btn_OK_clicked()
{
    QString strCode = ui->lineEdit->text();
    accept();

    emit verificationCodeInputed(strCode);
}
