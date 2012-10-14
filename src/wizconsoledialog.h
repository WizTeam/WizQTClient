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
public:
    CWizConsoleDialog(CWizExplorerApp& app, QWidget* parent = 0);
    const QString* data() { return &m_data; }

private:
    Ui::CWizConsoleDialog* m_ui;
    CWizExplorerApp& m_app;

    QString m_data;

    void load();

private Q_SLOTS:
    void on_editConsole_textChanged();
};

#endif // WIZCONSOLEDIALOG_H
