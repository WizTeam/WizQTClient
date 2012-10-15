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

    // who invoke exec() or show() method who response for ajust scrollbar position.
    QScrollBar* vScroll;

private:
    CWizExplorerApp& m_app;

    Ui::CWizConsoleDialog* m_ui;

    QString m_data;

    void load();
    void appendLogs(const QString& strLog);

public slots:
    void on_editConsole_textChanged();
    void on_buttonClear_clicked();
    void on_bufferLogReady();
};

#endif // WIZCONSOLEDIALOG_H
