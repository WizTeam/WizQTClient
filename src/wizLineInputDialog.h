#ifndef WIZLINEINPUTDIALOG_H
#define WIZLINEINPUTDIALOG_H

#include <QDialog>

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
                                 QWidget *parent = 0);
    ~CWizLineInputDialog();

public:
    QString input();

private:
    Ui::CWizLineInputDialog *ui;
    QString m_strDefault;
};

#endif // WIZLINEINPUTDIALOG_H
