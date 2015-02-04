#ifndef WIZPRIVATEDEPLOYDIALOG_H
#define WIZPRIVATEDEPLOYDIALOG_H

#include <QDialog>

namespace Ui {
class CWizPrivateDeployDialog;
}

class CWizPrivateDeployDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizPrivateDeployDialog(QWidget *parent = 0);
    ~CWizPrivateDeployDialog();

private slots:
    void on_checkBox_useCustomSettings_toggled(bool checked);

    void on_pushButton_cancel_clicked();

    void on_pushButton_ok_clicked();

    void on_label_proxyLink_linkActivated(const QString &link);

private:
    Ui::CWizPrivateDeployDialog *ui;
};

#endif // WIZPRIVATEDEPLOYDIALOG_H
