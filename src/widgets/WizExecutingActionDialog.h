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

    explicit WizExecutingActionDialog(QString description, int threadId, std::function<void(void)> fun, QWidget *parent = 0);
public:
    ~WizExecutingActionDialog();

    virtual void reject();
    virtual void showEvent(QShowEvent *);

private:
    Ui::WizExecutingActionDialog *ui;
    int m_threadId;
    std::function<void(void)> m_fun;
    bool m_first;
public:
    static void executeAction(QString description, int threadId, std::function<void(void)> fun);
};

#endif // WIZEXECUTINGACTIONDIALOG_H
