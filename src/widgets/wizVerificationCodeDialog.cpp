#include "wizVerificationCodeDialog.h"
#include "ui_wizVerificationCodeDialog.h"

#include <QIcon>
#include <QPixmap>
#include <QEventLoop>
#if QT_VERSION > 0x050000
#include <QtConcurrent>
#else
#include <QtConcurrentRun>
#endif
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "sync/apientry.h"
#include "utils/pathresolve.h"
#include "share/wizmisc.h"
#include "share/wizMessageBox.h"

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

int CWizVerificationCodeDialog::verificationRequest(const QString& strCaptchaID)
{
    m_strCaptchaID = strCaptchaID;
    ui->btn_image->setText(tr("Downloading..."));

    //
    downloadImage();

    return exec();
}

QString CWizVerificationCodeDialog::getVerificationCode() const
{
    return ui->lineEdit->text();
}

void CWizVerificationCodeDialog::downloadImage()
{
    QtConcurrent::run([this](){
        QNetworkAccessManager m_WebCtrl;
        QString strUrl = WizService::CommonApiEntry::captchaUrl(m_strCaptchaID);
        QNetworkRequest request(strUrl);
        QEventLoop loop;
        loop.connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)), SLOT(quit()));
        QNetworkReply* reply = m_WebCtrl.get(request);
        loop.exec();

        QByteArray byData = reply->readAll();

        QMetaObject::invokeMethod(this, "on_image_downloaded", Qt::QueuedConnection,
                                  Q_ARG(QByteArray, byData));
    });
}

void CWizVerificationCodeDialog::on_btn_image_clicked()
{
    ui->btn_image->setText(tr("Downloading..."));
    downloadImage();
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

void CWizVerificationCodeDialog::on_image_downloaded(const QByteArray& ba)
{
    QPixmap pix;
    pix.loadFromData(ba);
    if (pix.isNull() && !ba.isEmpty())
    {
        CWizMessageBox::warning(parentWidget(), tr("Info"), tr("Too many request, please wait for one minute."));
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
