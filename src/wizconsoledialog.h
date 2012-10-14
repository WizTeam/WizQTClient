#ifndef WIZCONSOLEDIALOG_H
#define WIZCONSOLEDIALOG_H

#include <QDialog>
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

private:
    Ui::CWizConsoleDialog* m_ui;
    CWizExplorerApp& m_app;

    QString m_data;

    void load();

    void appendLogs(const QString& strLog);

public slots:
    void on_editConsole_textChanged();
    void on_bufferLogReady();
};

#endif // WIZCONSOLEDIALOG_H
