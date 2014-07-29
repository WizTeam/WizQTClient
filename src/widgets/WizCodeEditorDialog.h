#ifndef WIZCODEEDITORDIALOG_H
#define WIZCODEEDITORDIALOG_H

#include <QDialog>

class CWizExplorerApp;
class QComboBox;
class QPlainTextEdit;
class QTextBrowser;

class WizCodeEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WizCodeEditorDialog(CWizExplorerApp& app, QWidget *parent = 0);

signals:

public slots:


private:
    QComboBox *m_codeType;
    QPlainTextEdit *m_codeEditor;
    QTextBrowser *m_codeBrowser;
};

#endif // WIZCODEEDITORDIALOG_H
