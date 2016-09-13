#include "WizVerificationCodeDialog.h"
#include "ui_WizVerificationCodeDialog.h"

#include <QIcon>
#include <QPixmap>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "sync/WizApiEntry.h"
#include "utils/WizPathResolve.h"
#include "share/WizMisc.h"
#include "share/WizMessageBox.h"
#include "share/WizThreads.h"
#include "share/WizEventLoop.h"

WizVerificationCodeDialog::WizVerificationCodeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizVerificationCodeDialog)
    , m_downloader(new WizVerificationCodeDownloader(this))
{
    ui->setupUi(this);
    ui->btn_image->setToolTip(tr("Click to refresh verification code"));
    connect(ui->lineEdit, SIGNAL(returnPressed()), SLOT(inputFinished()));

    connect(m_downloader, SIGNAL(downloadFinished(QByteArray)), SLOT(on_image_downloaded(QByteArray)));
}

WizVerificationCodeDialog::~WizVerificationCodeDialog()
{
    delete ui;
}

int WizVerificationCodeDialog::verificationRequest(const QString& strCaptchaID)
{
    m_strCaptchaID = strCaptchaID;
    ui->btn_image->setText(tr("Downloading..."));

    //
    downloadImage();

    return exec();
}

QString WizVerificationCodeDialog::getVerificationCode() const
{
    return ui->lineEdit->text();
}

void WizVerificationCodeDialog::downloadImage()
{
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        m_downloader->download(m_strCaptchaID);
    });
}

void WizVerificationCodeDialog::on_btn_image_clicked()
{
    ui->btn_image->setText(tr("Downloading..."));
    downloadImage();
}

void WizVerificationCodeDialog::on_btn_OK_clicked()
{
    QString strCode = ui->lineEdit->text();
    accept();

    emit verificationCodeInputed(strCode);
}

void WizVerificationCodeDialog::inputFinished()
{
    on_btn_OK_clicked();
}

void WizVerificationCodeDialog::on_image_downloaded(const QByteArray& ba)
{
    QPixmap pix;
    pix.loadFromData(ba);
    if (pix.isNull() && !ba.isEmpty())
    {
        WizMessageBox::warning(parentWidget(), tr("Info"), tr("Too many request, please wait for one minute."));
        reject();
    }
    else
    {
        QIcon icon(pix);
        ui->btn_image->setText("");
        ui->btn_image->setFixedSize(pix.size());
        ui->btn_image->setMinimumSize(pix.size());
        ui->btn_image->setIcon(icon);

        update();
    }
}


WizVerificationCodeDownloader::WizVerificationCodeDownloader(QObject* parent)
    : QObject(parent)
{

}

void WizVerificationCodeDownloader::download(const QString& strCaptchaID)
{
    QNetworkAccessManager m_WebCtrl;
    QString strUrl = WizCommonApiEntry::captchaUrl(strCaptchaID);
    QNetworkRequest request(strUrl);
    QNetworkReply* reply = m_WebCtrl.get(request);
    WizAutoTimeOutEventLoop loop(reply);
    loop.exec();

    QByteArray byData = loop.result();

    emit downloadFinished(byData);
}
