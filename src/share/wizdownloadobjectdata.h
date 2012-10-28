#ifndef WIZDOWNLOADOBJECTDATA_H
#define WIZDOWNLOADOBJECTDATA_H

#include "wizapi.h"


class CWizDownloadObjectData : public CWizApi
{
    Q_OBJECT

public:
    CWizDownloadObjectData(CWizDatabase& db, \
                           const CString& strAccountsApiURL, \
                           CWizSyncEvents& events, \
                           const WIZOBJECTDATA& data);

    void startDownload();

private:
    WIZOBJECTDATA m_data;
    BOOL m_bDownloaded;

protected:
    virtual void onXmlRpcError(const CString& strMethodName, \
                               WizXmlRpcError err, int errorCode, \
                               const CString& errorMessage);
    virtual void onClientLogin();
    virtual void onDownloadObjectDataCompleted(const WIZOBJECTDATA& data);
    virtual void onClientLogout();

Q_SIGNALS:
    void done(bool succeeded);
};

bool WizDownloadObjectData(CWizDatabase& db, const WIZOBJECTDATA& data, QWidget* parent);

bool WizPrepareDocument(CWizDatabase& db, const WIZDOCUMENTDATA& data, QWidget* parent);
bool WizPrepareAttachment(CWizDatabase& db, const WIZDOCUMENTATTACHMENTDATA& data, QWidget* parent);

#endif // WIZDOWNLOADOBJECTDATA_H
