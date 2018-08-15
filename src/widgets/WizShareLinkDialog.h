#ifndef CWIZSHARELINKDIALOG_H
#define CWIZSHARELINKDIALOG_H

#include <QDialog>
#include <QtWebEngine>
#include <QPropertyAnimation>
#include "share/WizObject.h"
#include <QWebEngineView>
#include "share/WizWebEngineView.h"



class WizUserSettings;
class WizWebEngineView;

class WizShareLinkDialog : public WizWebEngineViewContainerDialog
{
    Q_OBJECT
public:
    WizShareLinkDialog(WizUserSettings& settings, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~WizShareLinkDialog();

    virtual QSize sizeHint() const;

    void shareDocument(const WIZDOCUMENTDATA& doc);

    Q_INVOKABLE void notifyEvent(const QString& event, const QVariant& params);

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
    Q_INVOKABLE void setFormateISO8601StringParam(const QString& param);
    Q_INVOKABLE QString formateISO8601String();

    Q_PROPERTY(QString kbGuid READ getKbGuid)
    Q_PROPERTY(QString documentGuid READ getGuid)
    Q_PROPERTY(QString title READ getTitle)
    Q_PROPERTY(QString shareLinkFirstTips READ getShareLinkFirstTips WRITE setShareLinkFirstTips)
    Q_PROPERTY(QString localLanguage READ getLocalLanguage)

    Q_PROPERTY(QString formateISO8601StringParam WRITE setFormateISO8601StringParam)
    Q_PROPERTY(QString formateISO8601StringResult READ formateISO8601String NOTIFY formateISO8601StringChanged)

public slots:
    void loadHtml();


signals:
    void tokenObtained();
    void formateISO8601StringChanged();

private:
    WizUserSettings& m_settings;
    WizWebEngineView* m_view;
    WIZDOCUMENTDATA m_doc;
    QString m_formateISO8601StringParam;
};

#endif // CWIZSHARELINKDIALOG_H
