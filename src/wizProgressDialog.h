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
    explicit CWizProgressDialog(QWidget *parent = 0, bool showStop = true);
    ~CWizProgressDialog();

signals:
    void stopRequest();

public slots:
    void setProgress(QString strObjGUID, int nMax, int nCurrent);
    void setProgress(int nMax, int nCurrent);
    void setActionString(const QString& strAction);

    
private slots:
    void on_btn_stop_clicked();

    void on_btn_hide_clicked();

private:
    Ui::CWizProgressDialog *ui;
};

#endif // WIZPROGRESSDIALOG_H
