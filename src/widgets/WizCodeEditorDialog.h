#ifndef WIZCODEEDITORDIALOG_H
#define WIZCODEEDITORDIALOG_H

#include <QDialog>
#include <QWebEngineView>
#include <QPointer>
#include "wizdef.h"

class CWizExplorerApp;
class QComboBox;
class QWebView;
class QMenu;
class QPlainTextEdit;
class CWizDocumentWebView;
class WizWebEngineView;

class WizCodeEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WizCodeEditorDialog(CWizExplorerApp& app, CWizDocumentWebView *external, QWidget *parent = 0);
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
    CWizExplorerApp& m_app;
    CWizDocumentWebView *m_external;
};

#endif // WIZCODEEDITORDIALOG_H
