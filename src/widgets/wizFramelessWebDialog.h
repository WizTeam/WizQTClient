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
    void doNotShowDialologAgain();


public slots:
    virtual void OnExecuteCommand(const QString& strFunction, QVariant param1, QVariant param2,
                                  QVariant param3, QVariant param4, QVariant* pvRet);
    void onJavaScriptWindowObjectCleared();

private:
    void openUrlInBrowser(const QString& strUrl);

private:
    QWebFrame *m_frame;

};

#endif // WIZFRAMELESSWEBDIALOG_H
