#ifndef WIZLINEINPUTDIALOG_H
#define WIZLINEINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <functional>

namespace Ui {
    class WizLineInputDialog;
}

class WizLineInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizLineInputDialog(const QString& strTitle,
                                 const QString& strHint,
                                 const QString& strDefault = "",
                                 QWidget *parent = 0,
                                 QLineEdit::EchoMode echo = QLineEdit::Normal);
    ~WizLineInputDialog();

public:
    QString input();

    void setOKButtonEnable(bool enable);
    void setErrorMessage(QString message);
    //
    void setOKHandler(std::function<bool(QString)> handler) { m_okHandler = handler; }

    virtual void accept();
signals:
    void textChanged(const QString&);

private slots:

private:
    Ui::WizLineInputDialog *ui;
    QString m_strDefault;
    std::function<bool(QString)> m_okHandler;
};

#endif // WIZLINEINPUTDIALOG_H
