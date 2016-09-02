#ifndef INITBIZCERTDIALOG_H
#define INITBIZCERTDIALOG_H

#include <QDialog>

namespace Ui {
class InitBizCertDialog;
}

class QAbstractButton;
class CWizDatabase;

class InitBizCertDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InitBizCertDialog(CWizDatabase* pDatabase, QWidget *parent = 0);
    ~InitBizCertDialog();
    //
    QString userCertPassword() { return m_userPassword; }
private:
    void verifyCert();

public slots:
    virtual void accept();

private:
    Ui::InitBizCertDialog *ui;
    QString m_userPassword;
    CWizDatabase* m_pDb;
};

#endif // INITBIZCERTDIALOG_H
