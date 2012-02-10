#ifndef NEWFOLDERDIALOG_H
#define NEWFOLDERDIALOG_H

#include <QDialog>

namespace Ui {
    class NewFolderDialog;
}

class NewFolderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewFolderDialog(QWidget *parent = 0);
    ~NewFolderDialog();

public:
    QString folderName();
private:
    Ui::NewFolderDialog *ui;
};

#endif // NEWFOLDERDIALOG_H
