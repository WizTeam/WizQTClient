#include "wizdownloadobjectdata.h"
#include "wizdatabase.h"
#include "wizapi.h"
#include "wizdownloadobjectdatadialog.h"


CWizDownloadObjectData::CWizDownloadObjectData(CWizDatabase& db, const CString& strAccountsApiURL, CWizSyncEvents& events, const WIZOBJECTDATA& data)
    : CWizApi(db, strAccountsApiURL, events)
    , m_data(data)
    , m_bDownloaded(FALSE)
{

}

void CWizDownloadObjectData::onXmlRpcError(const CString& strMethodName, WizXmlRpcError err, int errorCode, const CString& errorMessage)
{
    CWizApi::onXmlRpcError(strMethodName, err, errorCode, errorMessage);
    //
    emit downloadOnjectDataDone(m_bDownloaded);
}

//step 1 login
void CWizDownloadObjectData::onClientLogin()
{
    downloadObjectData(m_data);
}
//
//step 2 download object data
void CWizDownloadObjectData::onDownloadObjectDataCompleted(const WIZOBJECTDATA& data)
{
    CWizApi::onDownloadObjectDataCompleted(data);
    m_data.arrayData = data.arrayData;
    m_bDownloaded = TRUE;
    callClientLogout();
}

//step 3 logout
void CWizDownloadObjectData::onClientLogout()
{
    emit downloadOnjectDataDone(m_bDownloaded);
}
//
void CWizDownloadObjectData::startDownload()
{
    callClientLogin(m_db.GetUserId(), m_db.GetPassword());
}



BOOL WizDownloadObjectData(CWizDatabase& db, const WIZOBJECTDATA& data, QWidget* parent)
{
    return WizDownloadObjectDataDialog::downloadObjectData(db, WIZ_API_URL, data, parent);
}
