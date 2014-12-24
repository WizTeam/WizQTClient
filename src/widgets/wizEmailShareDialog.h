#ifndef WIZEMAILSHAREDIALOG_H
#define WIZEMAILSHAREDIALOG_H

#include <QDialog>
#include "wizdef.h"
#include "share/wizobject.h"

namespace Ui {
class CWizEmailShareDialog;
}

class CWizEmailShareDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizEmailShareDialog(CWizExplorerApp& app, QWidget *parent = 0);
    ~CWizEmailShareDialog();

    void setNote(const WIZDOCUMENTDATA& note);

private slots:
    void on_toolButton_send_clicked();

    void on_toolButton_contracts_clicked();

private:
    QString getExInfo();
    void mailShareFinished(int nCode, const QString& returnMessage);
    void processReturnMessage(const QString& returnMessage, int& nCode, QString& message);

private:
    Ui::CWizEmailShareDialog *ui;
    WIZDOCUMENTDATA m_note;
    CWizExplorerApp& m_app;
};

#endif // WIZEMAILSHAREDIALOG_H
