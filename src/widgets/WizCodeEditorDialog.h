#ifndef WIZCODEEDITORDIALOG_H
#define WIZCODEEDITORDIALOG_H

#include <QDialog>
#include <QWebView>
#include <QWebEngineView>
#include <QPointer>
#include "wizdef.h"
#include "wizDocumentWebEngine.h"

class CWizExplorerApp;
class QComboBox;
class QMenu;
class QPlainTextEdit;
class CWizDocumentWebView;
class QLineEdit;
class WizCodeEditorDialog;
class CWizCodeExternal;


class WizCodeEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WizCodeEditorDialog(CWizExplorerApp& app, QObject *external, QWidget *parent = 0);
    void setCode(const QString& strCode);

signals:
    void insertHtmlRequest(QString strHtml);

public slots:
    void insertHtml(const QString& strResultDiv);

    QString getLastCodeType();
    void saveLastCodeType(const QString& codeType);
    //
    void onHtmlLoaded(bool ok);
    void runJs();
protected:
    void changeEvent(QEvent * event);

private:
    QWebEngineView *m_codeBrowser;
    CWizExplorerApp& m_app;
    CWizCodeExternal *m_external;
    QLineEdit* m_edit;
};

#endif // WIZCODEEDITORDIALOG_H
