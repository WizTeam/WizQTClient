#ifndef WIZVERIFICATIONCODEDIALOG_H
#define WIZVERIFICATIONCODEDIALOG_H

#include <QDialog>

namespace Ui {
class CWizVerificationCodeDialog;
}

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
};

#endif // WIZVERIFICATIONCODEDIALOG_H
