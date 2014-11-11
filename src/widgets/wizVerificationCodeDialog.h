#ifndef WIZVERIFICATIONCODEDIALOG_H
#define WIZVERIFICATIONCODEDIALOG_H

#include <QDialog>

class CWizVerificationCodeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CWizVerificationCodeDialog(QWidget *parent = 0);

    void showVerificationImage(const QString& strUrl);
    void showVerificationImage(const QImage& image);

    QString getVerificationCode();
signals:
    void verificationCodeInputed(const QString& strCode);

public slots:
    void on_OKButton_clicked();
    void on_image_downloaded(const QString& strFile);

private:
    void showImage(const QImage& image);
};

#endif // WIZVERIFICATIONCODEDIALOG_H
