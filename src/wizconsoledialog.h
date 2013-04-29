#ifndef WIZCONSOLEDIALOG_H
#define WIZCONSOLEDIALOG_H

#include <QDialog>
#include <QScrollBar>
#include <QFile>

#include "wizdef.h"

namespace Ui {
    class CWizConsoleDialog;
}

class CWizConsoleDialog: public QDialog
{
    Q_OBJECT

public:
    CWizConsoleDialog(CWizExplorerApp& app, QWidget* parent = 0);
    ~CWizConsoleDialog();

protected:
    virtual void showEvent(QShowEvent *event);

private:
    CWizExplorerApp& m_app;
    Ui::CWizConsoleDialog* m_ui;

    QTextCodec* m_codec;
    QString m_data;
    int m_nEntries;

    void load();
    void readByLine(QIODevice* dev);
    void resetCount();

public Q_SLOTS:
    void on_editConsole_textChanged();
    void on_editConsole_copyAvailable(bool yes);
    void on_buttonClear_clicked();
    void bufferLog_readyRead();
    void on_btnSaveAs_clicked();
    void on_btnCopyToClipboard_clicked();
};

#endif // WIZCONSOLEDIALOG_H
