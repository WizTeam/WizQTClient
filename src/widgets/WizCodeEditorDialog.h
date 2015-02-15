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

class CWizCodeEditorView : public QWebView
{
    Q_OBJECT
public:
    CWizCodeEditorView(QWidget *parent = 0) : QWebView(parent){}

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event)
    {
        //do nothing
        Q_UNUSED(event);
    }

};

class WizCodeEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WizCodeEditorDialog(CWizExplorerApp& app, QWidget *parent = 0);
    void setCode(const QString& strCode);
signals:
    void insertHtmlRequest(QString strHtml);

public slots:
    void renderCodeToHtml();
    void onButtonOKClicked();
    void onButtonCancelClicked();

protected:
    void changeEvent(QEvent * event);

private:
    void initCodeTypeCombox();
    void saveLastCodeType();
private:
    QComboBox *m_codeType;
    QPlainTextEdit *m_codeEditor;
    QWebView *m_codeBrowser;
    CWizExplorerApp& m_app;
};

#endif // WIZCODEEDITORDIALOG_H
