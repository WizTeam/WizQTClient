#ifndef WIZDOWNLOADOBJECTDATA_H
#define WIZDOWNLOADOBJECTDATA_H

#include "wizapi.h"


class CWizDownloadObjectData : public CWizApi
{
    Q_OBJECT

public:
    CWizDownloadObjectData(CWizDatabase& db, \
                           const CString& strAccountsApiURL, \
                           const WIZOBJECTDATA& data);

    void startDownload();

private:
    WIZOBJECTDATA m_data;
    bool m_bDownloaded;

protected:
    virtual void onXmlRpcError(const QString& strMethodName, \
                               WizXmlRpcError err, int errorCode, \
                               const QString& errorMessage);
    virtual void onClientLogin();
    virtual void onDownloadObjectDataCompleted(const WIZOBJECTDATA& data);
    virtual void onClientLogout();

Q_SIGNALS:
    void downloadDone(bool succeeded);
};

bool WizDownloadObjectData(CWizDatabase& db, const WIZOBJECTDATA& data, QWidget* parent);

bool WizPrepareDocument(CWizDatabase& db, const WIZDOCUMENTDATA& data, QWidget* parent);
bool WizPrepareAttachment(CWizDatabase& db, const WIZDOCUMENTATTACHMENTDATA& data, QWidget* parent);

#endif // WIZDOWNLOADOBJECTDATA_H
