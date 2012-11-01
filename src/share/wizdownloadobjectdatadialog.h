#ifndef WIZDOWNLOADOBJECTDATADIALOG_H
#define WIZDOWNLOADOBJECTDATADIALOG_H

#include <QDialog>
#include "wizdownloadobjectdata.h"

namespace Ui {
    class WizDownloadObjectDataDialog;
}

class WizDownloadObjectDataDialog
    : public QDialog
{
    Q_OBJECT

public:
    explicit WizDownloadObjectDataDialog(CWizDatabase& db, const CString& strAccountsApiURL, const WIZOBJECTDATA& data, QWidget *parent = 0);
    ~WizDownloadObjectDataDialog();

private:
    Ui::WizDownloadObjectDataDialog *ui;
    WIZOBJECTDATA m_data;
    CWizDownloadObjectData m_downloader;
    QStringList m_slError;
    bool m_bUserCanceled;

public Q_SLOTS:
    virtual void reject();
    void downloader_downloadObjectDataDone(bool succeeded);
    void downloader_progress(int pos);

public:
    static bool downloadObjectData(CWizDatabase& db, const CString& strAccountsApiURL, const WIZOBJECTDATA& data, QWidget* parent);
};



#endif // WIZDOWNLOADOBJECTDATADIALOG_H
