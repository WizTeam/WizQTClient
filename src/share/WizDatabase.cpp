#include "WizDatabase.h"

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

#include "WizHtml2Zip.h"
#include "share/WizZip.h"
#include "share/WizGlobal.h"

#include "sync/WizToken.h"

#include "html/WizHtmlCollector.h"
#include "share/jsoncpp/json/json.h"

#include "utils/WizPathResolve.h"
#include "utils/WizMisc.h"
#include "utils/WizLogger.h"
#include "sync/WizAvatarHost.h"
#include "WizObjectDataDownloader.h"
#include "WizProgressDialog.h"
#include "WizUserCipherForm.h"
#include "WizDatabaseManager.h"
#include "WizLineInputDialog.h"
#include "WizInitBizCertDialog.h"
#include "WizEnc.h"
#include "widgets/WizExecutingActionDialog.h"
#include "sync/WizKMServer.h"
#include "sync/WizApiEntry.h"
#include "share/WizThreads.h"
#include "share/WizMisc.h"
#include "WizMessageBox.h"

#define WIZNOTE_THUMB_VERSION "3"

#define WIZKMSYNC_EXIT_OK		0
#define WIZKMSYNC_EXIT_TRAFFIC_LIMIT		304
#define WIZKMSYNC_EXIT_STORAGE_LIMIT		305
#define WIZKMSYNC_EXIT_NOTE_COUNT_LIMIT		3032
#define WIZKMSYNC_EXIT_BIZ_SERVICE_EXPR		380

#define WIZKMSYNC_EXIT_INFO     "WIZKMSYNC_EXIT_INFO"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class CWizDocument

QString WizDatabase::m_strUserId = QString();

QString GetResoucePathFromFile(const QString& strHtmlFileName)
{
    if (!QFile::exists(strHtmlFileName))
        return QString();

    QString strTitle = Utils::WizMisc::extractFileTitle(strHtmlFileName);
    QString strPath = Utils::WizMisc::extractFilePath(strHtmlFileName);
    QString strPath1 = strPath + strTitle + "_files/";
    QString strPath2 = strPath + strTitle + ".files/";
    if (QFile::exists(strPath1))
        return strPath1;
    if (QFile::exists(strPath2))
        return strPath2;

    return QString();
}


class WizUserCertPassword
{
private:
    WizUserCertPassword() : m_mutex(QMutex::Recursive){}
public:
    static WizUserCertPassword& Instance()
    {
        static WizUserCertPassword passwords;
        return passwords;
    }
private:
    std::map<QString, QString> m_passwords;
    QMutex m_mutex;
public:
    void setPassword(const QString& strBizGUID, const QString& strPassword)
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);
        //
        m_passwords[strBizGUID] = strPassword;
    }
    QString getPassword(const QString& strBizGUID)
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);
        //
        return m_passwords[strBizGUID];
    }
    void clear()
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);
        //
        m_passwords.clear();
    }
};


WizDocument::WizDocument(WizDatabase& db, const WIZDOCUMENTDATA& data)
    : m_db(db)
    , m_data(data)
{
}

void WizDocument::makeSureObjectDataExists()
{
    ::WizMakeSureDocumentExistAndBlockWidthDialog(m_db, m_data);

    CWizDocumentAttachmentDataArray arrayAttach;
    if (m_db.getDocumentAttachments(m_data.strGUID, arrayAttach))
    {
        for (WIZDOCUMENTATTACHMENTDATAEX attach : arrayAttach)
        {
            ::WizMakeSureAttachmentExistAndBlockWidthDialog(m_db, attach);
        }
    }
}

bool WizDocument::UpdateDocument4(const QString& strHtml, const QString& strURL, int nFlags)
{
    return m_db.updateDocumentData(m_data, strHtml, strURL, nFlags);
}

void WizDocument::deleteToTrash()
{
    // move document to trash

//    int nVersion = m_data.nVersion;
    moveTo(m_db.GetDeletedItemsFolder());
    m_db.setDocumentVersion(m_data.strGUID, 0);

    // delete document from server
    CWizDocumentAttachmentDataArray arrayAttachment;
    m_db.getDocumentAttachments(m_data.strGUID, arrayAttachment);
    CWizDocumentAttachmentDataArray::const_iterator it;
    for (it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        m_db.logDeletedGuid(it->strGUID, wizobjectDocumentAttachment);
    }

    m_db.logDeletedGuid(m_data.strGUID, wizobjectDocument);
}

void WizDocument::deleteFromTrash()
{
    CWizDocumentAttachmentDataArray arrayAttachment;
    m_db.getDocumentAttachments(m_data.strGUID, arrayAttachment);

    CWizDocumentAttachmentDataArray::const_iterator it;
    for (it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        CString strFileName = m_db.getAttachmentFileName(it->strGUID);
        ::WizDeleteFile(strFileName);

        m_db.deleteAttachment(*it, true, true);
    }

    if (!m_db.deleteDocument(m_data, true)) {
        TOLOG1("Failed to delete document: %1", m_data.strTitle);
        return;
    }

    CString strZipFileName = m_db.getDocumentFileName(m_data.strGUID);
    if (WizPathFileExists(strZipFileName))
    {
        WizDeleteFile(strZipFileName);
    }
}

bool WizDocument::isInDeletedItemsFolder()
{
    QString strDeletedItemsFolderLocation = m_db.getDeletedItemsLocation();

    return m_data.strLocation.startsWith(strDeletedItemsFolderLocation);
}

void WizDocument::PermanentlyDelete()
{
    CWizDocumentAttachmentDataArray arrayAttachment;
    m_db.getDocumentAttachments(m_data.strGUID, arrayAttachment);

    CWizDocumentAttachmentDataArray::const_iterator it;
    for (it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        CString strFileName = m_db.getAttachmentFileName(it->strGUID);
        ::WizDeleteFile(strFileName);

        m_db.deleteAttachment(*it, true, true);
    }

    if (!m_db.deleteDocument(m_data, true)) {
        TOLOG1("Failed to delete document: %1", m_data.strTitle);
        return;
    }

    CString strZipFileName = m_db.getDocumentFileName(m_data.strGUID);
    if (WizPathFileExists(strZipFileName))
    {
        WizDeleteFile(strZipFileName);
    }
}

void WizDocument::moveTo(QObject* p)
{
    WizFolder* folder = dynamic_cast<WizFolder*>(p);
    if (!folder)
        return;

    moveTo(folder);
}

bool WizDocument::moveTo(WizFolder* pFolder)
{
    if (!pFolder)
        return false;

    QString strNewLocation = pFolder->location();
    QString strOldLocation = m_data.strLocation;

    if (strNewLocation == strOldLocation)
        return true;

    m_data.strLocation = strNewLocation;
    if (!m_db.modifyDocumentLocation(m_data))
    {
        m_data.strLocation = strOldLocation;
        TOLOG1("Failed to modify document location %1.", m_data.strLocation);
        return false;
    }

    return true;
}

bool WizDocument::moveTo(WizDatabase& targetDB, WizFolder* pFolder)
{
    qDebug() << "wizdocmove  to : " << pFolder->location();
    if (targetDB.kbGUID() == m_db.kbGUID())
        return moveTo(pFolder);

    QString newDocGUID;
    if (!copyTo(targetDB, pFolder, true, true, newDocGUID))
    {
        TOLOG1("Failed to copy document %1. Stop move", m_data.strTitle);
        return false;
    }

    Delete();
    return true;
}

bool WizDocument::moveTo(WizDatabase& targetDB, const WIZTAGDATA& targetTag)
{
    qDebug() << "wizdoc move to : " << targetTag.strName;
    if (targetDB.kbGUID() == m_db.kbGUID())
    {
        if (m_data.strLocation == LOCATION_DELETED_ITEMS)
        {
            WizFolder folder(m_db, m_db.getDefaultNoteLocation());
            moveTo(&folder);
        }
        //
        CWizStdStringArray tags;
        tags.push_back(targetTag.strGUID);
        //
        bool ret = m_db.setDocumentTags2(m_data, tags, true);
        return ret;
    }

    //
    if (!copyTo(targetDB, targetTag, true))
    {
        TOLOG1("Failed to copy document %1. Stop move", m_data.strTitle);
        return false;
    }

    qDebug() << " after copy doc delete this doc";
    Delete();
    return true;
}

bool WizDocument::copyTo(WizDatabase& targetDB, WizFolder* pFolder, bool keepDocTime,
                          bool keepDocTag, QString& newDocGUID)
{
    qDebug() << "wizdocu copy to : " << pFolder->location();
    QString strLocation = pFolder->location();
    WIZTAGDATA tagEmpty;
    if (!copyDocumentTo(m_data.strGUID, targetDB, strLocation, tagEmpty, newDocGUID, keepDocTime))
    {
        TOLOG1("Failed to copy document %1.", m_data.strTitle);
        return false;
    }

    if (keepDocTag && !m_db.isGroup() && m_db.kbGUID() == targetDB.kbGUID())
    {
        WIZDOCUMENTDATA newDoc;
        if (!targetDB.documentFromGuid(newDocGUID, newDoc))
            return false;

        CWizStdStringArray arrayTag;
        if (m_db.getDocumentTags(m_data.strGUID, arrayTag))
        {
            for (CString tagGUID : arrayTag)
            {
                targetDB.insertDocumentTag(newDoc, tagGUID);
            }
        }
    }
    return true;
}

bool WizDocument::copyTo(WizDatabase& targetDB, const WIZTAGDATA& targetTag,
                          bool keepDocTime)
{
    qDebug() << "wizdocu copy to : " << targetTag.strName;
    QString strLocation = targetDB.getDefaultNoteLocation();
    QString strNewDocGUID;
    return copyDocumentTo(m_data.strGUID, targetDB, strLocation, targetTag, strNewDocGUID, keepDocTime);
}

bool WizDocument::addTag(const WIZTAGDATA& dataTag)
{
    CWizStdStringArray arrayTag;
    m_db.getDocumentTags(m_data.strGUID, arrayTag);

    if (-1 != WizFindInArray(arrayTag, dataTag.strGUID))
        return true;

    if (!m_db.insertDocumentTag(m_data, dataTag.strGUID)) {
        TOLOG1("Failed to insert document tag: %1", m_data.strTitle);
        return false;
    }

    return true;
}

bool WizDocument::removeTag(const WIZTAGDATA& dataTag)
{
    CWizStdStringArray arrayTag;
    m_db.getDocumentTags(m_data.strGUID, arrayTag);

    if (-1 == WizFindInArray(arrayTag, dataTag.strGUID))
        return true;

    if (!m_db.deleteDocumentTag(m_data, dataTag.strGUID)) {
        TOLOG1("Failed to delete document tag: %1", m_data.strTitle);
        return false;
    }

    return true;
}

void WizDocument::Delete()
{
    return deleteFromTrash();
}


bool WizDocument::copyDocumentTo(const QString& sourceGUID, WizDatabase& targetDB,
                                  const QString& strTargetLocation, const WIZTAGDATA& targetTag, QString& resultGUID, bool keepDocTime)
{
    TOLOG("Copy document");
    WIZDOCUMENTDATA sourceDoc;
    if (!m_db.documentFromGuid(sourceGUID, sourceDoc))
        return false;

    if (!WizMakeSureDocumentExistAndBlockWidthEventloop(m_db, sourceDoc))
        return false;

    if (!m_db.tryAccessDocument(sourceDoc))
        return false;

    QByteArray ba;
    if (!m_db.loadDocumentDecryptedData(sourceDoc.strGUID, ba))
        return false;

    WIZDOCUMENTDATA newDoc;
    if (!targetDB.createDocumentAndInit(sourceDoc, ba, strTargetLocation, targetTag, newDoc))
    {
        TOLOG("Failed to new document!");
        return false;
    }

    resultGUID = newDoc.strGUID;

    if (!copyDocumentAttachment(sourceDoc, targetDB, newDoc))
        return false;

    if (keepDocTime)
    {
        newDoc.tCreated = sourceDoc.tCreated;
        newDoc.tAccessed = sourceDoc.tAccessed;
        newDoc.tDataModified = sourceDoc.tDataModified;
        newDoc.tModified = sourceDoc.tModified;
    }
    newDoc.nAttachmentCount = targetDB.getDocumentAttachmentCount(newDoc.strGUID);
    targetDB.modifyDocumentInfoEx(newDoc);

    return true;
}

bool WizDocument::copyDocumentAttachment(const WIZDOCUMENTDATA& sourceDoc,
                                          WizDatabase& targetDB, WIZDOCUMENTDATA& targetDoc)
{
    CWizDocumentAttachmentDataArray arrayAttachment;
    m_db.getDocumentAttachments(sourceDoc.strGUID, arrayAttachment);

    for (CWizDocumentAttachmentDataArray::const_iterator it = arrayAttachment.begin();
         it != arrayAttachment.end(); it++)
    {
        WIZDOCUMENTATTACHMENTDATAEX attachData(*it);
        if (!WizMakeSureAttachmentExistAndBlockWidthEventloop(m_db, attachData))
            continue;

        WIZDOCUMENTATTACHMENTDATAEX newAttach;
        QString strNewAttachFileName;

        CString targetAttachPath = targetDB.getAttachmentsDataPath();
        CString strTempFileName = targetAttachPath + attachData.strName;
        ::WizGetNextFileName(strTempFileName);
        if (!::WizCopyFile(m_db.getAttachmentFileName(attachData.strGUID), strTempFileName, FALSE))
            return false;

        newAttach = attachData;
        newAttach.strGUID = QString();
        newAttach.strURL = strTempFileName;
        strNewAttachFileName = strTempFileName;

//        if (!m_db.CopyDocumentAttachment(attachData, targetDB, newAttach, strNewAttachFileName))
//            continue;

        newAttach.strKbGUID = targetDoc.strKbGUID;
        newAttach.nVersion = -1;
        targetDB.addAttachment(targetDoc, strNewAttachFileName, newAttach);
    }

    return true;
}


/*
* Class CWizFolder
*/

WizFolder::WizFolder(WizDatabase& db, const QString& strLocation)
    : m_db(db)
    , m_strLocation(strLocation)
{
    Q_ASSERT(strLocation.right(1) == "/");
    Q_ASSERT(strLocation.left(1) == "/");
}

bool WizFolder::isDeletedItems() const
{
    return location() == LOCATION_DELETED_ITEMS;
}

bool WizFolder::isInDeletedItems() const
{
    return !isDeletedItems() && location().startsWith(LOCATION_DELETED_ITEMS);
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

void WizFolder::Delete()
{
    if (isDeletedItems())
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
    m_db.getDocumentsByLocation(location(), arrayDocument, true);
    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++)
    {
        WIZDOCUMENTDATAEX doc = *it;
        WizDocument document(m_db, doc);
        document.deleteToTrash();
    }

    m_db.deleteExtraFolder(location());
    m_db.setLocalValueVersion("folders", -1);

}

void WizFolder::moveTo(QObject* dest)
{
    WizFolder* pFolder = dynamic_cast<WizFolder*>(dest);
    if (!pFolder)
        return;

    if (isDeletedItems())
        return;

    if (!canMove(this, pFolder)) {
        TOLOG2("Can move %1 to %2", location(), pFolder->location());
        return;
    }

    return moveToLocation(pFolder->location());
}

bool WizFolder::canMove(const QString& strSource, const QString& strDest)
{
    if (LOCATION_DELETED_ITEMS == strSource)
        return false;

    // sub folder relationship or the same folder
    if (strDest.startsWith(strSource))
        return false;

    return true;
}

bool WizFolder::canMove(WizFolder* pSrc, WizFolder* pDest) const
{
    return canMove(pSrc->location(), pDest->location());
}

void WizFolder::moveToLocation(const QString& strDestLocation)
{
    Q_ASSERT(strDestLocation.right(1) == "/");
    Q_ASSERT(strDestLocation.left(1) == "/");

    if (!canMove(location(), strDestLocation))
        return;

    QString strOldLocation = location();

    CWizDocumentDataArray arrayDocument;
    if (!m_db.getDocumentsByLocation(strOldLocation, arrayDocument, true)) {
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

        if (!m_db.modifyDocumentLocation(data)) {
            TOLOG("Failed to move note to new folder!");
            continue;
        }

        Q_EMIT moveDocument(arrayDocument.size(), i++, strOldLocation, strDestLocation, data);
    }

    CWizStdStringArray arrayFolder;
    m_db.getExtraFolder(arrayFolder);
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
            m_db.addExtraFolder(strFolder);
        }
    }

    qDebug() << "Delete old location ; " << strOldLocation;
    m_db.deleteExtraFolder(strOldLocation);
    m_db.setLocalValueVersion("folders", -1);
}


/* ------------------------------CWizDatabase------------------------------ */

const QString g_strAccountSection = "Account";
const QString g_strCertSection = "Cert";
const QString g_strGroupSection = "Groups";
const QString g_strDatabaseInfoSection = "Database";

#define WIZ_META_KBINFO_SECTION "KB_INFO"
#define WIZ_META_SYNCINFO_SECTION "SYNC_INFO"
#define VERSION_NAME_2_META_KEY(x) "KEY_" + x + "_VERSION"

WizDatabase::WizDatabase()
    : m_ziwReader(new WizZiwReader())
    , m_bIsPersonal(true)
    , m_mutexCache(QMutex::Recursive)
{
    m_ziwReader->setDatabase(this);
}

QString WizDatabase::getUserId()
{
    return m_strUserId;
}

QString WizDatabase::getUserGuid()
{
    return getMetaDef(g_strAccountSection, "GUID");
}

QString WizDatabase::getPassword()
{
    CString strPassword;
    getPassword(strPassword);

    if (!strPassword.isEmpty()) {
        strPassword = ::WizDecryptPassword(strPassword);
    }

    return strPassword;
}

qint64 WizDatabase::getObjectVersion(const QString& strObjectName)
{
    return getMetaInt64("SYNC_INFO", strObjectName, -1);
}

bool WizDatabase::setObjectVersion(const QString& strObjectName,
                                    qint64 nVersion)
{
    return setMetaInt64("SYNC_INFO", strObjectName, nVersion);
}

bool WizDatabase::getModifiedDeletedList(CWizDeletedGUIDDataArray& arrayData)
{
    return getDeletedGuids(arrayData);
}

bool WizDatabase::getModifiedTagList(CWizTagDataArray& arrayData)
{
    return getModifiedTags(arrayData);
}

bool WizDatabase::getModifiedStyleList(CWizStyleDataArray& arrayData)
{
    return getModifiedStyles(arrayData);
}

bool WizDatabase::getModifiedParamList(CWizDocumentParamDataArray& arrayData)
{
    return getModifiedParams(arrayData);
}

bool WizDatabase::getModifiedDocumentList(CWizDocumentDataArray& arrayData)
{
    CWizDocumentDataArray docList;
    if (!getModifiedDocuments(docList))
        return false;

    // remove invalid document
    CWizDocumentDataArray::iterator it;
    for (it = docList.begin(); it != docList.end(); it++)
    {
        WIZDOCUMENTDATA doc = *it;
        if (canEditDocument(doc) && !isInDeletedItems(doc.strLocation))  // do not upload doc in trash
        {
            arrayData.push_back(doc);
        }
    }
    return true;
}

bool WizDatabase::getModifiedAttachmentList(CWizDocumentAttachmentDataArray& arrayData)
{
    return getModifiedAttachments(arrayData);
}

bool WizDatabase::getModifiedMessageList(CWizMessageDataArray& arrayData)
{
    return getModifiedMessages(arrayData);
}

bool WizDatabase::getObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayObject)
{
    return getAllObjectsNeedToBeDownloaded(arrayObject, getObjectSyncTimeline());
}

bool WizDatabase::onDownloadDeletedList(const CWizDeletedGUIDDataArray& arrayData)
{
    CWizDeletedGUIDDataArray arrayDeleted;
    CWizDeletedGUIDDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        WIZDELETEDGUIDDATA data(*it);
        data.strKbGUID = kbGUID();
        arrayDeleted.push_back(data);
    }

    return updateDeletedGuids(arrayDeleted);
}

bool WizDatabase::onDownloadTagList(const CWizTagDataArray& arrayData)
{
    CWizTagDataArray arrayTag;
    CWizTagDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        WIZTAGDATA data(*it);
        data.strKbGUID = kbGUID();
        //NOTE:当前同步数据时不会从服务器中下载tag的position数据。
        //将position数据保留为原本的数据。如果后期规则修改，此处需要修改
        WIZTAGDATA dataTemp;
        if (tagFromGuid(data.strGUID, dataTemp))
        {
            data.nPosition = dataTemp.nPosition;
        }
        arrayTag.push_back(data);
    }

    return updateTags(arrayTag);
}

bool WizDatabase::onDownloadStyleList(const CWizStyleDataArray& arrayData)
{
    CWizStyleDataArray arrayStyle;
    CWizStyleDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        WIZSTYLEDATA data(*it);
        data.strKbGUID = kbGUID();
        arrayStyle.push_back(data);
    }

    return updateStyles(arrayStyle);
}

bool WizDatabase::onDownloadDocumentList(const CWizDocumentDataArray& arrayData)
{
    for (std::deque<WIZDOCUMENTDATAEX>::const_iterator itDocument = arrayData.begin();
         itDocument != arrayData.end();
         itDocument++)
    {
        //m_pEvents->OnStatus(WizFormatString1(_TR("Update note information: %1"), itDocument->strTitle));
        //
        if (!onDownloadDocument(*itDocument))
        {
            //m_pEvents->OnError(WizFormatString1("Cannot update note information: %1", itDocument->strTitle));
            return FALSE;
        }
    }
    return true;
}



bool WizDatabase::onDownloadAttachmentList(const CWizDocumentAttachmentDataArray& arrayData)
{
    CWizDocumentAttachmentDataArray arrayAttach;
    CWizDocumentAttachmentDataArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++) {
        WIZDOCUMENTATTACHMENTDATAEX data(*it);
        data.strKbGUID = kbGUID();
        arrayAttach.push_back(data);
    }

    return updateAttachments(arrayAttach);
}
bool WizDatabase::onDownloadMessageList(const CWizMessageDataArray& arrayData)
{
    return updateMessages(arrayData);
}

bool WizDatabase::onDownloadParamList(const CWizDocumentParamDataArray& arrayData)
{
    for (auto param : arrayData) {
        if (!updateDocumentParam(param))
            return false;
    }
    //
    return true;
}


bool WizDatabase::onDownloadDocument(const WIZDOCUMENTDATAEX& data)
{
    WIZDOCUMENTDATAEX d(data);
    d.strKbGUID = kbGUID();
    return updateDocument(d);
}

qint64 WizDatabase::getObjectLocalVersion(const QString& strObjectGUID,
                                           const QString& strObjectType)
{
    return WizIndex::getObjectLocalVersion(strObjectGUID, strObjectType);
}

qint64 WizDatabase::getObjectLocalVersionEx(const QString& strObjectGUID, const QString& strObjectType, bool& bObjectVersion)
{
     return WizIndex::getObjectLocalVersionEx(strObjectGUID, strObjectType, bObjectVersion);
}

qint64 WizDatabase::getObjectLocalServerVersion(const QString& strObjectGUID,
                                                 const QString& strObjectType)
{
    return WizIndex::getObjectLocalVersion(strObjectGUID, strObjectType);
}

bool WizDatabase::setObjectLocalServerVersion(const QString& strObjectGUID,
                                               const QString& strObjectType,
                                               qint64 nVersion)
{
    return modifyObjectVersion(strObjectGUID, strObjectType, nVersion);
}

void WizDatabase::onObjectUploaded(const QString& strObjectGUID, const QString& strObjectType)
{
    if (strObjectType == "document")
    {
        emit documentUploaded(kbGUID(), strObjectGUID);
    }
}

bool WizDatabase::documentFromGuid(const QString& strGUID,
                                    WIZDOCUMENTDATA& dataExists)
{
    return WizIndex::documentFromGuid(strGUID, dataExists);
}

bool WizDatabase::isObjectDataDownloaded(const QString& strGUID,
                                          const QString& strType)
{
    return WizIndex::isObjectDataDownloaded(strGUID, strType);
}

bool WizDatabase::setObjectDataDownloaded(const QString& strGUID,
                                           const QString& strType,
                                           bool bDownloaded)
{
    return WizIndex::setObjectDataDownloaded(strGUID, strType, bDownloaded);
}

bool WizDatabase::setObjectServerDataInfo(const QString& strGUID,
                                           const QString& strType,
                                           WizOleDateTime& tServerDataModified,
                                           const QString& strServerMD5)
{
    if (strType == WIZDOCUMENTDATA::objectName()) {
        WIZDOCUMENTDATA data;
        if (!documentFromGuid(strGUID, data))
            return false;

        data.tDataModified = tServerDataModified;
        data.strDataMD5 = strServerMD5;

        return modifyDocumentInfoEx(data);
    } else {
        Q_ASSERT(0);
    }

    return false;
}

bool WizDatabase::updateObjectData(const QString& strDisplayName,
                                    const QString& strObjectGUID,
                                    const QString& strObjectType,
                                    const QByteArray& stream)
{   
    WIZOBJECTDATA data;
    data.strDisplayName = strDisplayName;
    data.strObjectGUID = strObjectGUID;
    data.arrayData = stream;

    if (strObjectType == WIZDOCUMENTDATAEX::objectName())
    {
        data.eObjectType = wizobjectDocument;
    }
    else if (strObjectType == WIZDOCUMENTATTACHMENTDATAEX::objectName())
    {
        data.eObjectType = wizobjectDocumentAttachment;
    }
    else
    {
        Q_ASSERT(0);
    }

    return updateSyncObjectLocalData(data);
}

bool WizDatabase::initDocumentData(const QString& strGUID,
                                    WIZDOCUMENTDATAEX& data,
                                   bool forceUploadData)
{
    if (!documentFromGuid(strGUID, data)) {
        return false;
    }
    //
    getDocumentTags(strGUID, data.arrayTagGUID);
    //
    if (data.nVersion == -1)
    {
        if (data.nInfoChanged == 0 && data.nDataChanged == 0)   //old data
        {
            data.nInfoChanged = 1;
            data.nDataChanged = 1;
        }
    }

    if (data.nDataChanged || forceUploadData) {
        if (!loadDocumentZiwData(strGUID, data.arrayData)) {
            return false;
        }
    }

    return true;
}

bool WizDatabase::modifyAttachmentDataMd5(const QString& strGUID, const QString& md5)
{
    QString sql = QString("update %1 set ATTACHMENT_DATA_MD5='%2' where ATTACHMENT_GUID='%3'")
            .arg(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT)
            .arg(md5)
            .arg(strGUID);
    //
    return execSQL(sql);
}

bool WizDatabase::initAttachmentData(const QString& strGUID,
                                      WIZDOCUMENTATTACHMENTDATAEX& data)
{
    if (!attachmentFromGuid(strGUID, data)) {
        return false;
    }
    if (!loadCompressedAttachmentData(strGUID, data.arrayData)) {
        return false;
    }
    //
    if (data.strDataMD5.isEmpty()) {
        data.strDataMD5 = ::WizMd5StringNoSpaceJava(data.arrayData);
        modifyAttachmentDataMd5(strGUID, data.strDataMD5);
    }

    return true;
}

bool WizDatabase::onUploadObject(const QString& strGUID,
                                  const QString& strObjectType)
{
    if (strObjectType == WIZDELETEDGUIDDATA::objectName()) {
        return deleteDeletedGuid(strGUID);
    } else {
        return modifyObjectVersion(strGUID, strObjectType, 0);
    }
}

bool WizDatabase::onUploadParam(const QString& strDocumentGuid, const QString& strName)
{
    return modifyDocumentParamVersion(strDocumentGuid, strName, 0);
}

bool WizDatabase::modifyMessagesLocalChanged(CWizMessageDataArray& arrayData)
{
    for (WIZMESSAGEDATA msg : arrayData)
    {
        modifyMessageLocalChanged(msg);
    }

    return true;
}

bool WizDatabase::copyDocumentData(const WIZDOCUMENTDATA& sourceDoc, WizDatabase& targetDB, WIZDOCUMENTDATA& targetDoc)
{
    QByteArray ba;
    if (!loadDocumentDecryptedData(sourceDoc.strGUID, ba))
        return false;
    //
    return targetDB.writeDataToDocument(targetDoc.strGUID, ba);
}


bool WizDatabase::getBizMetaName(const QString &strBizGUID, QString &strMetaName)
{
    if (strBizGUID.isEmpty())
        return false;

    int count = getMetaDef("Bizs", "Count").toInt();
    //
    for (int i = 0; i < count; i++)
    {
        QString bizSection = "Biz_" + QString::number(i);

        if (strBizGUID == getMetaDef(bizSection, "GUID"))
        {
            strMetaName = bizSection;
            return true;
        }
    }

    return false;
}

bool WizDatabase::initZiwReaderForEncryption()
{
    if (!m_ziwReader->isRSAKeysAvailable())
    {
        if (!hasCert() || !loadUserCert())
        {
            if (!refreshCertFromServer())
            {
                QMessageBox::information(0, tr("Info"), tr("No password cert founded. Please create password" \
                                         " cert from windows client first."));
                return false;
            }
        }
        //
        if (!hasCert() || !loadUserCert())
        {
            QMessageBox::information(0, tr("Info"), tr("No password cert founded. Please create password" \
                                     " cert from windows client first."));
            return false;
        }
    }
    //
    return QueryCertPassword();
}

bool WizDatabase::onDownloadGroups(const CWizGroupDataArray& arrayGroup)
{
    bool ret = setAllGroupInfo(arrayGroup);
    Q_EMIT groupsInfoDownloaded(arrayGroup);
    return ret;
}

bool WizDatabase::onDownloadBizs(const CWizBizDataArray& arrayBiz)
{
    bool ret = setAllBizInfo(arrayBiz);
    Q_EMIT bizInfoDownloaded(arrayBiz);
    return ret;
}

bool WizDatabase::onDownloadBizUsers(const QString& kbGuid, const CWizBizUserDataArray& arrayUser)
{
    if (arrayUser.empty())
        return false;
    //

    CWizBizUserDataArray oldUsers;
    if (users(kbGuid, oldUsers))
    {
        std::map<QString, WIZBIZUSER> old;
        for (const auto& user: oldUsers)
        {
            old[user.userGUID] = user;
        }
        //
        bool bHasError = false;
        CWizBizUserDataArray::const_iterator it;
        for (it = arrayUser.begin(); it != arrayUser.end(); it++)
        {
            const WIZBIZUSER& user = *it;
            const WIZBIZUSER oldUser = old[user.userGUID];
            //
            if (oldUser == user)
                continue;
            //
            if (oldUser.userGUID.isEmpty())
            {
                if (createUserEx(user)) {
                    bHasError = true;
                }
            }
            else
            {
                if (modifyUserEx(user)) {
                    bHasError = true;
                }
            }
        }
        return !bHasError;
    }
    else
    {
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
}


IWizSyncableDatabase* WizDatabase::getGroupDatabase(const WIZGROUPDATA& group)
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

    WizDatabase* db = &WizDatabaseManager::instance()->db(group.strGroupGUID);
    return db;
}

void WizDatabase::closeGroupDatabase(IWizSyncableDatabase* pDatabase)
{
//    CWizDatabase* db = dynamic_cast<CWizDatabase*>(pDatabase);

//    Q_ASSERT(db);

//    db->Close();
//    db->deleteLater();

}

IWizSyncableDatabase* WizDatabase::getPersonalDatabase()
{
    return personalDatabase();
}
WizDatabase *WizDatabase::personalDatabase()
{
    WizDatabase* db = &WizDatabaseManager::instance()->db();
    return db;
}

void WizDatabase::setKbInfo(const QString& strKBGUID, const WIZKBINFO& info)
{
    Q_ASSERT(strKBGUID == kbGUID() || (strKBGUID.isEmpty() && m_bIsPersonal));

    setMetaInt64(WIZ_META_KBINFO_SECTION, "STORAGE_LIMIT_N", info.nStorageLimit);
    setMetaInt64(WIZ_META_KBINFO_SECTION, "STORAGE_USAGE_N", info.nStorageUsage);
    setMetaInt64(WIZ_META_KBINFO_SECTION, "TRAFFIC_LIMIT_N", info.nTrafficLimit);
    setMetaInt64(WIZ_META_KBINFO_SECTION, "TRAFFIC_USAGE_N", info.nTrafficUsage);
}

QString WizDatabase::getGroupName()
{
    if (!isGroup())
        return "";

    WizDatabase* personDb = personalDatabase();

    WIZGROUPDATA group;
    personDb->getGroupData(kbGUID(), group);
    return group.strGroupName;
}

WIZGROUPDATA WizDatabase::getGroupInfo()
{
    if (!isGroup())
        return WIZGROUPDATA();

    WizDatabase* personDb = personalDatabase();
    WIZGROUPDATA data;
    personDb->getGroupData(kbGUID(), data);
    return data;
}


void WizDatabase::setUserInfo(const WIZUSERINFO& userInfo)
{
    setMeta(g_strDatabaseInfoSection, "KBGUID", userInfo.strKbGUID);

    setMeta(g_strAccountSection, "GUID", userInfo.strUserGUID);
    setMeta(g_strAccountSection, "DisplayName", userInfo.strDisplayName);
    setMeta(g_strAccountSection, "UserType", userInfo.strUserType);
    setMeta(g_strAccountSection, "UserLevelName", userInfo.strUserLevelName);
    setMeta(g_strAccountSection, "UserLevel", QString::number(userInfo.nUserLevel));
    setMeta(g_strAccountSection, "UserPoints", QString::number(userInfo.nUserPoints));
    setMeta(g_strAccountSection, "MywizMail", userInfo.strMywizEmail);
    setMeta(g_strAccountSection, "DateSignUp", userInfo.tCreated.toString());
    setMeta(g_strAccountSection, "kbServer", userInfo.strKbServer);

    Q_EMIT userInfoChanged();
}

bool WizDatabase::isGroup()
{
    if (m_bIsPersonal)
        return false;

    return true;
}

bool WizDatabase::isPersonalGroup()
{
    if (!isGroup())
        return false;

    WizDatabase* personDb = personalDatabase();

    WIZGROUPDATA group;
    personDb->getGroupData(kbGUID(), group);
    return !group.isBiz();
}

bool WizDatabase::hasBiz()
{
    WizDatabase* personDb = personalDatabase();

    QString count = personDb->getMetaDef("Bizs", "Count");
    return atoi(count.toUtf8()) != 0;
}

bool WizDatabase::isGroupAdmin()
{
    if (permission() <= WIZ_USERGROUP_ADMIN)
        return true;

    return false;
}

bool WizDatabase::isGroupOwner()
{
    return m_info.bOwner;
}

bool WizDatabase::isGroupSuper()
{
    if (permission() <= WIZ_USERGROUP_SUPER)
        return true;

    return false;
}

bool WizDatabase::isGroupEditor()
{
    if (permission() <= WIZ_USERGROUP_EDITOR)
        return true;

    return false;
}

bool WizDatabase::isGroupAuthor()
{
    if (permission() <= WIZ_USERGROUP_AUTHOR)
        return true;

    return false;
}

bool WizDatabase::isGroupReader()
{
    if (permission() <= WIZ_USERGROUP_READER)
        return true;

    return false;
}

bool WizDatabase::canEditDocument(const WIZDOCUMENTDATA& data)
{
    if (permission() < WIZ_USERGROUP_AUTHOR ||
                (permission() == WIZ_USERGROUP_AUTHOR && data.strOwner == getUserId())) {
            return true;
    }

    return false;
}

bool WizDatabase::canEditAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    WIZDOCUMENTDATA doc;
    if (!documentFromGuid(data.strDocumentGUID, doc)) {
        return false;
    }

    return canEditDocument(doc);
}

bool WizDatabase::createConflictedCopy(const QString& strObjectGUID,
                                        const QString& strObjectType)
{
    // FIXME
    Q_UNUSED(strObjectGUID);
    Q_UNUSED(strObjectType);
    Q_ASSERT(0);

    return false;
}

bool WizDatabase::saveLastSyncTime()
{
    uint secs = QDateTime::currentDateTime().toTime_t();
    return setMeta("SYNC_INFO", "TIME", QString::number(secs));
}

WizOleDateTime WizDatabase::getLastSyncTime()
{
    uint secs = getMetaDef("SYNC_INFO", "TIME").toUInt();
    return WizOleDateTime(secs);
}

bool WizDatabase::WizDayOnce(const QString& strName)
{
    WizDatabase* db = personalDatabase();
    QString strDate = db->meta("WizDayOnce", strName);
    QDateTime curDt = QDateTime::currentDateTime();
    db->setMeta("WizDayOnce", strName, curDt.toString(Qt::ISODate));

    if (strDate.isEmpty())
        return true;

    QDateTime dt = QDateTime::fromString(strDate, Qt::ISODate);
    return dt.daysTo(curDt) >= 1;
}

long WizDatabase::getLocalFlags(const QString& strObjectGUID,
                                 const QString& strObjectType)
{
    Q_UNUSED(strObjectGUID);
    Q_UNUSED(strObjectType);

    Q_ASSERT(0);
    return 0;
}

bool WizDatabase::setLocalFlags(const QString& strObjectGUID,
                                 const QString& strObjectType,
                                 long flags)
{
    Q_UNUSED(strObjectGUID);
    Q_UNUSED(strObjectType);
    Q_UNUSED(flags);

    Q_ASSERT(0);
    return false;
}

void WizDatabase::getAccountKeys(CWizStdStringArray& arrayKey)
{
    Q_ASSERT(!isGroup());
}

qint64 WizDatabase::getAccountLocalValueVersion(const QString& strKey)
{
    return getMetaInt64(WIZ_META_SYNCINFO_SECTION, "KEY_" + strKey + "_VERSION", 0);
}

void WizDatabase::setAccountLocalValue(const QString& strKey,
                                        const QString& strValue,
                                        qint64 nServerVersion,
                                        bool bSaveVersion)
{
    Q_ASSERT(!isGroup());

    if (bSaveVersion) {
        setMetaInt64(WIZ_META_SYNCINFO_SECTION, "KEY_" + strKey + "_VERSION", nServerVersion);
    }
}

void WizDatabase::getKBKeys(CWizStdStringArray& arrayKey)
{
    if (isGroup())
    {
        arrayKey.push_back("group_tag_pos");
    }
    else
    {
        arrayKey.push_back("folders");
        arrayKey.push_back("folders_pos");
        arrayKey.push_back("favorites");
    }
}

bool WizDatabase::processValue(const QString& key)
{
    CString strKey(key);
    strKey.makeLower();

    if (strKey == "folders"
        || strKey == "folders_pos")
    {
        return !isGroup();
    }

    return true;
}

qint64 WizDatabase::getLocalValueVersion(const QString& key)
{
    CString strKey(key);
    strKey.makeLower();

    return getMetaInt64(WIZ_META_SYNCINFO_SECTION, "KEY_" + strKey + "_VERSION", 0);

    //if (strKey == "folders")
    //{
    //    return GetMetaInt64(key, "version", 0);
    //}
    //else
    //{
    //    return GetMetaInt64(key, "version", 0);
    //}
}

QString WizDatabase::getLocalValue(const QString& key)
{
    CString strKey(key);
    strKey.makeLower();

    if (strKey == "folders")
    {
        return getFolders();
    }
    else if (strKey == "folders_pos")
    {
        return getFoldersPos();
    }
    else if (strKey == "group_tag_pos")
    {
        return getGroupTagsPos();
    }
    else if (strKey == "favorites")
    {
        return getFavorites();
    }
    else
    {
        return CString();
    }
}

void WizDatabase::setLocalValueVersion(const QString& strKey,
                                        qint64 nServerVersion)
{
    setMetaInt64(WIZ_META_SYNCINFO_SECTION, "KEY_" + strKey + "_VERSION", nServerVersion);
}

void WizDatabase::setLocalValue(const QString& key, const QString& value,
                                 qint64 nServerVersion, bool bSaveVersion)
{
    CString strKey(key);
    strKey.makeLower();

    if (strKey == "folders")
    {
        setFolders(value, nServerVersion, bSaveVersion);
    }
    else if (strKey == "folders_pos")
    {
        setFoldersPos(value, nServerVersion);
    }
    else if (strKey == "group_tag_pos")
    {
        setGroupTagsPos(value, nServerVersion);
    }
    else if (strKey == "favorites")
    {
        setFavorites(value, nServerVersion);
    }
    else
    {
        setLocalValueVersion(key, nServerVersion);
    }
}

void WizDatabase::getAllBizUserIds(CWizStdStringArray& arrayText)
{
    CWizBizUserDataArray arrayUser;
    if (!getAllUsers(arrayUser))
        return;

    CWizBizUserDataArray::const_iterator it;
    for (it = arrayUser.begin(); it != arrayUser.end(); it++) {
        const WIZBIZUSER& user = *it;
        arrayText.push_back(user.userId);
    }
}

bool WizDatabase::getAllBizUsers(CWizBizUserDataArray& arrayUser)
{
    return getAllUsers(arrayUser);
}

void WizDatabase::clearLastSyncError()
{
    setMeta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorCode", QString::number(WIZKMSYNC_EXIT_OK));
    setMeta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorMessage", "");

    //
    if (getPersonalDatabase() != this)
        return;

    //
    int bizCount = getMetaDef("Bizs", "Count").toInt();
    for (int i = 0; i < bizCount; i++)
    {
        QString bizSection = "Biz_" + QString::number(i);
        setMeta(bizSection, "LastSyncErrorCode", QString::number(WIZKMSYNC_EXIT_OK));
        setMeta(bizSection, "LastSyncErrorMessage", "");
    }
}

QString WizDatabase::getLastSyncErrorMessage()
{
    return meta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorMessage");
}

void WizDatabase::onTrafficLimit(const QString& strErrorMessage)
{
    setMeta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorCode", QString::number(WIZKMSYNC_EXIT_TRAFFIC_LIMIT));
    setMeta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorMessage", strErrorMessage);
}

void WizDatabase::onStorageLimit(const QString& strErrorMessage)
{
    setMeta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorCode", QString::number(WIZKMSYNC_EXIT_STORAGE_LIMIT));
    setMeta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorMessage", strErrorMessage);
}

void WizDatabase::onNoteCountLimit(const QString& strErrorMessage)
{
    setMeta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorCode", QString::number(WIZKMSYNC_EXIT_NOTE_COUNT_LIMIT));
    setMeta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorMessage", strErrorMessage);
}

void WizDatabase::onBizServiceExpr(const QString& strBizGUID, const QString& strErrorMessage)
{
    if (strBizGUID.isEmpty())
        return;

    WizDatabase* db = personalDatabase();
    if (!db)
        return;

    QString strMetaSection;
    if (!db->getBizMetaName(strBizGUID, strMetaSection))
        return;
    //
    db->setMeta(strMetaSection, "LastSyncErrorCode", QString::number(WIZKMSYNC_EXIT_BIZ_SERVICE_EXPR));
    db->setMeta(strMetaSection, "LastSyncErrorMessage", strErrorMessage);
}

bool WizDatabase::isTrafficLimit()
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorCode");

    return strLastError.toInt() == WIZKMSYNC_EXIT_TRAFFIC_LIMIT;
}

bool WizDatabase::isStorageLimit()
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorCode");

    return strLastError.toInt() == WIZKMSYNC_EXIT_STORAGE_LIMIT;
}

bool WizDatabase::isNoteCountLimit()
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorCode");

    return strLastError.toInt() == WIZKMSYNC_EXIT_NOTE_COUNT_LIMIT;
}

bool WizDatabase::isBizServiceExpr(const QString& strBizGUID)
{
    WizDatabase* db = personalDatabase();
    if (!db)
        return false;

    QString strMetaSection;
    if (!db->getBizMetaName(strBizGUID, strMetaSection))
        return false;

    QString strLastError = db->meta(strMetaSection, "LastSyncErrorCode");

    return strLastError.toInt() == WIZKMSYNC_EXIT_BIZ_SERVICE_EXPR;
}

bool WizDatabase::getStorageLimitMessage(QString &strErrorMessage)
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorCode");

    if (strLastError.toInt() == WIZKMSYNC_EXIT_STORAGE_LIMIT)
    {
        strErrorMessage = meta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorMessage");
        return true;
    }

    return false;
}

bool WizDatabase::getTrafficLimitMessage(QString& strErrorMessage)
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorCode");

    if (strLastError.toInt() == WIZKMSYNC_EXIT_TRAFFIC_LIMIT)
    {
        strErrorMessage = meta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorMessage");
        return true;
    }

    return false;
}

bool WizDatabase::getNoteCountLimit(QString& strErrorMessage)
{
    QString strLastError = meta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorCode");

    if (strLastError.toInt() == WIZKMSYNC_EXIT_NOTE_COUNT_LIMIT)
    {
        strErrorMessage = meta(WIZKMSYNC_EXIT_INFO, "LastSyncErrorMessage");
        return true;
    }

    return false;
}

bool WizDatabase::setMeta(const QString& strSection, const QString& strKey, const QString& strValue)
{
    return WizIndex::setMeta(strSection, strKey, strValue);
}

QString WizDatabase::meta(const QString& strSection, const QString& strKey)
{
    return getMetaDef(strSection, strKey);
}

void WizDatabase::setFoldersPos(const QString& foldersPos, qint64 nVersion)
{
    setLocalValueVersion("folders_pos", nVersion);
    setMeta("SYNC_INFO", "FOLDERS_POS", foldersPos);

    bool bPositionChanged = false;

    CString str(foldersPos);
    str.trim();
    str.trim('{');
    str.trim('}');
    str.replace("\n", "");
    str.replace("\r", "");
    if (str.isEmpty())
        return;

    CWizStdStringArray arrPos;
    ::WizSplitTextToArray(str, ',', arrPos);

    QSettings* setting = WizGlobal::settings();
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
        strLocation.trim();
        strLocation.trim('\"');

        int nPos = wiz_ttoi(strPos);
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

QString WizDatabase::getFolders()
{
    CWizStdStringArray arrayFolder;
    getAllLocations(arrayFolder);

    CWizStdStringArray arrayExtra;
    getExtraFolder(arrayExtra);
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

QString WizDatabase::getFoldersPos()
{
    return meta("SYNC_INFO", "FOLDERS_POS");
}

QString WizDatabase::getGroupTagsPos()
{
    CWizTagDataArray arrayTag;
    getAllTags(arrayTag);
    if (arrayTag.size() == 0)
        return QString();

    QString strTagPos;
    for (CWizTagDataArray::const_iterator it = arrayTag.begin();
         it != arrayTag.end();
         it++)
    {
        WIZTAGDATA tag = *it;
        strTagPos.append(tag.strGUID + ":" + QString::number(tag.nPosition) + "*");
    }
    strTagPos.remove(strTagPos.length() - 1, 1);
    return strTagPos;
}

void WizDatabase::setFolders(const QString& strFolders, qint64 nVersion, bool bSaveVersion)
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
    getAllLocationsWithExtra(arrayLocation);

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

        if (0 == strLocation.find("/Deleted Items/"))
            continue;

        if (setLocalFolders.find(strLocation) == setLocalFolders.end())
        {
            // server exists, local does not exists, create folders.
            addExtraFolder(strLocation);
        }
    }

    for (std::set<CString>::const_iterator it = setLocalFolders.begin();
        it != setLocalFolders.end();
        it++)
    {
        CString strLocation = *it;

        if (0 == strLocation.find("/Deleted Items/"))
            continue;

        if (setServerFolders.find(strLocation) == setServerFolders.end())
        {
            // local exists, server does not exists, delete local folders.
            int nSize = 0;
            if (getDocumentsCountByLocation(strLocation, nSize, true))
            {
                if (nSize == 0)
                {
                    deleteExtraFolder(strLocation);
                }
            }
        }
    }

    if (bSaveVersion)
    {
        setLocalValueVersion("folders", nVersion);
    }
}

void WizDatabase::setGroupTagsPos(const QString& tagsPos, qint64 nVersion)
{
    setLocalValueVersion("group_tag_pos", nVersion);
    //
    CWizTagDataArray arrayTag;
    getAllTags(arrayTag);
    std::map<QString, WIZTAGDATA> tags;
    for (const WIZTAGDATA& tag : arrayTag)
    {
        tags[tag.strGUID] = tag;
    }

    bool bPositionChanged = false;

    CString str(tagsPos);
    str.trim();
    if (str.isEmpty())
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
        //
        WIZTAGDATA tagData = tags[strGUID];
        if (!tagData.strGUID.isEmpty() && tagData.nPosition != nPos)
        {
            tagData.nPosition = nPos;
            modifyTagPosition(tagData);
            bPositionChanged = true;
        }
    }

    if (bPositionChanged) {
        Q_EMIT tagsPositionChanged(kbGUID());
    }
}

QString WizDatabase::getFavorites()
{
    return meta("SYNC_INFO", "FAVORITES");
}

void WizDatabase::setFavorites(const QString& favorites, qint64 nVersion)
{
    setLocalValueVersion("favorites", nVersion);
    setMeta("SYNC_INFO", "FAVORITES", favorites);

    if (nVersion != -1)
    {
        emit favoritesChanged(favorites);
    }
}

void WizDatabase::setObjectSyncTimeLine(int nDays)
{
    setMeta("SYNC_INFO", "TIMELINE", QString::number(nDays));
}

int WizDatabase::getObjectSyncTimeline()
{
    int nDays = getMetaDef("SYNC_INFO", "TIMELINE").toInt();
    if (m_bIsPersonal && !nDays) {
        return 99999;
    } else if (!m_bIsPersonal && !nDays) {
        return 1;
    }

    return nDays;
}

void WizDatabase::setDownloadAttachmentsAtSync(bool download)
{
    CString strD = download ? "1" : "0";
    setMeta("QT_WIZNOTE", "SyncDownloadAttachment", strD);
}

bool WizDatabase::getDownloadAttachmentsAtSync()
{
    return getMetaDef("QT_WIZNOTE", "SyncDownloadAttachment", "0").toInt() != 0;
}

class WizCompareString
{
public:
    WizCompareString(const QString& text): m_text(text){}

    bool operator() (const QString& folder) const
    {
        return folder.compare(m_text, Qt::CaseInsensitive) == 0;
    }
private:
    QString m_text;
};

bool WizDatabase::isFolderExists(const QString& folder)
{
    CWizStdStringArray arrayFolder;
    getAllLocations(arrayFolder);

    CWizStdStringArray::const_iterator pos = std::find_if(arrayFolder.begin(), arrayFolder.end(),
                                                WizCompareString(folder));

    if (pos != arrayFolder.end())
        return true;

    CWizStdStringArray arrayExtra;
    getExtraFolder(arrayExtra);
    pos = std::find_if(arrayExtra.begin(), arrayExtra.end(), WizCompareString(folder));

    return pos != arrayExtra.end();
}

void WizDatabase::setFoldersPosModified()
{
    setLocalValueVersion("folders_pos", -1);
    setLocalValueVersion("folders", -1);
}

void WizDatabase::setGroupTagsPosModified()
{
    setLocalValueVersion("group_tag_pos", -1);
}

bool WizDatabase::getAllNotesOwners(CWizStdStringArray& arrayOwners)
{
    return getAllDocumentsOwners(arrayOwners);
}
//
bool WizDatabase::deleteDocumentFromLocal(const QString& strDocumentGuid)
{
    WIZDOCUMENTDATA doc;
    if (!documentFromGuid(strDocumentGuid, doc))
        return false;
    //
    return deleteDocument(doc, false);
}

bool WizDatabase::deleteAttachmentFromLocal(const QString& strAttachmentGuid)
{
    WIZDOCUMENTATTACHMENTDATA att;
    if (!attachmentFromGuid(strAttachmentGuid, att))
        return false;
    //
    return deleteAttachment(att, false, true, true);
}

/* ---------------------------------------------------------------------- */

bool WizDatabase::updateMessages(const CWizMessageDataArray& arrayMsg)
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
        setObjectVersion(WIZMESSAGEDATA::objectName(), nVersion);
    }

    return !bHasError;
}

bool WizDatabase::setAllBizInfoCore(const CWizBizDataArray& arrayBiz)
{
    class CBizUserAvatar
    {
    public:
        static __int64 GetProcessedAvatarVersion(QString strBizGUID, WizDatabase& db)
        {
            return wiz_ttoi64(db.meta("sync", CString("AvatarChangesVer_") + strBizGUID));
        }
        //
        static void SetProcessedAvatarVersion(QString strBizGUID, WizDatabase& db, __int64 v)
        {
            db.setMeta("sync", CString("AvatarChangesVer_") + strBizGUID, WizInt64ToStr(v));
        }
        //
        static void ProcessBizAvatarVersion(QString strBizGUID, WizDatabase& db, const std::map<QString, QString>& mapAvatars)
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
                __int64 v = wiz_ttoi64(it->first);
                if (v > oldVer)
                {
                    newVer = std::max<__int64>(v, newVer);
                    //
                    TOLOG1("[sync] User avatar changed : %1", it->second);
                    WizAvatarHost::reload(it->second);
                }
            }
            //
            if (newVer > oldVer)
            {
                SetProcessedAvatarVersion(strBizGUID, db, newVer);
            }
        }
    };


    setMeta("Bizs", "Count", QString::number(arrayBiz.size()));
    //
    for (int i = 0; i < arrayBiz.size(); i++)
    {
        const WIZBIZDATA& biz = arrayBiz[i];
        QString bizSection = "Biz_" + QString::number(i);
        setMeta(bizSection, "GUID", biz.bizGUID);
        setMeta(bizSection, "Name", biz.bizName);
        setMeta(bizSection, "UserRole", QString::number(biz.bizUserRole));
        setMeta(bizSection, "Level", QString::number(biz.bizLevel));
        QString BizIsDue = biz.bizIsDue ? "1" : "0";
        setMeta(bizSection, "IsDue", BizIsDue);
        //
        CBizUserAvatar::ProcessBizAvatarVersion(biz.bizGUID, *this, biz.mapAvatarChanges);
    }
    //
    return true;
}
//
//
bool WizDatabase::isEmptyBiz(const CWizGroupDataArray& arrayGroup, const QString& bizGUID)
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

bool WizDatabase::getAllBizInfoCore(const CWizGroupDataArray& arrayGroup, CWizBizDataArray& arrayBiz)
{
    int count = getMetaDef("Bizs", "Count").toInt();
    //
    for (int i = 0; i < count; i++)
    {
        QString bizSection = "Biz_" + QString::number(i);
        //
        WIZBIZDATA biz;
        biz.bizGUID = getMetaDef(bizSection, "GUID");
        biz.bizName = getMetaDef(bizSection, "Name");
        biz.bizUserRole = getMetaDef(bizSection, "UserRole").toInt();
        biz.bizLevel = getMetaDef(bizSection, "Level").toInt();
        biz.bizIsDue = getMetaDef(bizSection, "IsDue") == "1";
        //
        if (biz.bizUserRole == WIZ_BIZROLE_OWNER || !isEmptyBiz(arrayGroup, biz.bizGUID))
        {
            arrayBiz.push_back(biz);
        }
    }
    //
    return true;
}
bool WizDatabase::getBizData(const QString& bizGUID, WIZBIZDATA& biz)
{
    if (bizGUID.isEmpty())
        return false;

    WizDatabase* db = personalDatabase();
    if (!db)
        return false;

    CWizBizDataArray arrayBiz;
    if (!db->getAllBizInfo(arrayBiz))
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

bool WizDatabase::getBizGuid(const QString &strGroupGUID, QString &strBizGUID)
{
    if (strGroupGUID.isEmpty())
        return false;

    IWizSyncableDatabase* db = getPersonalDatabase();
    if (!db)
        return false;

    QString groupSection = strGroupGUID.toUpper() + "_BIZGUID";
    strBizGUID = db->meta("GROUPS", groupSection);

    return true;
}

bool WizDatabase::getGroupData(const QString& groupGUID, WIZGROUPDATA& group)
{
    group.strGroupGUID = groupGUID;
    group.strGroupName = getMetaDef(g_strGroupSection, groupGUID);

    group.strKbServer = getMetaDef(g_strGroupSection, groupGUID + "_KbServer");
    group.bizGUID = getMetaDef(g_strGroupSection, groupGUID + "_BizGUID");
    group.bizName= getMetaDef(g_strGroupSection, groupGUID + "_BizName");
    group.bOwn = getMetaDef(g_strGroupSection, groupGUID + "_Own") == "1";
    group.nUserGroup = getMetaDef(g_strGroupSection, groupGUID + "_Role", QString::number(WIZ_USERGROUP_MAX)).toInt();
    group.strMyWiz = getMetaDef(g_strGroupSection, group.strGroupGUID + "_MyWizEmail");
    group.bEncryptData = getMetaDef(g_strGroupSection, groupGUID + "_EncryptData") == "1";
    //
    if (group.bizGUID.isEmpty())
    {
        //load biz data from old settings
        //
        QString section = "BizGroups";
        group.bizGUID = getMetaDef(section, group.strGroupGUID);
        if (!group.bizGUID.isEmpty())
        {
            group.bizName = getMetaDef(section, group.bizGUID);
        }
    }

    return !group.strGroupName.isEmpty();
}

QString WizDatabase::getKbServer(const QString &kbGuid) {
    //
    IWizSyncableDatabase* pDatabase = this;
    if (isGroup()) {
        pDatabase = getPersonalDatabase();
    }
    //
    WizDatabase* db = dynamic_cast<WizDatabase*>(pDatabase);
    if (!db) {
        return QString();
    }
    //
    if (db->kbGUID() == kbGuid || kbGuid.isEmpty()) {
        WIZUSERINFO userInfo;
        db->getUserInfo(userInfo);
        return userInfo.strKbServer;
    }
    //
    WIZGROUPDATA group;
    db->getGroupData(kbGuid, group);
    return group.strKbServer;
}

bool WizDatabase::getOwnGroups(const CWizGroupDataArray& arrayAllGroup, CWizGroupDataArray& arrayOwnGroup)
{
    for (CWizGroupDataArray::const_iterator it = arrayAllGroup.begin();
         it != arrayAllGroup.end();
         it++)
    {
        if (it->isBiz())
            continue;
        if (it->isOwn())
        {
            arrayOwnGroup.push_back(*it);
        }
    }
    return true;
}

bool WizDatabase::getJionedGroups(const CWizGroupDataArray& arrayAllGroup, CWizGroupDataArray& arrayJionedGroup)
{
    for (CWizGroupDataArray::const_iterator it = arrayAllGroup.begin();
         it != arrayAllGroup.end();
         it++)
    {
        if (it->isBiz())
            continue;
        if (!it->isOwn())
        {
            arrayJionedGroup.push_back(*it);
        }
    }
    return true;
}

bool WizDatabase::setAllGroupInfoCore(const CWizGroupDataArray& arrayGroup)
{
    int nTotal = arrayGroup.size();
    // set group info
    setMeta(g_strGroupSection, "Count", QString::number(nTotal));

    for (int i = 0; i < nTotal; i++) {
        const WIZGROUPDATA& group = arrayGroup[i];
        setMeta(g_strGroupSection, QString::number(i), group.strGroupGUID);
        setMeta(g_strGroupSection, group.strGroupGUID, group.strGroupName);

        setMeta(g_strGroupSection, group.strGroupGUID + "_KbServer", group.strKbServer);
        setMeta(g_strGroupSection, group.strGroupGUID + "_BizGUID", group.bizGUID);
        setMeta(g_strGroupSection, group.strGroupGUID + "_BizName", group.bizName);
        setMeta(g_strGroupSection, group.strGroupGUID + "_Own", group.bOwn ? "1" : "0");
        setMeta(g_strGroupSection, group.strGroupGUID + "_Role", QString::number(group.nUserGroup));
        setMeta(g_strGroupSection, group.strGroupGUID + "_MyWizEmail", group.strMyWiz);
        setMeta(g_strGroupSection, group.strGroupGUID + "_EncryptData", group.bEncryptData ? "1" : "0");
    }

    return true;
}


bool WizDatabase::getAllGroupInfoCore(CWizGroupDataArray& arrayGroup)
{
    CString strTotal;
    bool bExist;

    if (!getMeta(g_strGroupSection, "Count", strTotal, "", &bExist)) {
        return false;
    }

    // it's ok, user has no group data
    if (!bExist) {
        return true;
    }

    int nTotal = strTotal.toInt();
    for (int i = 0; i < nTotal; i++) {
        WIZGROUPDATA group;

        group.strGroupGUID = getMetaDef(g_strGroupSection, QString::number(i));
        //
        if (!group.strGroupGUID.isEmpty())
        {
            getGroupData(group.strGroupGUID, group);
            //
            arrayGroup.push_back(group);
        }
    }

    return true;
}

bool WizDatabase::getAllGroupInfo(CWizGroupDataArray& arrayGroup)
{
    QMutexLocker locker(&m_mutexCache);
    bool isEmpty = m_cachedGroups.empty();
    bool ret = true;
    if (isEmpty)
    {
        ret = getAllGroupInfoCore(m_cachedGroups);
    }
    arrayGroup = m_cachedGroups;
    return ret;
}

bool WizDatabase::setAllGroupInfo(const CWizGroupDataArray& arrayGroup)
{
    QMutexLocker locker(&m_mutexCache);
    m_cachedGroups = arrayGroup;
    //
    return setAllGroupInfoCore(arrayGroup);
}


bool WizDatabase::getAllBizInfo(CWizBizDataArray& arrayBiz)
{
    CWizGroupDataArray arrayGroup;
    getAllGroupInfo(arrayGroup);
    //
    QMutexLocker locker(&m_mutexCache);
    bool isEmpty = m_cachedBizs.empty();
    bool ret = true;
    if (isEmpty)
    {
        ret = getAllBizInfoCore(arrayGroup, m_cachedBizs);
    }
    arrayBiz = m_cachedBizs;
    return ret;
}

//
bool WizDatabase::setAllBizInfo(const CWizBizDataArray& arrayBiz)
{
    QMutexLocker locker(&m_mutexCache);
    //
    //不能直接使用服务器的biz列表，因为有空的。强制失效，从本地重新获取
    //m_cachedBizs = arrayBiz;
    m_cachedBizs.clear();
    //
    return setAllBizInfoCore(arrayBiz);
}


bool WizDatabase::updateDeletedGuids(const CWizDeletedGUIDDataArray& arrayDeletedGUID)
{
    if (arrayDeletedGUID.empty())
        return true;

    qint64 nVersion = -1;

    bool bHasError = false;

    CWizDeletedGUIDDataArray::const_iterator it;
    for (it = arrayDeletedGUID.begin(); it != arrayDeletedGUID.end(); it++) {
        const WIZDELETEDGUIDDATA& data = *it;

        if (!updateDeletedGuid(data)) {
            bHasError = true;
        }

        nVersion = qMax(nVersion, data.nVersion);
    }

    if (!bHasError) {
        setObjectVersion(WIZDELETEDGUIDDATA::objectName(), nVersion);
    }

    return !bHasError;
}


bool WizDatabase::open(const QString& strAccountFolderName, const QString& strKbGUID /* = NULL */)
{
    Q_ASSERT(!strAccountFolderName.isEmpty());

    m_strAccountFolderName = strAccountFolderName;

    if (strKbGUID.isEmpty()) {
        m_bIsPersonal = true;
    } else {
        m_bIsPersonal = false;
        setKbGUID(strKbGUID);
    }

    if (!WizIndex::open(getIndexFileName())) {
        // If can not open db, try again. If db still can not be opened, delete the db file and download data from server.
        if (!WizIndex::open(getIndexFileName())) {
            QFile::remove(getIndexFileName());
            return false;
        }
    }

    // user private database opened, try to load kb guid
    if (strKbGUID.isEmpty()) {
        // user private kb_guid should be set before actually open for operating
        QString strUserKbGuid = getMetaDef(g_strDatabaseInfoSection, "KBGUID");
        if (!strUserKbGuid.isEmpty())
            setKbGUID(strUserKbGuid);
    }

    // FIXME
    if (!WizThumbIndex::openThumb(getThumbFileName(), getThumIndexVersion())) {
        // If can not open db, try again. If db still can not be opened, delete the db file and download data from server.
        if (!WizThumbIndex::openThumb(getThumbFileName(), getThumIndexVersion())) {
            QFile::remove(getThumbFileName());
            return false;
        }
    }

    setThumbIndexVersion(WIZNOTE_THUMB_VERSION);

    loadDatabaseInfo();

    return true;
}

bool WizDatabase::loadDatabaseInfo()
{
    QString strUserId = getMetaDef(g_strAccountSection, "USERID");
    if (!strUserId.isEmpty())
    {
        m_strUserId = strUserId;
    }

    if (!kbGUID().isEmpty())
    {
        m_info.bizName = getMetaDef(g_strDatabaseInfoSection, "BizName");
        m_info.bizGUID = getMetaDef(g_strDatabaseInfoSection, "BizGUID");
    }

    m_info.name = getMetaDef(g_strDatabaseInfoSection, "Name");
    m_info.nPermission = getMetaDef(g_strDatabaseInfoSection, "Permission").toInt();
    m_info.bOwner = getMetaDef(g_strDatabaseInfoSection, "Owner") == "1";
    m_info.bEncryptData = getMetaDef(g_strDatabaseInfoSection, "EncryptData") == "1";

    return true;
}


bool WizDatabase::initDatabaseInfo(const WIZDATABASEINFO& dbInfo)
{
    Q_ASSERT(!dbInfo.name.isEmpty());

    int nErrors = 0;

    // general
    if (!setMeta(g_strDatabaseInfoSection, "Name", dbInfo.name))
        nErrors++;
    if (!setMeta(g_strDatabaseInfoSection, "Permission", QString::number(dbInfo.nPermission)))
        nErrors++;
    if (!setMeta(g_strDatabaseInfoSection, "Owner", dbInfo.bOwner ? "1" : "0"))
        nErrors++;
    if (!setMeta(g_strDatabaseInfoSection, "EncryptData", dbInfo.bEncryptData ? "1" : "0"))
        nErrors++;

    // biz group info
    if (!dbInfo.bizGUID.isEmpty() && !dbInfo.bizName.isEmpty()) {
        if (!setMeta(g_strDatabaseInfoSection, "BizName", dbInfo.bizName))
            nErrors++;
        if (!setMeta(g_strDatabaseInfoSection, "BizGUID", dbInfo.bizGUID))
            nErrors++;
    }

    if (!setMeta(g_strDatabaseInfoSection, "KbGUID", kbGUID()))
        nErrors++;

    if (!setMeta(g_strDatabaseInfoSection, "Version", WIZ_DATABASE_VERSION))
        nErrors++;
    //
    loadDatabaseInfo();

    if (nErrors)
        return false;

    return true;
}


bool WizDatabase::setDatabaseInfo(const WIZDATABASEINFO& dbInfo)
{
    Q_ASSERT(!dbInfo.name.isEmpty());

    int nErrors = 0;

    // general
    if (m_info.name != dbInfo.name) {
        m_info.name = dbInfo.name;

        if (!setMeta(g_strDatabaseInfoSection, "Name", dbInfo.name))
            nErrors++;

        Q_EMIT databaseRename(kbGUID());
    }

    if (m_info.nPermission != dbInfo.nPermission) {
        m_info.nPermission = dbInfo.nPermission;

        if (!setMeta(g_strDatabaseInfoSection, "Permission", QString::number(dbInfo.nPermission)))
            nErrors++;

        Q_EMIT databasePermissionChanged(kbGUID());
    }


    // biz group info
    if (!dbInfo.bizGUID.isEmpty() && !dbInfo.bizName.isEmpty()) {
        bool bResetBiz = false;
        if (m_info.bizName != dbInfo.bizName) {
            m_info.bizName = dbInfo.bizName;

            if (!setMeta(g_strDatabaseInfoSection, "BizName", dbInfo.bizName))
                nErrors++;

            bResetBiz = true;
        }

        if (m_info.bizGUID != dbInfo.bizGUID) {
            m_info.bizGUID = dbInfo.bizGUID;

            if (!setMeta(g_strDatabaseInfoSection, "BizGUID", dbInfo.bizGUID))
                nErrors++;

            bResetBiz = true;
        }

        if (bResetBiz) {
            Q_EMIT databaseBizChanged(kbGUID());
        }
    }

    if (!setMeta(g_strDatabaseInfoSection, "KbGUID", kbGUID()))
        nErrors++;

    if (!setMeta(g_strDatabaseInfoSection, "Version", WIZ_DATABASE_VERSION))
        nErrors++;
    //
    if (!setMeta(g_strDatabaseInfoSection, "EncryptData", dbInfo.bEncryptData ? "1" : "0"))
        nErrors++;

    if (nErrors)
        return false;

    return true;
}


QString WizDatabase::getAccountPath() const
{
    Q_ASSERT(!m_strAccountFolderName.isEmpty());

    QString strPath = Utils::WizPathResolve::dataStorePath() + m_strAccountFolderName + "/";
    WizEnsurePathExists(strPath);

    return strPath;
}

QString WizDatabase::getAccountFolderName() const
{
    return m_strAccountFolderName;
}

QString WizDatabase::getDataPath() const
{
    QString strPath;

    if (m_bIsPersonal) {
        strPath = getAccountPath() + "data/";
    } else {
        strPath = getAccountPath() + "group/" + kbGUID() + "/";
    }

    WizEnsurePathExists(strPath);
    return strPath;
}

QString WizDatabase::getIndexFileName() const
{
    return getDataPath() + "index.db";
}

QString WizDatabase::getThumbFileName() const
{
    return getDataPath() + "wizthumb.db";
}

QString WizDatabase::getDocumentsDataPath() const
{
    QString strPath = getDataPath() + "notes/";
    WizEnsurePathExists(strPath);
    return  strPath;
}

QString WizDatabase::getAttachmentsDataPath() const
{
    QString strPath = getDataPath() + "attachments/";
    WizEnsurePathExists(strPath);
    return  strPath;
}

QString WizDatabase::getDocumentFileName(const QString& strGUID) const
{
    return getDocumentsDataPath() + "{" + strGUID + "}";
}

QString WizDatabase::getAttachmentFileName(const QString& strGUID)
{
    WIZDOCUMENTATTACHMENTDATA attach;
    attachmentFromGuid(strGUID, attach);
    //
    QString strOldFileName = getAttachmentsDataPath() + "{" + strGUID + "}";
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

QString WizDatabase::getAvatarPath() const
{
    QString strPath = getAccountPath() + "avatar/";
    WizEnsurePathExists(strPath);

    return strPath;
}

QString WizDatabase::getDefaultNoteLocation() const
{
    if (m_bIsPersonal)
        return LOCATION_DEFAULT;
    else
        return "/"+m_strUserId+"/";
}

QString WizDatabase::getDocumentAuthorAlias(const WIZDOCUMENTDATA& doc)
{
    if (!doc.strAuthor.isEmpty())
        return doc.strAuthor;

    WizDatabase* personDb = personalDatabase();
    if (!personDb)
        return QString();

    if (doc.strKbGUID.isEmpty() || doc.strKbGUID == personDb->kbGUID())
    {
        QString displayName;
        if (personDb->getUserDisplayName(displayName))
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

QString WizDatabase::getDocumentOwnerAlias(const WIZDOCUMENTDATA& doc)
{
    WizDatabase* personDb = personalDatabase();
    if (!personDb)
        return QString();

    QString strUserID = doc.strOwner;

    //NOTE: 用户可能使用手机号登录，此时owner为手机号，需要使用昵称
    if (!strUserID.contains('@') && strUserID == personDb->getUserId())
    {
        personDb->getUserDisplayName(strUserID);
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

bool WizDatabase::getUserName(QString& strUserName)
{
    strUserName = getMetaDef(g_strAccountSection, "UserName");
    return true;
}

bool WizDatabase::setUserName(const QString& strUserName)
{
    CString strOld;
    getUserName(strOld);
    if (!strOld.isEmpty()) {
        TOLOG("Can not set user name: user name exists!");
        return false;
    }

    if (!setMeta(g_strAccountSection, "UserName", strUserName)) {
        TOLOG("Failed to set user name while setUserName");
        return false;
    }

    return true;
}

bool WizDatabase::getUserDisplayName(QString &strDisplayName)
{
    strDisplayName = getMetaDef(g_strAccountSection, "DISPLAYNAME");
    return true;
}

QString WizDatabase::getUserAlias()
{
    WizDatabase* personDb = personalDatabase();
    if (!personDb)
        return QString();

    QString strUserGUID = personDb->getUserGuid();
    WIZBIZUSER bizUser;
    personDb->userFromGUID(kbGUID(), strUserGUID, bizUser);
    if (!bizUser.alias.isEmpty()) {
        return bizUser.alias;
    } else {
        QString strUserName;
        personDb->getUserDisplayName(strUserName);
        return strUserName;
    }

    return QString();
}

QString WizDatabase::getEncryptedPassword()
{
    return getMetaDef(g_strAccountSection, "Password");
}

bool WizDatabase::getPassword(CString& strPassword)
{
    if (!m_strPassword.isEmpty()) {
        strPassword = m_strPassword;
        return true;
    }

    bool bExists = false;
    if (!getMeta(g_strAccountSection, "Password", strPassword, "", &bExists)) {
        TOLOG("Failed to get password while getPassword");
        return false;
    }

    //if (strPassword.IsEmpty())
    //    return true;

    //strPassword = WizDecryptPassword(strPassword);

    return true;
}

bool WizDatabase::setPassword(const QString& strPassword, bool bSave)
{
    m_strPassword = strPassword;

    if (bSave) {
        if (!setMeta(g_strAccountSection, "Password", strPassword)) {
            TOLOG("Failed to set user password");
            return false;
        }
    }

    return true;
}

UINT WizDatabase::getPasswordFalgs()
{
    CString strFlags = getMetaDef(g_strAccountSection, "PasswordFlags", "");
    return (UINT)atoi(strFlags.toUtf8());
}

bool WizDatabase::setPasswordFalgs(UINT nFlags)
{
    return setMeta(g_strAccountSection, "PasswordFlags", WizIntToStr(int(nFlags)));
}

bool WizDatabase::getUserCert(QString& strN, QString& stre, QString& strd, QString& strHint)
{
    if (isGroup())
    {
        Q_ASSERT(!m_info.bizGUID.isEmpty());
        //
        WizDatabase* pDatabase = personalDatabase();
        QString key = QString(g_strCertSection) + "/" + m_info.bizGUID;
        strN = pDatabase->getMetaDef(key, "N");
        stre = pDatabase->getMetaDef(key, "E");
        strd = pDatabase->getMetaDef(key, "D");
        strHint = pDatabase->getMetaDef(key, "Hint");
    }
    else
    {
        strN = getMetaDef(g_strCertSection, "N");
        stre = getMetaDef(g_strCertSection, "E");
        strd = getMetaDef(g_strCertSection, "D");
        strHint = getMetaDef(g_strCertSection, "Hint");
    }

    if (strN.isEmpty() || stre.isEmpty() || strd.isEmpty())
        return false;

    return true;
}

bool WizDatabase::setUserCert(const QString& strN, const QString& stre, const QString& strd, const QString& strHint)
{
    if (isGroup())
    {
        Q_ASSERT(!m_info.bizGUID.isEmpty());
        //
        WizDatabase* pDatabase = personalDatabase();
        QString key = QString(g_strCertSection) + "/" + m_info.bizGUID;
        return pDatabase->setMeta(key, "N", strN) \
                && pDatabase->setMeta(key, "E", stre) \
                && pDatabase->setMeta(key, "D", strd) \
                && pDatabase->setMeta(key, "Hint", strHint);
    }
    else
    {
        return setMeta(g_strCertSection, "N", strN) \
                && setMeta(g_strCertSection, "E", stre) \
                && setMeta(g_strCertSection, "D", strd) \
                && setMeta(g_strCertSection, "Hint", strHint);
    }
}

bool WizDatabase::getUserInfo(WIZUSERINFO& userInfo)
{
    userInfo.strUserGUID = getMetaDef(g_strAccountSection, "GUID");
    userInfo.strDisplayName = getMetaDef(g_strAccountSection, "DisplayName");
    userInfo.strUserType = getMetaDef(g_strAccountSection, "UserType");
    userInfo.strUserLevelName = getMetaDef(g_strAccountSection, "UserLevelName");
    userInfo.nUserLevel = getMetaDef(g_strAccountSection, "UserLevel").toInt();
    userInfo.nUserPoints = getMetaDef(g_strAccountSection, "UserPoints").toInt();
    userInfo.strMywizEmail = getMetaDef(g_strAccountSection, "MywizMail");
    userInfo.tCreated = QDateTime::fromString(getMetaDef(g_strAccountSection, "DateSignUp"));
    userInfo.strKbServer = getMetaDef(g_strAccountSection, "KbServer");

    return true;
}

void WizDatabase::updateInvalidData()
{
    int nVersion = meta(USER_SETTINGS_SECTION, "AppVersion").toInt();
    if (nVersion < 202006)
    {
        setMeta(USER_SETTINGS_SECTION, "EditorBackgroundColor", "");
    }

    setMeta(USER_SETTINGS_SECTION, "AppVersion", QString::number(Utils::WizMisc::getVersionCode()));
}

bool WizDatabase::updateBizUser(const WIZBIZUSER& user)
{
    bool bRet = false;

    WIZBIZUSER userTemp;
    if (userFromGUID(user.kbGUID, user.userGUID, userTemp)) {
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


bool WizDatabase::updateMessage(const WIZMESSAGEDATA& msg)
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

bool WizDatabase::updateDeletedGuid(const WIZDELETEDGUIDDATA& data)
{
    bool bRet = false;

    QString strType = WIZOBJECTDATA::objectTypeToTypeString(data.eType);
    bool bExists = false;
    objectExists(data.strGUID, strType, bExists);
    if (!bExists)
        return true;    

    qDebug() << "delete object: " << strType << " guid: " << data.strGUID;

    bRet = deleteObject(data.strGUID, strType, false);

    if (!bRet) {
        Q_EMIT updateError("Failed to delete object: " + data.strGUID + " type: " + strType);
    }

    return bRet;
}

bool WizDatabase::updateTag(const WIZTAGDATA& data)
{
    bool bRet = false;

    WIZTAGDATA dataTemp;
    if (tagFromGuid(data.strGUID, dataTemp)) {
        bRet = modifyTagEx(data);
    } else {
        bRet = createTagEx(data);
    }

    if (!bRet) {
        Q_EMIT updateError("Failed to update tag: " + data.strName);
    }

    return bRet;
}

bool WizDatabase::updateStyle(const WIZSTYLEDATA& data)
{
    bool bRet = false;

    WIZSTYLEDATA dataTemp;
    if (styleFromGuid(data.strGUID, dataTemp)) {
        bRet = modifyStyleEx(data);
    } else {
        bRet = createStyleEx(data);
    }

    if (!bRet) {
        Q_EMIT updateError("Failed to update style: " + data.strName);
    }

    return bRet;
}

bool WizDatabase::updateDocument(const WIZDOCUMENTDATAEX& d)
{
    WIZDOCUMENTDATAEX data = d;
    //
    Q_ASSERT(data.nVersion != -1);

    bool bRet = false;

    WIZDOCUMENTDATAEX dataTemp;
    if (documentFromGuid(data.strGUID, dataTemp)) {
        if (dataTemp.nVersion == -1)
        {
            if (dataTemp.nDataChanged) //本地数据被修改了，则不覆盖
            {
                qDebug() << "local data changed, skip to overwrite: " << data.strTitle;
                return true;
            }
        }
        //
        if (data.nVersion == dataTemp.nVersion)
        {
#ifdef      QT_DEBUG
            qDebug() << "No changed: " << data.strTitle << ", skip it";
#endif
            return true;
        }

        data.nInfoChanged = dataTemp.nInfoChanged;
        bRet = modifyDocumentInfoEx(data);
        if (dataTemp.strDataMD5 != data.strDataMD5) {
            setObjectDataDownloaded(data.strGUID, "document", false);
        }
    } else {
        bRet = createDocumentEx(data);
    }

    if (!bRet) {
        Q_EMIT updateError("Failed to update document: " + data.strTitle);
    }

    WIZDOCUMENTDATA doc(data);

    if (!data.arrayTagGUID.empty()) {
        setDocumentTags(doc, data.arrayTagGUID, false);
    }

    return bRet;
}

bool WizDatabase::updateAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data)
{
    bool bRet = false;

    WIZDOCUMENTATTACHMENTDATAEX dataTemp;
    if (attachmentFromGuid(data.strGUID, dataTemp)) {
        bRet = modifyAttachmentInfoEx(data);

        bool changed = dataTemp.strDataMD5 != data.strDataMD5;
        if (changed) {
            setObjectDataDownloaded(data.strGUID, "attachment", false);
        }
    } else {
        bRet = createAttachmentEx(data);
        updateDocumentAttachmentCount(data.strDocumentGUID, false);
    }

    if (!bRet) {
        Q_EMIT updateError("Failed to update attachment: " + data.strName);
    }

    return bRet;
}

bool WizDatabase::updateTags(const CWizTagDataArray& arrayTag)
{
    if (arrayTag.empty())
        return true;

    qint64 nVersion = -1;

    bool bHasError = false;
    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        const WIZTAGDATA& tag = *it;

        Q_EMIT processLog("tag: " + tag.strName);

        if (!updateTag(tag)) {
            bHasError = true;
        }

        nVersion = qMax(nVersion, tag.nVersion);
    }

    if (!bHasError) {
        setObjectVersion(WIZTAGDATA::objectName(), nVersion);
    }

    return !bHasError;
}


bool WizDatabase::updateStyles(const CWizStyleDataArray& arrayStyle)
{
    if (arrayStyle.empty())
        return true;

    qint64 nVersion = -1;

    bool bHasError = false;
    CWizStyleDataArray::const_iterator it;
    for (it = arrayStyle.begin(); it != arrayStyle.end(); it++) {
        const WIZSTYLEDATA& data = *it;

        Q_EMIT processLog("style: " + data.strName);

        if (!updateStyle(data)) {
            bHasError = true;
        }

        nVersion = qMax(nVersion, data.nVersion);
    }

    if (!bHasError) {
        setObjectVersion(WIZSTYLEDATA::objectName(), nVersion);
    }

    return !bHasError;
}

bool WizDatabase::updateDocuments(const std::deque<WIZDOCUMENTDATAEX>& arrayDocument)
{
    if (arrayDocument.empty())
        return true;

    qint64 nVersion = -1;

    bool bHasError = false;
    std::deque<WIZDOCUMENTDATAEX>::const_iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        const WIZDOCUMENTDATAEX& data = *it;

        Q_EMIT processLog("note: " + data.strTitle);

        if (!updateDocument(data)) {
            bHasError = true;
        }

        nVersion = qMax(nVersion, data.nVersion);
    }

    if (!bHasError) {
        setObjectVersion(WIZDOCUMENTDATAEX::objectName(), nVersion);
    }

    return !bHasError;
}

bool WizDatabase::updateAttachments(const CWizDocumentAttachmentDataArray& arrayAttachment)
{
    if (arrayAttachment.empty())
        return true;

    qint64 nVersion = -1;

    bool bHasError = false;

    std::deque<WIZDOCUMENTATTACHMENTDATAEX>::const_iterator it;
    for (it = arrayAttachment.begin(); it != arrayAttachment.end(); it++) {
        const WIZDOCUMENTATTACHMENTDATAEX& data = *it;

        Q_EMIT processLog("attachment: " + data.strName);

        if (!updateAttachment(data)) {
            bHasError = true;
        }

        nVersion = qMax(nVersion, data.nVersion);
    }

    if (!bHasError) {
        setObjectVersion(WIZDOCUMENTATTACHMENTDATAEX::objectName(), nVersion);
    }

    emit attachmentsUpdated();

    return !bHasError;
}


bool WizDatabase::setDocumentFlags(const QString& strDocumentGuid, const QString& strFlags)
{
    return setDocumentParam(strDocumentGuid, TABLE_KEY_WIZ_DOCUMENT_PARAM_FLAGS, strFlags);
}

bool WizDatabase::updateDocumentData(WIZDOCUMENTDATA& data,
                                      const QString& strHtml,
                                      const QString& strURL,
                                      int nFlags,
                                      bool notifyDataModify /*= true*/)
{
    QString strProcessedHtml(strHtml);
    QString strResourcePath = GetResoucePathFromFile(strURL);
    if (!strResourcePath.isEmpty()) {
        QUrl urlResource = QUrl::fromLocalFile(strResourcePath);
        strProcessedHtml.replace(urlResource.toString(), "index_files/");
    }
    //
    if (isEncryptAllData())
        data.nProtected = 1;
    //
    WizDocument doc(*this, data);
    //
    CString strZipFileName = getDocumentFileName(data.strGUID);
    if (!data.nProtected) {
        bool bZip = ::WizHtml2Zip(strURL, strProcessedHtml, strResourcePath, nFlags, strZipFileName);
        if (!bZip) {
            return false;
        }
    } else {
        CString strTempFile = Utils::WizPathResolve::tempPath() + data.strGUID + "-decrypted";
        bool bZip = ::WizHtml2Zip(strURL, strProcessedHtml, strResourcePath, nFlags, strTempFile);
        if (!bZip) {
            return false;
        }

        if (!m_ziwReader->encryptFileToFile(strTempFile, strZipFileName)) {
            return false;
        }
    }

    setObjectDataDownloaded(data.strGUID, "document", true);

    return updateDocumentDataMD5(data, strZipFileName, notifyDataModify);
}


bool WizDatabase::updateDocumentDataWithFolder(WIZDOCUMENTDATA& data,
                                      const QString& strFolder,
                                      bool notifyDataModify /*= true*/)
{
    CString strZipFileName = getDocumentFileName(data.strGUID);
    if (!data.nProtected) {
        bool bZip = ::WizFolder2Zip(strFolder, strZipFileName);
        if (!bZip) {
            return false;
        }
    } else {
        CString strTempFile = Utils::WizPathResolve::tempPath() + data.strGUID + "-decrypted";
        bool bZip = ::WizFolder2Zip(strFolder, strTempFile);
        if (!bZip) {
            return false;
        }

        if (!m_ziwReader->encryptFileToFile(strTempFile, strZipFileName)) {
            return false;
        }
    }

    setObjectDataDownloaded(data.strGUID, "document", true);

    return updateDocumentDataMD5(data, strZipFileName, notifyDataModify);
}

void WizDatabase::clearUnusedImages(const QString& strHtml, const QString& strFilePath)
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

bool WizDatabase::updateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName, bool notifyDataModify /*= true*/)
{
    bool bRet = WizIndex::updateDocumentDataMD5(data, strZipFileName, notifyDataModify);

    updateDocumentAbstract(data.strGUID);

    return bRet;
}

bool WizDatabase::deleteObject(const QString& strGUID, const QString& strType, bool bLog)
{
    if (strGUID.isEmpty() || strType.isEmpty())
        return false;

    if (!isGroup())
    {
        bool objectExists = FALSE;
        if (-1 == getObjectLocalVersionEx(strGUID, strType, objectExists))
        {
            if (objectExists)
            {
                TOLOG2("[%1] object [%2] is modified on local, skip to delete it", strType, strGUID);
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
        if (tagFromGuid(strGUID, data)) {
            deleteTag(data, bLog, false);
        }
        return true;
    }
    else if (0 == strType.compare("style", Qt::CaseInsensitive))
    {
        WIZSTYLEDATA data;
        if (styleFromGuid(strGUID, data)) {
            return deleteStyle(data, bLog, false);
        }
        return true;
    }
    else if (0 == strType.compare("document", Qt::CaseInsensitive))
    {
        WIZDOCUMENTDATA data;
        if (documentFromGuid(strGUID, data)) {
            CString strZipFileName = getDocumentFileName(strGUID);
            if (WizPathFileExists(strZipFileName))
            {
                WizDeleteFile(strZipFileName);
            }
            return deleteDocument(data, bLog);
        }
        return true;
    }
    else if (0 == strType.compare("attachment", Qt::CaseInsensitive))
    {
        WIZDOCUMENTATTACHMENTDATA data;
        if (attachmentFromGuid(strGUID, data)) {
            CString strZipFileName = getAttachmentFileName(strGUID);
            if (WizPathFileExists(strZipFileName))
            {
                WizDeleteFile(strZipFileName);
            }
            return deleteAttachment(data, bLog, false);
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


bool WizDatabase::deleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data,
                                    bool bLog, bool bReset, bool updateAttachList)
{
    CString strFileName = getAttachmentFileName(data.strGUID);
    bool bRet = WizIndex::deleteAttachment(data, bLog, bReset, updateAttachList);
    if (WizPathFileExists(strFileName)) {
        ::WizDeleteFile(strFileName);
    }

    if (updateAttachList) {
        emit attachmentsUpdated();
    }

    return bRet;
}

bool WizDatabase::deleteGroupFolder(const WIZTAGDATA& data, bool bLog)
{
    CWizTagDataArray arrayChildTag;
    getChildTags(data.strGUID, arrayChildTag);
    foreach (const WIZTAGDATA& childTag, arrayChildTag)
    {
        deleteGroupFolder(childTag, bLog);
    }

    CWizDocumentDataArray arrayDocument;
    getDocumentsByTag(data, arrayDocument);

    for (WIZDOCUMENTDATAEX doc : arrayDocument)
    {
        WizDocument document(*this, doc);
        document.deleteToTrash();
    }

    return deleteTag(data, bLog);
}

bool WizDatabase::isDocumentModified(const CString& strGUID)
{
    return WizIndex::isObjectDataModified(strGUID, "document");
}

bool WizDatabase::isAttachmentModified(const CString& strGUID)
{
    return WizIndex::isObjectDataModified(strGUID, "attachment");
}

bool WizDatabase::isDocumentDownloaded(const CString& strGUID)
{
    return WizIndex::isObjectDataDownloaded(strGUID, "document");
}

bool WizDatabase::isAttachmentDownloaded(const CString& strGUID)
{
    return WizIndex::isObjectDataDownloaded(strGUID, "attachment");
}

bool WizDatabase::getAllObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayData, int nTimeLine)
{
    if (nTimeLine == -1)
        return true;
    //
    CWizDocumentDataArray arrayDocument;
    CWizDocumentAttachmentDataArray arrayAttachment;
    getNeedToBeDownloadedDocuments(arrayDocument);
    getNeedToBeDownloadedAttachments(arrayAttachment);
    arrayData.assign(arrayAttachment.begin(), arrayAttachment.end());
    arrayData.insert(arrayData.begin(), arrayDocument.begin(), arrayDocument.end());

    if (nTimeLine == -1) {
        arrayData.clear();
        return true;
    } else if (nTimeLine == 9999) {
        return true;
    } else {
        WizOleDateTime tNow = ::WizGetCurrentTime();
        size_t count = arrayData.size();
        for (intptr_t i = count - 1; i >= 0; i--)
        {
            WizOleDateTime t = arrayData[i].tTime;
            t = t.addDays(nTimeLine);
            if (t < tNow) {
                arrayData.erase(arrayData.begin() + i);
                continue;
            }

            WizDatabase* privateDB = personalDatabase();
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

bool WizDatabase::updateSyncObjectLocalData(const WIZOBJECTDATA& data)
{
    qDebug() << "update object data, name: " << data.strDisplayName << "guid: " << data.strObjectGUID;

    if (data.eObjectType == wizobjectDocumentAttachment)
    {
        if (!saveCompressedAttachmentData(data.strObjectGUID, data.arrayData))
        {
            Q_EMIT updateError("Failed to save attachment data: " + data.strDisplayName);
            return false;
        }
    }
    else if (data.eObjectType == wizobjectDocument)
    {
        WIZDOCUMENTDATA document;
        if (!documentFromGuid(data.strObjectGUID, document)) {
            qCritical() << "Update object data failed, can't find database record!\n";
            return false;
        }

        CString strFileName = getObjectFileName(data);
        if (!::WizSaveDataToFile(strFileName, data.arrayData))
        {
            Q_EMIT updateError("Failed to save document data: " + data.strDisplayName);
            return false;
        }

        Q_EMIT documentDataModified(document);
        updateDocumentAbstract(data.strObjectGUID);
        setDocumentSearchIndexed(data.strObjectGUID, false);
    } else {
        Q_ASSERT(0);
    }

    setObjectDataDownloaded(data.strObjectGUID, WIZOBJECTDATA::objectTypeToTypeString(data.eObjectType), true);

    return true;
}


CString WizDatabase::getObjectFileName(const WIZOBJECTDATA& data)
{
    if (data.eObjectType == wizobjectDocument)
        return getDocumentFileName(data.strObjectGUID);
    else if (data.eObjectType == wizobjectDocumentAttachment)
        return getAttachmentFileName(data.strObjectGUID);
    else
    {
        Q_ASSERT(false);
        return CString();
    }
}

bool WizDatabase::getAllRootLocations(const CWizStdStringArray& arrayAllLocation, \
                                       CWizStdStringArray& arrayLocation)
{
    std::set<QString> setRoot;

    CWizStdStringArray::const_iterator it;
    for (it = arrayAllLocation.begin(); it != arrayAllLocation.end(); it++) {
        setRoot.insert(getRootLocation(*it));
    }

    arrayLocation.assign(setRoot.begin(), setRoot.end());

    return true;
}

bool WizDatabase::getChildLocations(const CWizStdStringArray& arrayAllLocation, \
                                     const QString& strLocation, \
                                     CWizStdStringArray& arrayLocation)
{
    if (strLocation.isEmpty())
        return getAllRootLocations(arrayAllLocation, arrayLocation);

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

bool WizDatabase::isInDeletedItems(const CString& strLocation)
{
    return strLocation.startsWith(LOCATION_DELETED_ITEMS);
}

bool WizDatabase::getDocumentTitleStartWith(const QString& titleStart, int nMaxCount, CWizStdStringArray& arrayTitle)
{
    QString sql = QString("SELECT DISTINCT DOCUMENT_TITLE FROM WIZ_DOCUMENT WHERE "
                          "DOCUMENT_TITLE LIKE '" + titleStart + "%' LIMIT " + QString::number(nMaxCount) + ";");

//    CString strSQL;
//    strSQL.Format(_T("SELECT DISTINCT DOCUMENT_TITLE FROM WIZ_DOCUMENT WHERE "
//                     "DOCUMENT_TITLE LIKE 's%%' LIMIT s%;"),
//                  titleStart.utf16(),
//                  WizIntToStr(nMaxCount).utf16());

    return sqlToStringArray(sql, 0, arrayTitle);
}

QString WizDatabase::getDocumentLocation(const WIZDOCUMENTDATA& doc)
{
//    WIZDOCUMENTDATA doc;
//    if (!DocumentFromGUID(doc.strGUID, doc))
//        return QString();

    if (!isGroup())
        return doc.strLocation;

    CWizTagDataArray arrayTag;
    if (getDocumentTags(doc.strGUID, arrayTag))
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

bool WizDatabase::hasCert()
{
    QString n;
    QString e;
    QString d;
    QString hint;
    return getUserCert(n, e, d, hint);
}

bool WizDatabase::refreshCertFromServer()
{
    auto refreshCore = [&]{
        if (isGroup())
        {
            WizDatabase* db = personalDatabase();
            WizKMAccountsServer server;
            if (!server.login(db->getUserId(), db->getPassword()))
                return false;
            //
            QString strN, stre, strd, strHint;

            if (!server.getUserBizCert(bizGuid(), strN, stre, strd, strHint))
                return false;
            //
            return setUserCert(strN, stre, strd, strHint);
        }
        else
        {
            QString strN, stre, strd, strHint;
            //
            WizKMAccountsServer server;
            server.setUserInfo(WizToken::userInfo());
            if (!server.getCert(strN, stre, strd, strHint))
                return false;
            //
            return setUserCert(strN, stre, strd, strHint);
        }
    };

    bool ret = false;
    WizExecutingActionDialog::executeAction(tr("Downloading cert..."), WIZ_THREAD_NETWORK, [&]{
        ret = refreshCore();
    });
    //
    return ret;
}

bool WizDatabase::initCert(bool queryPassword)
{
    if (!hasCert())
    {
        if (isGroup())
        {
            if (!refreshCertFromServer())
            {
                if (!initBizCert())
                    return false;
            }
        }
        else
        {
            if (!refreshCertFromServer())
                return false;
        }
    }
    //
    if (!queryPassword)
        return true;
    //
    return QueryCertPassword();
}

bool WizDatabase::createDocumentAndInit(const CString& strHtml, \
                                         const CString& strHtmlUrl, \
                                         int nFlags, \
                                         const CString& strTitle, \
                                         const CString& strName, \
                                         const CString& strLocation, \
                                         const CString& strURL, \
                                         WIZDOCUMENTDATA& data)
{
    bool encrypt = isEncryptAllData();
    if (encrypt)
    {
        if (!initCert(true))
            return false;
        //
        if (!initZiwReaderForEncryption())
            return false;
        //
        data.nProtected = 1;
    }
    //
    bool bRet = false;
    try
    {
        beginUpdate();

        data.strKbGUID = kbGUID();
        data.strOwner = getUserId();
        bRet = createDocument(strTitle, strName, strLocation, strHtmlUrl, data.nProtected, data);
        if (bRet)
        {
            bRet = updateDocumentData(data, strHtml, strURL, nFlags);

            Q_EMIT documentCreated(data);
        }
    }
    catch (...)
    {

    }

    endUpdate();

    return bRet;
}

bool WizDatabase::createDocumentAndInit(const WIZDOCUMENTDATA& sourceDoc, const QByteArray& baData,
                                         const QString& strLocation, const WIZTAGDATA& tag, WIZDOCUMENTDATA& newDoc)
{
    bool encrypt = isEncryptAllData();
    if (encrypt)
    {
        if (!initCert(true))
            return false;
        //
        if (!initZiwReaderForEncryption())
            return false;
        //
        newDoc.nProtected = 1;
    }
    //
    bool bRet = false;
    try
    {
        beginUpdate();

        newDoc.strKbGUID = kbGUID();
        newDoc.strOwner = getUserId();        

        bRet = createDocument(sourceDoc.strTitle, sourceDoc.strName, strLocation, "", sourceDoc.strAuthor,
                              sourceDoc.strKeywords, sourceDoc.strType, getUserId(), sourceDoc.strFileType,
                              sourceDoc.strStyleGUID, 0, 0, newDoc.nProtected, newDoc);
        if (bRet)
        {
            if (newDoc.nProtected)
            {
                CString strZipFileName = getDocumentFileName(newDoc.strGUID);
                bRet = m_ziwReader->encryptDataToFile(baData, strZipFileName);
            }
            else
            {
                bRet = writeDataToDocument(newDoc.strGUID, baData);
            }
            //
            if (bRet)
            {
                setObjectDataDownloaded(newDoc.strGUID, "document", true);
                //
                CString strFileName = getDocumentFileName(newDoc.strGUID);
                bRet = updateDocumentDataMD5(newDoc, strFileName);
                //
            } else {
                bRet = false;
            }

            Q_EMIT documentCreated(newDoc);
        }
    }
    catch (...)
    {

    }

    endUpdate();

    if (!tag.strGUID.isEmpty()) {
        WizDocument doc(*this, newDoc);
        doc.addTag(tag);
    }

    return bRet;
}

bool WizDatabase::createDocumentByTemplate(const QString& templateZiwFile, const QString& strLocation,
                                            const WIZTAGDATA& tag, WIZDOCUMENTDATA& newDoc)
{
    QByteArray ba;
    if (!loadFileData(templateZiwFile, ba))
        return false;

    QString strTitle = Utils::WizMisc::extractFileTitle(templateZiwFile);
    if (newDoc.strTitle.isEmpty())
    {
        newDoc.strTitle = strTitle;
    }
    if (newDoc.strType.isEmpty()) {
        newDoc.strType = "TemplateNote";
    }

    return createDocumentAndInit(newDoc, ba, strLocation, tag, newDoc);
}

bool WizDatabase::addAttachment(const WIZDOCUMENTDATA& document, const CString& strFileName,
                                 WIZDOCUMENTATTACHMENTDATA& dataRet)
{
    dataRet.strKbGUID = document.strKbGUID;
    dataRet.strDocumentGUID = document.strGUID;

    CString strMD5 = ::WizMd5FileString(strFileName);
    if (!createAttachment(document.strGUID, Utils::WizMisc::extractFileName(strFileName), strFileName, "", strMD5, dataRet))
        return false;

    if (!::WizCopyFile(strFileName, getAttachmentFileName(dataRet.strGUID), false))
    {
        deleteAttachment(dataRet, false, true);
        return false;
    }

    setAttachmentDataDownloaded(dataRet.strGUID, true);
    updateDocumentAttachmentCount(document.strGUID);

    return true;
}

bool WizDatabase::deleteTagWithChildren(const WIZTAGDATA& data, bool bLog)
{
    CWizTagDataArray arrayChildTag;
    getChildTags(data.strGUID, arrayChildTag);
    foreach (const WIZTAGDATA& childTag, arrayChildTag)
    {
        deleteTagWithChildren(childTag, bLog);
    }

    deleteTag(data, bLog);

    return true;
}

bool WizDatabase::loadDocumentZiwData(const QString& strDocumentGUID, QByteArray& arrayData)
{
    CString strFileName = getDocumentFileName(strDocumentGUID);
    if (!WizPathFileExists(strFileName))
        return false;
    //
    return loadFileData(strFileName, arrayData);
}

bool WizDatabase::loadDocumentDecryptedData(const QString& strDocumentGUID, QByteArray& arrayData)
{
    CString strFileName = getDocumentFileName(strDocumentGUID);
    if (!WizPathFileExists(strFileName))
        return false;

    WIZDOCUMENTDATA document;
    if (!documentFromGuid(strDocumentGUID, document))
        return false;

    if (document.nProtected) {
        QString password = getCertPassword();
        if (password.isEmpty())
            return false;

        QString strDecryptedFileName = Utils::WizPathResolve::tempPath() + document.strGUID + "-decrypted";
        QFile::remove(strDecryptedFileName);

        if (!m_ziwReader->decryptFileToFile(strFileName, strDecryptedFileName)) {
            // force clear usercipher
            WizUserCertPassword::Instance().setPassword(bizGuid(), "");
            return false;
        }
        //
        return loadFileData(strDecryptedFileName, arrayData);
        //
    } else {
        //
        return loadFileData(strFileName, arrayData);
    }
}

bool WizDatabase::loadFileData(const QString& strFileName, QByteArray& arrayData)
{
    QFile file(strFileName);
    if (!file.open(QFile::ReadOnly))
        return false;

    arrayData = file.readAll();
    file.close();
    return !arrayData.isEmpty();
}

bool WizDatabase::writeDataToDocument(const QString& strDocumentGUID, const QByteArray& arrayData)
{
    CString strFileName = getDocumentFileName(strDocumentGUID);

    QFile file(strFileName);
    if (!file.open(QFile::WriteOnly | QIODevice::Truncate))
        return false;

    return (file.write(arrayData) != -1);
}

bool WizDatabase::loadAttachmentData(const CString& strDocumentGUID, QByteArray& arrayData)
{
    CString strFileName = getAttachmentFileName(strDocumentGUID);
    if (!WizPathFileExists(strFileName))
    {
        return false;
    }

    QFile file(strFileName);
    if (!file.open(QFile::ReadOnly))
        return false;

    arrayData = file.readAll();

    return !arrayData.isEmpty();
}

bool WizDatabase::loadCompressedAttachmentData(const QString& strGUID, QByteArray& arrayData)
{
    CString strFileName = getAttachmentFileName(strGUID);
    if (!WizPathFileExists(strFileName))
    {
        return false;
    }
    CString strTempZipFileName = Utils::WizPathResolve::tempPath() + WizIntToStr(WizGetTickCount()) + ".tmp";
    WizZipFile zip;
    if (!zip.open(strTempZipFileName))
    {
        Q_EMIT updateError("Failed to create temp zip file: " + strTempZipFileName);
        return false;
    }

    WIZDOCUMENTATTACHMENTDATA attach;
    attachmentFromGuid(strGUID, attach);
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
    //
    file.close();
    //
    QFile::remove(strTempZipFileName);

    return !arrayData.isEmpty();
}

bool WizDatabase::saveCompressedAttachmentData(const CString& strGUID, const QByteArray& arrayData)
{
    CString strTempZipFileName = Utils::WizPathResolve::tempPath() + WizIntToStr(WizGetTickCount()) + ".tmp";
    if (!WizSaveDataToFile(strTempZipFileName, arrayData))
    {
        Q_EMIT updateError("Failed to save attachment data to temp file: " + strTempZipFileName);
        return false;
    }
    WizUnzipFile zip;
    if (!zip.open(strTempZipFileName))
    {
        Q_EMIT updateError("Failed to open temp zip file: " + strTempZipFileName);
        return false;
    }

    CString strFileName = getAttachmentFileName(strGUID);
    if (!zip.extractFile(0, strFileName))
    {
        Q_EMIT updateError("Failed to extract attachment file: " + strFileName);
        return false;
    }

    zip.close();
    //
    QFile::remove(strTempZipFileName);
    //
    return true;
}

bool WizDatabase::updateDocumentAbstract(const QString& strDocumentGUID)
{
    CString strFileName = getDocumentFileName(strDocumentGUID);
    if (!WizPathFileExists(strFileName)) {
        return false;
    }

    WIZDOCUMENTDATA data;
    if (!documentFromGuid(strDocumentGUID, data)) {
        qDebug() << "[updateDocumentAbstract]invalide guid: " << strDocumentGUID;
        return false;
    }

    if (data.nProtected) {
        //check if note abstract is empty
        WIZABSTRACT abstract;
        padAbstractFromGuid(data.strGUID, abstract);
        if (abstract.image.isNull() && abstract.text.isEmpty())
        {
            return false;
        }
        else
        {
            WIZABSTRACT emptyAbstract;
            emptyAbstract.guid = data.strGUID;
            bool ret = updatePadAbstract(emptyAbstract);
            if (!ret) {
                Q_EMIT updateError("Failed to update note abstract!");
            }
            Q_EMIT documentAbstractModified(data);

            return ret;
        }
    }

    QString strTempFolder = Utils::WizPathResolve::tempPath() + data.strGUID + "-thumb/";
    // delete folder to clear unused images.
    ::WizDeleteAllFilesInFolder(strTempFolder);
    if (!documentToHtmlFile(data, strTempFolder)) {
        qDebug() << "[updateDocumentAbstract]decompress to temp failed, guid: "
                 << strDocumentGUID;
        return false;
    }
    CString strHtmlFileName = strTempFolder + "index.html";

    CString strHtmlTempPath = Utils::WizMisc::extractFilePath(strHtmlFileName);

    CString strHtml;
    ::WizLoadUnicodeTextFromFile(strHtmlFileName, strHtml);

    WIZABSTRACT abstract;
    abstract.guid = strDocumentGUID;

    WizHtmlToPlainText htmlConverter;
    htmlConverter.toText(strHtml, abstract.text);
    abstract.text = abstract.text.left(2000);

    CString strResourcePath = Utils::WizMisc::extractFilePath(strHtmlFileName) + "index_files/";
    CWizStdStringArray arrayImageFileName;
    ::WizEnumFiles(strResourcePath, "*.jpg;*.png;*.bmp;*.gif", arrayImageFileName, 0);
    if (!arrayImageFileName.empty())
    {
        CString strImageFileName;

        qint64 m = 0;
        CWizStdStringArray::const_iterator it;
        for (it = arrayImageFileName.begin(); it != arrayImageFileName.end(); it++) {
            CString strFileName = *it;
            //
            QString name = Utils::WizMisc::extractFileName(strFileName);
            if (name.startsWith("wizIcon"))
                continue;
            if (name.startsWith("checked"))
                continue;
            if (name.startsWith("unchecked"))
                continue;
            //
            qint64 size = Utils::WizMisc::getFileSize(strFileName);
            if (size > m)
            {
                //FIXME:此处是特殊处理，解析Html的CSS时候存在问题，目前暂不删除冗余图片。
                //缩略图需要判断当前图片确实被使用
                QString strName = Utils::WizMisc::extractFileName(strFileName);
                if (!strHtml.contains(strName))
                    continue;

                strImageFileName = strFileName;
                m = size;
            }
        }

        QImage img;
        if (!strImageFileName.isEmpty() && img.load(strImageFileName))
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

    bool ret = updatePadAbstract(abstract);
    if (!ret) {
        Q_EMIT updateError("Failed to update note abstract!");
    }

    ::WizDeleteFolder(strHtmlTempPath);

    Q_EMIT documentAbstractModified(data);

    return ret;
}

CString WizDatabase::getRootLocation(const CString& strLocation)
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

CString WizDatabase::getLocationName(const CString& strLocation)
{
    //FIXME:容错处理，如果路径的结尾不是 '/'，则增加该结尾符号
    QString strLoc = strLocation;
    if (strLoc.right(1) != "/")
        strLoc.append('/');

    int index = strLoc.lastIndexOf('/', strLocation.length() - 2);
    if (index == -1)
        return CString();

    CString str = strLoc.right(strLocation.length() - index - 1);

    str.trim('/');

    return str;
}

CString WizDatabase::getLocationDisplayName(const CString& strLocation)
{
    if (isRootLocation(strLocation)) {
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

    return getLocationName(strLocation);
}

bool WizDatabase::getDocumentsByTag(const WIZTAGDATA& tag, CWizDocumentDataArray& arrayDocument)
{
    return WizIndex::getDocumentsByTag("", tag, arrayDocument, false);
}

bool WizDatabase::loadUserCert()
{
    QString strN, stre, strEncryptedd, strHint;
    if (!getUserCert(strN, stre, strEncryptedd, strHint)) {
        TOLOG("can't get user cert from db");
        return false;
    }

    m_ziwReader->setRSAKeys(strN.toUtf8(), stre.toUtf8(), strEncryptedd.toUtf8(), strHint);
    return true;
}

bool WizDatabase::isEncryptAllData()
{
    if (isGroup())
        return m_info.bEncryptData;
    //
    return false;
}

bool WizDatabase::prepareBizCert()
{
    bool encrypt = isEncryptAllData();
    if (!encrypt)
        return true;
    //
    if (!initCert(true))
        return false;
    //
    if (!initZiwReaderForEncryption())
        return false;
    //
    return true;
}


bool WizDatabase::initBizCert()
{
    WizInitBizCertDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        WizUserCertPassword::Instance().setPassword(bizGuid(), dlg.userCertPassword());
        return true;
    }
    //
    return false;
}

void WizDatabase::clearCertPassword()
{
    WizUserCertPassword::Instance().clear();
}

QString WizDatabase::getCertPassword()
{
    QString password = WizUserCertPassword::Instance().getPassword(m_info.bizGUID);
    return password;
}
QString WizDatabase::getCertPasswordHint()
{
    QString n;
    QString e;
    QString hint;
    QString encrypted_d;
    getUserCert(n, e, encrypted_d, hint);
    //
    return hint;
}

bool WizDatabase::verifyCertPassword(QString password)
{
    QString n;
    QString e;
    QString hint;
    QString encrypted_d;
    getUserCert(n, e, encrypted_d, hint);
    //
    QString d;
    //
    if (WizAESDecryptBase64StringToString(password, encrypted_d, d)
            && d.length() > 0)
    {
        if (atoi(d.left(6).toUtf8()) != 0) {
            WizUserCertPassword::Instance().setPassword(m_info.bizGUID, password);
            return true;
        }
    }
    //
    if (!refreshCertFromServer())
        return false;
    //
    getUserCert(n, e, encrypted_d, hint);
    //
    if (WizAESDecryptBase64StringToString(password, encrypted_d, d)
            && d.length() > 0)
    {
        loadUserCert();
        if (atoi(d.left(6).toUtf8()) != 0) {
            WizUserCertPassword::Instance().setPassword(m_info.bizGUID, password);
            return true;
        }
    }
    //
    return false;
}


bool WizDatabase::QueryCertPassword()
{
    QString password = getCertPassword();
    if (!password.isEmpty())
        return true;
    //
    QString hint = getCertPasswordHint();
    //
    QString description;
    if (isGroup())
    {
        QString bizName = m_info.bizName;
        description = QObject::tr("Please enter the password of team cert: %1\nPassword hint: %2").arg(bizName).arg(hint);
    }
    else
    {
        description = QObject::tr("Please enter the password of cert:\nPassword hint: %1").arg(hint);
    }
    //
    WizLineInputDialog dlg(QObject::tr("Cert Password"), description, "", NULL, QLineEdit::Password);
    dlg.setOKHandler([&](QString password) {
        //
        if (verifyCertPassword(password))
            return true;
        //
        WizMessageBox::critical(&dlg, tr("Invalid password."));
        //
        return false;
    });
    //
    if (dlg.exec() == QDialog::Accepted)
        return true;
    //
    return false;
}

bool WizDatabase::documentToTempHtmlFile(const WIZDOCUMENTDATA& document,
                                          QString& strFullPathFileName)
{
    QString strTempFolder = Utils::WizPathResolve::tempPath() + document.strGUID + "/";
    ::WizEnsurePathExists(strTempFolder);

    if (!documentToHtmlFile(document, strTempFolder))
        return false;

    strFullPathFileName = strTempFolder + "index.html";
    return WizPathFileExists(strFullPathFileName);
}

bool WizDatabase::documentToHtmlFile(const WIZDOCUMENTDATA& document,
                                          const QString& strPath)
{
    //避免编辑的时候临时文件被删除导致图片等丢失
    //::WizDeleteAllFilesInFolder(strPath);
    ::WizEnsurePathExists(strPath);
    if (!extractZiwFileToFolder(document, strPath))
        return false;

    QString strTempHtmlFileName = strPath + "index.html";
    return WizPathFileExists(strTempHtmlFileName);
}

bool WizDatabase::exportToHtmlFile(const WIZDOCUMENTDATA& document, const QString& strIndexFileName)
{
    QString strTempPath = Utils::WizPathResolve::tempPath() + WizGenGUIDLowerCaseLetterOnly() + "/";
    if (!extractZiwFileToFolder(document, strTempPath))
        return false;

    QString strText;
    QString strTempHtmlFileName = strTempPath + "index.html";
    if (!WizLoadUnicodeTextFromFile(strTempHtmlFileName, strText))
        return false;
    //
    QString fileTitle = Utils::WizMisc::extractFileTitle(strIndexFileName);

    QString strResFolder = fileTitle.toHtmlEscaped() + "_files/";
    strText.replace("index_files/", strResFolder);

    if (!WizSaveUnicodeTextToUtf8File(strIndexFileName, strText))
        return false;

    QString strPath = Utils::WizMisc::extractFilePath(strIndexFileName);
    bool bCoverIfExists = true;
    if (!WizCopyFolder(strTempPath + "index_files/", strPath + strResFolder, bCoverIfExists))
        return false;
    //
    return true;
}

bool WizDatabase::extractZiwFileToFolder(const WIZDOCUMENTDATA& document,
                                              const QString& strFolder)
{
    CString strZipFileName = getDocumentFileName(document.strGUID);
    if (!WizPathFileExists(strZipFileName)) {
        return false;
    }
    //
    bool isProtected = WizZiwReader::isEncryptedFile(strZipFileName);

    if (!isProtected) {
        return WizUnzipFile::extractZip(strZipFileName, strFolder);
    }
    //
    QString password = WizUserCertPassword::Instance().getPassword(bizGuid());
    if (password.isEmpty()) {
        return false;
    }

    QString strDecryptedFileName = Utils::WizPathResolve::tempPath() + document.strGUID + "-decrypted";
    QFile::remove(strDecryptedFileName);

    if (!m_ziwReader->decryptFileToFile(strZipFileName, strDecryptedFileName)) {
        // force clear usercipher
        WizUserCertPassword::Instance().setPassword(bizGuid(), "");
        return false;
    }
    //
    return WizUnzipFile::extractZip(strDecryptedFileName, strFolder);
}

bool WizDatabase::encryptDocument(WIZDOCUMENTDATA& document)
{
    if (document.nProtected || document.strKbGUID != kbGUID())
        return false;

    //
    QString strFolder = Utils::WizPathResolve::tempDocumentFolder(document.strGUID);
    if (!extractZiwFileToFolder(document, strFolder))
    {
        TOLOG("extract ziw file failed!");
        return false;
    }
    //
    if (!initZiwReaderForEncryption())
        return false;

    //
    document.nProtected = 1;
    QString strFileName = getDocumentFileName(document.strGUID);
    if (compressFolderToZiwFile(document, strFolder, strFileName))
    {
        emit documentDataModified(document);
        return true;
    }

    return false;
}

bool WizDatabase::compressFolderToZiwFile(WIZDOCUMENTDATA &document, \
                                           const QString& strFileFolder)
{
    QString strFileName = getDocumentFileName(document.strGUID);
    return compressFolderToZiwFile(document, strFileFolder, strFileName);
}

bool WizDatabase::compressFolderToZiwFile(WIZDOCUMENTDATA& document, const QString& strFileFolder,
                                          const QString& strZiwFileName)
{
    QFile::remove(strZiwFileName);

    WizDocument doc(*this, document);
    //
    if (!document.nProtected)
    {
        bool bZip = ::WizFolder2Zip(strFileFolder, strZiwFileName);
        if (!bZip)
            return false;
    }
    else
    {
        CString strTempFile = Utils::WizPathResolve::tempPath() + document.strGUID + "-decrypted";
        bool bZip = ::WizFolder2Zip(strFileFolder, strTempFile);
        if (!bZip)
            return false;

        if (!m_ziwReader->encryptFileToFile(strTempFile, strZiwFileName))
            return false;
    }

    setObjectDataDownloaded(document.strGUID, "document", true);

    /*不需要将笔记modified信息通知关联内容.此前页面显示已是最新,不需要relaod.如果relaod较大笔记
    可能会造成页面闪烁*/
    bool notify = false;
    return updateDocumentDataMD5(document, strZiwFileName, notify);
}

bool WizDatabase::cancelDocumentEncryption(WIZDOCUMENTDATA& document)
{
    if (!document.nProtected || kbGUID() != document.strKbGUID)
        return false;

    //
    if (!initZiwReaderForEncryption())
        return false;

    QString strFolder = Utils::WizPathResolve::tempDocumentFolder(document.strGUID);
    if (!extractZiwFileToFolder(document, strFolder))
    {
        TOLOG("extract ziw file failed!");
        return false;
    }

    //
    document.nProtected = 0;
    QString strFileName = getDocumentFileName(document.strGUID);
    if (compressFolderToZiwFile(document, strFolder, strFileName))
    {
        emit documentDataModified(document);
        return true;
    }

    return false;
}

bool WizDatabase::isFileAccessible(const WIZDOCUMENTDATA& document)
{
    CString strZipFileName = getDocumentFileName(document.strGUID);
    if (!WizPathFileExists(strZipFileName)) {
        return false;
    }

    if (document.nProtected) {
        if (getCertPassword().isEmpty()) {
            return false;
        }

        if (!m_ziwReader->isFileAccessible(strZipFileName)) {
            return false;
        }
    }

    return true;
}


QObject* WizDatabase::GetFolderByLocation(const QString& strLocation, bool create)
{
    Q_UNUSED(create);

    return new WizFolder(*this, strLocation);
}

void WizDatabase::onAttachmentModified(const QString strKbGUID, const QString& strGUID,
                                        const QString& strFileName, const QString& strMD5, const QDateTime& dtLastModified)
{
    Q_UNUSED(strFileName);

    WizDatabase& db = WizDatabaseManager::instance()->db(strKbGUID);
    WIZDOCUMENTATTACHMENTDATA attach;
    if (db.attachmentFromGuid(strGUID, attach))
    {
        if (strMD5 != attach.strDataMD5)
        {
            attach.strDataMD5 = strMD5;
            attach.nVersion = -1;
            TOLOG("[Edit] attachment data modified");
        }
        attach.tDataModified = dtLastModified;
        attach.tInfoModified = WizGetCurrentTime();
        attach.strInfoMD5 = calDocumentAttachmentInfoMD5(attach);

        db.modifyAttachmentInfoEx(attach);
    }
}

bool WizDatabase::tryAccessDocument(const WIZDOCUMENTDATA &doc)
{
    if (doc.nProtected) {
        //
        if (!loadUserCert())
            return false;
        //
        if (!QueryCertPassword())
            return false;
        //
        if (!isFileAccessible(doc)) {
            QMessageBox::information(0, tr("Info"), tr("password error!"));
            //
            WizUserCertPassword::Instance().setPassword(bizGuid(), "");
            //
            return false;
        }
    }
    return true;
}


QObject* WizDatabase::GetDeletedItemsFolder()
{
    return new WizFolder(*this, getDeletedItemsLocation());
}

//QObject* CWizDatabase::DocumentFromGUID(const QString& strGUID)
//{
//    WIZDOCUMENTDATA data;
//    if (!DocumentFromGUID(strGUID, data))
//        return NULL;

//    CWizDocument* pDoc = new CWizDocument(*this, data);
//    return pDoc;
//}




class WizDocumentDataMutexes
{
    QMutex m_globalLocker;
    std::map<QString, QMutex*> m_lockers;

    QMutex* getDocumentMutexesCore(QString docGuid)
    {
        QMutexLocker locker(&m_globalLocker);
        auto it = m_lockers.find(docGuid);
        if (it != m_lockers.end()) {
            return it->second;
        }
        //
        QMutex* mutex = new QMutex();
        m_lockers[docGuid] = mutex;
        return mutex;
    }
    //
public:
    static QMutex* getDocumentMutexes(QString docGuid) {
        static WizDocumentDataMutexes g;
        return g.getDocumentMutexesCore(docGuid);
    }
};

WizDocumentDataLocker::WizDocumentDataLocker(QString docGuid)
{
#ifdef QT_DEBUG
    m_docGuid = docGuid;
    DEBUG_TOLOG1("try access doc: %1", docGuid);
#endif
    //
    m_mutex = WizDocumentDataMutexes::getDocumentMutexes(docGuid);
    m_mutex->lock();
    //
#ifdef QT_DEBUG
    DEBUG_TOLOG1("begin access doc: %1", docGuid);
#endif
}
WizDocumentDataLocker::~WizDocumentDataLocker()
{
#ifdef QT_DEBUG
    DEBUG_TOLOG1("end access doc: %1", m_docGuid);
#endif
    //
    m_mutex->unlock();
}
