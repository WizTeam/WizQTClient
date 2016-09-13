#ifndef INITBIZCERTDIALOG_H
#define INITBIZCERTDIALOG_H

#include <QDialog>

namespace Ui {
class WizInitBizCertDialog;
}

class QAbstractButton;
class WizDatabase;

class WizInitBizCertDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizInitBizCertDialog(WizDatabase* pDatabase, QWidget *parent = 0);
    ~WizInitBizCertDialog();
    //
    QString userCertPassword() { return m_userPassword; }
private:
    void verifyCert();

public slots:
    virtual void accept();

private:
    Ui::WizInitBizCertDialog *ui;
    QString m_userPassword;
    WizDatabase* m_pDb;
};

#endif // INITBIZCERTDIALOG_H
