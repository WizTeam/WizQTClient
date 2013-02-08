#ifndef WIZNEWDIALOG_H
#define WIZNEWDIALOG_H

#include <QDialog>

namespace Ui {
    class CWizNewDialog;
}

class CWizNewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizNewDialog(const QString& hint, QWidget *parent = 0);
    ~CWizNewDialog();

public:
    void clear();
    QString input();

private:
    Ui::CWizNewDialog *ui;
};

#endif // WIZNEWDIALOG_H
