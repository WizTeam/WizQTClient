#ifndef WIZVERIFICATIONCODEDIALOG_H
#define WIZVERIFICATIONCODEDIALOG_H

#include <QDialog>

namespace Ui {
class CWizVerificationCodeDialog;
}

class CWizVerificationCodeDownloader : public QObject
{
    Q_OBJECT
public:
    CWizVerificationCodeDownloader(QObject* parent = 0);

    void download(const QString& strCaptchaID);

signals:
    void downloadFinished(const QByteArray& byData);
};

class CWizVerificationCodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizVerificationCodeDialog(QWidget *parent = 0);
    virtual ~CWizVerificationCodeDialog();

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

    Ui::CWizVerificationCodeDialog *ui;
    QString m_strCaptchaID;
    CWizVerificationCodeDownloader* m_downloader;
};

#endif // WIZVERIFICATIONCODEDIALOG_H
