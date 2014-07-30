#ifndef WIZCODEEDITORDIALOG_H
#define WIZCODEEDITORDIALOG_H

#include <QDialog>
#include <QWebView>
#include <QPointer>

class CWizExplorerApp;
class QComboBox;
class QWebView;
class QMenu;

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
    explicit WizCodeEditorDialog(QWidget *parent = 0);

signals:
    void insertHtmlRequest(QString strHtml);

public slots:
    void renderCodeToHtml();
    void onButtonOKClicked();
    void onButtonCancleClicked();

private:
    void initCodeTypeCombox();

private:
    QComboBox *m_codeType;
    CWizCodeEditorView *m_codeEditor;
    QWebView *m_codeBrowser;
};

#endif // WIZCODEEDITORDIALOG_H
