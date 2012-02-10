#include "wizdatabase.h"
#include <QDir>
#include <QUrl>

#include <algorithm>

#include "wizhtml2zip.h"

#include "zip/wizzip.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class CWizDocument


CString GetResoucePathFromFile(const CString& strHtmlFileName)
{
    if (!PathFileExists(strHtmlFileName))
        return CString();
    //
    CString strTitle = WizExtractFileTitle(strHtmlFileName);
    //
    CString strPath = ::WizExtractFilePath(strHtmlFileName);
    CString strPath1 = strPath + strTitle + "_files/";
    CString strPath2 = strPath + strTitle + ".files/";
    if (PathFileExists(strPath1))
        return strPath1;
    if (PathFileExists(strPath2))
        return strPath2;
    return CString();
}

CWizDocument::CWizDocument(CWizDatabase& db, const WIZDOCUMENTDATA& data)
    : m_db(db)
    , m_data(data)
{
}
bool CWizDocument::UpdateDocument4(const QString& strHtml, const QString& strURL, int nFlags)
{
    return m_db.UpdateDocumentData(m_data, strHtml, strURL, nFlags);
}

BOOL CWizDocument::IsInDeletedItemsFolder()
{
    CString strDeletedItemsFolderLocation = m_db.GetDeletedItemsLocation();
    //
    return m_data.strLocation.startsWith(strDeletedItemsFolderLocation);
}


void CWizDocument::PermanentlyDelete(void)
{
    CWizDocumentAttachmentDataArray arrayAttachment;
    m_db.GetDocumentAttachments(m_data.strGUID, arrayAttachment);
    for (CWizDocumentAttachmentDataArray::const_iterator it = arrayAttachment.begin();
    it != arrayAttachment.end();
    it++)
    {
        CString strFileName = m_db.GetAttachmentFileName(it->strGUID);
        ::WizDeleteFile(strFileName);
        //
        m_db.DeleteAttachment(*it, true);
    }
    //
    if (!m_db.DeleteDocument(m_data, TRUE))
    {
        TOLOG1(_T("Failed to delete document: %1"), m_data.strTitle);
        return;
    }
    //
    CString strZipFileName = m_db.GetDocumentFileName(m_data.strGUID);
    if (PathFileExists(strZipFileName))
    {
        WizDeleteFile(strZipFileName);
    }
}

void CWizDocument::MoveTo(QObject* p)
{
    CWizFolder* folder = dynamic_cast<CWizFolder*>(p);
    if (!folder)
        return;
    //
    MoveDocument(folder);
}

BOOL CWizDocument::MoveDocument(CWizFolder* pFolder)
{
    if (!pFolder)
        return FALSE;
    //
    CString strNewLocation = pFolder->Location();
    CString strOldLocation = m_data.strLocation;
    //

    m_data.strLocation = strNewLocation;
    if (!m_db.ModifyDocumentInfo(m_data))
    {
        m_data.strLocation = strOldLocation;
        TOLOG1(_T("Failed to modify document location %1."), m_data.strLocation);
        return FALSE;
    }
    //
    return TRUE;
}

void CWizDocument::Delete()
{
    if (IsInDeletedItemsFolder())
    {
        return PermanentlyDelete();
    }
    else
    {
        return MoveTo(m_db.GetDeletedItemsFolder());
    }

}

CString CWizDocument::GetMetaText()
{
    return "";
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class CWizFolder
CWizFolder::CWizFolder(CWizDatabase& db, const CString& strLocation)
    : m_db(db)
    , m_strLocation(strLocation)
{

}
BOOL CWizFolder::IsDeletedItems() const
{
    return m_db.GetDeletedItemsLocation() == Location();
}

BOOL CWizFolder::IsInDeletedItems() const
{
    return !IsDeletedItems() && Location().startsWith(m_db.GetDeletedItemsLocation());
}

QObject* CWizFolder::CreateDocument2(const QString& strTitle, const QString& strURL)
{
    WIZDOCUMENTDATA data;
    if (!m_db.CreateDocument(strTitle, "", m_strLocation, strURL, data))
        return NULL;
    //
    CWizDocument* pDoc = new CWizDocument(m_db, data);
    //pDoc->ref();
    return pDoc;
}
void CWizFolder::Delete()
{
    if (IsDeletedItems())
        return;
    //
    if (IsInDeletedItems())
    {
        //if (IDYES != WizMessageBox1(IDS_DELETE_FOLDER, GetName(), MB_YESNO | MB_ICONQUESTION))
        //    return S_FALSE;
        //
        if (!m_db.DeleteDocumentsByLocation(Location()))
        {
            TOLOG1(_T("Failed to delete documents by location; %1"), Location());
            return;
        }
        //
        m_db.LogDeletedFolder(Location());
    }
    else
    {
        CWizFolder deletedItems(m_db, m_db.GetDeletedItemsLocation());
        MoveTo(&deletedItems);
    }
}
void CWizFolder::MoveTo(QObject* dest)
{
    CWizFolder* pFolder = dynamic_cast<CWizFolder*>(dest);
    if (!pFolder)
        return;
    //
    if (IsDeletedItems())
        return;
    //
    if (!CanMove(this, pFolder))
    {
        TOLOG2(_T("Can move %1 to %2"), Location(), pFolder->Location());
        return;
    }
    //
    return MoveToLocation(pFolder->Location());
}

BOOL CWizFolder::CanMove(const CString& strSrcLocation, const CString& strDestLocation) const
{
    if (m_db.GetDeletedItemsLocation() == strSrcLocation)
        return FALSE;
    //
    if (strDestLocation.startsWith(strSrcLocation))
        return FALSE;
    //
    return TRUE;
}
BOOL CWizFolder::CanMove(CWizFolder* pSrc, CWizFolder* pDest) const
{
    return CanMove(pSrc->Location(), pDest->Location());
}

void CWizFolder::MoveToLocation(const CString& strDestLocation)
{
    if (!CanMove(Location(), strDestLocation))
        return;
    //
    CString strOldLocation = Location();
    CString strNewLocation;
    //
    CString strLocationName = CWizDatabase::GetLocationName(strOldLocation);
    //
    if (strDestLocation.IsEmpty())
    {
        strNewLocation = "/" + strLocationName + "/";
    }
    else
    {
        strNewLocation = strDestLocation + strLocationName + "/";
    }
    //
    CWizDocumentDataArray arrayDocument;
    if (!m_db.GetDocumentsByLocationIncludeSubFolders(strOldLocation, arrayDocument))
    {
        TOLOG1(_T("Failed to get documents by location (include sub folders): %1"), strOldLocation);
        return;
    }
    //
    for (CWizDocumentDataArray::const_iterator it = arrayDocument.begin();
        it != arrayDocument.end();
        it++)
    {
        WIZDOCUMENTDATA data = *it;
        //
        ATLASSERT(data.strLocation.startsWith(strOldLocation));
        if (!data.strLocation.startsWith(strOldLocation))
        {
            TOLOG(_T("Error location of document!"));
            continue;
        }
        //
        data.strLocation.Delete(0, strOldLocation.GetLength());
        data.strLocation.Insert(0, strNewLocation);
        //
        if (!m_db.ModifyDocumentInfo(data))
        {
            TOLOG(_T("Failed to move document to new folder!"));
            continue;
        }
    }
    //
    m_db.LogDeletedFolder(strOldLocation);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class CWizDatabase
CWizDatabase::CWizDatabase()
{
}

BOOL CWizDatabase::Open(const CString& strUserId, const CString& strPassword)
{
    m_strUserId = strUserId;
    m_strPassword = strPassword;
    BOOL bRet = CIndex::Open(GetIndexFileName()) && CThumbIndex::OpenThumb(GetThumbFileName());
    //
    //
    return bRet;
}

CString CWizDatabase::GetAccountDataPath() const
{
    ATLASSERT(!m_strUserId.isEmpty());
    //
    CString strPath = ::WizGetDataStorePath()+ m_strUserId + "/";
    WizEnsurePathExists(strPath);
    //
    return strPath;
}
CString CWizDatabase::GetIndexFileName() const
{
    CString strPath = GetAccountDataPath();
    return strPath + "index.db";
}
CString CWizDatabase::GetThumbFileName() const
{
    CString strPath = GetAccountDataPath();
    return strPath + "wizthumb.db";
}

CString CWizDatabase::GetDocumentsDataPath() const
{
    CString strPath = GetAccountDataPath() + "Notes/";
    WizEnsurePathExists(strPath);
    return  strPath;
}
CString CWizDatabase::GetAttachmentsDataPath() const
{
    CString strPath = GetAccountDataPath() + "Attachments/";
    WizEnsurePathExists(strPath);
    return  strPath;
}
__int64 CWizDatabase::GetObjectVersion(const CString& strObjectName)
{
    return GetMetaInt64("SYNC_INFO", strObjectName, -1) + 1;
}

BOOL CWizDatabase::SetObjectVersion(const CString& strObjectName, __int64 nVersion)
{
    return SetMetaInt64("SYNC_INFO", strObjectName, nVersion);
}

BOOL CWizDatabase::UpdateDeletedGUID(const WIZDELETEDGUIDDATA& data, CWizSyncEvents* pEvents)
{
    BOOL bRet = FALSE;
    //
    CString strType = WIZOBJECTDATA::ObjectTypeToTypeString(data.eType);
    BOOL bExists = FALSE;
    ObjectExists(data.strGUID, strType, bExists);
    if (!bExists)
        return TRUE;
    //
    bRet = DeleteObject(data.strGUID, strType, FALSE);
    //
    if (!bRet && pEvents)
    {
        pEvents->addErrorLog("Failed to delete object: " + data.strGUID + " type: " + strType);
    }
    //
    return bRet;
}

BOOL CWizDatabase::UpdateTag(const WIZTAGDATA& data, CWizSyncEvents* pEvents)
{
    BOOL bRet = FALSE;
    //
    WIZTAGDATA dataTemp;
    if (TagFromGUID(data.strGUID, dataTemp))
    {
        bRet = ModifyTagEx(data);
    }
    else
    {
        bRet = CreateTagEx(data);
    }
    //
    if (!bRet && pEvents)
    {
        pEvents->addErrorLog("Failed to update tag: " + data.strName);
    }
    //
    return bRet;
}
BOOL CWizDatabase::UpdateStyle(const WIZSTYLEDATA& data, CWizSyncEvents* pEvents)
{
    BOOL bRet = FALSE;
    //
    WIZSTYLEDATA dataTemp;
    if (StyleFromGUID(data.strGUID, dataTemp))
    {
        bRet = ModifyStyleEx(data);
    }
    else
    {
        bRet = CreateStyleEx(data);
    }
    //
    if (!bRet && pEvents)
    {
        pEvents->addErrorLog("Failed to update style: " + data.strName);
    }
    //
    return bRet;
}
BOOL CWizDatabase::UpdateDocument(const WIZDOCUMENTDATAEX& data, CWizSyncEvents* pEvents)
{
    BOOL bRet = FALSE;
    //
    WIZDOCUMENTDATAEX dataTemp;
    if (DocumentFromGUID(data.strGUID, dataTemp))
    {
        if (data.nObjectPart & WIZKM_XMLRPC_OBJECT_PART_INFO)
        {
            bRet = ModifyDocumentInfoEx(data);
            if (dataTemp.strDataMD5 != data.strDataMD5)
            {
                SetObjectDataDownloaded(data.strGUID, "document", FALSE);
            }
        }
        else
        {
            bRet = TRUE;
        }
    }
    else
    {
        ATLASSERT(data.nObjectPart & WIZKM_XMLRPC_OBJECT_PART_INFO);
        //
        bRet = CreateDocumentEx(data);
    }
    //
    if (!bRet && pEvents)
    {
        pEvents->addErrorLog("Failed to update document: " + data.strTitle);
    }
    //
    WIZDOCUMENTDATA dataRet = data;
    //
    if (!data.arrayParam.empty())
    {
        SetDocumentParams(dataRet, data.arrayParam);
    }
    if (!data.arrayTagGUID.empty())
    {
        SetDocumentTags(dataRet, data.arrayTagGUID);
    }
    //
    return bRet;
}
BOOL CWizDatabase::UpdateAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data, CWizSyncEvents* pEvents)
{
    BOOL bRet = FALSE;
    //
    WIZDOCUMENTATTACHMENTDATAEX dataTemp;
    if (AttachmentFromGUID(data.strGUID, dataTemp))
    {
        bRet = ModifyAttachmentInfoEx(data);
        //
        if (dataTemp.strDataMD5 != data.strDataMD5)
        {
            SetObjectDataDownloaded(data.strGUID, "attachment", FALSE);
        }
    }
    else
    {
        bRet = CreateAttachmentEx(data);
    }
    //
    if (!bRet && pEvents)
    {
        pEvents->addErrorLog("Failed to update attachment: " + data.strName);
    }
    //
    return bRet;
}
//

BOOL CWizDatabase::UpdateDeletedGUIDs(const std::deque<WIZDELETEDGUIDDATA>& arrayDeletedGUID, CWizSyncEvents* pEvents)
{
    if (arrayDeletedGUID.empty())
        return TRUE;
    //
    __int64 nVersion = -1;
    //
    BOOL bHasError = FALSE;
    for (std::deque<WIZDELETEDGUIDDATA>::const_iterator it = arrayDeletedGUID.begin();
        it != arrayDeletedGUID.end();
        it++)
    {
        const WIZDELETEDGUIDDATA& data = *it;
        //
        if (!UpdateDeletedGUID(data, pEvents))
        {
            bHasError = true;
        }
        //
        nVersion = std::max<__int64>(nVersion, data.nVersion);
    }
    //
    if (!bHasError)
    {
        SetObjectVersion(WIZDELETEDGUIDDATA::ObjectName(), nVersion);
    }
    //
    return !bHasError;
}

BOOL CWizDatabase::UpdateTags(const std::deque<WIZTAGDATA>& arrayTag, CWizSyncEvents* pEvents)
{
    if (arrayTag.empty())
        return FALSE;
    //
    __int64 nVersion = -1;
    //
    BOOL bHasError = FALSE;
    for (std::deque<WIZTAGDATA>::const_iterator it = arrayTag.begin();
        it != arrayTag.end();
        it++)
    {
        const WIZTAGDATA& data = *it;
        //
        if (pEvents)
        {
            pEvents->addLog("tag: " + data.strName);
        }
        //
        if (!UpdateTag(data, pEvents))
        {
            bHasError = true;
        }
        //
        nVersion = std::max<__int64>(nVersion, data.nVersion);
    }
    if (!bHasError)
    {
        SetObjectVersion(WIZTAGDATA::ObjectName(), nVersion);
    }
    //
    return !bHasError;
}


BOOL CWizDatabase::UpdateStyles(const std::deque<WIZSTYLEDATA>& arrayStyle, CWizSyncEvents* pEvents)
{
    if (arrayStyle.empty())
        return TRUE;
    //
    __int64 nVersion = -1;
    //
    BOOL bHasError = FALSE;
    for (std::deque<WIZSTYLEDATA>::const_iterator it = arrayStyle.begin();
        it != arrayStyle.end();
        it++)
    {
        const WIZSTYLEDATA& data = *it;
        //
        if (pEvents)
        {
            pEvents->addLog("style: " + data.strName);
        }
        //
        if (!UpdateStyle(data, pEvents))
        {
            bHasError = true;
        }
        //
        nVersion = std::max<__int64>(nVersion, data.nVersion);
    }
    if (!bHasError)
    {
        SetObjectVersion(WIZSTYLEDATA::ObjectName(), nVersion);
    }
    //
    return !bHasError;
}

BOOL CWizDatabase::UpdateDocuments(const std::deque<WIZDOCUMENTDATAEX>& arrayDocument, CWizSyncEvents* pEvents)
{
    if (arrayDocument.empty())
        return TRUE;
    //
    __int64 nVersion = -1;
    //
    BOOL bHasError = FALSE;
    for (std::deque<WIZDOCUMENTDATAEX>::const_iterator it = arrayDocument.begin();
        it != arrayDocument.end();
        it++)
    {
        const WIZDOCUMENTDATAEX& data = *it;
        //
        if (pEvents)
        {
            pEvents->addLog("note: " + data.strTitle);
        }
        //
        if (!UpdateDocument(data, pEvents))
        {
            bHasError = TRUE;
        }
        //
        nVersion = std::max<__int64>(nVersion, data.nVersion);
    }
    if (!bHasError)
    {
        SetObjectVersion(WIZDOCUMENTDATAEX::ObjectName(), nVersion);
    }
    //
    return !bHasError;
}


BOOL CWizDatabase::UpdateAttachments(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayAttachment, CWizSyncEvents* pEvents)
{
    if (arrayAttachment.empty())
        return TRUE;
    //
    __int64 nVersion = -1;
    //
    BOOL bHasError = FALSE;
    for (std::deque<WIZDOCUMENTATTACHMENTDATAEX>::const_iterator it = arrayAttachment.begin();
        it != arrayAttachment.end();
        it++)
    {
        const WIZDOCUMENTATTACHMENTDATAEX& data = *it;
        //
        if (pEvents)
        {
            pEvents->addLog("attachment: " + data.strName);
        }
        //
        if (!UpdateAttachment(data, pEvents))
        {
            bHasError = true;
        }
        //
        nVersion = std::max<__int64>(nVersion, data.nVersion);
    }
    if (!bHasError)
    {
        SetObjectVersion(WIZDOCUMENTATTACHMENTDATAEX::ObjectName(), nVersion);
    }
    //
    return !bHasError;
}

bool CWizDatabase::UpdateDocumentData(WIZDOCUMENTDATA& data, const QString& strHtml, const QString& strURL, int nFlags)
{
    CString strProcessedHtml(strHtml);
    CString strResourcePath = GetResoucePathFromFile(strURL);
    if (!strResourcePath.IsEmpty())
    {
        QUrl urlResource = QUrl::fromLocalFile(strResourcePath);
        strProcessedHtml.replace(urlResource.toString(), "index_files/");
    }
    //
    CString strZipFileName = GetDocumentFileName(data.strGUID);
    bool bZip = ::WizHtml2Zip(strURL, strProcessedHtml, strResourcePath, nFlags, "", strZipFileName);
    if (!bZip)
        return false;
    //
    return UpdateDocumentDataMD5(data, strZipFileName);
}

BOOL CWizDatabase::UpdateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName)
{
    BOOL bRet = CIndex::UpdateDocumentDataMD5(data, strZipFileName);
    //
    UpdateDocumentAbstract(data.strGUID);
    //
    return bRet;
}

BOOL CWizDatabase::IsDocumentDownloaded(const CString& strGUID)
{
    return IsObjectDataDownloaded(strGUID, "document");
}

BOOL CWizDatabase::IsAttachmentDownloaded(const CString& strGUID)
{
    return IsObjectDataDownloaded(strGUID, "attachment");
}

BOOL CWizDatabase::GetAllObjectsNeedToBeDownloaded(std::deque<WIZOBJECTDATA>& arrayData)
{
    CWizDocumentDataArray arrayDocument;
    CWizDocumentAttachmentDataArray arrayAttachment;
    GetNeedToBeDownloadedDocuments(arrayDocument);
    GetNeedToBeDownloadedAttachments(arrayAttachment);
    //
    arrayData.assign(arrayAttachment.begin(), arrayAttachment.end());
    arrayData.insert(arrayData.begin(), arrayDocument.begin(), arrayDocument.end());
    //
    return TRUE;
}
BOOL CWizDatabase::UpdateObjectLocalData(const WIZOBJECTDATA& data)
{
    CString strFileName = GetObjectFileName(data);
    //
    //
    // force to close file and stream //
    //
    {
        QFile file(strFileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            return FALSE;
        }
        //
        QDataStream out(&file);   // we will serialize the data into the file
        //
        out << data.arrayData;
    }
    //
    //
    if (data.eObjectType == wizobjectDocument)
    {
        WIZDOCUMENTDATA document;
        if (DocumentFromGUID(data.strObjectGUID, document))
        {
            emit documentDataModified(document);
        }
        //
        UpdateDocumentAbstract(data.strObjectGUID);
    }
    //
    SetObjectDataDownloaded(data.strObjectGUID, WIZOBJECTDATA::ObjectTypeToTypeString(data.eObjectType), TRUE);
    //
    return TRUE;
}

CString CWizDatabase::GetDocumentFileName(const CString& strGUID)
{
    return GetDocumentsDataPath() + strGUID + _T(".ziw");
}

CString CWizDatabase::GetAttachmentFileName(const CString& strGUID)
{
    return GetAttachmentsDataPath() + strGUID + _T(".dat");
}

CString CWizDatabase::GetObjectFileName(const WIZOBJECTDATA& data)
{
    if (data.eObjectType == wizobjectDocument)
        return GetDocumentFileName(data.strObjectGUID);
    else if (data.eObjectType == wizobjectDocumentAttachment)
        return GetAttachmentFileName(data.strObjectGUID);
    else
    {
        ATLASSERT(FALSE);
        return CString();
    }
}

BOOL CWizDatabase::GetAllRootLocations(CWizStdStringArray& arrayLocation)
{
    CWizStdStringArray arrayAll;
    GetAllLocations(arrayAll);
    //
    std::set<CString> setRoot;
    for (CWizStdStringArray::const_iterator it = arrayAll.begin();
    it != arrayAll.end();
    it++)
    {
        setRoot.insert(GetRootLocation(*it));
    }
    //
    arrayLocation.assign(setRoot.begin(), setRoot.end());
    //
    return TRUE;
}
BOOL CWizDatabase::GetChildLocations(const CString& strLocation, CWizStdStringArray& arrayLocation)
{
    return FALSE;
}



BOOL CWizDatabase::GetAllRootLocations(const CWizStdStringArray& arrayAllLocation, CWizStdStringArray& arrayLocation)
{
    std::set<CString> setRoot;
    for (CWizStdStringArray::const_iterator it = arrayAllLocation.begin();
        it != arrayAllLocation.end();
        it++)
    {
        setRoot.insert(GetRootLocation(*it));
    }
    //
    arrayLocation.assign(setRoot.begin(), setRoot.end());
    //
    return TRUE;
}
BOOL CWizDatabase::GetChildLocations(const CWizStdStringArray& arrayAllLocation, const CString& strLocation, CWizStdStringArray& arrayLocation)
{
    if (strLocation.IsEmpty())
        return GetAllRootLocations(arrayAllLocation, arrayLocation);
    //
    std::set<CString> setLocation;
    for (CWizStdStringArray::const_iterator it = arrayAllLocation.begin();
        it != arrayAllLocation.end();
        it++)
    {
        const CString& str = *it;
        if (str.length() > strLocation.length()
            && str.startsWith(strLocation))
        {
            int index = str.indexOf('/', strLocation.length() + 1);
            if (index > 0)
            {
                CString strChild = str.left(index + 1);
                setLocation.insert(strChild);
            }
        }
    }
    //
    arrayLocation.assign(setLocation.begin(), setLocation.end());
    //
    return TRUE;
}


BOOL CWizDatabase::IsInDeletedItems(const CString& strLocation)
{
    return strLocation.startsWith(GetDeletedItemsLocation());
}

BOOL CWizDatabase::LoadDocumentData(const CString& strDocumentGUID, QByteArray& arrayData)
{
    CString strFileName = GetDocumentFileName(strDocumentGUID);
    if (!PathFileExists(strFileName))
    {
        return FALSE;
    }
    //
    QFile file(strFileName);
    if (!file.open(QFile::ReadOnly))
        return FALSE;
    //
    arrayData = file.readAll();
    //
    return !arrayData.isEmpty();
}
BOOL CWizDatabase::LoadAttachmentData(const CString& strDocumentGUID, QByteArray& arrayData)
{
    CString strFileName = GetAttachmentFileName(strDocumentGUID);
    if (!PathFileExists(strFileName))
    {
        return FALSE;
    }
    //
    QFile file(strFileName);
    if (!file.open(QFile::ReadOnly))
        return FALSE;
    //
    arrayData = file.readAll();
    //
    return !arrayData.isEmpty();
}

BOOL CWizDatabase::UpdateDocumentAbstract(const CString& strDocumentGUID)
{
    CString strFileName = GetDocumentFileName(strDocumentGUID);
    if (!PathFileExists(strFileName))
        return FALSE;
    //
    WIZDOCUMENTDATA data;
    if (!DocumentFromGUID(strDocumentGUID, data))
        return FALSE;
    //
    CString strHtmlFileName;
    if (!DocumentToTempHtmlFile(data, strHtmlFileName))
        return FALSE;
    //
    CString strHtmlTempPath = WizExtractFilePath(strHtmlFileName);
    //
    CString strHtml;
    CString strAbstractFileName = strHtmlTempPath + "wiz_full.html";
    if (PathFileExists(strAbstractFileName))
    {
        ::WizLoadUnicodeTextFromFile(strAbstractFileName, strHtml);
    }
    else
    {
        ::WizLoadUnicodeTextFromFile(strHtmlFileName, strHtml);
    }
    //
    WIZABSTRACT abstract;
    abstract.guid = strDocumentGUID;
    ::WizHtml2Text(strHtml, abstract.text);
    //
    abstract.text.replace("\r", "");
    abstract.text.replace("\t", " ");
    abstract.text.replace(QRegExp("[\\n]+"), " ");
    abstract.text.replace(QRegExp("[\\s]+"), " ");
    if (abstract.text.length() > 2000)
    {
        abstract.text = abstract.text.left(2000);
    }
    //
    CString strResourcePath = WizExtractFilePath(strHtmlFileName) + "index_files/";
    CWizStdStringArray arrayImageFileName;
    ::WizEnumFiles(strResourcePath, "*.jpg;*.png;*.bmp;*.gif", arrayImageFileName, 0);
    if (!arrayImageFileName.empty())
    {
        CString strImageFileName = arrayImageFileName[0];
        //
        __int64 m = 0;
        for (CWizStdStringArray::const_iterator it = arrayImageFileName.begin() + 1;
        it != arrayImageFileName.end();
        it++)
        {
            CString strFileName = *it;
            __int64 size = ::WizGetFileSize(strFileName);
            if (size > m)
            {
                strImageFileName = strFileName;
                m = size;
            }
        }
        //
        QImage img;
        if (img.load(strImageFileName))
        {
            if (img.width() > 32 && img.height() > 32)
            {
                abstract.image = img;
            }
        }
    }
    //
    //
    BOOL ret = UpdatePadAbstract(abstract);
    //
    ::WizDeleteFolder(strHtmlTempPath);
    //
    emit documentAbstractModified(data);
    //
    return ret;
}

CString CWizDatabase::GetRootLocation(const CString& strLocation)
{
    int index = strLocation.indexOf('/', 1);
    if (index == -1)
        return CString();
    //
    return strLocation.left(index + 1);
}

CString CWizDatabase::GetLocationName(const CString& strLocation)
{
    int index = strLocation.lastIndexOf('/', strLocation.length() - 2);
    if (index == -1)
        return CString();
    //
    CString str = strLocation.right(strLocation.length() - index - 1);
    //
    str.Trim('/');
    //
    return str;
}
CString CWizDatabase::GetLocationDisplayName(const CString& strLocation)
{
    if (IsRootLocation(strLocation))
    {
        if (strLocation.startsWith("My "))
        {
            if (strLocation == "/My Notes/")
            {
                return tr("My Notes");
            }
            else if (strLocation == "/My Journals/")
            {
                return tr("My Journals");
            }
            else if (strLocation == "/My Events/")
            {
                return tr("My Events");
            }
            else if (strLocation == "/My Sticky Notes/")
            {
                return tr("My Sticky Notes");
            }
            else if (strLocation == "/My Emails/")
            {
                return tr("My Emails");
            }
            else if (strLocation == "/My Drafts/")
            {
                return tr("My Drafts");
            }
        }
    }
    else if (strLocation == "/My Tasks/Inbox/")
    {
        return tr("Inbox");
    }
    else if (strLocation == "/My Tasks/Completed/")
    {
        return tr("Completed");
    }
    return GetLocationName(strLocation);
}

BOOL CWizDatabase::GetDocumentsByTag(const WIZTAGDATA& tag, CWizDocumentDataArray& arrayDocument)
{
    return CIndex::GetDocumentsByTag("", tag, arrayDocument);
}

BOOL CWizDatabase::DocumentToTempHtmlFile(const WIZDOCUMENTDATA& document, CString& strTempHtmlFileName)
{
    CString strZipFileName = GetDocumentFileName(document.strGUID);
    if (!PathFileExists(strZipFileName))
    {
        return FALSE;
    }
    //
    CString strTempPath = ::WizGlobal()->GetTempPath() + document.strGUID + "/";
    ::WizEnsurePathExists(strTempPath);
    //
    CWizUnzipFile::extractZip(strZipFileName, strTempPath);
    //
    strTempHtmlFileName = strTempPath + "index.html";
    //
    CString strText;
    ::WizLoadUnicodeTextFromFile(strTempHtmlFileName, strText);
    //
    QUrl url = QUrl::fromLocalFile(strTempPath + "index_files/");
    strText.replace("index_files/", url.toString());
    //
    WizSaveUnicodeTextToUtf8File(strTempHtmlFileName, strText);
    //
    return PathFileExists(strTempHtmlFileName);
}

//////////////////////////////////////////////////////////////////////
QObject* CWizDatabase::GetFolderByLocation(const QString& strLocation, bool create)
{
    return new CWizFolder(*this, strLocation);
}
QObject* CWizDatabase::GetDeletedItemsFolder()
{
    return new CWizFolder(*this, GetDeletedItemsLocation());
}


QObject* CWizDatabase::DocumentFromGUID(const QString& strGUID)
{
    WIZDOCUMENTDATA data;
    if (!DocumentFromGUID(strGUID, data))
        return NULL;
    //
    CWizDocument* pDoc = new CWizDocument(*this, data);
    return pDoc;
}


