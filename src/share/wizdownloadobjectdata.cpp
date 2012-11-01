#include "wizdownloadobjectdata.h"
#include "wizdatabase.h"
#include "wizapi.h"
#include "wizdownloadobjectdatadialog.h"


CWizDownloadObjectData::CWizDownloadObjectData(CWizDatabase& db, \
                                               const CString& strAccountsApiURL, \
                                               const WIZOBJECTDATA& data)
    : CWizApi(db, strAccountsApiURL)
    , m_data(data)
    , m_bDownloaded(false)
{

}

void CWizDownloadObjectData::startDownload()
{
    callClientLogin(m_db.GetUserId(), m_db.GetPassword2());
}

void CWizDownloadObjectData::onXmlRpcError(const CString& strMethodName, WizXmlRpcError err, int errorCode, const CString& errorMessage)
{
    CWizApi::onXmlRpcError(strMethodName, err, errorCode, errorMessage);
    Q_EMIT downloadDone(m_bDownloaded);
}

void CWizDownloadObjectData::onClientLogin()
{
    downloadObjectData(m_data);
}

void CWizDownloadObjectData::onDownloadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    CWizApi::onDownloadObjectDataCompleted(data);
    m_data.arrayData = data.arrayData;
    m_bDownloaded = true;
    callClientLogout();
}

void CWizDownloadObjectData::onClientLogout()
{
    Q_EMIT downloadDone(m_bDownloaded);
}


bool WizDownloadObjectData(CWizDatabase& db, const WIZOBJECTDATA& data, QWidget* parent)
{
    return WizDownloadObjectDataDialog::downloadObjectData(db, WIZ_API_URL, data, parent);
}

bool WizPrepareDocument(CWizDatabase& db, const WIZDOCUMENTDATA& data, QWidget* parent)
{
    if (!db.IsObjectDataDownloaded(data.strGUID, "document")
        || !PathFileExists(db.GetDocumentFileName(data.strGUID))
        )
    {
        WIZOBJECTDATA obj;
        obj.strObjectGUID = data.strGUID;
        obj.strDisplayName = data.strTitle;
        obj.eObjectType = wizobjectDocument;
        if (!WizDownloadObjectData(db, obj, parent))
            return false;
    }

    return true;
}

bool WizPrepareAttachment(CWizDatabase& db, const WIZDOCUMENTATTACHMENTDATA& data, QWidget* parent)
{
    if (!db.IsObjectDataDownloaded(data.strGUID, "attachment")
        || !PathFileExists(db.GetAttachmentFileName(data.strGUID))
        )
    {
        WIZOBJECTDATA obj;
        obj.strObjectGUID = data.strGUID;
        obj.strDisplayName = data.strName;
        obj.eObjectType = wizobjectDocumentAttachment;
        if (!WizDownloadObjectData(db, obj, parent))
            return false;
    }

    return true;
}

