#ifndef WIZUSERSERVICEEXPRDIALOG_H
#define WIZUSERSERVICEEXPRDIALOG_H

#include <QDialog>


namespace Ui {
class WizUserServiceExprDialog;
}


class WizUserServiceExprDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizUserServiceExprDialog(QWidget *parent);
    //
    void setUserInfo(bool free, bool biz, QString groupName);
public Q_SLOTS:
    void helpClicked();
    void okClicked();
private:
    Ui::WizUserServiceExprDialog *ui;
};


#endif // WIZUSERSERVICEEXPRDIALOG_H
