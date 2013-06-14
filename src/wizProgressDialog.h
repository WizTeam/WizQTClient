#ifndef WIZPROGRESSDIALOG_H
#define WIZPROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class CWizProgressDialog;
}

class CWizProgressDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CWizProgressDialog(QWidget *parent = 0);
    ~CWizProgressDialog();

    void setActionString(const QString& strAction);
    void setNotifyString(const QString& strNotify);
    void setProgress(int nMax, int nCurrent);
    
private:
    Ui::CWizProgressDialog *ui;
};

#endif // WIZPROGRESSDIALOG_H
