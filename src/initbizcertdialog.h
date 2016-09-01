#ifndef INITBIZCERTDIALOG_H
#define INITBIZCERTDIALOG_H

#include <QDialog>

namespace Ui {
class InitBizCertDialog;
}

class QAbstractButton;

class InitBizCertDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InitBizCertDialog(QWidget *parent = 0);
    ~InitBizCertDialog();

private:
    void VerifyCert();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::InitBizCertDialog *ui;
};

#endif // INITBIZCERTDIALOG_H
