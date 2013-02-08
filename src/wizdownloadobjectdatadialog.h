#ifndef WIZDOWNLOADOBJECTDATADIALOG_H
#define WIZDOWNLOADOBJECTDATADIALOG_H

#include <QDialog>
#include <QPointer>

#include "share/wizDatabaseManager.h"
#include "share/wizdownloadobjectdata.h"

namespace Ui {
    class CWizDownloadObjectDataDialog;
}

class CWizDownloadObjectDataDialog
    : public QDialog
{
    Q_OBJECT

public:
    explicit CWizDownloadObjectDataDialog(CWizDatabaseManager& db, QWidget *parent = 0);
    ~CWizDownloadObjectDataDialog();

    // call this method instead of open()/show()/exec()
    void downloadData(const WIZOBJECTDATA& data);

    bool isUserCancled() const { return m_bUserCancled; }

private:
    Ui::CWizDownloadObjectDataDialog *ui;
    QPointer<CWizDownloadObjectData> m_downloader;
    bool m_bUserCancled;

public Q_SLOTS:
    void downloader_done(bool succeeded);
    void downloader_progress(int pos);
    void onButtonCancle_clicked();

};


#endif // WIZDOWNLOADOBJECTDATADIALOG_H
