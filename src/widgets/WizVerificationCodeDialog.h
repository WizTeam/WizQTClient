#ifndef WIZVERIFICATIONCODEDIALOG_H
#define WIZVERIFICATIONCODEDIALOG_H

#include <QDialog>

namespace Ui {
class WizVerificationCodeDialog;
}

class WizVerificationCodeDownloader : public QObject
{
    Q_OBJECT
public:
    WizVerificationCodeDownloader(QObject* parent = 0);

    void download(const QString& strCaptchaID);

signals:
    void downloadFinished(const QByteArray& byData);
};

class WizVerificationCodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizVerificationCodeDialog(QWidget *parent = 0);
    virtual ~WizVerificationCodeDialog();

    int verificationRequest(const QString& strCaptchaID);
    QString getVerificationCode() const;

signals:
    void verificationCodeInputed(const QString& strId);

private slots:
    void on_btn_image_clicked();

    void on_btn_OK_clicked();

    void inputFinished();

    void on_image_downloaded(const QByteArray& ba);

private:
    void downloadImage();

    Ui::WizVerificationCodeDialog *ui;
    QString m_strCaptchaID;
    WizVerificationCodeDownloader* m_downloader;
};

#endif // WIZVERIFICATIONCODEDIALOG_H
