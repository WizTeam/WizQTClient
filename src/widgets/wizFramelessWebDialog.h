#ifndef WIZFRAMELESSWEBDIALOG_H
#define WIZFRAMELESSWEBDIALOG_H

#include <QDialog>

class QWebFrame;

class CWizFramelessWebDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CWizFramelessWebDialog(const QString& strUrl, QWidget *parent = 0);

signals:
    void doNotShowThisAgain(bool bAgain);


public slots:
    void Execute(const QString& strFunction, QVariant param1, QVariant param2,
                                  QVariant param3, QVariant param4);
    void onJavaScriptWindowObjectCleared();

private:
    void openUrlInBrowser(const QString& strUrl);

private:
    QWebFrame *m_frame;

};

#endif // WIZFRAMELESSWEBDIALOG_H
