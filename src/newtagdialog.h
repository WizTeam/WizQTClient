#ifndef NEWTAGDIALOG_H
#define NEWTAGDIALOG_H

#include <QDialog>

namespace Ui {
    class NewTagDialog;
}

class NewTagDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewTagDialog(QWidget *parent = 0);
    ~NewTagDialog();

public:
    QString tagName() const;

private:
    Ui::NewTagDialog *ui;
};

#endif // NEWTAGDIALOG_H
