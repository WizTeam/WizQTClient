#include "WizInitBizCertDialog.h"
#include "ui_WizInitBizCertDialog.h"
#include "share/WizMessageBox.h"
#include "sync/WizKMServer.h"
#include "widgets/WizExecutingActionDialog.h"
#include "share/WizThreads.h"
#include "share/WizDatabase.h"
#include "share/WizEnc.h"
#include "sync/WizApiEntry.h"

WizInitBizCertDialog::WizInitBizCertDialog(WizDatabase* pDatabase, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WizInitBizCertDialog),
    m_pDb(pDatabase)
{
    ui->setupUi(this);
}

WizInitBizCertDialog::~WizInitBizCertDialog()
{
    delete ui;
}

void WizInitBizCertDialog::verifyCert()
{
    QString adminPassword = ui->editAdminPassword->text();
    QString userPassword = ui->editUserPassword->text();
    QString userPassword2 = ui->editUserPassword2->text();
    QString hint = ui->editPasswordHint->text();
    //
    if (adminPassword.isEmpty())
    {
        WizMessageBox::information(this, tr("Please enter the password of cert created by administrator."));
        ui->editAdminPassword->setFocus();
        return;
    }
    if (userPassword.isEmpty())
    {
        WizMessageBox::information(this, tr("Please enter the user password."));
        ui->editUserPassword->setFocus();
        return;
    }
    if (userPassword != userPassword2)
    {
        WizMessageBox::information(this, tr("The passwords does not match."));
        ui->editUserPassword->setFocus();
        return;
    }
    //
    WizDatabase* personalDb = m_pDb->personalDatabase();
    QString userId = personalDb->getUserId();
    QString password = personalDb->getPassword();
    //
    QString bizGuid = m_pDb->bizGuid();
    //
    WizExecutingActionDialog::executeAction(tr("Verifying cert..."), WIZ_THREAD_DEFAULT, [&]{

        bool ret = false;
        QString error;
        //
        WizKMAccountsServer asServer;
        if (asServer.login(userId, password))
        {
            QString n;
            QString e;
            QString encrypted_d;
            QString adminHint;
            if (asServer.getAdminBizCert(bizGuid, n, e, encrypted_d, adminHint))
            {
                QString d;
                if (WizAESDecryptBase64StringToString(adminPassword, encrypted_d, d) && !d.isEmpty())
                {
                    QString newEncrypted_d;
                    if (WizAESEncryptStringToBase64String(userPassword, d, newEncrypted_d) && !newEncrypted_d.isEmpty())
                    {
                        //
                        if (asServer.setUserBizCert(bizGuid, n, e, newEncrypted_d, hint))
                        {
                            if (m_pDb->setUserCert(n, e, newEncrypted_d, hint))
                            {
                                m_userPassword = userPassword;
                                ret = true;
                            }
                            else
                            {
                                error = tr("Can't save cert.");
                            }
                        }
                        else
                        {
                            error = asServer.getLastErrorMessage();
                        }

                    }
                    else
                    {
                        error = tr("Can't encrypt cert.");
                    }
                }
                else
                {
                    error = tr("Invalid admin password.");
                }
            }
            else
            {
                error = asServer.getLastErrorMessage();
            }
        }
        else
        {
            error = asServer.getLastErrorMessage();
        }
        //
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
            if (ret)
            {
                QDialog::accept();
            }
            else
            {
                WizMessageBox::critical(this, error);
            }
        });
        //
    });
}

void WizInitBizCertDialog::accept()
{
    verifyCert();
}

