#include "wizDatabase.h"

#include <QDir>
#include <QUrl>
#include <QDebug>
#include <QTextCodec>
#include <algorithm>
#include <QSettings>

#include <extensionsystem/pluginmanager.h>

#include "wizhtml2zip.h"
#include "share/wizzip.h"
#include "html/wizhtmlcollector.h"
#include "rapidjson/document.h"

#include "utils/pathresolve.h"
#include "utils/logger.h"

#define WIZNOTE_THUMB_VERSION "3"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class CWizDocument


QString GetResoucePathFromFile(const QString& strHtmlFileName)
{
    if (!QFile::exists(strHtmlFileName))
        return NULL;

    QString strTitle = WizExtractFileTitle(strHtmlFileName);
    QString strPath = ::WizExtractFilePath(strHtmlFileName);
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

bool CWizDocument::UpdateDocument4(const QString& strHtml, const QString& strURL, int nFlags)
{
    return m_db.UpdateDocumentData(m_data, strHtml, strURL, nFlags);
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

        m_db.DeleteAttachment(*it, true);
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

    MoveDocument(folder);
}

bool CWizDocument::MoveDocument(CWizFolder* pFolder)
{
    if (!pFolder)
        return false;

    QString strNewLocation = pFolder->Location();
    QString strOldLocation = m_data.strLocation;

    if (strNewLocation == strOldLocation)
        return true;

    m_data.strLocation = strNewLocation;
    if (!m_db.ModifyDocumentInfo(m_data))
    {
        m_data.strLocation = strOldLocation;
        TOLOG1(_T("Failed to modify document location %1."), m_data.strLocation);
        return false;
    }

    return true;
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
        return PermanentlyDelete();
    } else {
        return MoveTo(m_db.GetDeletedItemsFolder());
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

    if (IsInDeletedItems()) {
        // FIXME: should use CWizDocument to delete document data, attachments.
        if (!m_db.DeleteDocumentsByLocation(Location())) {
            TOLOG1("Failed to delete documents by location; %1", Location());
            return;
        }

        m_db.DeleteExtraFolder(Location());
        m_db.SetLocalValueVersion("folders", -1);
    } else {
        CWizFolder deletedItems(m_db, LOCATION_DELETED_ITEMS + Location().right(Location().size() - 1));
        MoveTo(&deletedItems);
    }
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

        if (!m_db.ModifyDocumentInfo(data)) {
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
        if (strFolder.startsWith(strOldLocation)) {
            strFolder.remove(0, strOldLocation.length());
            strFolder.insert(0, strDestLocation);
            m_db.AddExtraFolder(strFolder);
        }
    }

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
    qDebug() << "set object: " << strObjectName << " version: " << nVersion;

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
    return GetModifiedDocuments(arrayData);
}

bool CWizDatabase::GetModifiedAttachmentList(CWizDocumentAttachmentDataArray& arrayData)
{
    return GetModifiedAttachments(arrayData);
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

bool CWizDatabase::DocumentFromGUID(const QString& strGUID,
                                    WIZDOCUMENTDATA& dataExists)
{
    return CWizIndex::DocumentFromGUID(strGUID, dataExists);
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

    CWizDatabase* db = new CWizDatabase();
    if (!db->Open(m_strUserId, group.strGroupGUID)) {
        delete db;
        return NULL;
    }

    // pass this pointer to database manager for signal redirect and managament
    // CWizDatabaseManager will take ownership
    Q_EMIT databaseOpened(db, group.strGroupGUID);

    return db;
}

void CWizDatabase::CloseGroupDatabase(IWizSyncableDatabase* pDatabase)
{
    Q_UNUSED(pDatabase);
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

    Q_EMIT userInfoChanged();
}

bool CWizDatabase::IsGroup()
{
    if (m_bIsPersonal)
        return false;

    return true;
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
    }
    else
    {
        arrayKey.push_back("folders");
        arrayKey.push_back("folders_pos");
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
        return "";
        //return GetFoldersPos();
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

void CWizDatabase::ClearError()
{
    // FIXME
}

void CWizDatabase::OnTrafficLimit(const QString& strErrorMessage)
{
    // FIXME
    Q_UNUSED(strErrorMessage);
}

void CWizDatabase::OnStorageLimit(const QString& strErrorMessage)
{
    // FIXME
    Q_UNUSED(strErrorMessage);
}

bool CWizDatabase::IsTrafficLimit()
{
    // FIXME
    return false;
}

bool CWizDatabase::IsStorageLimit()
{
    // FIXME
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

        QSettings* setting = ExtensionSystem::PluginManager::settings();
        int nPosOld = setting->value("FolderPosition/" + strLocation).toInt();
        if (nPosOld != nPos) {
            setting->setValue("FolderPosition/" + strLocation, nPos);
            bPositionChanged = true;
        }
    }

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
    GetAllLocations(arrayLocation);

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
            if (GetDocumentsSizeByLocation(strLocation, nSize, true)) {
                if (nSize == 0) {
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

bool CWizDatabase::GetGroupData(const QString& groupGUID, WIZGROUPDATA& group)
{
    group.strGroupGUID = groupGUID;
    group.strGroupName = GetMetaDef(g_strGroupSection, groupGUID);

    group.bizGUID = GetMetaDef(g_strGroupSection, groupGUID + "_BizGUID");
    group.bizGUID = GetMetaDef(g_strGroupSection, groupGUID + "_BizName");
    group.bOwn = GetMetaDef(g_strGroupSection, groupGUID + "_Own") == "1";
    group.nUserGroup = GetMetaDef(g_strGroupSection, groupGUID + "_Role", QString::number(WIZ_USERGROUP_MAX)).toInt();
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
        GetGroupData(group.strGroupGUID, group);
        //
        arrayGroup.push_back(group);
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


bool CWizDatabase::Open(const QString& strUserId, const QString& strKbGUID /* = NULL */)
{
    Q_ASSERT(!strUserId.isEmpty());

    m_strUserId = strUserId;

    if (strKbGUID.isEmpty()) {
        m_bIsPersonal = true;
    } else {
        m_bIsPersonal = false;
        setKbGUID(strKbGUID);
    }

    if (!CWizIndex::Open(GetIndexFileName())) {
        return false;
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
        return false;
    }

    setThumbIndexVersion(WIZNOTE_THUMB_VERSION);

    LoadDatabaseInfo();

    return true;
}

bool CWizDatabase::LoadDatabaseInfo()
{
    if (!kbGUID().isEmpty()) {
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
    Q_ASSERT(!m_strUserId.isEmpty());

    QString strPath = ::WizGetDataStorePath()+ m_strUserId + "/";
    WizEnsurePathExists(strPath);

    return strPath;
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

QString CWizDatabase::GetAttachmentFileName(const QString& strGUID) const
{
    return GetAttachmentsDataPath() + "{" + strGUID + "}";
}

QString CWizDatabase::GetAvatarPath() const
{
    QString strPath = GetAccountPath() + "avatar/";
    WizEnsurePathExists(strPath);

    return strPath;
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

    return true;
}

bool CWizDatabase::updateBizUser(const WIZBIZUSER& user)
{
    bool bRet = false;

    WIZBIZUSER userTemp;
    if (userFromGUID(user.bizGUID, user.userGUID, userTemp)) {
        // only modify user when alias changed
        //if (userTemp.alias != user.alias) {
        bRet = modifyUserEx(user);
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

    return !bHasError;
}

bool CWizDatabase::UpdateDocumentData(WIZDOCUMENTDATA& data,
                                      const QString& strHtml,
                                      const QString& strURL,
                                      int nFlags)
{
    QString strProcessedHtml(strHtml);
    QString strResourcePath = GetResoucePathFromFile(strURL);
    if (!strResourcePath.isEmpty()) {
        QUrl urlResource = QUrl::fromLocalFile(strResourcePath);
        strProcessedHtml.replace(urlResource.toString(), "index_files/");
    }

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

    return UpdateDocumentDataMD5(data, strZipFileName);
}

bool CWizDatabase::UpdateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName)
{
    bool bRet = CWizIndex::UpdateDocumentDataMD5(data, strZipFileName);

    UpdateDocumentAbstract(data.strGUID);

    return bRet;
}

bool CWizDatabase::DeleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data,
                                    bool bLog,
                                    bool bReset /* = true */)
{
    bool bRet = CWizIndex::DeleteAttachment(data, bLog, bReset);
    CString strFileName = GetAttachmentFileName(data.strGUID);
    if (PathFileExists(strFileName)) {
        ::WizDeleteFile(strFileName);
    }

    return bRet;
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

            if (arrayData[i].eObjectType == wizobjectDocumentAttachment) {
                arrayData.erase(arrayData.begin() + i);
                continue;
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
            qDebug() << "\n[Fatal] update object data failed, can't find database record!\n";
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
        data.strOwner = getUserId();
        bRet = CreateDocument(strTitle, strName, strLocation, strHtmlUrl, data);
        if (bRet)
        {
            bRet = UpdateDocumentData(data, strHtml, strURL, nFlags);

            Q_EMIT documentCreated(data);
        }
    }
    catch (...)
    {

    }

    EndUpdate();

    return bRet;
}

bool CWizDatabase::AddAttachment(const WIZDOCUMENTDATA& document, const CString& strFileName, WIZDOCUMENTATTACHMENTDATA& dataRet)
{
    CString strMD5 = ::WizMd5FileString(strFileName);
    if (!CreateAttachment(document.strGUID, WizExtractFileName(strFileName), strFileName, "", strMD5, dataRet))
        return false;

    if (!::WizCopyFile(strFileName, GetAttachmentFileName(dataRet.strGUID), false))
    {
        DeleteAttachment(dataRet, false);
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

bool CWizDatabase::LoadDocumentData(const QString& strDocumentGUID, QByteArray& arrayData)
{
    CString strFileName = GetDocumentFileName(strDocumentGUID);
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

bool CWizDatabase::LoadCompressedAttachmentData(const QString& strDocumentGUID, QByteArray& arrayData)
{
    CString strFileName = GetAttachmentFileName(strDocumentGUID);
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

    if (!zip.compressFile(strFileName, "data"))
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

bool CWizDatabase::SaveCompressedAttachmentData(const CString& strDocumentGUID, const QByteArray& arrayData)
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

    CString strFileName = GetAttachmentFileName(strDocumentGUID);
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
        return false;
    }

    CString strHtmlFileName;
    if (!DocumentToTempHtmlFile(data, strHtmlFileName)) {
        qDebug() << "[updateDocumentAbstract]decompress to temp failed, guid: "
                 << strDocumentGUID;
        return false;
    }

    CString strHtmlTempPath = WizExtractFilePath(strHtmlFileName);

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

    CString strResourcePath = WizExtractFilePath(strHtmlFileName) + "index_files/";
    CWizStdStringArray arrayImageFileName;
    ::WizEnumFiles(strResourcePath, "*.jpg;*.png;*.bmp;*.gif", arrayImageFileName, 0);
    if (!arrayImageFileName.empty())
    {
        CString strImageFileName = arrayImageFileName[0];

        qint64 m = 0;
        CWizStdStringArray::const_iterator it;
        for (it = arrayImageFileName.begin() + 1; it != arrayImageFileName.end(); it++) {
            CString strFileName = *it;
            qint64 size = ::WizGetFileSize(strFileName);
            if (size > m)
            {
                strImageFileName = strFileName;
                m = size;
            }
        }

        QImage img;
        if (img.load(strImageFileName))
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
    int index = strLocation.indexOf('/', 1);
    if (index == -1)
        return CString();

    return strLocation.left(index + 1);
}

CString CWizDatabase::GetLocationName(const CString& strLocation)
{
    int index = strLocation.lastIndexOf('/', strLocation.length() - 2);
    if (index == -1)
        return CString();

    CString str = strLocation.right(strLocation.length() - index - 1);

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

bool CWizDatabase::DocumentToTempHtmlFile(const WIZDOCUMENTDATA& document, QString& strTempHtmlFileName)
{
    CString strZipFileName = GetDocumentFileName(document.strGUID);
    if (!PathFileExists(strZipFileName)) {
        return false;
    }

    CString strTempPath = Utils::PathResolve::tempPath() + document.strGUID + "/";
    ::WizEnsurePathExists(strTempPath);

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

    CWizUnzipFile::extractZip(strZipFileName, strTempPath);

    strTempHtmlFileName = strTempPath + "index.html";

    QString strText;
    ::WizLoadUnicodeTextFromFile(strTempHtmlFileName, strText);

    QUrl url = QUrl::fromLocalFile(strTempPath + "index_files/");
    strText.replace("index_files/", url.toString());

    WizSaveUnicodeTextToUtf8File(strTempHtmlFileName, strText);

    return PathFileExists(strTempHtmlFileName);
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

QObject* CWizDatabase::GetFolderByLocation(const QString& strLocation, bool create)
{
    Q_UNUSED(create);

    return new CWizFolder(*this, strLocation);
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
