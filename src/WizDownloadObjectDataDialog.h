#ifndef WIZDOWNLOADOBJECTDATADIALOG_H
#define WIZDOWNLOADOBJECTDATADIALOG_H

#include <QDialog>
#include <QPointer>

#include "share/WizDatabaseManager.h"
#include "share/WizDownloadObjectData.h"

namespace Ui {
    class WizDownloadObjectDataDialog;
}

class WizDownloadObjectDataDialog
    : public QDialog
{
    Q_OBJECT

public:
    explicit WizDownloadObjectDataDialog(WizDatabaseManager& db, QWidget *parent = 0);
    ~WizDownloadObjectDataDialog();

    // call this method instead of open()/show()/exec()
    void downloadData(const WIZOBJECTDATA& data);

    bool isUserCancled() const { return m_bUserCancled; }

private:
    Ui::WizDownloadObjectDataDialog *ui;
    QPointer<WizDownloadObjectData> m_downloader;
    bool m_bUserCancled;

public Q_SLOTS:
    void downloader_done(bool succeeded);
    void downloader_progress(int pos);
    void onButtonCancle_clicked();

};


#endif // WIZDOWNLOADOBJECTDATADIALOG_H
