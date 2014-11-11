#include "wizVerificationCodeDialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QImage>
#include <QFile>

CWizVerificationCodeDialog::CWizVerificationCodeDialog(QWidget *parent) :
    QDialog(parent)
{

}

void CWizVerificationCodeDialog::showVerificationImage(const QString& strUrl)
{
    //download image
}

void CWizVerificationCodeDialog::showVerificationImage(const QImage& image)
{
    showImage(image);
}

void CWizVerificationCodeDialog::on_OKButton_clicked()
{
    accept();

    QString strCode;
    emit verificationCodeInputed(strCode);
}

void CWizVerificationCodeDialog::on_image_downloaded(const QString& strFile)
{
    if (QFile::exists(strFile))
    {
        QImage image(strFile);
        showImage(image);
    }
}

void CWizVerificationCodeDialog::showImage(const QImage& image)
{

    exec();
}
