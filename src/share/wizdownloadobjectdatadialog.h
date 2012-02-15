#ifndef WIZDOWNLOADOBJECTDATADIALOG_H
#define WIZDOWNLOADOBJECTDATADIALOG_H

#include <QDialog>
#include "wizdownloadobjectdata.h"

namespace Ui {
    class WizDownloadObjectDataDialog;
}

class WizDownloadObjectDataDialog
    : public QDialog
    , public CWizSyncEvents
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
protected:
    virtual void addErrorLog(const CString& strMsg);
    virtual void changeObjectDataProgress(int pos);
public slots:
    virtual void reject();
    void on_downloader_downloadObjectDataDone(bool succeeded);
public:
    static bool downloadObjectData(CWizDatabase& db, const CString& strAccountsApiURL, const WIZOBJECTDATA& data, QWidget* parent);
};



#endif // WIZDOWNLOADOBJECTDATADIALOG_H
