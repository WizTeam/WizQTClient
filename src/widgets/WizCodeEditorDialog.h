#ifndef WIZCODEEDITORDIALOG_H
#define WIZCODEEDITORDIALOG_H

#include <QDialog>
#include <QWebEngineView>
#include <QPointer>
#include "WizDef.h"
#include "share/WizWebEngineView.h"

class WizExplorerApp;
class QComboBox;
class QWebView;
class QMenu;
class QPlainTextEdit;
class WizDocumentWebView;
class WizWebEngineView;

class WizCodeEditorDialog : public WizWebEngineViewContainerDialog
{
    Q_OBJECT
public:
    explicit WizCodeEditorDialog(WizExplorerApp& app, WizDocumentWebView *external, QWidget *parent = 0);
    void setCode(const QString& strCode);
    //

    static bool selectAll();
    static bool undo();
    static bool copy();
    static bool cut();
    static bool paste();

signals:
    void insertHtmlRequest(QString strHtml);

public slots:
    void insertHtml(const QString& strResultDiv);

    QString getLastCodeType();
    void saveLastCodeType(const QString& codeType);

protected:
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void closeEvent(QCloseEvent *);

private:
    WizWebEngineView *m_codeBrowser;
    WizExplorerApp& m_app;
    WizDocumentWebView *m_external;
};

#endif // WIZCODEEDITORDIALOG_H
