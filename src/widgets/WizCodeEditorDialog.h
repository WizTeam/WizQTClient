#ifndef WIZCODEEDITORDIALOG_H
#define WIZCODEEDITORDIALOG_H

#include <QDialog>
#include <QWebView>
#include <QPointer>
#include "wizdef.h"

class CWizExplorerApp;
class QComboBox;
class QWebView;
class QMenu;
class QPlainTextEdit;
class CWizDocumentWebView;

class WizCodeEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WizCodeEditorDialog(CWizExplorerApp& app, CWizDocumentWebView *external, QWidget *parent = 0);
    void setCode(const QString& strCode);
signals:
    void insertHtmlRequest(QString strHtml);

public slots:
    void registerJSObject();
    void insertHtml(const QString& strResultDiv);

    QString getLastCodeType();
    void saveLastCodeType(const QString& codeType);

        // if use webengine
//    void onHtmlLoaded(bool ok);
//    void runJs();
protected:
    void changeEvent(QEvent * event);

private:
    QWebView *m_codeBrowser;
    CWizExplorerApp& m_app;
    CWizDocumentWebView *m_external;
};

#endif // WIZCODEEDITORDIALOG_H
