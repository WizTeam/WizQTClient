#ifndef CWIZSHARELINKDIALOG_H
#define CWIZSHARELINKDIALOG_H

#include <QDialog>
#include <QWebView>
#include <QPropertyAnimation>
#include "share/wizobject.h"

class CWizUserSettings;
class CWizShareLinkDialog : public QDialog
{
    Q_OBJECT
public:
    CWizShareLinkDialog(CWizUserSettings& settings, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~CWizShareLinkDialog();

    virtual QSize sizeHint() const;

    void shareDocument(const WIZDOCUMENTDATA& doc);

    Q_INVOKABLE void logAction(const QString& strAction);
    Q_INVOKABLE void writeToLog(const QString& strLog);
    Q_INVOKABLE void getToken();

    Q_INVOKABLE QString getKbGuid();
    Q_INVOKABLE QString getGuid();
    Q_INVOKABLE QString getTitle();
    Q_INVOKABLE void resizeEx(int nWidth, int nHeight);
    Q_INVOKABLE void openindefaultbrowser(const QString& url);
    Q_INVOKABLE void dragcaption(int x, int y);
    Q_INVOKABLE void copyLink(const QString& link, const QString& callBack);

    Q_INVOKABLE QString getShareLinkFirstTips();
    Q_INVOKABLE void setShareLinkFirstTips(const QString& value);
    Q_INVOKABLE QString getLocalLanguage();
    Q_INVOKABLE QString formateISO8601String(const QString& value);

public slots:
    void loadHtml();

    void onJavaScriptWindowObject();


signals:
    void tokenObtained();

private:
    CWizUserSettings& m_settings;
    QWebView* m_view;
    WIZDOCUMENTDATA m_doc;
    QPropertyAnimation* m_animation;
};

#endif // CWIZSHARELINKDIALOG_H
