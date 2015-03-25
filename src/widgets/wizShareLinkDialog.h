#ifndef CWIZSHARELINKDIALOG_H
#define CWIZSHARELINKDIALOG_H

#include <QDialog>
#include <QWebView>

class CWizShareLinkDialog : public QDialog
{
    Q_OBJECT
public:
    CWizShareLinkDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~CWizShareLinkDialog();

    Q_INVOKABLE void logAction(const QString& strAction);
    Q_INVOKABLE void writeToLog(const QString& strLog);
    Q_INVOKABLE void getToken(const QString& callback);

public slots:
    void loadHtml();

    void onJavaScriptWindowObject();

signals:
    void tokenObtained(const QString& strToken, const QString& callback);

private:
    QWebView* m_view;
};

#endif // CWIZSHARELINKDIALOG_H
