#ifndef WIZCRASHREPORTDIALOG_H
#define WIZCRASHREPORTDIALOG_H

#include <QDialog>

#ifdef Q_OS_MAC

namespace Ui {
class WizCrashReportDialog;
}

class QPlainTextEdit;
class WizCrashReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizCrashReportDialog(const QString& text, QWidget *parent = 0);
    ~WizCrashReportDialog();

private slots:
    void on_btn_yes_clicked();

    void on_btn_no_clicked();

    void on_btn_details_clicked();

private:
    Ui::WizCrashReportDialog *ui;
    QString m_reports;
    QPlainTextEdit* m_textEdit;
};

#endif

#endif // WIZCRASHREPORTDIALOG_H
