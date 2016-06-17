#ifndef WIZEXECUTINGACTIONDIALOG_H
#define WIZEXECUTINGACTIONDIALOG_H

#include <QDialog>
#include <functional>


namespace Ui {
class WizExecutingActionDialog;
}

class WizExecutingActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizExecutingActionDialog(QString description, int threadId, std::function<void(void)> fun, QWidget *parent = 0);
    ~WizExecutingActionDialog();

    virtual void reject();

private:
    Ui::WizExecutingActionDialog *ui;
public:
    static void executeAction(QString description, int threadId, std::function<void(void)> fun);
};

#endif // WIZEXECUTINGACTIONDIALOG_H
