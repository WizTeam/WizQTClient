#include "initbizcertdialog.h"
#include "ui_initbizcertdialog.h"
#include "share/wizMessageBox.h"
#include "sync/wizKMServer.h"
#include "widgets/wizexecutingactiondialog.h"
#include "share/wizthreads.h"
#include "share/wizDatabase.h"
#include "share/wizenc.h"
#include "sync/apientry.h"

InitBizCertDialog::InitBizCertDialog(CWizDatabase* pDatabase, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InitBizCertDialog),
    m_pDb(pDatabase)
{
    ui->setupUi(this);
}

InitBizCertDialog::~InitBizCertDialog()
{
    delete ui;
}

void InitBizCertDialog::verifyCert()
{
    QString adminPassword = ui->editAdminPassword->text();
    QString userPassword = ui->editUserPassword->text();
    QString userPassword2 = ui->editUserPassword2->text();
    QString hint = ui->editPasswordHint->text();
    //
    if (adminPassword.isEmpty())
    {
        CWizMessageBox::information(this, tr("Please enter the password of cert created by administrator."));
        ui->editAdminPassword->setFocus();
        return;
    }
    if (userPassword.isEmpty())
    {
        CWizMessageBox::information(this, tr("Please enter the user password."));
        ui->editUserPassword->setFocus();
        return;
    }
    if (userPassword != userPassword2)
    {
        CWizMessageBox::information(this, tr("The passwords does not match."));
        ui->editUserPassword->setFocus();
        return;
    }
    //
    CWizDatabase* personalDb = m_pDb->getPersonalDatabase();
    QString userId = personalDb->GetUserId();
    QString password = personalDb->GetPassword();
    //
    QString bizGuid = m_pDb->bizGuid();
    //
    WizExecutingActionDialog::executeAction(tr("Verifying cert..."), WIZ_THREAD_DEFAULT, [&]{

        bool ret = false;
        QString error;
        //
        CWizKMAccountsServer asServer(CommonApiEntry::syncUrl());
        if (asServer.Login(userId, password))
        {
            QString n;
            QString e;
            QString encrypted_d;
            QString adminHint;
            if (asServer.GetAdminBizCert(asServer.GetToken(), bizGuid, n, e, encrypted_d, adminHint))
            {
                QString d;
                if (WizAESDecryptBase64StringToString(adminPassword, encrypted_d, d) && !d.isEmpty())
                {
                    QString newEncrypted_d;
                    if (WizAESEncryptStringToBase64String(userPassword, d, newEncrypted_d) && !newEncrypted_d.isEmpty())
                    {
                        //
                        if (asServer.SetUserBizCert(bizGuid, n, e, newEncrypted_d, hint))
                        {
                            if (m_pDb->SetUserCert(n, e, newEncrypted_d, hint))
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
                            error = asServer.GetLastErrorMessage();
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
                error = asServer.GetLastErrorMessage();
            }
        }
        else
        {
            error = asServer.GetLastErrorMessage();
        }
        //
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
            if (ret)
            {
                QDialog::accept();
            }
            else
            {
                CWizMessageBox::critical(this, error);
            }
        });
        //
    });
}

void InitBizCertDialog::accept()
{
    verifyCert();
}

