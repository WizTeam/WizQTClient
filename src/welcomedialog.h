#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>
#include "share/wizverifyaccount.h"
#include "share/wizcreateaccount.h"

class QAbstractButton;
class QUrl;

namespace Ui {
    class WelcomeDialog;
}

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget *parent = 0);
    ~WelcomeDialog();

private:
    Ui::WelcomeDialog *ui;
    //
    CWizVerifyAccount m_verifyAccount;
    //
    void enableControls(bool b);
public:
    void setUserId(const QString& strUserId);
    QString userId() const;
    QString password() const;
    //
public Q_SLOTS:
    virtual void accept();

public slots:
    void verifyAccountDone(bool succeeded, const CString& errorMessage);
    void on_web_linkClicked(const QUrl & url);
    void on_labelProxy_linkActivated(const QString & link);
};

#endif // WELCOMEDIALOG_H
