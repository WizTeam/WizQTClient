#ifndef WIZLINEINPUTDIALOG_H
#define WIZLINEINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>

namespace Ui {
    class CWizLineInputDialog;
}

class CWizLineInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizLineInputDialog(const QString& strTitle,
                                 const QString& strHint,
                                 const QString& strDefault = "",
                                 QWidget *parent = 0,
                                 QLineEdit::EchoMode echo = QLineEdit::Normal);
    ~CWizLineInputDialog();

public:
    QString input();

    void setOKButtonEnable(bool enable);
    //
    void setOKHandler(std::function<bool(QString)> handler) { m_okHandler = handler; }

    virtual void accept();
signals:
    void textChanged(const QString&);

private slots:

private:
    Ui::CWizLineInputDialog *ui;
    QString m_strDefault;
    std::function<bool(QString)> m_okHandler;
};

#endif // WIZLINEINPUTDIALOG_H
