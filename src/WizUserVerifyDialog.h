#ifndef WIZUSERVERIFYDIALOG_H
#define WIZUSERVERIFYDIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;

class WizUserVerifyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizUserVerifyDialog(const QString& strUser,
                                  const QString& strHint,
                                  QWidget* parent = 0);

    QString password();

protected:
    QLineEdit* m_editUser;
    QLineEdit* m_editPasswd;
    QLabel* m_labelHint;

public Q_SLOTS:
    void on_btnAccept_clicked();
};

#endif // WIZUSERVERIFYDIALOG_H
