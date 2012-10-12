#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

#include "wizdef.h"

namespace Ui {
    class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(CWizExplorerApp& app, QWidget *parent = 0);
    ~AboutDialog();

private:
    CWizExplorerApp& m_app;
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
