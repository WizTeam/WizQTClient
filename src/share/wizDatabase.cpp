#include "wizDatabase.h"

#include <QDir>
#include <QUrl>
#include <QDebug>
#include <QTextCodec>
#include <algorithm>
#include <QSettings>
#include <QMessageBox>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QDateTime>

#include <extensionsystem/pluginmanager.h>

#include "wizhtml2zip.h"
#include "share/wizzip.h"
#include "html/wizhtmlcollector.h"
#include "rapidjson/document.h"

#include "utils/pathresolve.h"
#include "utils/misc.h"
#include "utils/logger.h"
#include "sync/avatar.h"
#include "wizObjectDataDownloader.h"
#include "wizProgressDialog.h"
#include "wizusercipherform.h"
#include "wizDatabaseManager.h"
#include "wizLineInputDialog.h"

#define WIZNOTE_THUMB_VERSION "3"

#define WIZKMSYNC_EXIT_OK		0
#define WIZKMSYNC_EXIT_TRAFFIC_LIMIT		304
#define WIZKMSYNC_EXIT_STORAGE_LIMIT		305
#define WIZKMSYNC_EXIT_NOTE_COUNT_LIMIT		3032
#define WIZKMSYNC_EXIT_BIZ_SERVICE_EXPR		380

#define WIZKMSYNC_EXIT_INFO     "WIZKMSYNC_EXIT_INFO"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class CWizDocument

QString CWizDatabase::m_strUserId = QString();

QString GetResoucePathFromFile(const QString& strHtmlFileName)
{
    if (!QFile::exists(strHtmlFileName))
        return NULL;

    QString strTitle = Utils::Misc::extractFileTitle(strHtmlFileName);
    QString strPath = Utils::Misc::extractFilePath(strHtmlFileName);
    QString strPath1 = strPath + strTitle + "_files/";
    QString strPath2 = strPath + strTitle + ".files/";
    if (QFile::exists(strPath1))
        return strPath1;
    if (QFile::exists(strPath2))
        return strPath2;

    return NULL;
}

CWizDocument::CWizDocument(CWizDatabase& db, const WIZDOCUMENTDATA& data)
    : m_db(db)
    , m_data(data)
{
}

void CWizDocument::makeSureObjectDataExists(CWizObjectDataDownloaderHost* downloader)
{
    ::WizMakeSureDocumentExistAndBlockWidthDialog(m_db, m_data, downloader);

    CWizDocumentAttachmentDataArray arrayAttach;
    if (m_db.GetDocumentAttachments(m_data.strGUID, arrayAttach))
    {
        for (WIZDOCUMENTATTACHMENTDATAEX attach : arrayAttach)
        {
            ::WizMakeSureAttachmentExistAndBlockWidthDialog(m_db, attach, downloader);
        }
    }
}

bool CWizDocument::UpdateDocument4(const QString& strHtml, const QString& strURL, int nFlags)
{
    return m_db.UpdateDocumentData(m_data, strHtml, strURL, nFlags);
}

void CWizDocument::deleteToTrash()
{
    // move document to trash

//    int nVersion = m_data.nVersion;
    MoveTo(m_db.GetDeletedItemsFolder());
    m_db.SetDocumentVersion(m_data.strGUID, 0);

    // delete document from server
    CWizDocumentAttachmentDataArray arrayAttachment;
    m_db.GetDocumentAttachments(m_data.strGUID, arrayAttachment);
    CWizDocumentAttachmentDataArray::const_iterator it;
    for (it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        m_db.LogDeletedGUID(it->strGUID, wizobjectDocumentAttachment);
    }

    m_db.LogDeletedGUID(m_data.strGUID, wizobjectDocument);
}

void CWizDocument::deleteFromTrash()
{
    //NOTE: 在普通文件夹中删除笔记的时候，会把数据存放在deletedguid中，此处需要判断是否已将删除数据同步到
    //服务器上，如果没有同步，则不能删除deletedguid中的数据
    bool bWaitUpload = m_db.IsObjectDeleted(m_data.strGUID);

    CWizDocumentAttachmentDataArray arrayAttachment;
    m_db.GetDocumentAttachments(m_data.strGUID, arrayAttachment);

    CWizDocumentAttachmentDataArray::const_iterator it;
    for (it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        CString strFileName = m_db.GetAttachmentFileName(it->strGUID);
        ::WizDeleteFile(strFileName);

        m_db.DeleteAttachment(*it, true, true);
        if (!bWaitUpload) {
            m_db.DeleteDeletedGUID(it->strGUID);
        }
    }

    if (!m_db.DeleteDocument(m_data, true)) {
        TOLOG1("Failed to delete document: %1", m_data.strTitle);
        return;
    }
    if (!bWaitUpload) {
        //NOTE: 笔记移动到已删除时已通知服务器删除，在已删除中删除数据时候不再记录到已删除目录
        m_db.DeleteDeletedGUID(m_data.strGUID);
    }

    CString strZipFileName = m_db.GetDocumentFileName(m_data.strGUID);
    if (PathFileExists(strZipFileName))
    {
        WizDeleteFile(strZipFileName);
    }
}

bool CWizDocument::IsInDeletedItemsFolder()
{
    QString strDeletedItemsFolderLocation = m_db.GetDeletedItemsLocation();

    return m_data.strLocation.startsWith(strDeletedItemsFolderLocation);
}

void CWizDocument::PermanentlyDelete()
{
    CWizDocumentAttachmentDataArray arrayAttachment;
    m_db.GetDocumentAttachments(m_data.strGUID, arrayAttachment);

    CWizDocumentAttachmentDataArray::const_iterator it;
    for (it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        CString strFileName = m_db.GetAttachmentFileName(it->strGUID);
        ::WizDeleteFile(strFileName);

        m_db.DeleteAttachment(*it, true, true);
    }

    if (!m_db.DeleteDocument(m_data, true)) {
        TOLOG1("Failed to delete document: %1", m_data.strTitle);
        return;
    }

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

    MoveTo(folder);
}

bool CWizDocument::MoveTo(CWizFolder* pFolder)
{
    if (!pFolder)
        return false;

    QString strNewLocation = pFolder->Location();
    QString strOldLocation = m_data.strLocation;

    if (strNewLocation == strOldLocation)
        return true;

    m_data.strLocation = strNewLocation;
    m_data.nVersion = -1;
    if (!m_db.ModifyDocumentInfoEx(m_data))
    {
        m_data.strLocation = strOldLocation;
        TOLOG1(_T("Failed to modify document location %1."), m_data.strLocation);
        return false;
    }

    return true;
}

bool CWizDocument::MoveTo(CWizDatabase& targetDB, CWizFolder* pFolder, CWizObjectDataDownloaderHost* downloader)
{
    qDebug() << "wizdocmove  to : " << pFolder->Location();
    if (targetDB.kbGUID() == m_db.kbGUID())
        return MoveTo(pFolder);

    QString newDocGUID;
    if (!CopyTo(targetDB, pFolder, true, true, newDocGUID, downloader))
    {
        TOLOG1(_T("Failed to copy document %1. Stop move"), m_data.strTitle);
        return false;
    }

    Delete();
    return true;
}

bool CWizDocument::MoveTo(CWizDatabase& targetDB, const WIZTAGDATA& targetTag, CWizObjectDataDownloaderHost* downloader)
{
    qDebug() << "wizdoc move to : " << targetTag.strName;
    if (targetDB.kbGUID() == m_db.kbGUID())
    {
        if (m_data.strLocation == LOCATION_DELETED_ITEMS)
        {
            CWizFolder folder(m_db, m_db.GetDefaultNoteLocation());
            MoveTo(&folder);
        }

        CWizTagDataArray arrayTag;
        m_db.GetDocumentTags(m_data.strGUID, arrayTag);
        if (arrayTag.size() > 0)
        {
            for (CWizTagDataArray::const_iterator it = arrayTag.begin(); it != arrayTag.end(); it++)
            {
                RemoveTag(*it);
            }
        }
        return AddTag(targetTag);
    }

    //
    if (!CopyTo(targetDB, targetTag, true, downloader))
    {
        TOLOG1(_T("Failed to copy document %1. Stop move"), m_data.strTitle);
        return false;
    }

    qDebug() << " after copy doc delete this doc";
    Delete();
    return true;
}

bool CWizDocument::CopyTo(CWizDatabase& targetDB, CWizFolder* pFolder, bool keepDocTime,
                          bool keepDocTag, QString& newDocGUID, CWizObjectDataDownloaderHost* downloader)
{
    qDebug() << "wizdocu copy to : " << pFolder->Location();
    QString strLocation = pFolder->Location();
    WIZTAGDATA tagEmpty;
    if (!copyDocumentTo(m_data.strGUID, targetDB, strLocation, tagEmpty, newDocGUID, downloader, keepDocTime))
    {
        TOLOG1(_T("Failed to copy document %1."), m_data.strTitle);
        return false;
    }

    if (keepDocTag && !m_db.IsGroup() && m_db.kbGUID() == targetDB.kbGUID())
    {
        WIZDOCUMENTDATA newDoc;
        if (!targetDB.DocumentFromGUID(newDocGUID, newDoc))
            return false;

        CWizStdStringArray arrayTag;
        if (m_db.GetDocumentTags(m_data.strGUID, arrayTag))
        {
            for (CString tagGUID : arrayTag)
            {
                targetDB.InsertDocumentTag(newDoc, tagGUID);
            }
        }
    }
    return true;
}

bool CWizDocument::CopyTo(CWizDatabase& targetDB, const WIZTAGDATA& targetTag,
                          bool keepDocTime, CWizObjectDataDownloaderHost* downloader)
{
    qDebug() << "wizdocu copy to : " << targetTag.strName;
    QString strLocation = targetDB.GetDefaultNoteLocation();
    QString strNewDocGUID;
    return copyDocumentTo(m_data.strGUID, targetDB, strLocation, targetTag, strNewDocGUID, downloader, keepDocTime);
}

bool CWizDocument::AddTag(const WIZTAGDATA& dataTag)
{
    CWizStdStringArray arrayTag;
    m_db.GetDocumentTags(m_data.strGUID, arrayTag);

    if (-1 != WizFindInArray(arrayTag, dataTag.strGUID))
        return true;

    if (!m_db.InsertDocumentTag(m_data, dataTag.strGUID)) {
        TOLOG1(_T("Failed to insert document tag: %1"), m_data.strTitle);
        return false;
    }

    return true;
}

bool CWizDocument::RemoveTag(const WIZTAGDATA& dataTag)
{
    CWizStdStringArray arrayTag;
    m_db.GetDocumentTags(m_data.strGUID, arrayTag);

    if (-1 == WizFindInArray(arrayTag, dataTag.strGUID))
        return true;

    if (!m_db.DeleteDocumentTag(m_data, dataTag.strGUID)) {
        TOLOG1(_T("Failed to delete document tag: %1"), m_data.strTitle);
        return false;
    }

    return true;
}

void CWizDocument::Delete()
{
    if (IsInDeletedItemsFolder()) {
//        return PermanentlyDelete();
        return deleteFromTrash();
    } else {
//        return MoveTo(m_db.GetDeletedItemsFolder());
        return deleteToTrash();
    }

}



static const char* g_lpszZiwMeta = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<META>\n\
    <CLASS>ziw file</CLASS>\n\
    <VERSION>1.0</VERSION>\n\
    <DESCRIPTION>Wiz file formate</DESCRIPTION>\n\
    <TITLE><![CDATA[%title%]]></TITLE>\n\
    <URL><![CDATA[%url%]]></URL>\n\
    <TAGS><![CDATA[%tags%]]></TAGS>\n\
</META>\n\
";

inline CString WizGetZiwMetaText(const CString& strTitle, const CString& strURL, const CString& strTagsText)
{
    CString strText(g_lpszZiwMeta);
    //
    strText.Replace("%title%", strTitle);
    strText.Replace("%url%", strURL);
    strText.Replace("%tags%", strTagsText);
    return strText;
}

QString CWizDocument::GetMetaText()
{
    return WizGetZiwMetaText(m_data.strTitle, m_data.strURL, m_db.GetDocumentTagsText(m_data.strGUID));
}

bool CWizDocument::copyDocumentTo(const QString& sourceGUID, CWizDatabase& targetDB,
                                  const QString& strTargetLocation, const WIZTAGDATA& targetTag, QString& resultGUID,
                                  CWizObjectDataDownloaderHost* downloaderHost, bool keepDocTime)
{
    TOLOG("Copy document");
    WIZDOCUMENTDATA sourceDoc;
    if (!m_db.DocumentFromGUID(sourceGUID, sourceDoc))
        return false;

    if (!WizMakeSureDocumentExistAndBlockWidthEventloop(m_db, sourceDoc, downloaderHost))
        return false;

    if (!m_db.tryAccessDocument(sourceDoc))
        return false;

    QByteArray ba;
    if (!m_db.LoadDocumentData(sourceDoc.strGUID, ba, false))
        return false;

    WIZDOCUMENTDATA newDoc;
    if (!targetDB.CreateDocumentAndInit(sourceDoc, ba, strTargetLocation, targetTag, newDoc))
    {
        TOLOG("Failed to new document!");
        return false;
    }

    resultGUID = newDoc.strGUID;

    if (!copyDocumentAttachment(sourceDoc, targetDB, newDoc, downloaderHost))
        return false;

    if (keepDocTime)
    {
        newDoc.tCreated = sourceDoc.tCreated;
        newDoc.tAccessed = sourceDoc.tAccessed;
        newDoc.tDataModified = sourceDoc.tDataModified;
        newDoc.tModified = sourceDoc.tModified;
        newDoc.tInfoModified = sourceDoc.tInfoModified;
        newDoc.tParamModified = sourceDoc.tParamModified;
    }
    newDoc.nAttachmentCount = targetDB.GetDocumentAttachmentCount(newDoc.strGUID);
    targetDB.ModifyDocumentInfoEx(newDoc);

    return true;
}

bool CWizDocument::copyDocumentAttachment(const WIZDOCUMENTDATA& sourceDoc,
                                          CWizDatabase& targetDB, WIZDOCUMENTDATA& targetDoc,
                                          CWizObjectDataDownloaderHost* downloaderHost)
{
    CWizDocumentAttachmentDataArray arrayAttachment;
    m_db.GetDocumentAttachments(sourceDoc.strGUID, arrayAttachment);

    for (CWizDocumentAttachmentDataArray::const_iterator it = arrayAttachment.begin();
         it != arrayAttachment.end(); it++)
    {
        WIZDOCUMENTATTACHMENTDATAEX attachData(*it);
        if (!WizMakeSureAttachmentExistAndBlockWidthEventloop(m_db, attachData, downloaderHost))
            continue;

        WIZDOCUMENTATTACHMENTDATAEX newAttach;
        QString strNewAttachFileName;

        CString targetAttachPath = targetDB.GetAttachmentsDataPath();
        CString strTempFileName = targetAttachPath + attachData.strName;
        ::WizGetNextFileName(strTempFileName);
        if (!::WizCopyFile(m_db.GetAttachmentFileName(attachData.strGUID), strTempFileName, FALSE))
            return false;

        newAttach = attachData;
        newAttach.strGUID = QString();
        newAttach.strURL = strTempFileName;
        strNewAttachFileName = strTempFileName;

//        if (!m_db.CopyDocumentAttachment(attachData, targetDB, newAttach, strNewAttachFileName))
//            continue;

        newAttach.strKbGUID = targetDoc.strKbGUID;
        newAttach.nVersion = -1;
        targetDB.AddAttachment(targetDoc, strNewAttachFileName, newAttach);
    }

    return true;
}


/*
* Class CWizFolder
*/

CWizFolder::CWizFolder(CWizDatabase& db, const QString& strLocation)
    : m_db(db)
    , m_strLocation(strLocation)
{
    Q_ASSERT(strLocation.right(1) == "/");
    Q_ASSERT(strLocation.left(1) == "/");
}

bool CWizFolder::IsDeletedItems() const
{
    return Location() == LOCATION_DELETED_ITEMS;
}

bool CWizFolder::IsInDeletedItems() const
{
    return !IsDeletedItems() && Location().startsWith(LOCATION_DELETED_ITEMS);
}

//QObject* CWizFolder::CreateDocument2(const QString& strTitle, const QString& strURL)
//{
//    WIZDOCUMENTDATA data;
//    if (!m_db.CreateDocument(strTitle, "", m_strLocation, strURL, data))
//        return NULL;
//
//    CWizDocument* pDoc = new CWizDocument(m_db, data);
//
//    return pDoc;
//}

void CWizFolder::Delete()
{
    if (IsDeletedItems())
        return;

//    if (IsInDeletedItems()) {
//        // FIXME: should use CWizDocument to delete document data, attachments.
//        if (!m_db.DeleteDocumentsByLocation(Location())) {
//            TOLOG1("Failed to delete documents by location; %1", Location());
//            return;
//        }

//        m_db.DeleteExtraFolder(Location());
//        m_db.SetLocalValueVersion("folders", -1);
//    } else {
//        CWizFolder deletedItems(m_db, LOCATION_DELETED_ITEMS + Location().right(Location().size() - 1));
//        MoveTo(&deletedItems);
//    }
    CWizDocumentDataArray arrayDocument;
    m_db.GetDocumentsByLocation(Location(), arrayDocument, true);
    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++)
    {
        WIZDOCUMENTDATAEX doc = *it;
        CWizDocument document(m_db, doc);
        document.deleteToTrash();
    }

    m_db.DeleteExtraFolder(Location());
    m_db.SetLocalValueVersion("folders", -1);

}

void CWizFolder::MoveTo(QObject* dest)
{
    CWizFolder* pFolder = dynamic_cast<CWizFolder*>(dest);
    if (!pFolder)
        return;

    if (IsDeletedItems())
        return;

    if (!CanMove(this, pFolder)) {
        TOLOG2("Can move %1 to %2", Location(), pFolder->Location());
        return;
    }

    return MoveToLocation(pFolder->Location());
}

bool CWizFolder::CanMove(const QString& strSource, const QString& strDest)
{
    if (LOCATION_DELETED_ITEMS == strSource)
        return false;

    // sub folder relationship or the same folder
    if (strDest.startsWith(strSource))
        return false;

    return true;
}

bool CWizFolder::CanMove(CWizFolder* pSrc, CWizFolder* pDest) const
{
    return CanMove(pSrc->Location(), pDest->Location());
}

void CWizFolder::MoveToLocation(const QString& strDestLocation)
{
    Q_ASSERT(strDestLocation.right(1) == "/");
    Q_ASSERT(strDestLocation.left(1) == "/");

    if (!CanMove(Location(), strDestLocation))
        return;

    QString strOldLocation = Location();

    CWizDocumentDataArray arrayDocument;
    if (!m_db.GetDocumentsByLocation(strOldLocation, arrayDocument, true)) {
        TOLOG1("Failed to get documents by location (include sub folders): %1", strOldLocation);
        return;
    }

    int i = 0;
    CWizDocumentDataArray::const_iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        WIZDOCUMENTDATA data = *it;

        if (!data.strLocation.startsWith(strOldLocation)) {
            TOLOG("Error location of document!");
            continue;
        }

        data.strLocation.remove(0, strOldLocation.length());
        data.strLocation.insert(0, strDestLocation);
        data.nVersion = -1;

        if (!m_db.ModifyDocumentLocation(data)) {
            TOLOG("Failed to move note to new folder!");
            continue;
        }

        Q_EMIT moveDocument(arrayDocument.size(), i++, strOldLocation, strDestLocation, data);
    }

    CWizStdStringArray arrayFolder;
    m_db.GetExtraFolder(arrayFolder);
    for (CWizStdStringArray::const_iterator it = arrayFolder.begin();
         it != arrayFolder.end();
         it++) {
        QString strFolder = *it;
        if (strFolder.right(1) != "/") {
            strFolder.append("/");
        }
        if (strFolder.startsWith(strOldLocation)) {
            strFolder.remove(0, strOldLocation.length());
            strFolder.insert(0, strDestLocation);
            qDebug() << "Add new folder ; " << strFolder;
            m_db.AddExtraFolder(strFolder);
        }
    }

    qDebug() << "Delete old location ; " << strOldLocation;
    m_db.DeleteExtraFolder(strOldLocation);
    m_db.SetLocalValueVersion("folders", -1);
}


/* ------------------------------CWizDatabase------------------------------ */

const QString g_strAccountSection = "Account";
const QString g_strCertSection = "Cert";
const QString g_strGroupSection = "Groups";
const QString g_strDatabaseInfoSection = "Database";

#define WIZ_META_KBINFO_SECTION "KB_INFO"
#define WIZ_META_SYNCINFO_SECTION "SYNC_INFO"
#define VERSION_NAME_2_META_KEY(x) "KEY_" + x + "_VERSION"

CWizDatabase::CWizDatabase()
    : m_ziwReader(new CWizZiwReader())
    , m_bIsPersonal(true)
{
}

QString CWizDatabase::GetUserId()
{
    return m_strUserId;
}

QString CWizDatabase::GetUserGUID()
{
    return GetMetaDef(g_strAccountSection, "GUID");
}

QString CWizDatabase::GetPassword()
{
    CString strPassword;
    GetPassword(strPassword);

    if (!strPassword.isEmpty()) {
        strPassword = ::WizDecryptPassword(strPassword);
    }

    return strPassword;
}

qint64 CWizDatabase::GetObjectVersion(const QString& strObjectName)
{
    return GetMetaInt64("SYNC_INFO", strObjectName, -1);
}

bool CWizDatabase::SetObjectVersion(const QString& strObjectName,
                                    qint64 nVersion)
{
    return SetMetaInt64("SYNC_INFO", strObjectName, nVersion);
}

bool CWizDatabase::GetModifiedDeletedList(CWizDeletedGUIDDataArray& arrayData)
{
    return GetDeletedGUIDs(arrayData);
}

bool CWizDatabase::GetModifiedTagList(CWizTagDataArray& arrayData)
{
    return GetModifiedTags(arrayData);
}

bool CWizDatabase::GetModifiedStyleList(CWizStyleDataArray& arrayData)
{
    return GetModifiedStyles(arrayData);
}

bool CWizDatabase::GetModifiedDocumentList(CWizDocumentDataArray& arrayData)
{
    CWizDocumentDataArray docList;
    if (!GetModifiedDocuments(docList))
        return false;

    // remove invalid document
    CWizDocumentDataArray::iterator it;
    for (it = docList.begin(); it != docList.end(); it++)
    {
        WIZDOCUMENTDATA doc = *it;
        if (CanEditDocument(doc) && !IsInDeletedItems(doc.strLocation))  // do not upload doc in trash
        {
            arrayData.push_back(doc);
        }
    }
    return true;
}

bool CWizDatabase::GetModifiedAttachmentList(CWizDocumentAttachmentDataArray& arrayData)
{
    return GetModifiedAttachments(arrayData);
}

bool CWizDatabase::GetModifiedMessageList(CWizMessageDataArray& arrayData)
{
    return getModifiedMessages(arrayData);
}

bool CWizDatabase::GetObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayObject)
{
    return GetAllObjectsNeedToBeDownloaded(arrayObject, GetObjectSyncTimeline());
}

bool CWizDatabase::OnDownloadDeletedList(const CWizDeletedGUIDDataArray& arrayData)
{
    CWizDeletedGUIDDataArray arrayDeleted;
    CWizDeletedGUIDDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        WIZDELETEDGUIDDATA data(*it);
        data.strKbGUID = kbGUID();
        arrayDeleted.push_back(data);
    }

    return UpdateDeletedGUIDs(arrayDeleted);
}

bool CWizDatabase::OnDownloadTagList(const CWizTagDataArray& arrayData)
{
    CWizTagDataArray arrayTag;
    CWizTagDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        WIZTAGDATA data(*it);
        data.strKbGUID = kbGUID();
        //NOTE:当前同步数据时不会从服务器中下载tag的position数据。
        //将position数据保留为原本的数据。如果后期规则修改，此处需要修改
        WIZTAGDATA dataTemp;
        if (TagFromGUID(data.strGUID, dataTemp))
        {
            data.nPostion = dataTemp.nPostion;
        }
        arrayTag.push_back(data);
    }

    return UpdateTags(arrayTag);
}

bool CWizDatabase::OnDownloadStyleList(const CWizStyleDataArray& arrayData)
{
    CWizStyleDataArray arrayStyle;
    CWizStyleDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        WIZSTYLEDATA data(*it);
        data.strKbGUID = kbGUID();
        arrayStyle.push_back(data);
    }

    return UpdateStyles(arrayStyle);
}

bool CWizDatabase::OnDownloadAttachmentList(const CWizDocumentAttachmentDataArray& arrayData)
{
    CWizDocumentAttachmentDataArray arrayAttach;
    CWizDocumentAttachmentDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        WIZDOCUMENTATTACHMENTDATAEX data(*it);
        data.strKbGUID = kbGUID();
        arrayAttach.push_back(data);
    }

    return UpdateAttachments(arrayAttach);
}

bool CWizDatabase::OnDownloadMessages(const CWizUserMessageDataArray& arrayData)
{
    CWizMessageDataArray arrayMsg;

    CWizUserMessageDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        WIZMESSAGEDATA data(*it);
        arrayMsg.push_back(data);
    }

    return UpdateMessages(arrayMsg);
}

bool CWizDatabase::OnDownloadDocument(int part, const WIZDOCUMENTDATAEX& data)
{
    WIZDOCUMENTDATAEX d(data);
    d.nObjectPart = part;
    d.strKbGUID = kbGUID();
    return UpdateDocument(d);
}

qint64 CWizDatabase::GetObjectLocalVersion(const QString& strObjectGUID,
                                           const QString& strObjectType)
{
    return CWizIndex::GetObjectLocalVersion(strObjectGUID, strObjectType);
}

qint64 CWizDatabase::GetObjectLocalVersionEx(const QString& strObjectGUID, const QString& strObjectType, bool& bObjectVersion)
{
     return CWizIndex::GetObjectLocalVersionEx(strObjectGUID, strObjectType, bObjectVersion);
}

qint64 CWizDatabase::GetObjectLocalServerVersion(const QString& strObjectGUID,
                                                 const QString& strObjectType)
{
    return CWizIndex::GetObjectLocalVersion(strObjectGUID, strObjectType);
}

bool CWizDatabase::SetObjectLocalServerVersion(const QString& strObjectGUID,
                                               const QString& strObjectType,
                                               qint64 nVersion)
{
    return ModifyObjectVersion(strObjectGUID, strObjectType, nVersion);
}

void CWizDatabase::OnObjectUploaded(const QString& strObjectGUID, const QString& strObjectType)
{
    if (strObjectType == "document")
    {
        emit documentUploaded(kbGUID(), strObjectGUID);
    }
}

bool CWizDatabase::DocumentFromGUID(const QString& strGUID,
                                    WIZDOCUMENTDATA& dataExists)
{
    return CWizIndex::DocumentFromGUID(strGUID, dataExists);
}

bool CWizDatabase::DocumentWithExFieldsFromGUID(const CString& strGUID,
                                                WIZDOCUMENTDATA& dataExists)
{
    return CWizIndex::DocumentWithExFieldsFromGUID(strGUID, dataExists);
}

bool CWizDatabase::IsObjectDataDownloaded(const QString& strGUID,
                                          const QString& strType)
{
    return CWizIndex::IsObjectDataDownloaded(strGUID, strType);
}

bool CWizDatabase::SetObjectDataDownloaded(const QString& strGUID,
                                           const QString& strType,
                                           bool bDownloaded)
{
    return CWizIndex::SetObjectDataDownloaded(strGUID, strType, bDownloaded);
}

bool CWizDatabase::SetObjectServerDataInfo(const QString& strGUID,
                                           const QString& strType,
                                           COleDateTime& tServerDataModified,
                                           const QString& strServerMD5)
{
    if (strType == WIZDOCUMENTDATA::ObjectName()) {
        WIZDOCUMENTDATA data;
        if (!DocumentFromGUID(strGUID, data))
            return false;

        data.tDataModified = tServerDataModified;
        data.strDataMD5 = strServerMD5;

        return ModifyDocumentInfoEx(data);
    } else {
        Q_ASSERT(0);
    }

    return false;
}

bool CWizDatabase::UpdateObjectData(const QString& strObjectGUID,
                                    const QString& strObjectType,
                                    const QByteArray& stream)
{   
    WIZOBJECTDATA data;
    data.strObjectGUID = strObjectGUID;
    data.arrayData = stream;

    if (strObjectType == WIZDOCUMENTDATAEX::ObjectName())
    {
        data.eObjectType = wizobjectDocument;
    }
    else if (strObjectType == WIZDOCUMENTATTACHMENTDATAEX::ObjectName())
    {
        data.eObjectType = wizobjectDocumentAttachment;
    }
    else
    {
        Q_ASSERT(0);
    }

    return UpdateSyncObjectLocalData(data);
}

bool CWizDatabase::InitDocumentData(const QString& strGUID,
                                    WIZDOCUMENTDATAEX& data,
                                    UINT part)
{
    bool bInfo = (part & WIZKM_XMKRPC_DOCUMENT_PART_INFO) ? true : false;
    bool bParam = (part & WIZKM_XMKRPC_DOCUMENT_PART_PARAM) ? true : false;
    bool bData = (part & WIZKM_XMKRPC_DOCUMENT_PART_DATA) ? true : false;

    data.nObjectPart = part;

    if (bInfo) {
        if (!DocumentFromGUID(strGUID, data)) {
            return false;
        }

        GetDocumentTags(strGUID, data.arrayTagGUID);
    }

    if (bParam) {
        if (!GetDocumentParams(strGUID, data.arrayParam)) {
            return false;
        }
    }

    if (bData) {
        if (!LoadDocumentData(strGUID, data.arrayData)) {
            return false;
        }
    }

    return true;
}

bool CWizDatabase::InitAttachmentData(const QString& strGUID,
                                      WIZDOCUMENTATTACHMENTDATAEX& data,
                                      UINT part)
{
    bool bInfo = (part & WIZKM_XMKRPC_ATTACHMENT_PART_INFO) ? true : false;
    bool bData = (part & WIZKM_XMKRPC_ATTACHMENT_PART_DATA) ? true : false;

    data.nObjectPart = part;

    if (bInfo) {
        if (!AttachmentFromGUID(strGUID, data)) {
            return false;
        }
    }

    if (bData) {
        if (!LoadCompressedAttachmentData(strGUID, data.arrayData)) {
            return false;
        }
    }

    return true;
}

bool CWizDatabase::OnUploadObject(const QString& strGUID,
                                  const QString& strObjectType)
{
    if (strObjectType == WIZDELETEDGUIDDATA::ObjectName()) {
        return DeleteDeletedGUID(strGUID);
    } else {
        return ModifyObjectVersion(strGUID, strObjectType, 0);
    }
}

bool CWizDatabase::ModifyDocumentsVersion(CWizDocumentDataArray& arrayData)
{
    CWizDocumentDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        WIZDOCUMENTDATA data(*it);
        SetDocumentVersion(data.strGUID, data.nVersion);
    }

    return true;
}

bool CWizDatabase::ModifyMessagesLocalChanged(CWizMessageDataArray& arrayData)
{
    for (WIZMESSAGEDATA msg : arrayData)
    {
        modifyMessageLocalChanged(msg);
    }

    return true;
}

bool CWizDatabase::CopyDocumentData(const WIZDOCUMENTDATA& sourceDoc, CWizDatabase& targetDB, WIZDOCUMENTDATA& targetDoc)
{
    QByteArray ba;
    LoadDocumentData(sourceDoc.strGUID, ba, false);
    targetDB.WriteDataToDocument(targetDoc.strGUID, ba);
    return true;
}


bool CWizDatabase::GetBizMetaName(const QString &strBizGUID, QString &strMetaName)
{
    if (strBizGUID.isEmpty())
        return false;

    int count = GetMetaDef("Bizs", "Count").toInt();
    //
    for (int i = 0; i < count; i++)
    {
        QString bizSection = "Biz_" + QString::number(i);

        if (strBizGUID == GetMetaDef(bizSection, "GUID"))
        {
            strMetaName = bizSection;
            return true;
        }
    }

    return false;
}

bool CWizDatabase::initZiwReaderForEncryption(const QString& strUserCipher)
{
    if (!m_ziwReader->isRSAKeysAvailable())
    {
        CWizDatabase* persionDB = getPersonalDatabase();
        if (!persionDB->checkUserCertExists() || !persionDB->loadUserCert())
        {
            QMessageBox::information(0, tr("Info"), tr("No password cert founded. Please create password" \
                                     " cert from windows client first."));
            return false;
        }
    }

    if (!m_ziwReader->isZiwCipherAvailable())
    {
        if (m_ziwReader->userCipher().isEmpty())
        {
            QString userCipher = strUserCipher;
            if (userCipher.isEmpty())
            {
                CWizLineInputDialog dlg(tr("Password"), tr("Please input document password to encrypt")
                                        , "", 0, QLineEdit::Password);
                if (dlg.exec() == QDialog::Rejected)
                    return false;

                userCipher = dlg.input();

                if (userCipher.isEmpty())
                    return false;
            }
            m_ziwReader->setUserCipher(userCipher);
        }

        m_ziwReader->createZiwHeader();
        bool initResult = m_ziwReader->initZiwCipher();
        m_ziwReader->setUserCipher(QString());

        //
        if (!initResult)
        {
            QMessageBox::warning(0, tr("Info"), tr("User password check failed!"));
            return false;
        }
    }

    return true;
}

bool CWizDatabase::OnDownloadGroups(const CWizGroupDataArray& arrayGroup)
{
    bool ret = SetUserGroupInfo(arrayGroup);
    Q_EMIT groupsInfoDownloaded(arrayGroup);
    return ret;
}

bool CWizDatabase::OnDownloadBizs(const CWizBizDataArray& arrayBiz)
{
    bool ret = SetUserBizInfo(arrayBiz);
    Q_EMIT bizInfoDownloaded(arrayBiz);
    return ret;
}

IWizSyncableDatabase* CWizDatabase::GetGroupDatabase(const WIZGROUPDATA& group)
{
    Q_ASSERT(!group.strGroupGUID.isEmpty());

//    CWizDatabaseManager::instance()->db(group.strGroupGUID);

//    CWizDatabase* db = new CWizDatabase();
//    if (!db->Open(m_strUserId, group.strGroupGUID)) {
//        delete db;
//        return NULL;
//    }

    // pass this pointer to database manager for signal redirect and managament
    // CWizDatabaseManager will take ownership
//    Q_EMIT databaseOpened(db, group.strGroupGUID);

    CWizDatabase* db = &CWizDatabaseManager::instance()->db(group.strGroupGUID);
    return db;
}

void CWizDatabase::CloseGroupDatabase(IWizSyncableDatabase* pDatabase)
{
//    CWizDatabase* db = dynamic_cast<CWizDatabase*>(pDatabase);

//    Q_ASSERT(db);

//    db->Close();
//    db->deleteLater();

}

IWizSyncableDatabase* CWizDatabase::GetPersonalDatabase()
{
    return getPersonalDatabase();
}

CWizDatabase *CWizDatabase::getPersonalDatabase()
{
    CWizDatabase* db = &CWizDatabaseManager::instance()->db();
    return db;
}

void CWizDatabase::SetKbInfo(const QString& strKBGUID, const WIZKBINFO& info)
{
    Q_ASSERT(strKBGUID == kbGUID() || (strKBGUID.isEmpty() && m_bIsPersonal));

    SetMeta(WIZ_META_KBINFO_SECTION, "STORAGE_LIMIT_S", info.strStorageLimit);
    SetMetaInt64(WIZ_META_KBINFO_SECTION, "STORAGE_LIMIT_N", info.nStorageLimit);

    SetMeta(WIZ_META_KBINFO_SECTION, "STORAGE_USAGE_S", info.strStorageUsage);
    SetMetaInt64(WIZ_META_KBINFO_SECTION, "STORAGE_USAGE_N", info.nStorageUsage);

    SetMeta(WIZ_META_KBINFO_SECTION, "TRAFFIC_LIMIT_S", info.strTrafficLimit);
    SetMetaInt64(WIZ_META_KBINFO_SECTION, "TRAFFIC_LIMIT_N", info.nTrafficLimit);

    SetMeta(WIZ_META_KBINFO_SECTION, "TRAFFIC_USAGE_S", info.strTrafficUsage);
    SetMetaInt64(WIZ_META_KBINFO_SECTION, "TRAFFIC_USAGE_N", info.nTrafficUsage);
}

void CWizDatabase::SetUserInfo(const WIZUSERINFO& userInfo)
{
    SetMeta(g_strDatabaseInfoSection, "KBGUID", userInfo.strKbGUID);

    SetMeta(g_strAccountSection, "GUID", userInfo.strUserGUID);
    SetMeta(g_strAccountSection, "DisplayName", userInfo.strDisplayName);
    SetMeta(g_strAccountSection, "UserType", userInfo.strUserType);
    SetMeta(g_strAccountSection, "UserLevelName", userInfo.strUserLevelName);
    SetMeta(g_strAccountSection, "UserLevel", QString::number(userInfo.nUserLevel));
    SetMeta(g_strAccountSection, "UserPoints", QString::number(userInfo.nUserPoints));
    SetMeta(g_strAccountSection, "MywizMail", userInfo.strMywizEmail);
    SetMeta(g_strAccountSection, "DateSignUp", userInfo.tCreated.toString());

    Q_EMIT userInfoChanged();
}

bool CWizDatabase::IsGroup()
{
    if (m_bIsPersonal)
        return false;

    return true;
}

bool CWizDatabase::HasBiz()
{
    CWizDatabase* personDb = getPersonalDatabase();

    return !personDb->GetMetaDef("Bizs", "Count").IsEmpty();
}

bool CWizDatabase::IsGroupAdmin()
{
    if (permission() <= WIZ_USERGROUP_ADMIN)
        return true;

    return false;
}

bool CWizDatabase::IsGroupOwner()
{
    return m_info.bOwner;
}

bool CWizDatabase::IsGroupSuper()
{
    if (permission() <= WIZ_USERGROUP_SUPER)
        return true;

    return false;
}

bool CWizDatabase::IsGroupEditor()
{
    if (permission() <= WIZ_USERGROUP_EDITOR)
        return true;

    return false;
}

bool CWizDatabase::IsGroupAuthor()
{
    if (permission() <= WIZ_USERGROUP_AUTHOR)
        return true;

    return false;
}

bool CWizDatabase::IsGroupReader()
{
    if (permission() <= WIZ_USERGROUP_READER)
        return true;

    return false;
}

bool CWizDatabase::CanEditDocument(const WIZDOCUMENTDATA& data)
{
    if (permission() < WIZ_USERGROUP_AUTHOR ||
                (permission() == WIZ_USERGROUP_AUTHOR && data.strOwner == GetUserId())) {
            return true;
    }

    return false;
}

bool CWizDatabase::CanEditAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    WIZDOCUMENTDATA doc;
    if (!DocumentFromGUID(data.strDocumentGUID, doc)) {
        return false;
    }

    return CanEditDocument(doc);
}

bool CWizDatabase::CreateConflictedCopy(const QString& strObjectGUID,
                                        const QString& strObjectType)
{
    // FIXME
    Q_UNUSED(strObjectGUID);
    Q_UNUSED(strObjectType);
    Q_ASSERT(0);

    return false;
}

bool CWizDatabase::SaveLastSyncTime()
{
    uint secs = QDateTime::currentDateTime().toTime_t();
    return SetMeta("SYNC_INFO", "TIME", QString::number(secs));
}

COleDateTime CWizDatabase::GetLastSyncTime()
{
    uint secs = GetMetaDef("SYNC_INFO", "TIME").toUInt();
    return COleDateTime(secs);
}

bool CWizDatabase::WizDayOnce(const QString& strName)
{
    CWizDatabase* db = getPersonalDatabase();
    QString strDate = db->meta("WizDayOnce", strName);
    QDateTime curDt = QDateTime::currentDateTime();
    db->setMeta("WizDayOnce", strName, curDt.toString(Qt::ISODate));

    if (strDate.isEmpty())
        return true;

    QDateTime dt = QDateTime::fromString(strDate, Qt::ISODate);
    return dt.daysTo(curDt) >= 1;
}

long CWizDatabase::GetLocalFlags(const QString& strObjectGUID,
                                 const QString& strObjectType)
{
    Q_UNUSED(strObjectGUID);
    Q_UNUSED(strObjectType);

    Q_ASSERT(0);
    return 0;
}

bool CWizDatabase::SetLocalFlags(const QString& strObjectGUID,
                                 const QString& strObjectType,
                                 long flags)
{
    Q_UNUSED(strObjectGUID);
    Q_UNUSED(strObjectType);
    Q_UNUSED(flags);

    Q_ASSERT(0);
    return false;
}

void CWizDatabase::GetAccountKeys(CWizStdStringArray& arrayKey)
{
    Q_ASSERT(!IsGroup());

    //QMap<QString, QString> mapBiz;
    //GetBizGroupInfo(mapBiz);
    //for (QMap<QString, QString>::const_iterator it = mapBiz.begin();
    //     it != mapBiz.end(); it++) {
    //    arrayKey.push_back("biz_users/" + it.key());
    //}
}

qint64 CWizDatabase::GetAccountLocalValueVersion(const QString& strKey)
{
    return GetMetaInt64(WIZ_META_SYNCINFO_SECTION, "KEY_" + strKey + "_VERSION", 0);
}

void CWizDatabase::SetAccountLocalValue(const QString& strKey,
                                        const QString& strValue,
                                        qint64 nServerVersion,
                                        bool bSaveVersion)
{
    Q_ASSERT(!IsGroup());

    if (strKey.startsWith("biz_users/", Qt::CaseInsensitive)) {
        /*
        QMap<QString, QString> mapBiz;
        GetBizGroupInfo(mapBiz);
        for (QMap<QString, QString>::const_iterator it = mapBiz.begin();
             it != mapBiz.end(); it++) {
            if (strKey.endsWith(it.key(), Qt::CaseInsensitive)) {
                SetBizUsers(it.key(), strValue);
            }
        }
        */
    } else {
        Q_ASSERT(0);
    }

    if (bSaveVersion) {
        SetMetaInt64(WIZ_META_SYNCINFO_SECTION, "KEY_" + strKey + "_VERSION", nServerVersion);
    }
}

void CWizDatabase::GetKBKeys(CWizStdStringArray& arrayKey)
{
    if (IsGroup())
    {
        arrayKey.push_back("group_tag_oem");
        arrayKey.push_back("group_tag_config_oem");
        arrayKey.push_back("group_tag_pos");
    }
    else
    {
        arrayKey.push_back("folders");
        arrayKey.push_back("folders_pos");
        arrayKey.push_back("favorites");
    }
}

bool CWizDatabase::ProcessValue(const QString& key)
{
    CString strKey(key);
    strKey.MakeLower();

    if (strKey == "folders"
        || strKey == "folders_pos")
    {
        return !IsGroup();
    }
    else if (strKey == "group_tag_oem"
        || strKey == "group_tag_config_oem")
    {
        return IsGroup();
    }

    return true;
}

qint64 CWizDatabase::GetLocalValueVersion(const QString& key)
{
    CString strKey(key);
    strKey.MakeLower();

    return GetMetaInt64(WIZ_META_SYNCINFO_SECTION, "KEY_" + strKey + "_VERSION", 0);

    //if (strKey == "folders")
    //{
    //    return GetMetaInt64(key, "version", 0);
    //}
    //else
    //{
    //    return GetMetaInt64(key, "version", 0);
    //}
}

QString CWizDatabase::GetLocalValue(const QString& key)
{
    CString strKey(key);
    strKey.MakeLower();

    if (strKey == "folders")
    {
        return GetFolders();
    }
    else if (strKey == "folders_pos")
    {
        return GetFoldersPos();
    }
    else if (strKey == "group_tag_pos")
    {
        return GetGroupTagsPos();
    }
    else if (strKey == "favorites")
    {
        return GetFavorites();
    }
    else if (strKey == "group_tag_oem")
    {
        Q_ASSERT(false);
        return "";
        //return GetGroupTagPropertiesOEM();
    }
    else if (strKey == "group_tag_config_oem")
    {
        Q_ASSERT(false);
        return "";
        //return GetGroupTagConfigOEM();
    }
    else
    {
        return CString();
    }
}

void CWizDatabase::SetLocalValueVersion(const QString& strKey,
                                        qint64 nServerVersion)
{
    SetMetaInt64(WIZ_META_SYNCINFO_SECTION, "KEY_" + strKey + "_VERSION", nServerVersion);
}

void CWizDatabase::SetLocalValue(const QString& key, const QString& value,
                                 qint64 nServerVersion, bool bSaveVersion)
{
    CString strKey(key);
    strKey.MakeLower();

    if (strKey == "folders")
    {
        SetFolders(value, nServerVersion, bSaveVersion);
    }
    else if (strKey == "folders_pos")
    {
        SetFoldersPos(value, nServerVersion);
    }
    else if (strKey == "group_tag_pos")
    {
        SetGroupTagsPos(value, nServerVersion);
    }
    else if (strKey == "favorites")
    {
        SetFavorites(value, nServerVersion);
    }
    else if (strKey == "group_tag_oem")
    {
        //SetGroupTagPropertiesOEM(lpszValue, nServerVersion);
        SetLocalValueVersion(key, nServerVersion);
    }
    else if (strKey == "group_tag_config_oem")
    {
        //SetGroupTagConfigOEM(lpszValue, nServerVersion);
        SetLocalValueVersion(key, nServerVersion);
    }
    else
    {
        SetLocalValueVersion(key, nServerVersion);
    }
}

void CWizDatabase::GetAllBizUserIds(CWizStdStringArray& arrayText)
{
    CWizBizUserDataArray arrayUser;
    if (!GetAllUsers(arrayUser))
        return;

    CWizBizUserDataArray::const_iterator it;
    for (it = arrayUser.begin(); it != arrayUser.end(); it++) {
        const WIZBIZUSER& user = *it;
        arrayText.push_back(user.userId);
    }
}

bool CWizDatabase::GetAllBizUsers(CWizBizUserDataArray& arrayUser)
{
    return GetAllUsers(arrayUser);
}

void CWizDatabase::ClearLastSyncError()
{
    setMeta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorCode"), QString::number(WIZKMSYNC_EXIT_OK));
    setMeta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorMessage"), "");

    //
    if (getPersonalDatabase() != this)
        return;

    //
    int bizCount = GetMetaDef("Bizs", "Count").toInt();
    for (int i = 0; i < bizCount; i++)
    {
        QString bizSection = "Biz_" + QString::number(i);
        setMeta(bizSection, _T("LastSyncErrorCode"), QString::number(WIZKMSYNC_EXIT_OK));
        setMeta(bizSection, _T("LastSyncErrorMessage"), "");
    }
}

QString CWizDatabase::GetLastSyncErrorMessage()
{
    return meta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorMessage"));
}

void CWizDatabase::OnTrafficLimit(const QString& strErrorMessage)
{
    setMeta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorCode"), QString::number(WIZKMSYNC_EXIT_TRAFFIC_LIMIT));
    setMeta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorMessage"), strErrorMessage);
}

void CWizDatabase::OnStorageLimit(const QString& strErrorMessage)
{
    setMeta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorCode"), QString::number(WIZKMSYNC_EXIT_STORAGE_LIMIT));
    setMeta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorMessage"), strErrorMessage);
}

void CWizDatabase::OnNoteCountLimit(const QString& strErrorMessage)
{
    setMeta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorCode"), QString::number(WIZKMSYNC_EXIT_NOTE_COUNT_LIMIT));
    setMeta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorMessage"), strErrorMessage);
}

void CWizDatabase::OnBizServiceExpr(const QString& strBizGUID, const QString& strErrorMessage)
{
    if (strBizGUID.isEmpty())
        return;

    CWizDatabase* db = getPersonalDatabase();
    if (!db)
        return;

    QString strMetaSection;
    if (!db->GetBizMetaName(strBizGUID, strMetaSection))
        return;
    //
    db->setMeta(strMetaSection, _T("LastSyncErrorCode"), QString::number(WIZKMSYNC_EXIT_BIZ_SERVICE_EXPR));
    db->setMeta(strMetaSection, _T("LastSyncErrorMessage"), strErrorMessage);
}

bool CWizDatabase::IsTrafficLimit()
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorCode"));

    return strLastError.toInt() == WIZKMSYNC_EXIT_TRAFFIC_LIMIT;
}

bool CWizDatabase::IsStorageLimit()
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorCode"));

    return strLastError.toInt() == WIZKMSYNC_EXIT_STORAGE_LIMIT;
}

bool CWizDatabase::IsNoteCountLimit()
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorCode"));

    return strLastError.toInt() == WIZKMSYNC_EXIT_NOTE_COUNT_LIMIT;
}

bool CWizDatabase::IsBizServiceExpr(const QString& strBizGUID)
{
    CWizDatabase* db = getPersonalDatabase();
    if (!db)
        return false;

    QString strMetaSection;
    if (!db->GetBizMetaName(strBizGUID, strMetaSection))
        return false;

    QString strLastError = db->meta(strMetaSection, _T("LastSyncErrorCode"));

    return strLastError.toInt() == WIZKMSYNC_EXIT_BIZ_SERVICE_EXPR;
}

bool CWizDatabase::GetStorageLimitMessage(QString &strErrorMessage)
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorCode"));

    if (strLastError.toInt() == WIZKMSYNC_EXIT_STORAGE_LIMIT)
    {
        strErrorMessage = meta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorMessage"));
        return true;
    }

    return false;
}

bool CWizDatabase::GetTrafficLimitMessage(QString& strErrorMessage)
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorCode"));

    if (strLastError.toInt() == WIZKMSYNC_EXIT_TRAFFIC_LIMIT)
    {
        strErrorMessage = meta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorMessage"));
        return true;
    }

    return false;
}

bool CWizDatabase::GetNoteCountLimit(QString& strErrorMessage)
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorCode"));

    if (strLastError.toInt() == WIZKMSYNC_EXIT_NOTE_COUNT_LIMIT)
    {
        strErrorMessage = meta(WIZKMSYNC_EXIT_INFO, _T("LastSyncErrorMessage"));
        return true;
    }

    return false;
}

bool CWizDatabase::setMeta(const QString& strSection, const QString& strKey, const QString& strValue)
{
    return SetMeta(strSection, strKey, strValue);
}

QString CWizDatabase::meta(const QString& strSection, const QString& strKey)
{
    return GetMetaDef(strSection, strKey);
}

void CWizDatabase::setBizGroupUsers(const QString& strkbGUID, const QString& strJson)
{
    SetBizUsers(strkbGUID, strJson);
}

void CWizDatabase::SetFoldersPos(const QString& foldersPos, qint64 nVersion)
{
    SetLocalValueVersion("folders_pos", nVersion);
    SetMeta("SYNC_INFO", "FOLDERS_POS", foldersPos);

    bool bPositionChanged = false;

    CString str(foldersPos);
    str.Trim();
    str.Trim('{');
    str.Trim('}');
    str.Replace("\n", "");
    str.Replace("\r", "");
    if (str.IsEmpty())
        return;

    CWizStdStringArray arrPos;
    ::WizSplitTextToArray(str, ',', arrPos);

    QSettings* setting = ExtensionSystem::PluginManager::settings();
    setting->beginGroup("FolderPosition");
    setting->remove("");
    setting->endGroup();

    CWizStdStringArray::const_iterator it;
    for (it= arrPos.begin(); it != arrPos.end(); it++) {
        CString strLine = *it;
        CString strLocation;
        CString strPos;
        if (!::WizStringSimpleSplit(strLine, ':', strLocation, strPos))
            continue;
        strLocation.Trim();
        strLocation.Trim('\"');

        int nPos = _ttoi(strPos);
        if (0 == nPos)
            continue;

        int nPosOld = setting->value("FolderPosition/" + strLocation).toInt();
        if (nPosOld != nPos) {
            setting->setValue("FolderPosition/" + strLocation, nPos);
            bPositionChanged = true;
        }
    }

    setting->sync();

    if (bPositionChanged) {
        Q_EMIT folderPositionChanged();
    }
}

QString CWizDatabase::GetFolders()
{
    CWizStdStringArray arrayFolder;
    GetAllLocations(arrayFolder);

    CWizStdStringArray arrayExtra;
    GetExtraFolder(arrayExtra);
    for (CWizStdStringArray::const_iterator it = arrayExtra.begin();
         it != arrayExtra.end(); it++)
    {
        if (-1 == ::WizFindInArray(arrayFolder, *it)) {
            arrayFolder.push_back(*it);
        }
    }

    CString str;
    ::WizStringArrayToText(arrayFolder, str, "*");

    return str;
}

QString CWizDatabase::GetFoldersPos()
{
    return meta("SYNC_INFO", "FOLDERS_POS");
}

QString CWizDatabase::GetGroupTagsPos()
{
    CWizTagDataArray arrayTag;
    GetAllTags(arrayTag);
    if (arrayTag.size() == 0)
        return QString();

    QString strTagPos;
    for (CWizTagDataArray::const_iterator it = arrayTag.begin();
         it != arrayTag.end();
         it++)
    {
        WIZTAGDATA tag = *it;
        strTagPos.append(tag.strGUID + ":" + QString::number(tag.nPostion) + "*");
    }
    strTagPos.remove(strTagPos.length() - 1, 1);
    return strTagPos;
}

void CWizDatabase::SetFolders(const QString& strFolders, qint64 nVersion, bool bSaveVersion)
{
    if (strFolders.isEmpty())
        return;

    std::set<CString> setServerFolders;
    QStringList listFolders = strFolders.split('*', QString::SkipEmptyParts);
    for (QStringList::const_iterator it = listFolders.begin();
         it != listFolders.end();
         it++)
    {
        setServerFolders.insert(*it);
    }

    std::set<CString> setLocalFolders;
    CWizStdStringArray arrayLocation;
    GetAllLocationsWithExtra(arrayLocation);

    for (CWizStdStringArray::const_iterator it = arrayLocation.begin();
         it != arrayLocation.end();
         it++)
    {
        setLocalFolders.insert(*it);
    }

    for (std::set<CString>::const_iterator it = setServerFolders.begin();
         it != setServerFolders.end();
         it++)
    {
        CString strLocation = *it;

        if (0 == strLocation.Find("/Deleted Items/"))
            continue;

        if (setLocalFolders.find(strLocation) == setLocalFolders.end())
        {
            // server exists, local does not exists, create folders.
            AddExtraFolder(strLocation);
        }
    }

    for (std::set<CString>::const_iterator it = setLocalFolders.begin();
        it != setLocalFolders.end();
        it++)
    {
        CString strLocation = *it;

        if (0 == strLocation.Find("/Deleted Items/"))
            continue;

        if (setServerFolders.find(strLocation) == setServerFolders.end())
        {
            // local exists, server does not exists, delete local folders.
            int nSize = 0;
            if (GetDocumentsCountByLocation(strLocation, nSize, true))
            {
                if (nSize == 0)
                {
                    DeleteExtraFolder(strLocation);
                }
            }
        }
    }

    if (bSaveVersion)
    {
        SetLocalValueVersion("folders", nVersion);
    }
}

void CWizDatabase::SetGroupTagsPos(const QString& tagsPos, qint64 nVersion)
{
    SetLocalValueVersion("group_tag_pos", nVersion);

    bool bPositionChanged = false;

    CString str(tagsPos);
    str.Trim();
    if (str.IsEmpty())
        return;

    CWizStdStringArray arrPos;
    ::WizSplitTextToArray(str, '*', arrPos);

    CWizStdStringArray::const_iterator it;
    for (it= arrPos.begin(); it != arrPos.end(); it++) {
        CString strLine = *it;
        QStringList posList =  strLine.split(':');
        if (posList.count() != 2) {
            qDebug() << "Process tags pos error : " << strLine;
            continue;
        }

        QString strGUID = posList.first();
        int nPos = posList.last().toInt();
        WIZTAGDATA tagData;
        if (TagFromGUID(strGUID, tagData)) {
            if (tagData.nPostion != nPos) {
                tagData.nPostion = nPos;
                ModifyTag(tagData);
                bPositionChanged = true;
            }
        }
    }

    if (bPositionChanged) {
        Q_EMIT tagsPositionChanged(kbGUID());
    }
}

QString CWizDatabase::GetFavorites()
{
    return meta("SYNC_INFO", "FAVORITES");
}

void CWizDatabase::SetFavorites(const QString& favorites, qint64 nVersion)
{
    SetLocalValueVersion("favorites", nVersion);
    SetMeta("SYNC_INFO", "FAVORITES", favorites);

    if (nVersion != -1)
    {
        emit favoritesChanged(favorites);
    }
}

void CWizDatabase::SetObjectSyncTimeLine(int nDays)
{
    SetMeta("SYNC_INFO", "TIMELINE", QString::number(nDays));
}

int CWizDatabase::GetObjectSyncTimeline()
{
    int nDays = GetMetaDef("SYNC_INFO", "TIMELINE").toInt();
    if (m_bIsPersonal && !nDays) {
        return 99999;
    } else if (!m_bIsPersonal && !nDays) {
        return 1;
    }

    return nDays;
}

void CWizDatabase::setDownloadAttachmentsAtSync(bool download)
{
    CString strD = download ? "1" : "0";
    SetMeta("QT_WIZNOTE", "SyncDownloadAttachment", strD);
}

bool CWizDatabase::getDownloadAttachmentsAtSync()
{
    return GetMetaDef("QT_WIZNOTE", "SyncDownloadAttachment", "0").toInt() != 0;
}

void CWizDatabase::SetBizUsers(const QString& strBizGUID, const QString& strJsonUsers)
{
    CWizBizUserDataArray arrayUser;

    if (!loadBizUsersFromJson(strBizGUID, strJsonUsers, arrayUser)) {
        return;
    }

    if (!UpdateBizUsers(arrayUser)) {
        return;
    }
}

bool CWizDatabase::loadBizUsersFromJson(const QString& strBizGUID,
                                        const QString& strJsonRaw,
                                        CWizBizUserDataArray& arrayUser)
{
    rapidjson::Document d;
    d.Parse<0>(strJsonRaw.toUtf8().constData());

    if (d.FindMember("error_code")) {
        qDebug() << QString::fromUtf8(d.FindMember("error")->value.GetString());
        return false;
    }

    if (d.FindMember("return_code")) {
        int nCode = d.FindMember("return_code")->value.GetInt();
        if (nCode != 200) {
            qDebug() << QString::fromUtf8(d.FindMember("return_message")->value.GetString()) << ", code = " << nCode;
            return false;
        }
    }

    if (!d.FindMember("result")) {
        qDebug() << "Error occured when try to parse json of biz users";
        qDebug() << strJsonRaw;
        return false;
    }

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextDecoder* encoder = codec->makeDecoder();

    const rapidjson::Value& users = d["result"];
    for (rapidjson::SizeType i = 0; i < users.Size(); i++) {
        const rapidjson::Value& u = users[i];
        if (!u.IsObject()) {
            qDebug() << "Error occured when parse json of biz users";
            return false;
        }

        WIZBIZUSER user;
        user.alias = encoder->toUnicode(u["alias"].GetString(), u["alias"].GetStringLength());
        user.pinyin = encoder->toUnicode(u["pinyin"].GetString(), u["pinyin"].GetStringLength());
        user.userGUID = encoder->toUnicode(u["user_guid"].GetString(), u["user_guid"].GetStringLength());
        user.userId = encoder->toUnicode(u["user_id"].GetString(), u["user_id"].GetStringLength());
        user.bizGUID = strBizGUID;

        arrayUser.push_back(user);
    }

    delete encoder;

    return true;
}

void CWizDatabase::SetFoldersPosModified()
{
    SetLocalValueVersion("folders_pos", -1);
    SetLocalValueVersion("folders", -1);
}

void CWizDatabase::SetGroupTagsPosModified()
{
    SetLocalValueVersion("group_tag_pos", -1);
}

bool CWizDatabase::getAllNotesOwners(CWizStdStringArray& arrayOwners)
{
    return GetAllDocumentsOwners(arrayOwners);
}



/* ---------------------------------------------------------------------- */

bool CWizDatabase::UpdateMessages(const CWizMessageDataArray& arrayMsg)
{
    // TODO: delete messages not exist on remote
    if (arrayMsg.empty())
        return false;

    qint64 nVersion = -1;

    bool bHasError = false;
    CWizMessageDataArray::const_iterator it;
    for (it = arrayMsg.begin(); it != arrayMsg.end(); it++)
    {
        const WIZMESSAGEDATA& msg = *it;
        if (!updateMessage(msg)) {
            bHasError = true;
        }

        nVersion = qMax(nVersion, msg.nVersion);
    }

    if (!bHasError) {
        SetObjectVersion(WIZMESSAGEDATA::ObjectName(), nVersion);
    }

    return !bHasError;
}

bool CWizDatabase::SetUserBizInfo(const CWizBizDataArray& arrayBiz)
{
    class CBizUserAvatar
    {
    public:
        static __int64 GetProcessedAvatarVersion(QString strBizGUID, CWizDatabase& db)
        {
            return _ttoi64(db.meta(_T("Sync"), CString(_T("AvatarChangesVer_")) + strBizGUID));
        }
        //
        static void SetProcessedAvatarVersion(QString strBizGUID, CWizDatabase& db, __int64 v)
        {
            db.SetMeta(_T("Sync"), CString(_T("AvatarChangesVer_")) + strBizGUID, WizInt64ToStr(v));
        }
        //
        static void ProcessBizAvatarVersion(QString strBizGUID, CWizDatabase& db, const std::map<QString, QString>& mapAvatars)
        {
            if (mapAvatars.empty())
                return;
            //
            __int64 oldVer = GetProcessedAvatarVersion(strBizGUID, db);
            __int64 newVer = oldVer;
            //
            for (std::map<QString, QString>::const_iterator it = mapAvatars.begin();
                 it != mapAvatars.end();
                 it++)
            {
                __int64 v = ::_ttoi64(it->first);
                if (v > oldVer)
                {
                    newVer = std::max<__int64>(v, newVer);
                    //
                    TOLOG1("[Sync] User avatar changed : %1", it->second);
                    WizService::AvatarHost::deleteAvatar(it->second);
                }
            }
            //
            if (newVer > oldVer)
            {
                SetProcessedAvatarVersion(strBizGUID, db, newVer);
            }
        }
    };


    SetMeta("Bizs", "Count", QString::number(arrayBiz.size()));
    //
    for (int i = 0; i < arrayBiz.size(); i++)
    {
        const WIZBIZDATA& biz = arrayBiz[i];
        QString bizSection = "Biz_" + QString::number(i);
        SetMeta(bizSection, "GUID", biz.bizGUID);
        SetMeta(bizSection, "Name", biz.bizName);
        SetMeta(bizSection, "UserRole", QString::number(biz.bizUserRole));
        SetMeta(bizSection, "Level", QString::number(biz.bizLevel));
        QString BizIsDue = biz.bizIsDue ? "1" : "0";
        SetMeta(bizSection, "IsDue", BizIsDue);
        //
        CBizUserAvatar::ProcessBizAvatarVersion(biz.bizGUID, *this, biz.mapAvatarChanges);
    }
    //
    return true;
}
//
//
bool CWizDatabase::IsEmptyBiz(const CWizGroupDataArray& arrayGroup, const QString& bizGUID)
{
    for (CWizGroupDataArray::const_iterator it = arrayGroup.begin();
         it != arrayGroup.end();
         it++)
    {
        if (it->bizGUID == bizGUID)
            return false;
    }
    return true;
}

bool CWizDatabase::GetUserBizInfo(bool bAllowEmptyBiz, CWizBizDataArray& arrayBiz)
{
    CWizGroupDataArray arrayGroup;
    GetUserGroupInfo(arrayGroup);
    //
    return GetUserBizInfo(bAllowEmptyBiz, arrayGroup, arrayBiz);
}
//
bool CWizDatabase::GetUserBizInfo(bool bAllowEmptyBiz, const CWizGroupDataArray& arrayGroup, CWizBizDataArray& arrayBiz)
{
    BOOL inited = false;
    int count = GetMetaDef("Bizs", "Count").toInt();
    //
    for (int i = 0; i < count; i++)
    {
        QString bizSection = "Biz_" + QString::number(i);
        //
        WIZBIZDATA biz;
        biz.bizGUID = GetMetaDef(bizSection, "GUID");
        biz.bizName = GetMetaDef(bizSection, "Name");
        biz.bizUserRole = GetMetaDef(bizSection, "UserRole").toInt();
        biz.bizLevel = GetMetaDef(bizSection, "Level").toInt();
        biz.bizIsDue = GetMetaDef(bizSection, "IsDue") == "1";
        //
        if (bAllowEmptyBiz || !IsEmptyBiz(arrayGroup, biz.bizGUID))
        {
            arrayBiz.push_back(biz);
        }
        //
        inited = true;
    }
    //
    if (inited)
        return true;
    //
    //init from old data
    QString section = "BizGroups";
    //
    count = GetMetaDef(section, "Count").toInt();
    for (int i = 0; i < count; i++)
    {
        WIZBIZDATA biz;
        biz.bizGUID = GetMetaDef(section, QString::number(i));
        biz.bizName = GetMetaDef(section, biz.bizGUID);
        //
        arrayBiz.push_back(biz);
    }
    //
    return true;
}
bool CWizDatabase::GetBizData(const QString& bizGUID, WIZBIZDATA& biz)
{
    CWizBizDataArray arrayBiz;
    if (!GetUserBizInfo(true, arrayBiz))
        return false;
    //
    for (CWizBizDataArray::const_iterator it = arrayBiz.begin();
         it != arrayBiz.end();
         it++)
    {
        if (it->bizGUID == bizGUID)
        {
            biz = *it;
            return true;
        }
    }
    return false;
}

bool CWizDatabase::GetBizGUID(const QString &strGroupGUID, QString &strBizGUID)
{
    if (strGroupGUID.isEmpty())
        return false;

    IWizSyncableDatabase* db = GetPersonalDatabase();
    if (!db)
        return false;

    QString groupSection = strGroupGUID.toUpper() + "_BIZGUID";
    strBizGUID = db->meta("GROUPS", groupSection);

    return true;
}

bool CWizDatabase::GetGroupData(const QString& groupGUID, WIZGROUPDATA& group)
{
    group.strGroupGUID = groupGUID;
    group.strGroupName = GetMetaDef(g_strGroupSection, groupGUID);

    group.bizGUID = GetMetaDef(g_strGroupSection, groupGUID + "_BizGUID");
    group.bizGUID = GetMetaDef(g_strGroupSection, groupGUID + "_BizName");
    group.bOwn = GetMetaDef(g_strGroupSection, groupGUID + "_Own") == "1";
    group.nUserGroup = GetMetaDef(g_strGroupSection, groupGUID + "_Role", QString::number(WIZ_USERGROUP_MAX)).toInt();
    group.strDatabaseServer = GetMetaDef(g_strGroupSection, group.strGroupGUID + "_DatabaseServer");
    //
    if (group.bizGUID.isEmpty())
    {
        //load biz data from old settings
        //
        QString section = "BizGroups";
        group.bizGUID = GetMetaDef(section, group.strGroupGUID);
        if (!group.bizGUID.isEmpty())
        {
            group.bizName = GetMetaDef(section, group.bizGUID);
        }
    }

    return !group.strGroupName.isEmpty();
}

bool CWizDatabase::GetOwnGroups(const CWizGroupDataArray& arrayAllGroup, CWizGroupDataArray& arrayOwnGroup)
{
    for (CWizGroupDataArray::const_iterator it = arrayAllGroup.begin();
         it != arrayAllGroup.end();
         it++)
    {
        if (it->IsBiz())
            continue;
        if (it->IsOwn())
        {
            arrayOwnGroup.push_back(*it);
        }
    }
    return true;
}

bool CWizDatabase::GetJionedGroups(const CWizGroupDataArray& arrayAllGroup, CWizGroupDataArray& arrayJionedGroup)
{
    for (CWizGroupDataArray::const_iterator it = arrayAllGroup.begin();
         it != arrayAllGroup.end();
         it++)
    {
        if (it->IsBiz())
            continue;
        if (!it->IsOwn())
        {
            arrayJionedGroup.push_back(*it);
        }
    }
    return true;
}

bool CWizDatabase::SetUserGroupInfo(const CWizGroupDataArray& arrayGroup)
{
    int nTotal = arrayGroup.size();
    // set group info
    SetMeta(g_strGroupSection, "Count", QString::number(nTotal));

    for (int i = 0; i < nTotal; i++) {
        const WIZGROUPDATA& group = arrayGroup[i];
        SetMeta(g_strGroupSection, QString::number(i), group.strGroupGUID);
        SetMeta(g_strGroupSection, group.strGroupGUID, group.strGroupName);

        SetMeta(g_strGroupSection, group.strGroupGUID + "_BizGUID", group.bizGUID);
        SetMeta(g_strGroupSection, group.strGroupGUID + "_BizName", group.bizGUID);
        SetMeta(g_strGroupSection, group.strGroupGUID + "_Own", group.bOwn ? "1" : "0");
        SetMeta(g_strGroupSection, group.strGroupGUID + "_Role", QString::number(group.nUserGroup));
        SetMeta(g_strGroupSection, group.strGroupGUID + "_DatabaseServer", group.strDatabaseServer);
    }

    return true;
}


bool CWizDatabase::GetUserGroupInfo(CWizGroupDataArray& arrayGroup)
{
    CString strTotal;
    bool bExist;

    if (!GetMeta(g_strGroupSection, "Count", strTotal, "", &bExist)) {
        return false;
    }

    // it's ok, user has no group data
    if (!bExist) {
        return true;
    }

    int nTotal = strTotal.toInt();
    for (int i = 0; i < nTotal; i++) {
        WIZGROUPDATA group;

        group.strGroupGUID = GetMetaDef(g_strGroupSection, QString::number(i));
        //
        if (!group.strGroupGUID.isEmpty())
        {
            GetGroupData(group.strGroupGUID, group);
            //
            arrayGroup.push_back(group);
        }
    }

    return true;
}

bool CWizDatabase::UpdateDeletedGUIDs(const CWizDeletedGUIDDataArray& arrayDeletedGUID)
{
    if (arrayDeletedGUID.empty())
        return true;

    qint64 nVersion = -1;

    bool bHasError = false;

    CWizDeletedGUIDDataArray::const_iterator it;
    for (it = arrayDeletedGUID.begin(); it != arrayDeletedGUID.end(); it++) {
        const WIZDELETEDGUIDDATA& data = *it;

        if (!UpdateDeletedGUID(data)) {
            bHasError = true;
        }

        nVersion = qMax(nVersion, data.nVersion);
    }

    if (!bHasError) {
        SetObjectVersion(WIZDELETEDGUIDDATA::ObjectName(), nVersion);
    }

    return !bHasError;
}


bool CWizDatabase::Open(const QString& strAccountFolderName, const QString& strKbGUID /* = NULL */)
{
    Q_ASSERT(!strAccountFolderName.isEmpty());

    m_strAccountFolderName = strAccountFolderName;

    if (strKbGUID.isEmpty()) {
        m_bIsPersonal = true;
    } else {
        m_bIsPersonal = false;
        setKbGUID(strKbGUID);
    }

    if (!CWizIndex::Open(GetIndexFileName())) {
        // If can not open db, try again. If db still can not be opened, delete the db file and download data from server.
        if (!CWizIndex::Open(GetIndexFileName())) {
            QFile::remove(GetIndexFileName());
            return false;
        }
    }

    // user private database opened, try to load kb guid
    if (strKbGUID.isEmpty()) {
        // user private kb_guid should be set before actually open for operating
        QString strUserKbGuid = GetMetaDef(g_strDatabaseInfoSection, "KBGUID");
        if (!strUserKbGuid.isEmpty())
            setKbGUID(strUserKbGuid);
    }

    // FIXME
    if (!CThumbIndex::OpenThumb(GetThumbFileName(), getThumIndexVersion())) {
        // If can not open db, try again. If db still can not be opened, delete the db file and download data from server.
        if (!CThumbIndex::OpenThumb(GetThumbFileName(), getThumIndexVersion())) {
            QFile::remove(GetThumbFileName());
            return false;
        }
    }

    setThumbIndexVersion(WIZNOTE_THUMB_VERSION);

    LoadDatabaseInfo();

    return true;
}

bool CWizDatabase::LoadDatabaseInfo()
{
    QString strUserId = GetMetaDef(g_strAccountSection, "USERID");
    if (!strUserId.isEmpty())
    {
        m_strUserId = strUserId;
    }

    if (!kbGUID().isEmpty())
    {
        m_info.bizName = GetMetaDef(g_strDatabaseInfoSection, "BizName");
        m_info.bizGUID = GetMetaDef(g_strDatabaseInfoSection, "BizGUID");
    }

    m_info.name = GetMetaDef(g_strDatabaseInfoSection, "Name");
    m_info.nPermission = GetMetaDef(g_strDatabaseInfoSection, "Permission").toInt();
    m_info.bOwner = GetMetaDef(g_strDatabaseInfoSection, "Owner") == "1";

    return true;
}


bool CWizDatabase::InitDatabaseInfo(const WIZDATABASEINFO& dbInfo)
{
    Q_ASSERT(!dbInfo.name.isEmpty());

    int nErrors = 0;

    // general
    if (!SetMeta(g_strDatabaseInfoSection, "Name", dbInfo.name))
        nErrors++;
    if (!SetMeta(g_strDatabaseInfoSection, "Permission", QString::number(dbInfo.nPermission)))
        nErrors++;
    if (!SetMeta(g_strDatabaseInfoSection, "Owner", dbInfo.bOwner ? "1" : "0"))
        nErrors++;

    // biz group info
    if (!dbInfo.bizGUID.isEmpty() && !dbInfo.bizName.isEmpty()) {
        if (!SetMeta(g_strDatabaseInfoSection, "BizName", dbInfo.bizName))
            nErrors++;
        if (!SetMeta(g_strDatabaseInfoSection, "BizGUID", dbInfo.bizGUID))
            nErrors++;
    }

    if (!SetMeta(g_strDatabaseInfoSection, "KbGUID", kbGUID()))
        nErrors++;

    if (!SetMeta(g_strDatabaseInfoSection, "Version", WIZ_DATABASE_VERSION))
        nErrors++;
    //
    LoadDatabaseInfo();

    if (nErrors)
        return false;

    return true;
}


bool CWizDatabase::SetDatabaseInfo(const WIZDATABASEINFO& dbInfo)
{
    Q_ASSERT(!dbInfo.name.isEmpty());

    int nErrors = 0;

    // general
    if (m_info.name != dbInfo.name) {
        m_info.name = dbInfo.name;

        if (!SetMeta(g_strDatabaseInfoSection, "Name", dbInfo.name))
            nErrors++;

        Q_EMIT databaseRename(kbGUID());
    }

    if (m_info.nPermission != dbInfo.nPermission) {
        m_info.nPermission = dbInfo.nPermission;

        if (!SetMeta(g_strDatabaseInfoSection, "Permission", QString::number(dbInfo.nPermission)))
            nErrors++;

        Q_EMIT databasePermissionChanged(kbGUID());
    }


    // biz group info
    if (!dbInfo.bizGUID.isEmpty() && !dbInfo.bizName.isEmpty()) {
        bool bResetBiz = false;
        if (m_info.bizName != dbInfo.bizName) {
            m_info.bizName = dbInfo.bizName;

            if (!SetMeta(g_strDatabaseInfoSection, "BizName", dbInfo.bizName))
                nErrors++;

            bResetBiz = true;
        }

        if (m_info.bizGUID != dbInfo.bizGUID) {
            m_info.bizGUID = dbInfo.bizGUID;

            if (!SetMeta(g_strDatabaseInfoSection, "BizGUID", dbInfo.bizGUID))
                nErrors++;

            bResetBiz = true;
        }

        if (bResetBiz) {
            Q_EMIT databaseBizChanged(kbGUID());
        }
    }

    if (!SetMeta(g_strDatabaseInfoSection, "KbGUID", kbGUID()))
        nErrors++;

    if (!SetMeta(g_strDatabaseInfoSection, "Version", WIZ_DATABASE_VERSION))
        nErrors++;

    if (nErrors)
        return false;

    return true;
}


QString CWizDatabase::GetAccountPath() const
{
    Q_ASSERT(!m_strAccountFolderName.isEmpty());

    QString strPath = Utils::PathResolve::dataStorePath() + m_strAccountFolderName + "/";
    WizEnsurePathExists(strPath);

    return strPath;
}

QString CWizDatabase::GetAccountFolderName() const
{
    return m_strAccountFolderName;
}

QString CWizDatabase::GetDataPath() const
{
    QString strPath;

    if (m_bIsPersonal) {
        strPath = GetAccountPath() + "data/";
    } else {
        strPath = GetAccountPath() + "group/" + kbGUID() + "/";
    }

    WizEnsurePathExists(strPath);
    return strPath;
}

QString CWizDatabase::GetIndexFileName() const
{
    return GetDataPath() + "index.db";
}

QString CWizDatabase::GetThumbFileName() const
{
    return GetDataPath() + "wizthumb.db";
}

QString CWizDatabase::GetDocumentsDataPath() const
{
    QString strPath = GetDataPath() + "notes/";
    WizEnsurePathExists(strPath);
    return  strPath;
}

QString CWizDatabase::GetAttachmentsDataPath() const
{
    QString strPath = GetDataPath() + "attachments/";
    WizEnsurePathExists(strPath);
    return  strPath;
}

QString CWizDatabase::GetDocumentFileName(const QString& strGUID) const
{
    return GetDocumentsDataPath() + "{" + strGUID + "}";
}

QString CWizDatabase::GetAttachmentFileName(const QString& strGUID)
{
    WIZDOCUMENTATTACHMENTDATA attach;
    AttachmentFromGUID(strGUID, attach);
    //
    QString strOldFileName = GetAttachmentsDataPath() + "{" + strGUID + "}";
    QString strNewFileName = strOldFileName + attach.strName;

    // Compatible with the old version
    if (QFile::exists(strOldFileName))
    {
        if (!QFile::rename(strOldFileName, strNewFileName))
        {
            TOLOG2("[Attach] rename file failed from %1 to %2 ", strOldFileName, strNewFileName);
        }
    }
    return  strNewFileName;
}

QString CWizDatabase::GetAvatarPath() const
{
    QString strPath = GetAccountPath() + "avatar/";
    WizEnsurePathExists(strPath);

    return strPath;
}

QString CWizDatabase::GetDefaultNoteLocation() const
{
    if (m_bIsPersonal)
        return LOCATION_DEFAULT;
    else
        return "/"+m_strUserId+"/";
}

QString CWizDatabase::GetDocumentAuthorAlias(const WIZDOCUMENTDATA& doc)
{
    if (!doc.strAuthor.isEmpty())
        return doc.strAuthor;

    CWizDatabase* personDb = getPersonalDatabase();
    if (!personDb)
        return QString();

    if (doc.strKbGUID.isEmpty() || doc.strKbGUID == personDb->kbGUID())
    {
        QString displayName;
        if (personDb->GetUserDisplayName(displayName))
            return displayName;
    }

    QString strUserID = doc.strOwner;
    WIZBIZUSER bizUser;
    personDb->userFromID(doc.strKbGUID, strUserID, bizUser);
    if (bizUser.alias.isEmpty())
    {
        int index = doc.strOwner.indexOf('@');
        return index == -1 ? doc.strOwner :  doc.strOwner.left(index);
    }
    return bizUser.alias;
}

QString CWizDatabase::GetDocumentOwnerAlias(const WIZDOCUMENTDATA& doc)
{
    CWizDatabase* personDb = getPersonalDatabase();
    if (!personDb)
        return QString();

    QString strUserID = doc.strOwner;

    //NOTE: 用户可能使用手机号登录，此时owner为手机号，需要使用昵称
    if (!strUserID.contains('@') && strUserID == personDb->GetUserId())
    {
        personDb->GetUserDisplayName(strUserID);
    }

    WIZBIZUSER bizUser;
    personDb->userFromID(doc.strKbGUID, strUserID, bizUser);
    if (bizUser.alias.isEmpty())
    {
        int index = strUserID.indexOf('@');
        return index == -1 ? strUserID :  strUserID.left(index);
    }
    return bizUser.alias;
}

bool CWizDatabase::GetUserName(QString& strUserName)
{
    strUserName = GetMetaDef(g_strAccountSection, "UserName");
    return true;
}

bool CWizDatabase::SetUserName(const QString& strUserName)
{
    CString strOld;
    GetUserName(strOld);
    if (!strOld.IsEmpty()) {
        TOLOG("Can not set user name: user name exists!");
        return false;
    }

    if (!SetMeta(g_strAccountSection, _T("UserName"), strUserName)) {
        TOLOG("Failed to set user name while SetUserName");
        return false;
    }

    return true;
}

bool CWizDatabase::GetUserDisplayName(QString &strDisplayName)
{
    strDisplayName = GetMetaDef(g_strAccountSection, "DISPLAYNAME");
    return true;
}

QString CWizDatabase::GetUserAlias()
{
    CWizDatabase* personDb = getPersonalDatabase();
    if (!personDb)
        return QString();

    QString strUserGUID = personDb->GetUserGUID();
    WIZBIZUSER bizUser;
    personDb->userFromGUID(kbGUID(), strUserGUID, bizUser);
    if (!bizUser.alias.isEmpty()) {
        return bizUser.alias;
    } else {
        QString strUserName;
        personDb->GetUserDisplayName(strUserName);
        return strUserName;
    }

    return QString();
}

QString CWizDatabase::GetEncryptedPassword()
{
    return GetMetaDef(g_strAccountSection, "Password");
}

bool CWizDatabase::GetPassword(CString& strPassword)
{
    if (!m_strPassword.isEmpty()) {
        strPassword = m_strPassword;
        return true;
    }

    bool bExists = false;
    if (!GetMeta(g_strAccountSection, "Password", strPassword, "", &bExists)) {
        TOLOG("Failed to get password while GetPassword");
        return false;
    }

    //if (strPassword.IsEmpty())
    //    return true;

    //strPassword = WizDecryptPassword(strPassword);

    return true;
}

bool CWizDatabase::SetPassword(const QString& strPassword, bool bSave)
{
    m_strPassword = strPassword;

    if (bSave) {
        if (!SetMeta(g_strAccountSection, "Password", strPassword)) {
            TOLOG("Failed to set user password");
            return false;
        }
    }

    return true;
}

UINT CWizDatabase::GetPasswordFalgs()
{
    CString strFlags = GetMetaDef(g_strAccountSection, "PasswordFlags", "");
    return (UINT)atoi(strFlags.toUtf8());
}

bool CWizDatabase::SetPasswordFalgs(UINT nFlags)
{
    return SetMeta(g_strAccountSection, "PasswordFlags", WizIntToStr(int(nFlags)));
}

bool CWizDatabase::GetUserCert(QString& strN, QString& stre, QString& strd, QString& strHint)
{
    strN = GetMetaDef(g_strCertSection, "N");
    stre = GetMetaDef(g_strCertSection, "E");
    strd = GetMetaDef(g_strCertSection, "D");
    strHint = GetMetaDef(g_strCertSection, "Hint");

    if (strN.isEmpty() || stre.isEmpty() || strd.isEmpty())
        return false;

    return true;
}

bool CWizDatabase::SetUserCert(const QString& strN, const QString& stre, const QString& strd, const QString& strHint)
{
    return SetMeta(g_strCertSection, "N", strN) \
            && SetMeta(g_strCertSection, "E", stre) \
            && SetMeta(g_strCertSection, "D", strd) \
            && SetMeta(g_strCertSection, "Hint", strHint);
}

bool CWizDatabase::GetUserInfo(WIZUSERINFO& userInfo)
{
    userInfo.strUserGUID = GetMetaDef(g_strAccountSection, "GUID");
    userInfo.strDisplayName = GetMetaDef(g_strAccountSection, "DisplayName");
    userInfo.strUserType = GetMetaDef(g_strAccountSection, "UserType");
    userInfo.strUserLevelName = GetMetaDef(g_strAccountSection, "UserLevelName");
    userInfo.nUserLevel = GetMetaDef(g_strAccountSection, "UserLevel").toInt();
    userInfo.nUserPoints = GetMetaDef(g_strAccountSection, "UserPoints").toInt();
    userInfo.strMywizEmail = GetMetaDef(g_strAccountSection, "MywizMail");
    userInfo.tCreated = QDateTime::fromString(GetMetaDef(g_strAccountSection, "DateSignUp"));

    return true;
}

void CWizDatabase::UpdateInvalidData()
{
    int nVersion = meta(USER_SETTINGS_SECTION, "AppVersion").toInt();
    if (nVersion < 202006)
    {
        setMeta(USER_SETTINGS_SECTION, "EditorBackgroundColor", "");
    }

    setMeta(USER_SETTINGS_SECTION, "AppVersion", QString::number(Utils::Misc::getVersionCode()));
}

bool CWizDatabase::updateBizUser(const WIZBIZUSER& user)
{
    bool bRet = false;

    WIZBIZUSER userTemp;
    if (userFromGUID(user.bizGUID, user.userGUID, userTemp)) {
        // only modify user when alias changed
//        if (userTemp.alias != user.alias) {
        bRet = modifyUserEx(user);
        if (bRet && userTemp.userId != user.userId) {
            qDebug() << "User id changed " << userTemp.userId << user.userId;
            emit userIdChanged(userTemp.userId, user.userId);
        }
        //} else {
        //    bRet = true;
        //}
    } else {
        bRet = createUserEx(user);
    }

    if (!bRet) {
        Q_EMIT updateError("Failed to update user: " + user.alias);
    }

    return bRet;
}

bool CWizDatabase::UpdateBizUsers(const CWizBizUserDataArray& arrayUser)
{
    // TODO: delete users not exist on remote
    if (arrayUser.empty())
        return false;

    bool bHasError = false;
    CWizBizUserDataArray::const_iterator it;
    for (it = arrayUser.begin(); it != arrayUser.end(); it++)
    {
        const WIZBIZUSER& user = *it;
        if (!updateBizUser(user)) {
            bHasError = true;
        }
    }

    return !bHasError;
}

bool CWizDatabase::updateMessage(const WIZMESSAGEDATA& msg)
{
    bool bRet = false;

    WIZMESSAGEDATA msgTemp;
    if (messageFromId(msg.nId, msgTemp)) {
        bRet = modifyMessageEx(msg);
    } else {
        bRet = createMessageEx(msg);
    }

    if (!bRet) {
        Q_EMIT updateError("Failed to update message: " + msg.title);
    }

    return bRet;
}

bool CWizDatabase::UpdateDeletedGUID(const WIZDELETEDGUIDDATA& data)
{
    bool bRet = false;

    QString strType = WIZOBJECTDATA::ObjectTypeToTypeString(data.eType);
    bool bExists = false;
    ObjectExists(data.strGUID, strType, bExists);
    if (!bExists)
        return true;    

    qDebug() << "delete object: " << strType << " guid: " << data.strGUID;

    bRet = DeleteObject(data.strGUID, strType, false);

    if (!bRet) {
        Q_EMIT updateError("Failed to delete object: " + data.strGUID + " type: " + strType);
    }

    return bRet;
}

bool CWizDatabase::UpdateTag(const WIZTAGDATA& data)
{
    bool bRet = false;

    WIZTAGDATA dataTemp;
    if (TagFromGUID(data.strGUID, dataTemp)) {
        bRet = ModifyTagEx(data);
    } else {
        bRet = CreateTagEx(data);
    }

    if (!bRet) {
        Q_EMIT updateError("Failed to update tag: " + data.strName);
    }

    return bRet;
}

bool CWizDatabase::UpdateStyle(const WIZSTYLEDATA& data)
{
    bool bRet = false;

    WIZSTYLEDATA dataTemp;
    if (StyleFromGUID(data.strGUID, dataTemp)) {
        bRet = ModifyStyleEx(data);
    } else {
        bRet = CreateStyleEx(data);
    }

    if (!bRet) {
        Q_EMIT updateError("Failed to update style: " + data.strName);
    }

    return bRet;
}

bool CWizDatabase::UpdateDocument(const WIZDOCUMENTDATAEX& data)
{
    Q_ASSERT(data.nVersion != -1);

    bool bRet = false;

    WIZDOCUMENTDATAEX dataTemp;
    if (DocumentFromGUID(data.strGUID, dataTemp)) {
        if (data.nObjectPart & WIZKM_XMLRPC_OBJECT_PART_INFO) {
            bRet = ModifyDocumentInfoEx(data);
            if (dataTemp.strDataMD5 != data.strDataMD5) {
                SetObjectDataDownloaded(data.strGUID, "document", false);
            }
        } else {
            bRet = true;
        }
    } else {
        Q_ASSERT(data.nObjectPart & WIZKM_XMLRPC_OBJECT_PART_INFO);

        bRet = CreateDocumentEx(data);
    }

    if (!bRet) {
        Q_EMIT updateError("Failed to update document: " + data.strTitle);
    }

    WIZDOCUMENTDATA doc(data);

    if (!data.arrayParam.empty()) {
        SetDocumentParams(doc, data.arrayParam, false);
    }

    if (!data.arrayTagGUID.empty()) {
        SetDocumentTags(doc, data.arrayTagGUID, false);
    }

    return bRet;
}

bool CWizDatabase::UpdateAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    bool bRet = false;

    WIZDOCUMENTATTACHMENTDATAEX dataTemp;
    if (AttachmentFromGUID(data.strGUID, dataTemp)) {
        bRet = ModifyAttachmentInfoEx(data);

        bool changed = dataTemp.strDataMD5 != data.strDataMD5;
        if (changed) {
            SetObjectDataDownloaded(data.strGUID, "attachment", false);
        }
    } else {
        bRet = CreateAttachmentEx(data);
        UpdateDocumentAttachmentCount(data.strDocumentGUID, false);
    }

    if (!bRet) {
        Q_EMIT updateError("Failed to update attachment: " + data.strName);
    }

    return bRet;
}

bool CWizDatabase::UpdateTags(const CWizTagDataArray& arrayTag)
{
    if (arrayTag.empty())
        return true;

    qint64 nVersion = -1;

    bool bHasError = false;
    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        const WIZTAGDATA& tag = *it;

        Q_EMIT processLog("tag: " + tag.strName);

        if (!UpdateTag(tag)) {
            bHasError = true;
        }

        nVersion = qMax(nVersion, tag.nVersion);
    }

    if (!bHasError) {
        SetObjectVersion(WIZTAGDATA::ObjectName(), nVersion);
    }

    return !bHasError;
}


bool CWizDatabase::UpdateStyles(const CWizStyleDataArray& arrayStyle)
{
    if (arrayStyle.empty())
        return true;

    qint64 nVersion = -1;

    bool bHasError = false;
    CWizStyleDataArray::const_iterator it;
    for (it = arrayStyle.begin(); it != arrayStyle.end(); it++) {
        const WIZSTYLEDATA& data = *it;

        Q_EMIT processLog("style: " + data.strName);

        if (!UpdateStyle(data)) {
            bHasError = true;
        }

        nVersion = qMax(nVersion, data.nVersion);
    }

    if (!bHasError) {
        SetObjectVersion(WIZSTYLEDATA::ObjectName(), nVersion);
    }

    return !bHasError;
}

bool CWizDatabase::UpdateDocuments(const std::deque<WIZDOCUMENTDATAEX>& arrayDocument)
{
    if (arrayDocument.empty())
        return true;

    qint64 nVersion = -1;

    bool bHasError = false;
    std::deque<WIZDOCUMENTDATAEX>::const_iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        const WIZDOCUMENTDATAEX& data = *it;

        Q_EMIT processLog("note: " + data.strTitle);

        if (!UpdateDocument(data)) {
            bHasError = true;
        }

        nVersion = qMax(nVersion, data.nVersion);
    }

    if (!bHasError) {
        SetObjectVersion(WIZDOCUMENTDATAEX::ObjectName(), nVersion);
    }

    return !bHasError;
}

bool CWizDatabase::UpdateAttachments(const CWizDocumentAttachmentDataArray& arrayAttachment)
{
    if (arrayAttachment.empty())
        return true;

    qint64 nVersion = -1;

    bool bHasError = false;

    std::deque<WIZDOCUMENTATTACHMENTDATAEX>::const_iterator it;
    for (it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        const WIZDOCUMENTATTACHMENTDATAEX& data = *it;

        Q_EMIT processLog("attachment: " + data.strName);

        if (!UpdateAttachment(data)) {
            bHasError = true;
        }

        nVersion = qMax(nVersion, data.nVersion);
    }

    if (!bHasError) {
        SetObjectVersion(WIZDOCUMENTATTACHMENTDATAEX::ObjectName(), nVersion);
    }

    emit attachmentsUpdated();

    return !bHasError;
}

bool CWizDatabase::SetDocumentFlags(WIZDOCUMENTDATA& data, const QString& strFlags, bool bUpdateParamMd5)
{
    return SetDocumentParam(data, TABLE_KEY_WIZ_DOCUMENT_PARAM_FLAGS,strFlags, bUpdateParamMd5);
}

bool CWizDatabase::UpdateDocumentData(WIZDOCUMENTDATA& data,
                                      const QString& strHtml,
                                      const QString& strURL,
                                      int nFlags,
                                      bool notifyDataModify /*= true*/)
{
    m_mtxTempFile.lock();
    QString strProcessedHtml(strHtml);
    QString strResourcePath = GetResoucePathFromFile(strURL);
    if (!strResourcePath.isEmpty()) {
        QUrl urlResource = QUrl::fromLocalFile(strResourcePath);
        strProcessedHtml.replace(urlResource.toString(), "index_files/");
    }
    m_mtxTempFile.unlock();

    CWizDocument doc(*this, data);
    CString strMetaText = doc.GetMetaText();

    CString strZipFileName = GetDocumentFileName(data.strGUID);
    if (!data.nProtected) {
        bool bZip = ::WizHtml2Zip(strURL, strProcessedHtml, strResourcePath, nFlags, strMetaText, strZipFileName);
        if (!bZip) {
            return false;
        }
    } else {
        CString strTempFile = Utils::PathResolve::tempPath() + data.strGUID + "-decrypted";
        bool bZip = ::WizHtml2Zip(strURL, strProcessedHtml, strResourcePath, nFlags, strMetaText, strTempFile);
        if (!bZip) {
            return false;
        }

        if (!m_ziwReader->encryptDataToTempFile(strTempFile, strZipFileName)) {
            return false;
        }
    }

    SetObjectDataDownloaded(data.strGUID, "document", true);

    return UpdateDocumentDataMD5(data, strZipFileName, notifyDataModify);
}

void CWizDatabase::ClearUnusedImages(const QString& strHtml, const QString& strFilePath)
{
    CWizStdStringArray arrayImageFileName;
    ::WizEnumFiles(strFilePath, "*.jpg;*.png;*.bmp;*.gif", arrayImageFileName, 0);
    if (!arrayImageFileName.empty())
    {
        CWizStdStringArray::const_iterator it;
        for (it = arrayImageFileName.begin(); it != arrayImageFileName.end(); it++)
        {
            if (!strHtml.contains(*it))
            {
                QFile::remove(*it);
            }
        }
    }
}

bool CWizDatabase::UpdateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName, bool notifyDataModify /*= true*/)
{
    bool bRet = CWizIndex::UpdateDocumentDataMD5(data, strZipFileName, notifyDataModify);

    UpdateDocumentAbstract(data.strGUID);

    return bRet;
}

bool CWizDatabase::DeleteObject(const QString& strGUID, const QString& strType, bool bLog)
{
    if (strGUID.isEmpty() || strType.isEmpty())
        return false;

    if (!IsGroup())
    {
        BOOL objectExists = FALSE;
        if (-1 == GetObjectLocalVersionEx(strGUID, strType, objectExists))
        {
            if (objectExists)
            {
                TOLOG2(_T("[%1] object [%2] is modified on local, skip to delete it"), strType, strGUID);
                return S_FALSE;
            }
            else
            {
                return S_OK;
            }
        }
    }


    if (0 == strType.compare("tag", Qt::CaseInsensitive))
    {
        WIZTAGDATA data;
        if (TagFromGUID(strGUID, data)) {
            DeleteTag(data, bLog, false);
        }
        return true;
    }
    else if (0 == strType.compare("style", Qt::CaseInsensitive))
    {
        WIZSTYLEDATA data;
        if (StyleFromGUID(strGUID, data)) {
            return DeleteStyle(data, bLog, false);
        }
        return true;
    }
    else if (0 == strType.compare("document", Qt::CaseInsensitive))
    {
        WIZDOCUMENTDATA data;
        if (DocumentFromGUID(strGUID, data)) {
            CString strZipFileName = GetDocumentFileName(strGUID);
            if (PathFileExists(strZipFileName))
            {
                WizDeleteFile(strZipFileName);
            }
            return DeleteDocument(data, bLog);
        }
        return true;
    }
    else if (0 == strType.compare("attachment", Qt::CaseInsensitive))
    {
        WIZDOCUMENTATTACHMENTDATA data;
        if (AttachmentFromGUID(strGUID, data)) {
            CString strZipFileName = GetAttachmentFileName(strGUID);
            if (PathFileExists(strZipFileName))
            {
                WizDeleteFile(strZipFileName);
            }
            return DeleteAttachment(data, bLog, false);
        }
        return true;
    }
    else
    {
        Q_ASSERT(0);
        TOLOG1("Unknown object type: %1", strType);
        return false;
    }
}


bool CWizDatabase::DeleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data,
                                    bool bLog, bool bReset, bool updateAttachList)
{
    CString strFileName = GetAttachmentFileName(data.strGUID);
    bool bRet = CWizIndex::DeleteAttachment(data, bLog, bReset, updateAttachList);
    if (PathFileExists(strFileName)) {
        ::WizDeleteFile(strFileName);
    }

    if (updateAttachList) {
        emit attachmentsUpdated();
    }

    return bRet;
}

bool CWizDatabase::DeleteGroupFolder(const WIZTAGDATA& data, bool bLog)
{
    CWizTagDataArray arrayChildTag;
    GetChildTags(data.strGUID, arrayChildTag);
    foreach (const WIZTAGDATA& childTag, arrayChildTag)
    {
        DeleteGroupFolder(childTag, bLog);
    }

    CWizDocumentDataArray arrayDocument;
    GetDocumentsByTag(data, arrayDocument);

    for (WIZDOCUMENTDATAEX doc : arrayDocument)
    {
        CWizDocument document(*this, doc);
        document.deleteToTrash();
    }

    return DeleteTag(data, bLog);
}

bool CWizDatabase::IsDocumentModified(const CString& strGUID)
{
    return CWizIndex::IsObjectDataModified(strGUID, "document");
}

bool CWizDatabase::IsAttachmentModified(const CString& strGUID)
{
    return CWizIndex::IsObjectDataModified(strGUID, "attachment");
}

bool CWizDatabase::IsDocumentDownloaded(const CString& strGUID)
{
    return CWizIndex::IsObjectDataDownloaded(strGUID, "document");
}

bool CWizDatabase::IsAttachmentDownloaded(const CString& strGUID)
{
    return CWizIndex::IsObjectDataDownloaded(strGUID, "attachment");
}

bool CWizDatabase::GetAllObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayData, int nTimeLine)
{
    if (nTimeLine == -1)
        return true;
    //
    CWizDocumentDataArray arrayDocument;
    CWizDocumentAttachmentDataArray arrayAttachment;
    GetNeedToBeDownloadedDocuments(arrayDocument);
    GetNeedToBeDownloadedAttachments(arrayAttachment);
    arrayData.assign(arrayAttachment.begin(), arrayAttachment.end());
    arrayData.insert(arrayData.begin(), arrayDocument.begin(), arrayDocument.end());

    if (nTimeLine == -1) {
        arrayData.clear();
        return true;
    } else if (nTimeLine == 9999) {
        return true;
    } else {
        COleDateTime tNow = ::WizGetCurrentTime();
        size_t count = arrayData.size();
        for (intptr_t i = count - 1; i >= 0; i--)
        {
            COleDateTime t = arrayData[i].tTime;
            t = t.addDays(nTimeLine);
            if (t < tNow) {
                arrayData.erase(arrayData.begin() + i);
                continue;
            }

            CWizDatabase* privateDB = getPersonalDatabase();
            if (!privateDB->getDownloadAttachmentsAtSync())
            {
                if (arrayData[i].eObjectType == wizobjectDocumentAttachment) {
                    arrayData.erase(arrayData.begin() + i);
                    continue;
                }
            }
        }
    }

    return true;
}

bool CWizDatabase::UpdateSyncObjectLocalData(const WIZOBJECTDATA& data)
{
    qDebug() << "update object data, name: " << data.strDisplayName << "guid: " << data.strObjectGUID;

    if (data.eObjectType == wizobjectDocumentAttachment)
    {
        if (!SaveCompressedAttachmentData(data.strObjectGUID, data.arrayData))
        {
            Q_EMIT updateError("Failed to save attachment data: " + data.strDisplayName);
            return false;
        }
    }
    else if (data.eObjectType == wizobjectDocument)
    {
        WIZDOCUMENTDATA document;
        if (!DocumentFromGUID(data.strObjectGUID, document)) {
            qCritical() << "Update object data failed, can't find database record!\n";
            return false;
        }

        CString strFileName = GetObjectFileName(data);
        if (!::WizSaveDataToFile(strFileName, data.arrayData))
        {
            Q_EMIT updateError("Failed to save document data: " + data.strDisplayName);
            return false;
        }

        Q_EMIT documentDataModified(document);
        UpdateDocumentAbstract(data.strObjectGUID);
        setDocumentSearchIndexed(data.strObjectGUID, false);
    } else {
        Q_ASSERT(0);
    }

    SetObjectDataDownloaded(data.strObjectGUID, WIZOBJECTDATA::ObjectTypeToTypeString(data.eObjectType), true);

    return true;
}


CString CWizDatabase::GetObjectFileName(const WIZOBJECTDATA& data)
{
    if (data.eObjectType == wizobjectDocument)
        return GetDocumentFileName(data.strObjectGUID);
    else if (data.eObjectType == wizobjectDocumentAttachment)
        return GetAttachmentFileName(data.strObjectGUID);
    else
    {
        Q_ASSERT(false);
        return CString();
    }
}

bool CWizDatabase::GetAllRootLocations(const CWizStdStringArray& arrayAllLocation, \
                                       CWizStdStringArray& arrayLocation)
{
    std::set<QString> setRoot;

    CWizStdStringArray::const_iterator it;
    for (it = arrayAllLocation.begin(); it != arrayAllLocation.end(); it++) {
        setRoot.insert(GetRootLocation(*it));
    }

    arrayLocation.assign(setRoot.begin(), setRoot.end());

    return true;
}

bool CWizDatabase::GetChildLocations(const CWizStdStringArray& arrayAllLocation, \
                                     const QString& strLocation, \
                                     CWizStdStringArray& arrayLocation)
{
    if (strLocation.isEmpty())
        return GetAllRootLocations(arrayAllLocation, arrayLocation);

    std::set<QString> setLocation;

    CWizStdStringArray::const_iterator it;
    for (it = arrayAllLocation.begin(); it != arrayAllLocation.end(); it++) {
        const QString& str = *it;

        if (str.length() > strLocation.length() && str.startsWith(strLocation))
        {
            int index = str.indexOf('/', strLocation.length() + 1);
            if (index > 0)
            {
                QString strChild = str.left(index + 1);
                setLocation.insert(strChild);
            }
        }
    }

    arrayLocation.assign(setLocation.begin(), setLocation.end());

    return true;
}

bool CWizDatabase::IsInDeletedItems(const CString& strLocation)
{
    return strLocation.startsWith(LOCATION_DELETED_ITEMS);
}

bool CWizDatabase::GetDocumentTitleStartWith(const QString& titleStart, int nMaxCount, CWizStdStringArray& arrayTitle)
{
    QString sql = QString("SELECT DISTINCT DOCUMENT_TITLE FROM WIZ_DOCUMENT WHERE "
                          "DOCUMENT_TITLE LIKE '" + titleStart + "%' LIMIT " + QString::number(nMaxCount) + ";");

//    CString strSQL;
//    strSQL.Format(_T("SELECT DISTINCT DOCUMENT_TITLE FROM WIZ_DOCUMENT WHERE "
//                     "DOCUMENT_TITLE LIKE 's%%' LIMIT s%;"),
//                  titleStart.utf16(),
//                  WizIntToStr(nMaxCount).utf16());

    return SQLToStringArray(sql, 0, arrayTitle);
}

QString CWizDatabase::GetDocumentLocation(const WIZDOCUMENTDATA& doc)
{
//    WIZDOCUMENTDATA doc;
//    if (!DocumentFromGUID(doc.strGUID, doc))
//        return QString();

    if (!IsGroup())
        return doc.strLocation;

    CWizTagDataArray arrayTag;
    if (GetDocumentTags(doc.strGUID, arrayTag))
    {
        if (arrayTag.size() > 1) {
            TOLOG1("Group document should only have one tag: %1", doc.strTitle);
        }

        QString tagText;
        if (arrayTag.size()) {
            tagText = getTagTreeText(arrayTag[0].strGUID);
        }
        return "/" + name() + tagText + "/";
    }

    return QString();
}

bool CWizDatabase::CreateDocumentAndInit(const CString& strHtml, \
                                         const CString& strHtmlUrl, \
                                         int nFlags, \
                                         const CString& strTitle, \
                                         const CString& strName, \
                                         const CString& strLocation, \
                                         const CString& strURL, \
                                         WIZDOCUMENTDATA& data)
{
    bool bRet = false;
    try
    {
        BeginUpdate();

        data.strKbGUID = kbGUID();
        data.strOwner = GetUserId();
        bRet = CreateDocument(strTitle, strName, strLocation, strHtmlUrl, data);
        if (bRet)
        {
            //add default css
            QString newHtml = strHtml;
            QFile cssFile(Utils::PathResolve::resourcesPath() + "files/editor/cssForNewNote");
            cssFile.open(QFile::Text | QFile::ReadOnly);
            QString strCSS = cssFile.readAll();
            cssFile.close();
            if (strHtml.contains("<html>", Qt::CaseInsensitive))
            {
                QString strHead;
                Utils::Misc::splitHtmlToHeadAndBody(strHtml, strHead, newHtml);
                strHead.append(strCSS);
                newHtml = "<html><head>" + strHead + "</head><body>" + newHtml + "</body></html>";
            }
            else
            {
                newHtml = "<html><head>" + strCSS + "</head><body>" + strHtml + "</body></html>";
            }

            bRet = UpdateDocumentData(data, newHtml, strURL, nFlags);

            Q_EMIT documentCreated(data);
        }
    }
    catch (...)
    {

    }

    EndUpdate();

    return bRet;
}

bool CWizDatabase::CreateDocumentAndInit(const WIZDOCUMENTDATA& sourceDoc, const QByteArray& baData,
                                         const QString& strLocation, const WIZTAGDATA& tag, WIZDOCUMENTDATA& newDoc)
{
    bool bRet = false;
    try
    {
        BeginUpdate();

        newDoc.strKbGUID = kbGUID();
        newDoc.strOwner = GetUserId();        

        bRet = CreateDocument(sourceDoc.strTitle, sourceDoc.strName, strLocation, "", sourceDoc.strAuthor,
                              sourceDoc.strKeywords, sourceDoc.strType, GetUserId(), sourceDoc.strFileType,
                              sourceDoc.strStyleGUID, 0, 0, 0, newDoc);
        if (bRet)
        {
            if (WriteDataToDocument(newDoc.strGUID, baData)) {
                SetObjectDataDownloaded(newDoc.strGUID, "document", true);

                CString strFileName = GetDocumentFileName(newDoc.strGUID);
                bRet = UpdateDocumentDataMD5(newDoc, strFileName);
            } else {
                bRet = false;
            }

            Q_EMIT documentCreated(newDoc);
        }
    }
    catch (...)
    {

    }

    EndUpdate();

    if (!tag.strGUID.IsEmpty()) {
        CWizDocument doc(*this, newDoc);
        doc.AddTag(tag);
    }

    return bRet;
}

bool CWizDatabase::CreateDocumentByTemplate(const QString& templateZiwFile, const QString& strLocation,
                                            const WIZTAGDATA& tag, WIZDOCUMENTDATA& newDoc)
{
    QByteArray ba;
    if (!LoadFileData(templateZiwFile, ba))
        return false;

    QString strTitle = Utils::Misc::extractFileTitle(templateZiwFile);
    if (newDoc.strTitle.isEmpty())
    {
        newDoc.strTitle = strTitle;
    }

    return CreateDocumentAndInit(newDoc, ba, strLocation, tag, newDoc);
}

bool CWizDatabase::AddAttachment(const WIZDOCUMENTDATA& document, const CString& strFileName,
                                 WIZDOCUMENTATTACHMENTDATA& dataRet)
{
    dataRet.strKbGUID = document.strKbGUID;
    dataRet.strDocumentGUID = document.strGUID;

    CString strMD5 = ::WizMd5FileString(strFileName);
    if (!CreateAttachment(document.strGUID, Utils::Misc::extractFileName(strFileName), strFileName, "", strMD5, dataRet))
        return false;

    if (!::WizCopyFile(strFileName, GetAttachmentFileName(dataRet.strGUID), false))
    {
        DeleteAttachment(dataRet, false, true);
        return false;
    }

    SetAttachmentDataDownloaded(dataRet.strGUID, true);
    UpdateDocumentAttachmentCount(document.strGUID);

    return true;
}

bool CWizDatabase::DeleteTagWithChildren(const WIZTAGDATA& data, bool bLog)
{
    CWizTagDataArray arrayChildTag;
    GetChildTags(data.strGUID, arrayChildTag);
    foreach (const WIZTAGDATA& childTag, arrayChildTag)
    {
        DeleteTagWithChildren(childTag, bLog);
    }

    DeleteTag(data, bLog);

    return true;
}

bool CWizDatabase::LoadDocumentData(const QString& strDocumentGUID, QByteArray& arrayData, bool forceLoadData)
{
    CString strFileName = GetDocumentFileName(strDocumentGUID);
    if (!PathFileExists(strFileName))
    {
        return false;
    }

    if (!forceLoadData) {
        WIZDOCUMENTDATA document;
        if (!DocumentFromGUID(strDocumentGUID, document))
            return false;

        if (document.nProtected) {
            if (userCipher().isEmpty())
                return false;

            if (!m_ziwReader->setFile(strFileName))
                return false;

            strFileName = Utils::PathResolve::tempPath() + document.strGUID + "-decrypted";
            QFile::remove(strFileName);

            if (!m_ziwReader->decryptDataToTempFile(strFileName)) {
                // force clear usercipher
                m_ziwReader->setUserCipher(QString());
                return false;
            }
        }
    }

    return LoadFileData(strFileName, arrayData);
}

bool CWizDatabase::LoadFileData(const QString& strFileName, QByteArray& arrayData)
{
    QFile file(strFileName);
    if (!file.open(QFile::ReadOnly))
        return false;

    arrayData = file.readAll();
    file.close();
    return !arrayData.isEmpty();
}

bool CWizDatabase::WriteDataToDocument(const QString& strDocumentGUID, const QByteArray& arrayData)
{
    CString strFileName = GetDocumentFileName(strDocumentGUID);

    QFile file(strFileName);
    if (!file.open(QFile::WriteOnly | QIODevice::Truncate))
        return false;

    return (file.write(arrayData) != -1);
}

bool CWizDatabase::LoadAttachmentData(const CString& strDocumentGUID, QByteArray& arrayData)
{
    CString strFileName = GetAttachmentFileName(strDocumentGUID);
    if (!PathFileExists(strFileName))
    {
        return false;
    }

    QFile file(strFileName);
    if (!file.open(QFile::ReadOnly))
        return false;

    arrayData = file.readAll();

    return !arrayData.isEmpty();
}

bool CWizDatabase::LoadCompressedAttachmentData(const QString& strGUID, QByteArray& arrayData)
{
    CString strFileName = GetAttachmentFileName(strGUID);
    if (!PathFileExists(strFileName))
    {
        return false;
    }
    CString strTempZipFileName = Utils::PathResolve::tempPath() + WizIntToStr(GetTickCount()) + ".tmp";
    CWizZipFile zip;
    if (!zip.open(strTempZipFileName))
    {
        Q_EMIT updateError("Failed to create temp zip file: " + strTempZipFileName);
        return false;
    }

    WIZDOCUMENTATTACHMENTDATA attach;
    AttachmentFromGUID(strGUID, attach);
    if (!zip.compressFile(strFileName, attach.strName))
    {
        Q_EMIT updateError("Failed to compress file: " + strFileName);
        return false;
    }

    zip.close();

    QFile file(strTempZipFileName);
    if (!file.open(QFile::ReadOnly))
        return false;

    arrayData = file.readAll();

    return !arrayData.isEmpty();
}

bool CWizDatabase::SaveCompressedAttachmentData(const CString& strGUID, const QByteArray& arrayData)
{
    CString strTempZipFileName = Utils::PathResolve::tempPath() + WizIntToStr(GetTickCount()) + ".tmp";
    if (!WizSaveDataToFile(strTempZipFileName, arrayData))
    {
        Q_EMIT updateError("Failed to save attachment data to temp file: " + strTempZipFileName);
        return false;
    }
    CWizUnzipFile zip;
    if (!zip.open(strTempZipFileName))
    {
        Q_EMIT updateError("Failed to open temp zip file: " + strTempZipFileName);
        return false;
    }

    CString strFileName = GetAttachmentFileName(strGUID);
    if (!zip.extractFile(0, strFileName))
    {
        Q_EMIT updateError("Failed to extract attachment file: " + strFileName);
        return false;
    }

    zip.close();

    return true;
}

bool CWizDatabase::UpdateDocumentAbstract(const QString& strDocumentGUID)
{
    CString strFileName = GetDocumentFileName(strDocumentGUID);
    if (!PathFileExists(strFileName)) {
        return false;
    }

    WIZDOCUMENTDATA data;
    if (!DocumentFromGUID(strDocumentGUID, data)) {
        qDebug() << "[updateDocumentAbstract]invalide guid: " << strDocumentGUID;
        return false;
    }

    if (data.nProtected) {
        //check if note abstract is empty
        WIZABSTRACT abstract;
        PadAbstractFromGUID(data.strGUID, abstract);
        if (abstract.image.isNull() && abstract.text.IsEmpty())
        {
            return false;
        }
        else
        {
            WIZABSTRACT emptyAbstract;
            emptyAbstract.guid = data.strGUID;
            bool ret = UpdatePadAbstract(emptyAbstract);
            if (!ret) {
                Q_EMIT updateError("Failed to update note abstract!");
            }
            Q_EMIT documentAbstractModified(data);

            return ret;
        }
    }

    QString strTempFolder = Utils::PathResolve::tempPath() + data.strGUID + "-update/";
    // delete folder to clear unused images.
    ::WizDeleteAllFilesInFolder(strTempFolder);
    if (!DocumentToHtmlFile(data, strTempFolder, "uindex.html")) {
        qDebug() << "[updateDocumentAbstract]decompress to temp failed, guid: "
                 << strDocumentGUID;
        return false;
    }
    CString strHtmlFileName = strTempFolder + "uindex.html";

    CString strHtmlTempPath = Utils::Misc::extractFilePath(strHtmlFileName);

    CString strHtml;
    CString strAbstractFileName = strHtmlTempPath + "wiz_full.html";
    if (PathFileExists(strAbstractFileName)) {
        ::WizLoadUnicodeTextFromFile(strAbstractFileName, strHtml);
    } else {
        ::WizLoadUnicodeTextFromFile(strHtmlFileName, strHtml);
    }

    WIZABSTRACT abstract;
    abstract.guid = strDocumentGUID;

    CWizHtmlToPlainText htmlConverter;
    htmlConverter.toText(strHtml, abstract.text);
    abstract.text = abstract.text.left(2000);

    CString strResourcePath = Utils::Misc::extractFilePath(strHtmlFileName) + "index_files/";
    CWizStdStringArray arrayImageFileName;
    ::WizEnumFiles(strResourcePath, "*.jpg;*.png;*.bmp;*.gif", arrayImageFileName, 0);
    if (!arrayImageFileName.empty())
    {
        CString strImageFileName;

        qint64 m = 0;
        CWizStdStringArray::const_iterator it;
        for (it = arrayImageFileName.begin(); it != arrayImageFileName.end(); it++) {
            CString strFileName = *it;
            qint64 size = Utils::Misc::getFileSize(strFileName);
            if (size > m)
            {
                //FIXME:此处是特殊处理，解析Html的CSS时候存在问题，目前暂不删除冗余图片。
                //缩略图需要判断当前图片确实被使用
                QString strName = Utils::Misc::extractFileName(strFileName);
                if (!strHtml.contains(strName))
                    continue;

                strImageFileName = strFileName;
                m = size;
            }
        }

        QImage img;
        if (!strImageFileName.IsEmpty() && img.load(strImageFileName))
        {
            //DEBUG_TOLOG2("Abstract image size: %1 X %2", WizIntToStr(img.width()), WizIntToStr(img.height()));
            if (img.width() > 32 && img.height() > 32)
            {
                abstract.image = img;
            }
        }
        else
        {
            Q_EMIT updateError("Failed to load image file: " + strImageFileName);
        }
    }

    bool ret = UpdatePadAbstract(abstract);
    if (!ret) {
        Q_EMIT updateError("Failed to update note abstract!");
    }

    ::WizDeleteFolder(strHtmlTempPath);

    Q_EMIT documentAbstractModified(data);

    return ret;
}

CString CWizDatabase::GetRootLocation(const CString& strLocation)
{
    //FIXME:容错处理，如果路径的结尾不是 '/'，则增加该结尾符号
    QString strLoc = strLocation;
    if (strLoc.right(1) != "/")
        strLoc.append('/');

    int index = strLoc.indexOf('/', 1);
    if (index == -1)
        return CString();

    return strLoc.left(index + 1);
}

CString CWizDatabase::GetLocationName(const CString& strLocation)
{
    //FIXME:容错处理，如果路径的结尾不是 '/'，则增加该结尾符号
    QString strLoc = strLocation;
    if (strLoc.right(1) != "/")
        strLoc.append('/');

    int index = strLoc.lastIndexOf('/', strLocation.length() - 2);
    if (index == -1)
        return CString();

    CString str = strLoc.right(strLocation.length() - index - 1);

    str.Trim('/');

    return str;
}

CString CWizDatabase::GetLocationDisplayName(const CString& strLocation)
{
    if (IsRootLocation(strLocation)) {
        if (strLocation.startsWith("/My ")) {
            if (strLocation == "/My Notes/") {
                return tr("My Notes");
            } else if (strLocation == "/My Journals/") {
                return tr("My Journals");
            } else if (strLocation == "/My Contacts/") {
                return tr("My Contacts");
            } else if (strLocation == "/My Events/") {
                return tr("My Events");
            } else if (strLocation == "/My Sticky Notes/") {
                return tr("My Sticky Notes");
            } else if (strLocation == "/My Emails/") {
                return tr("My Emails");
            } else if (strLocation == "/My Drafts/") {
                return tr("My Drafts");
            } else if (strLocation == "/My Tasks/") {
                return tr("My Tasks");
            }
        }
    } else if (strLocation == "/My Tasks/Inbox/") {
        return tr("Inbox");
    } else if (strLocation == "/My Tasks/Completed/") {
        return tr("Completed");
    }

    return GetLocationName(strLocation);
}

bool CWizDatabase::GetDocumentsByTag(const WIZTAGDATA& tag, CWizDocumentDataArray& arrayDocument)
{
    return CWizIndex::GetDocumentsByTag("", tag, arrayDocument, false);
}

bool CWizDatabase::loadUserCert()
{
    QString strN, stre, strEncryptedd, strHint;
    if (!GetUserCert(strN, stre, strEncryptedd, strHint)) {
        TOLOG("can't get user cert from db");
        return false;
    }

    m_ziwReader->setRSAKeys(strN.toUtf8(), stre.toUtf8(), strEncryptedd.toUtf8(), strHint);
    return true;
}

void CWizDatabase::CopyDocumentLink(const WIZDOCUMENTDATA& document)
{
    QString strHtml, strLink;
    DocumentToHtmlLink(document, strHtml, strLink);
    //
    QClipboard* clip = QApplication::clipboard();

    QMimeData* data = new QMimeData();
    data->setHtml(strHtml);
    data->setText(strLink);
    clip->setMimeData(data);
}

void CWizDatabase::CopyDocumentsLink(const QList<WIZDOCUMENTDATA>& documents)
{
    QString strHtml, strLink;
    DocumentsToHtmlLink(documents, strHtml, strLink);

    QMimeData* data = new QMimeData();
    data->setHtml(strHtml);
    data->setText(strLink);
    QClipboard* clip = QApplication::clipboard();
    clip->setMimeData(data);
}

QString CWizDatabase::DocumentToWizKMURL(const WIZDOCUMENTDATA& document)
{
    CWizDatabase* dbPrivate = getPersonalDatabase();
    //
    if (document.strKbGUID == dbPrivate->kbGUID())
    {
        return WizFormatString3(_T("wiz://open_document?guid=%1&kbguid=%2&private_kbguid=%3"), document.strGUID, "", dbPrivate->kbGUID());
    }
    else
    {
        return WizFormatString2(_T("wiz://open_document?guid=%1&kbguid=%2"), document.strGUID, document.strKbGUID);
    }
    return QString();
}

void CWizDatabase::DocumentToHtmlLink(const WIZDOCUMENTDATA& document, QString& strHtml, QString& strLink)
{
    strLink = DocumentToWizKMURL(document);
    QString strTitle = document.strTitle;
    strTitle.replace(_T("<"), _T("&lt;"));
    strTitle.replace(_T(">"), _T("&gt;"));
    strTitle.replace(_T("&"), _T("&amp;"));
    //
    strHtml = WizFormatString2(_T("<a href=\"%1\">%2</a>"), strLink, strTitle);
}

void CWizDatabase::DocumentsToHtmlLink(const QList<WIZDOCUMENTDATA>& documents, QString& strHtml, QString& strLink)
{
    for (int i = 0; i < documents.count(); i++)
    {
        QString strOneHtml, strOneLink;
        DocumentToHtmlLink(documents.at(i), strOneHtml, strOneLink);
        strHtml += strOneHtml + "<br>";
        strLink += strOneLink + "\n";
    }
}

bool CWizDatabase::DocumentToTempHtmlFile(const WIZDOCUMENTDATA& document,
                                          QString& strFullPathFileName, const QString& strTargetFileName)
{
    QString strTempFolder = Utils::PathResolve::tempPath() + document.strGUID + "/";
    ::WizEnsurePathExists(strTempFolder);

    if (!DocumentToHtmlFile(document, strTempFolder, strTargetFileName))
        return false;

    strFullPathFileName = strTempFolder + strTargetFileName;

    return PathFileExists(strFullPathFileName);
}

bool CWizDatabase::DocumentToHtmlFile(const WIZDOCUMENTDATA& document,
                                          const QString& strPath,
                                          const QString& strHtmlFileName)
{
    ::WizEnsurePathExists(strPath);

    if (!ExtractZiwFileToFolder(document, strPath))
        return false;

    QString strTempHtmlFileName = strPath + "index.html";

    m_mtxTempFile.lock();
    QString strText;
    ::WizLoadUnicodeTextFromFile(strTempHtmlFileName, strText);

    QUrl url = QUrl::fromLocalFile(strPath + "index_files/");
    strText.replace("index_files/", url.toString());

    strTempHtmlFileName = strPath + strHtmlFileName;
    WizSaveUnicodeTextToUtf8File(strTempHtmlFileName, strText);
    m_mtxTempFile.unlock();

    return PathFileExists(strTempHtmlFileName);
}

bool CWizDatabase::ExportToHtmlFile(const WIZDOCUMENTDATA& document, const QString& strPath)
{
    QString strTempPath = Utils::PathResolve::tempPath() + WizGenGUIDLowerCaseLetterOnly() + "/";
    if (!ExtractZiwFileToFolder(document, strTempPath))
        return false;

    QString strText;
    QString strTempHtmlFileName = strTempPath + "index.html";
    if (!WizLoadUnicodeTextFromFile(strTempHtmlFileName, strText))
        return false;

#if QT_VERSION < 0x050000
    QString strResFolder = document.strTitle + "_files/";
#else
    QString strResFolder = document.strTitle.toHtmlEscaped() + "_files/";
#endif
    strText.replace("index_files/", strResFolder);

    QString strIndexFile = strPath + document.strTitle + ".html";
    if (!WizSaveUnicodeTextToUtf8File(strIndexFile, strText))
        return false;

    bool bCoverIfExists = true;
    if (!WizCopyFolder(strTempPath + "index_files/", strPath + strResFolder, bCoverIfExists))
        return false;

    return true;
}

bool CWizDatabase::ExtractZiwFileToFolder(const WIZDOCUMENTDATA& document,
                                              const QString& strFolder)
{
    CString strZipFileName = GetDocumentFileName(document.strGUID);
    if (!PathFileExists(strZipFileName)) {
        return false;
    }

    if (document.nProtected) {
        if (userCipher().isEmpty()) {
            return false;
        }

        if (!m_ziwReader->setFile(strZipFileName)) {
            return false;
        }

        strZipFileName = Utils::PathResolve::tempPath() + document.strGUID + "-decrypted";
        QFile::remove(strZipFileName);

        if (!m_ziwReader->decryptDataToTempFile(strZipFileName)) {
            // force clear usercipher
            m_ziwReader->setUserCipher(QString());
            return false;
        }
    }

    return CWizUnzipFile::extractZip(strZipFileName, strFolder);
}

bool CWizDatabase::EncryptDocument(WIZDOCUMENTDATA& document)
{
    if (document.nProtected || document.strKbGUID != kbGUID())
        return false;

    //
    QString strFolder = Utils::PathResolve::tempDocumentFolder(document.strGUID);
    if (!ExtractZiwFileToFolder(document, strFolder))
    {
        TOLOG("extract ziw file failed!");
        return false;
    }

    //
    if (!initZiwReaderForEncryption())
        return false;

    //
    document.nProtected = 1;
    QString strFileName = GetDocumentFileName(document.strGUID);
    if (CompressFolderToZiwFile(document, strFolder, strFileName))
    {
        emit documentDataModified(document);
        return true;
    }

    return false;
}

bool CWizDatabase::CompressFolderToZiwFile(WIZDOCUMENTDATA &document, \
                                           const QString& strFileFoler)
{
    QString strFileName = GetDocumentFileName(document.strGUID);
    return CompressFolderToZiwFile(document, strFileFoler, strFileName);
}

bool CWizDatabase::CompressFolderToZiwFile(WIZDOCUMENTDATA& document, const QString& strFileFoler,
                                          const QString& strZiwFileName)
{
    QFile::remove(strZiwFileName);

    CWizDocument doc(*this, document);
    QString strMetaText = doc.GetMetaText();

    //
    if (!document.nProtected)
    {
        bool bZip = ::WizFolder2Zip(strFileFoler, strMetaText, strZiwFileName);
        if (!bZip)
            return false;
    }
    else
    {
        CString strTempFile = Utils::PathResolve::tempPath() + document.strGUID + "-decrypted";
        bool bZip = ::WizFolder2Zip(strFileFoler, strMetaText, strTempFile);
        if (!bZip)
            return false;

        if (!m_ziwReader->encryptDataToTempFile(strTempFile, strZiwFileName))
            return false;
    }

    SetObjectDataDownloaded(document.strGUID, "document", true);

    /*不需要将笔记modified信息通知关联内容.此前页面显示已是最新,不需要relaod.如果relaod较大笔记
    可能会造成页面闪烁*/
    bool notify = false;
    return UpdateDocumentDataMD5(document, strZiwFileName, notify);
}

bool CWizDatabase::CancelDocumentEncryption(WIZDOCUMENTDATA& document, const QString& strUserCipher)
{
    if (!document.nProtected || kbGUID() != document.strKbGUID)
        return false;

    //
    if (!initZiwReaderForEncryption(strUserCipher))
        return false;

    m_ziwReader->setUserCipher(strUserCipher);
    //
    QString strFolder = Utils::PathResolve::tempDocumentFolder(document.strGUID);
    if (!ExtractZiwFileToFolder(document, strFolder))
    {
        TOLOG("extract ziw file failed!");
        return false;
    }

    //
    document.nProtected = 0;
    QString strFileName = GetDocumentFileName(document.strGUID);
    if (CompressFolderToZiwFile(document, strFolder, strFileName))
    {
        emit documentDataModified(document);
        return true;
    }

    return false;
}

bool CWizDatabase::IsFileAccessible(const WIZDOCUMENTDATA& document)
{
    CString strZipFileName = GetDocumentFileName(document.strGUID);
    if (!PathFileExists(strZipFileName)) {
        return false;
    }

    if (document.nProtected) {
        if (userCipher().isEmpty()) {
            return false;
        }

        if (!m_ziwReader->isFileAccessible(strZipFileName)) {
            return false;
        }
    }

    return true;
}

bool CWizDatabase::checkUserCertExists()
{
    QString strN, stre, strEncryptedd, strHint;
    if (GetUserCert(strN, stre, strEncryptedd, strHint))
    {
        if ((!strN.isEmpty()) && (!stre.isEmpty()) && (!strEncryptedd.isEmpty()))
        {
            return true;
        }
    }

    return false;
}

QObject* CWizDatabase::GetFolderByLocation(const QString& strLocation, bool create)
{
    Q_UNUSED(create);

    return new CWizFolder(*this, strLocation);
}

void CWizDatabase::onAttachmentModified(const QString strKbGUID, const QString& strGUID,
                                        const QString& strFileName, const QString& strMD5, const QDateTime& dtLastModified)
{
    Q_UNUSED(strFileName);

    CWizDatabase& db = CWizDatabaseManager::instance()->db(strKbGUID);
    WIZDOCUMENTATTACHMENTDATA attach;
    if (db.AttachmentFromGUID(strGUID, attach))
    {
        if (strMD5 != attach.strDataMD5)
        {
            attach.strDataMD5 = strMD5;
            attach.nVersion = -1;
            TOLOG("[Edit] attachment data modified");
        }
        attach.tDataModified = dtLastModified;
        attach.tInfoModified = WizGetCurrentTime();
        attach.strInfoMD5 = CalDocumentAttachmentInfoMD5(attach);

        db.ModifyAttachmentInfoEx(attach);
    }
}

bool CWizDatabase::tryAccessDocument(const WIZDOCUMENTDATA &doc)
{
    if (doc.nProtected) {
        if (userCipher().isEmpty()) {
            if (!loadUserCert())
                return false;

            QString strPassWord;
            CWizLineInputDialog dlg(tr("Doucment  %1  Password").arg(doc.strTitle),
                                    tr("Password :"), "", 0, QLineEdit::Password);

            if (dlg.exec() == QDialog::Rejected)
                return false;

            strPassWord = dlg.input();
            setUserCipher(strPassWord);
        }

        if (!IsFileAccessible(doc)) {
            QMessageBox::information(0, tr("Info"), tr("password error!"));
            setUserCipher(QString());
            return false;
        }
    }
    return true;
}


QObject* CWizDatabase::GetDeletedItemsFolder()
{
    return new CWizFolder(*this, GetDeletedItemsLocation());
}

//QObject* CWizDatabase::DocumentFromGUID(const QString& strGUID)
//{
//    WIZDOCUMENTDATA data;
//    if (!DocumentFromGUID(strGUID, data))
//        return NULL;

//    CWizDocument* pDoc = new CWizDocument(*this, data);
//    return pDoc;
//}
