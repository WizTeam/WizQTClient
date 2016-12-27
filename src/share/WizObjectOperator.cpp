#include "WizObjectOperator_p.h"
#include "WizObjectOperator.h"
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include "share/WizMessageBox.h"
#include "share/WizSettings.h"
#include "share/WizThreads.h"
#include "sync/WizKMSync.h"
#include "WizProgressDialog.h"
#include "WizObjectDataDownloader.h"
#include "WizDatabaseManager.h"
#include "WizDatabase.h"
#include "WizLineInputDialog.h"

WizDocumentOperatorPrivate::WizDocumentOperatorPrivate(OperatorData* data, QObject* parent)
    : QObject(parent)
    , m_data(data)
    , m_stop(false)
    , m_totoalCount(0)
    , m_counter(0)
{
}

WizDocumentOperatorPrivate::~WizDocumentOperatorPrivate()
{
    if (m_data)
    {
        delete m_data;
        m_data = nullptr;
    }
}

void WizDocumentOperatorPrivate::bindSignalsToProgressDialog(WizProgressDialog* progress)
{
    QObject::connect(this, SIGNAL(progress(int,int)), progress, SLOT(setProgress(int,int)));
    QObject::connect(this, SIGNAL(newAction(QString)), progress, SLOT(setActionString(QString)));
    QObject::connect(this, SIGNAL(finished()), progress, SLOT(accept()));
    QObject::connect(progress, SIGNAL(stopRequest()), this, SLOT(stop()), Qt::DirectConnection);
}

void WizDocumentOperatorPrivate::stop()
{
    m_stop = true;
}

void WizDocumentOperatorPrivate::copyDocumentToPersonalFolder()
{
    int nCounter = 0;
    CWizDocumentDataArray::iterator it;
    for (it = m_data->arrayDocument.begin(); it != m_data->arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        copyDocumentToPersonalFolder(doc);
        nCounter ++;
        emit progress(m_data->arrayDocument.size(), nCounter);
    }

    emit finished();
}

void WizDocumentOperatorPrivate::copyDocumentToGroupFolder()
{
    int nCounter = 0;
    CWizDocumentDataArray::iterator it;
    for (it = m_data->arrayDocument.begin(); it != m_data->arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        copyDocumentToGroupFolder(doc);
        nCounter ++;
        emit progress(m_data->arrayDocument.size(), nCounter);
    }

    emit finished();
}

void WizDocumentOperatorPrivate::moveDocumentToPersonalFolder()
{
    int nCounter = 0;
    CWizDocumentDataArray::iterator it;
    for (it = m_data->arrayDocument.begin(); it != m_data->arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        moveDocumentToPersonalFolder(doc);
        nCounter ++;
        emit progress(m_data->arrayDocument.size(), nCounter);
    }

    emit finished();
}

void WizDocumentOperatorPrivate::moveDocumentToGroupFolder()
{
    int nCounter = 0;
    CWizDocumentDataArray::iterator it;
    for (it = m_data->arrayDocument.begin(); it != m_data->arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        moveDocumentToGroupFolder(doc);
        nCounter ++;
        emit progress(m_data->arrayDocument.size(), nCounter);
    }

    emit finished();
}

void WizDocumentOperatorPrivate::deleteDocuments()
{
    //NOTE: 这个地方使用引用或者从主线程传递值过来，会造资源访问问题。使用单例获取地址
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();

    m_totoalCount = m_data->arrayDocument.size();
    m_counter = 0;
    for (WIZDOCUMENTDATA doc : m_data->arrayDocument)
    {
        emit newAction(tr("Delete note %1").arg(doc.strTitle));
        WizDatabase& db = dbMgr->db(doc.strKbGUID);
        WizDocument document(db, doc);
        document.Delete();
        emit progress(m_totoalCount, m_counter);

        if (m_stop)
            break;
    }

    emit finished();
}

void WizDocumentOperatorPrivate::copyPersonalFolderToPersonalDB()
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    m_totoalCount = documentCount(dbMgr->db(), m_data->sourceFolder);

    //
    QString sourceFolderName = WizDatabase::getLocationName(m_data->sourceFolder);
    QString uniqueFolder = getUniqueFolderName(m_data->targetFolder, sourceFolderName);
    if (m_data->combineFolders)
    {
        uniqueFolder.clear();
    }

    copyPersonalFolderToPersonalDB(m_data->sourceFolder, m_data->targetFolder, uniqueFolder);

    emit finished();
}

void WizDocumentOperatorPrivate::copyPersonalFolderToGroupDB()
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    m_totoalCount = documentCount(dbMgr->db(), m_data->sourceFolder);

    //    
    QString targetTagName = getUniqueFolderName(m_data->targetTag, m_data->sourceFolder, m_data->combineFolders);
    copyPersonalFolderToGroupDB(m_data->sourceFolder, m_data->targetTag, targetTagName);

    emit finished();
}

void WizDocumentOperatorPrivate::copyGroupFolderToPersonalDB()
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    m_totoalCount = documentCount(dbMgr->db(m_data->sourceTag.strKbGUID), m_data->sourceTag);
    //
    QString uniqueFolder = getUniqueFolderName(m_data->targetFolder, m_data->sourceTag.strName);
    if (uniqueFolder != m_data->sourceTag.strName && !m_data->combineFolders)
    {
        m_data->sourceTag.strName = uniqueFolder;
    }

    copyGroupFolderToPersonalDB(m_data->sourceTag, m_data->targetFolder);

    emit finished();
}

void WizDocumentOperatorPrivate::copyGroupFolderToGroupDB()
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    m_totoalCount = documentCount(dbMgr->db(m_data->sourceTag.strKbGUID), m_data->sourceTag);

    m_data->sourceTag.strName = getUniqueFolderName(m_data->targetTag, m_data->sourceTag, m_data->combineFolders);
    copyGroupFolderToGroupDB(m_data->sourceTag, m_data->targetTag);

    emit finished();
}

void WizDocumentOperatorPrivate::moveGroupFolderToPersonalDB()
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    m_totoalCount = documentCount(dbMgr->db(m_data->sourceTag.strKbGUID), m_data->sourceTag);

    //
    QString uniqueFolder = getUniqueFolderName(m_data->targetFolder, m_data->sourceTag.strName);
    if (uniqueFolder != m_data->sourceTag.strName && !m_data->combineFolders)
    {
        m_data->sourceTag.strName = uniqueFolder;
    }

    moveGroupFolderToPersonalDB(m_data->sourceTag, m_data->targetFolder);

    emit finished();
}

void WizDocumentOperatorPrivate::moveGroupFolderToGroupDB()
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    m_totoalCount = documentCount(dbMgr->db(m_data->sourceTag.strKbGUID), m_data->sourceTag);

    m_data->sourceTag.strName = getUniqueFolderName(m_data->targetTag, m_data->sourceTag, m_data->combineFolders, m_data->sourceTag.strGUID);
    moveGroupFolderToGroupDB(m_data->sourceTag, m_data->targetTag);

    emit finished();
}


void WizDocumentOperatorPrivate::movePersonalFolderToGroupDB()
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    m_totoalCount = documentCount(dbMgr->db(), m_data->sourceFolder);

    //    
    QString targetTagName = getUniqueFolderName(m_data->targetTag, m_data->sourceFolder, m_data->combineFolders);
    movePersonalFolderToGroupDB(m_data->sourceFolder, m_data->targetTag, targetTagName);

    emit finished();
}

void WizDocumentOperatorPrivate::copyDocumentToPersonalFolder(const WIZDOCUMENTDATA& doc)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    WizDatabase& sourceDb = dbMgr->db(doc.strKbGUID);
    WizDatabase& targetDb = dbMgr->db();
    WizFolder folder(targetDb, m_data->targetFolder);
    WizDocument wizDoc(sourceDb, doc);
    QString newDocGUID;
    if (!wizDoc.copyTo(targetDb, &folder, m_data->keepDocTime, m_data->keepTag, newDocGUID))
        return;

    if (!sourceDb.isGroup() && doc.nProtected == 1)
    {
        WIZDOCUMENTDATA newDoc;
        if (targetDb.documentFromGuid(newDocGUID, newDoc))
        {
            sourceDb.encryptDocument(newDoc);
        }
    }
}

void WizDocumentOperatorPrivate::copyDocumentToGroupFolder(const WIZDOCUMENTDATA& doc)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    WizDocument wizDoc(dbMgr->db(doc.strKbGUID), doc);
    wizDoc.copyTo(dbMgr->db(m_data->targetTag.strKbGUID), m_data->targetTag, m_data->keepDocTime);
}

void WizDocumentOperatorPrivate::moveDocumentToPersonalFolder(const WIZDOCUMENTDATA& doc)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    WizFolder folder(dbMgr->db(), m_data->targetFolder);
    WizDocument wizDoc(dbMgr->db(doc.strKbGUID), doc);
    wizDoc.moveTo(dbMgr->db(), &folder);
}

void WizDocumentOperatorPrivate::moveDocumentToGroupFolder(const WIZDOCUMENTDATA& doc)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    
    WizDocument wizDoc(dbMgr->db(doc.strKbGUID), doc);
    wizDoc.moveTo(dbMgr->db(m_data->targetTag.strKbGUID), m_data->targetTag);
}


void WizDocumentOperatorPrivate::copyPersonalFolderToPersonalDB(const QString& childFolder,
                                                          const QString& targetParentFolder, const QString& targetFolderName)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    WizDatabase& db = dbMgr->db();
    CWizDocumentDataArray arrayDocument;
    db.getDocumentsByLocation(childFolder, arrayDocument, false);
    //
    QString folderName = targetFolderName.isEmpty() ? db.getLocationName(childFolder) : targetFolderName;
    QString targetFolder = targetParentFolder + folderName + "/";
    m_data->targetFolder = targetFolder;

    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        copyDocumentToPersonalFolder(doc);
        m_counter ++;
        emit progress(m_totoalCount, m_counter);
    }

    if (m_stop)
        return;

    CWizStdStringArray arrayFolder;
    db.getAllLocations(arrayFolder);
    CWizStdStringArray arrayChild;
    if (db.getChildLocations(arrayFolder, childFolder, arrayChild))
    {
        for (QString newChildFolder : arrayChild)
        {
            copyPersonalFolderToPersonalDB(newChildFolder, targetFolder, "");
            if (m_stop)
                return;
        }
    }
}

void WizDocumentOperatorPrivate::copyPersonalFolderToGroupDB(const QString& childFolder,
                                                       const WIZTAGDATA& targetParentTag, const QString& targetTagName)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    WizDatabase& db = dbMgr->db();
    WizDatabase& targetDB = dbMgr->db(targetParentTag.strKbGUID);
    QString tagName = targetTagName.isEmpty() ? db.getLocationName(childFolder) : targetTagName;
    WIZTAGDATA targetTag;
    targetDB.createTag(targetParentTag.strGUID, tagName, "", targetTag);
    m_data->targetTag = targetTag;
    //
    CWizDocumentDataArray arrayDocument;
    db.getDocumentsByLocation(childFolder, arrayDocument, false);

    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        copyDocumentToGroupFolder(doc);
        m_counter ++;
        emit progress(m_totoalCount, m_counter);
    }

    if (m_stop)
        return;


    CWizStdStringArray arrayFolder;
    db.getAllLocations(arrayFolder);
    CWizStdStringArray arrayChild;
    if (db.getChildLocations(arrayFolder, childFolder, arrayChild))
    {
        for (QString newChildFolder : arrayChild)
        {
            copyPersonalFolderToGroupDB(newChildFolder, targetTag, "");
            if (m_stop)
                return;
        }
    }
}

void WizDocumentOperatorPrivate::copyGroupFolderToPersonalDB(const WIZTAGDATA& childFolder, const QString& targetParentFolder)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    WizDatabase& groupDB = dbMgr->db(childFolder.strKbGUID);
    CWizDocumentDataArray arrayDocument;
    groupDB.getDocumentsByTag(childFolder, arrayDocument);
    QString targetFolder = targetParentFolder + childFolder.strName + "/";
    m_data->targetFolder = targetFolder;

    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        copyDocumentToPersonalFolder(doc);
        m_counter ++;
        emit progress(m_totoalCount, m_counter);
    }

    if (m_stop)
        return;

    //
    CWizTagDataArray arrayTag;
    groupDB.getChildTags(childFolder.strGUID, arrayTag);
    for (WIZTAGDATA childTag : arrayTag)
    {
        copyGroupFolderToPersonalDB(childTag, targetFolder);
    }
}

void WizDocumentOperatorPrivate::copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    WizDatabase& targetDB = dbMgr->db(targetFolder.strKbGUID);
    WIZTAGDATA targetTag;
    targetDB.createTag(targetFolder.strGUID, sourceFolder.strName, sourceFolder.strDescription, targetTag);
    m_data->targetTag = targetTag;

    //
    WizDatabase& sourceDB = dbMgr->db(sourceFolder.strKbGUID);
    CWizDocumentDataArray arrayDocument;
    sourceDB.getDocumentsByTag(sourceFolder, arrayDocument);
    //
    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        copyDocumentToGroupFolder(doc);
        m_counter ++;
        emit progress(m_totoalCount, m_counter);
    }

    if (m_stop)
        return;

    //
    CWizTagDataArray arrayTag;
    sourceDB.getChildTags(sourceFolder.strGUID, arrayTag);
    for (WIZTAGDATA childTag : arrayTag)
    {
        copyGroupFolderToGroupDB(childTag, targetTag);
    }
}

void WizDocumentOperatorPrivate::moveGroupFolderToPersonalDB(const WIZTAGDATA& childFolder, const QString& targetParentFolder)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    WizDatabase& groupDB = dbMgr->db(childFolder.strKbGUID);
    CWizDocumentDataArray arrayDocument;
    groupDB.getDocumentsByTag(childFolder, arrayDocument);
    QString targetFolder = targetParentFolder + childFolder.strName + "/";
    m_data->targetFolder = targetFolder;
    //
    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Move note %1").arg(doc.strTitle));
        moveDocumentToPersonalFolder(doc);
        m_counter ++;
        emit progress(m_totoalCount, m_counter);
    }

    if (m_stop)
        return;

    //
    CWizTagDataArray arrayTag;
    groupDB.getChildTags(childFolder.strGUID, arrayTag);
    for (WIZTAGDATA childTag : arrayTag)
    {
        moveGroupFolderToPersonalDB(childTag, targetFolder);
    }

    // remove tag at last  if tag is empty
    arrayDocument.clear();
    groupDB.getDocumentsByTag(childFolder, arrayDocument);
    arrayTag.clear();
    groupDB.getChildTags(childFolder.strGUID, arrayTag);
    if (arrayDocument.size() == 0 && arrayTag.size() == 0)
    {
        groupDB.deleteTag(childFolder, true);
        emit newAction(tr("Delete folder %1").arg(childFolder.strName));
    }
}

void WizDocumentOperatorPrivate::moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetParentTag)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    WizDatabase& sourceDB = dbMgr->db(sourceFolder.strKbGUID);
    if (sourceFolder.strKbGUID == targetParentTag.strKbGUID)
    {
        qDebug() << "folder in same db, just modify parent tag guid";
        WIZTAGDATA sourceTag = sourceFolder;
        sourceTag.strParentGUID = targetParentTag.strGUID;
        sourceDB.modifyTag(sourceTag);
    }
    else
    {
        WizDatabase& targetDB = dbMgr->db(targetParentTag.strKbGUID);
        WIZTAGDATA targetTag;
        targetDB.createTag(targetParentTag.strGUID, sourceFolder.strName, sourceFolder.strDescription, targetTag);
        m_data->targetTag = targetTag;
        //
        CWizDocumentDataArray arrayDocument;
        sourceDB.getDocumentsByTag(sourceFolder, arrayDocument);
        //
        CWizDocumentDataArray::iterator it;
        for (it = arrayDocument.begin(); it != arrayDocument.end() && !m_stop; it++)
        {
            const WIZDOCUMENTDATA& doc = *it;
            emit newAction(tr("Move note %1").arg(doc.strTitle));
            moveDocumentToGroupFolder(doc);
            m_counter ++;
            emit progress(m_totoalCount, m_counter);
        }

        if (m_stop)
            return;

        //
        CWizTagDataArray arrayTag;
        sourceDB.getChildTags(sourceFolder.strGUID, arrayTag);
        for (WIZTAGDATA childTag : arrayTag)
        {
            moveGroupFolderToGroupDB(childTag, targetTag);
        }

        // remove tag at last  if tag is empty
        arrayDocument.clear();
        sourceDB.getDocumentsByTag(sourceFolder, arrayDocument);
        arrayTag.clear();
        sourceDB.getChildTags(sourceFolder.strGUID, arrayTag);
        if (arrayDocument.size() == 0 && arrayTag.size() == 0)
        {
            sourceDB.deleteTag(sourceFolder, true);
        }
    }
}


void WizDocumentOperatorPrivate::movePersonalFolderToGroupDB(const QString& sourceFolder,
                                                       const WIZTAGDATA& targetParentTag, const QString& targetTagName)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    WizDatabase& db = dbMgr->db();
    WizDatabase& targetDB = dbMgr->db(targetParentTag.strKbGUID);
    QString tagName = targetTagName.isEmpty() ? db.getLocationName(sourceFolder) : targetTagName;
    WIZTAGDATA targetTag;
    targetDB.createTag(targetParentTag.strGUID, tagName, "", targetTag);
    m_data->targetTag = targetTag;
    //
    CWizDocumentDataArray arrayDocument;
    db.getDocumentsByLocation(sourceFolder, arrayDocument, false);
    //
    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Move note %1").arg(doc.strTitle));
        moveDocumentToGroupFolder(doc);
        m_counter ++;
        emit progress(m_totoalCount, m_counter);
    }

    if (m_stop)
        return;
    //
    CWizStdStringArray arrayFolder;
    db.getAllLocations(arrayFolder);
    CWizStdStringArray arrayChild;
    if (db.getChildLocations(arrayFolder, sourceFolder, arrayChild))
    {
        for (QString childFolder : arrayChild)
        {
            movePersonalFolderToGroupDB(childFolder, targetTag, "");
        }
    }

    //  check if move sucessfull
    arrayDocument.clear();
    db.getDocumentsByLocation(sourceFolder, arrayDocument, true);
    if (arrayDocument.size() == 0)
    {
        db.deleteExtraFolder(sourceFolder);
    }
}

int documentCount(WizDatabase& db, const QString& personalFolder)
{
    int count = 0;
    db.getDocumentsCountByLocation(personalFolder, count, true);
    return count;
}

int documentCount(WizDatabase& db, const WIZTAGDATA& groupFolder)
{
    int count = 0;
    CWizTagDataArray arrayTag;
    db.getAllChildTags(groupFolder.strGUID, arrayTag);
    arrayTag.push_back(groupFolder);
    std::map<CString, int> mapTagDocumentCount;
    db.getAllTagsDocumentCount(mapTagDocumentCount);
    for (WIZTAGDATA tag : arrayTag)
    {
       std::map<CString, int>::const_iterator pos =  mapTagDocumentCount.find(tag.strGUID);
       if (pos != mapTagDocumentCount.end())
       {
           count += pos->second;
       }
    }

    return count;
}

void WizDocumentOperatorPrivate::combineSameNameGroupFolder(const WIZTAGDATA& parentTag, const WIZTAGDATA& childTag)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();

    WizDatabase& targetDB = dbMgr->db(parentTag.strKbGUID);
    CWizTagDataArray arrayTag;
    WIZTAGDATA sameNameBrother;
    if (targetDB.getChildTags(parentTag.strGUID, arrayTag))
    {
        for (WIZTAGDATA tag : arrayTag)
        {
            if (tag.strGUID != childTag.strGUID && tag.strName == childTag.strName)
            {
                sameNameBrother = tag;
                break;
            }
        }
    }
    // combine same name folders
    if (!sameNameBrother.strGUID.isEmpty())
    {
        if (m_data->combineFolders)
        {
            CWizDocumentDataArray arrayDocument;
            targetDB.getDocumentsByTag(childTag, arrayDocument);
            //
            m_data->targetTag = sameNameBrother;
            CWizDocumentDataArray::iterator it;
            for (it = arrayDocument.begin(); it != arrayDocument.end(); it++)
            {
                const WIZDOCUMENTDATA& doc = *it;
                moveDocumentToGroupFolder(doc);
            }

            //
            CWizTagDataArray arrayChild;
            targetDB.getChildTags(childTag.strGUID, arrayChild);
            for (WIZTAGDATA tag : arrayChild)
            {
                tag.strParentGUID = sameNameBrother.strGUID;
                targetDB.modifyTag(tag);
            }
            targetDB.deleteTag(childTag, false);
        }
        else
        {
            WIZTAGDATA tag = childTag;
            tag.strName = getUniqueTagName(parentTag, childTag);
            targetDB.modifyTag(tag);
        }
    }
}

QString WizDocumentOperatorPrivate::getUniqueTagName(const WIZTAGDATA& parentTag, const WIZTAGDATA& tag)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();

    QString name = tag.strName;
    int nRepeat = 0;
    CWizTagDataArray arrayTag;
    if(!dbMgr->db(parentTag.strKbGUID).getChildTags(parentTag.strGUID, arrayTag))
        return "";

    while (true)
    {
        if (nRepeat > 0)
        {
            name = tag.strName + "_" + QString::number(nRepeat);
        }
        bool continueLoop = false;
        CWizTagDataArray::const_iterator it;
        for (it = arrayTag.begin(); it != arrayTag.end(); it++)
        {
            WIZTAGDATA brother = *it;
            if (brother.strGUID != tag.strGUID && brother.strName == name)
            {
                nRepeat ++;
                continueLoop = true;
                break;
            }
        }
        if (!continueLoop)
            break;
    }
    return name;
}

QString WizDocumentOperatorPrivate::getUniqueFolderName(const QString& parentLocation, const QString& locationName)
{
    QString name = locationName;
    int nRepeat = 0;
    WizDatabase& db = WizDatabaseManager::instance()->db();
    CWizStdStringArray arrayAllLocations;
    if (!db.getAllLocations(arrayAllLocations))
        return "";
    CWizStdStringArray arrayChild;
    if (!db.getChildLocations(arrayAllLocations, parentLocation, arrayChild))
        return "";

    while (true)
    {
        if (nRepeat > 0)
        {
            name = locationName + "_" + QString::number(nRepeat);
        }
        bool continueLoop = false;
        CWizStdStringArray::const_iterator it;
        for (it = arrayChild.begin(); it != arrayChild.end(); it++)
        {
            QString brotherLocation = *it;
            if (WizDatabase::getLocationName(brotherLocation) == name)
            {
                nRepeat ++;
                continueLoop = true;
                break;
            }
        }
        if (!continueLoop)
            break;
    }
    return name;
}

QString WizDocumentOperatorPrivate::getUniqueFolderName(const WIZTAGDATA& parentTag, const WIZTAGDATA& sourceTag, bool combineFolder, const QString& exceptGUID)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();

    WizDatabase& targetDB = dbMgr->db(parentTag.strKbGUID);
    CWizTagDataArray arrayTag;
    WIZTAGDATA sameNameBrother;
    if (targetDB.getChildTags(parentTag.strGUID, arrayTag))
    {
        for (WIZTAGDATA tag : arrayTag)
        {
            if (tag.strGUID != exceptGUID && tag.strName == sourceTag.strName)
            {
                sameNameBrother = tag;
                break;
            }
        }
    }
    // combine same name folders
    if (!sameNameBrother.strGUID.isEmpty())
    {
        if (!combineFolder)
        {
            WIZTAGDATA uniqueTag = sameNameBrother;
            uniqueTag.strGUID.clear();
            return getUniqueTagName(parentTag, uniqueTag);
        }
    }

    return m_data->sourceTag.strName;
}

QString WizDocumentOperatorPrivate::getUniqueFolderName(const WIZTAGDATA& parentTag, const QString& sourceFolder, bool combineFolder)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();

    WizDatabase& targetDB = dbMgr->db(parentTag.strKbGUID);
    CWizTagDataArray arrayTag;
    WIZTAGDATA sameNameBrother;
    QString locationName = WizDatabase::getLocationName(sourceFolder);
    if (targetDB.getChildTags(parentTag.strGUID, arrayTag))
    {
        for (WIZTAGDATA tag : arrayTag)
        {
            if (tag.strName == locationName)
            {
                sameNameBrother = tag;
                break;
            }
        }
    }
    // combine same name folders
    if (!sameNameBrother.strGUID.isEmpty())
    {
        if (!combineFolder)
        {
            WIZTAGDATA uniqueTag = sameNameBrother;
            uniqueTag.strGUID.clear();
            return getUniqueTagName(parentTag, uniqueTag);
        }
    }

    return locationName;
}


bool getUserCipher(WizDatabase& db, const WIZDOCUMENTDATA& encryptedDoc)
{
    if (encryptedDoc.strGUID.isEmpty())
    {
        WizMessageBox::warning(qApp->activeWindow(), QObject::tr("Info"), QObject::tr("Encrypted notes are not downloaded to local, "
                                                                   "can not verify password!"));
        return false;
    }

    db.loadUserCert();
    if (!db.QueryCertPassword())
        return false;

    if (db.isDocumentDownloaded(encryptedDoc.strGUID))
    {
        if (!db.isFileAccessible(encryptedDoc))
        {
            WizMessageBox::warning(qApp->activeWindow(), QObject::tr("Info"), QObject::tr("Password error!"));
            return false;
        }
    }
    return true;
}

bool WizAskUserCipherToOperateEncryptedNotes(const QString& sourceFolder, WizDatabase& db)
{
    CWizDocumentDataArray arrayDoc;
    db.getDocumentsByLocation(sourceFolder, arrayDoc, true);

    bool includeEncrpyted = false;
    WIZDOCUMENTDATA encryptedDoc;
    for (WIZDOCUMENTDATA doc : arrayDoc)
    {
        if (doc.nProtected == 1)
        {
            includeEncrpyted = true;
            if (db.isDocumentDownloaded(doc.strGUID))
            {
                encryptedDoc = doc;
                break;
            }
        }
    }

    if (includeEncrpyted)
    {
        if (WizMessageBox::question(qApp->activeWindow(), QObject::tr("Info"), QObject::tr("Source folders contains "
                                "encrypted notes, are you sure to operate these notes?")) != QMessageBox::Yes)
            return false;

        return getUserCipher(db, encryptedDoc);
    }

    return true;
}

bool WizAskUserCipherToOperateEncryptedNote(const CWizDocumentDataArray& arrayDocument, WizDatabase& db)
{
    bool includeEncrpyted = false;
    WIZDOCUMENTDATA encryptedDoc;
    for (WIZDOCUMENTDATA doc : arrayDocument)
    {
        if (doc.nProtected == 1)
        {
            includeEncrpyted = true;
            if (db.isDocumentDownloaded(doc.strGUID))
            {
                encryptedDoc = doc;
                break;
            }
        }
    }

    if (includeEncrpyted)
    {
        if (WizMessageBox::question(qApp->activeWindow(), QObject::tr("Info"), QObject::tr("Source notes list contains "
                                "encrypted notes, are you sure to operate these notes?")) != QMessageBox::Yes)
            return false;

        return getUserCipher(db, encryptedDoc);
    }

    return true;
}


void WizClearUserCipher(WizDatabase& db, WizUserSettings& settings)
{
    if (!settings.isRememberNotePasswordForSession())
    {
        //TODO:wsj
    }
}


WizDocumentOperator::WizDocumentOperator(WizDatabaseManager& dbMgr, QObject* parent)
    : QObject(parent)
    , m_dbMgr(&dbMgr)
{
}

void WizDocumentOperator::copyDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument,
                                                         const QString& targetFolder, bool keepDocTime,
                                                         bool keepTag, bool showProgressDialog)
{    
    OperatorData* optData = new OperatorData();
    optData->arrayDocument =  arrayDocument;
    optData->targetFolder = targetFolder;
    optData->keepDocTime = keepDocTime;
    optData->keepTag = keepTag;
    if (showProgressDialog && arrayDocument.size() > 3)
    {
        WizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetFolder));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            WizDocumentOperatorPrivate helper(optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyDocumentToPersonalFolder();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            WizDocumentOperatorPrivate helper(optData);
            helper.copyDocumentToPersonalFolder();
        });
    }
}

void WizDocumentOperator::copyDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument,
                                                      const WIZTAGDATA& targetTag, bool keepDocTime,
                                                      bool showProgressDialog)
{
    WizDatabaseManager* dbMgr = WizDatabaseManager::instance();
    WizDatabase& db = dbMgr->db(targetTag.strKbGUID);
    //
    if (!db.prepareBizCert())
        return;

    OperatorData* optData = new OperatorData();
    optData->arrayDocument =  arrayDocument;
    optData->targetTag = targetTag;
    optData->keepDocTime = keepDocTime;
    if (showProgressDialog && arrayDocument.size() > 3)
    {
        WizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetTag.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            WizDocumentOperatorPrivate helper(optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyDocumentToGroupFolder();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            WizDocumentOperatorPrivate helper(optData);
            helper.copyDocumentToGroupFolder();
        });
    }
}

void WizDocumentOperator::moveDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument,
                                                         const QString& targetFolder, bool waitForSync)
{
    OperatorData* optData = new OperatorData();
    optData->arrayDocument =  arrayDocument;
    optData->targetFolder = targetFolder;
    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
        //
        if (waitForSync && arrayDocument.size() > 1)
        {
            WIZKM_WAIT_AND_PAUSE_SYNC();
        }
        //
        WizDocumentOperatorPrivate helper(optData);
        helper.moveDocumentToPersonalFolder();
    });
}

void WizDocumentOperator::moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument,
                                                      const WIZTAGDATA& targetTag, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->arrayDocument =  arrayDocument;
    optData->targetTag = targetTag;
    if (showProgressDialog && arrayDocument.size() > 3)
    {
        WizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Move notes to %1").arg(targetTag.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            //
            if (arrayDocument.size() > 1)
            {
                WIZKM_WAIT_AND_PAUSE_SYNC();
            }
            //
            WizDocumentOperatorPrivate helper(optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.moveDocumentToGroupFolder();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            //
            if (arrayDocument.size() > 1)
            {
                WIZKM_WAIT_AND_PAUSE_SYNC();
            }
            //
            WizDocumentOperatorPrivate helper(optData);
            helper.moveDocumentToGroupFolder();
        });
    }
}

void WizDocumentOperator::deleteDocuments(const CWizDocumentDataArray& arrayDocument)
{
    OperatorData* optData = new OperatorData();
    optData->arrayDocument =  arrayDocument;
    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
        WizDocumentOperatorPrivate helper(optData);
        helper.deleteDocuments();
    });
}

void WizDocumentOperator::copyPersonalFolderToPersonalDB(const QString& sourceFolder, const QString& targetParentFolder,
                                                          bool keepDocTime, bool keepTag, bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceFolder = sourceFolder;
    optData->targetFolder = targetParentFolder;
    optData->keepDocTime = keepDocTime;
    optData->keepTag = keepTag;
    optData->combineFolders = combineFolders;
    int count = documentCount(m_dbMgr->db(), sourceFolder);
    if (showProgressDialog && count > 3)
    {
        WizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetParentFolder));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            WizDocumentOperatorPrivate helper(optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyPersonalFolderToPersonalDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            WizDocumentOperatorPrivate helper(optData);
            helper.copyPersonalFolderToPersonalDB();
        });
    }
}

void WizDocumentOperator::copyPersonalFolderToGroupDB(const QString& sourceFolder, const WIZTAGDATA& targetParentTag,
                                                       bool keepDocTime, bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceFolder = sourceFolder;
    optData->targetTag = targetParentTag;
    optData->keepDocTime = keepDocTime;
    optData->combineFolders = combineFolders;
    int count = documentCount(m_dbMgr->db(), sourceFolder);
    if (showProgressDialog && count > 3)
    {
        WizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetParentTag.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            WizDocumentOperatorPrivate helper(optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyPersonalFolderToGroupDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            WizDocumentOperatorPrivate helper(optData);
            helper.copyPersonalFolderToGroupDB();
        });
    }
}

void WizDocumentOperator::copyGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder, const QString& targetParentFolder,
                                                       bool keepDocTime, bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceTag = groupFolder;
    optData->targetFolder = targetParentFolder;
    optData->keepDocTime = keepDocTime;
    optData->combineFolders = combineFolders;
    int count = documentCount(m_dbMgr->db(groupFolder.strKbGUID), groupFolder);
    if (showProgressDialog && count > 3)
    {
        WizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetParentFolder));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            //
            WIZKM_WAIT_AND_PAUSE_SYNC();
            //
            WizDocumentOperatorPrivate helper(optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyGroupFolderToPersonalDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            //
            WIZKM_WAIT_AND_PAUSE_SYNC();
            //
            WizDocumentOperatorPrivate helper(optData);
            helper.copyGroupFolderToPersonalDB();
        });
    }
}

void WizDocumentOperator::copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                                    bool keepDocTime, bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceTag = sourceFolder;
    optData->targetTag = targetFolder;
    optData->keepDocTime = keepDocTime;
    optData->combineFolders = combineFolders;
    int count = documentCount(m_dbMgr->db(sourceFolder.strKbGUID), sourceFolder);
    if (showProgressDialog && count > 3)
    {
        WizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetFolder.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            WizDocumentOperatorPrivate helper(optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyGroupFolderToGroupDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            WizDocumentOperatorPrivate helper(optData);
            helper.copyGroupFolderToGroupDB();
        });
    }
}

void WizDocumentOperator::moveGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder, const QString& targetParentFolder,
                                                       bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceTag = groupFolder;
    optData->targetFolder = targetParentFolder;
    optData->combineFolders = combineFolders;
    int count = documentCount(m_dbMgr->db(groupFolder.strKbGUID), groupFolder);
    if (showProgressDialog && count > 3)
    {
        WizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Move notes to %1").arg(targetParentFolder));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            //
            WIZKM_WAIT_AND_PAUSE_SYNC();
            //
            WizDocumentOperatorPrivate helper(optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.moveGroupFolderToPersonalDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            //
            WIZKM_WAIT_AND_PAUSE_SYNC();
            //
            WizDocumentOperatorPrivate helper(optData);
            helper.moveGroupFolderToPersonalDB();
        });
    }
}

void WizDocumentOperator::moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                                    bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceTag = sourceFolder;
    optData->targetTag = targetFolder;
    optData->combineFolders = combineFolders;
    int count = documentCount(m_dbMgr->db(sourceFolder.strKbGUID), sourceFolder);
    if (showProgressDialog && count > 3)
    {
        WizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Move notes to %1").arg(targetFolder.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            //
            WIZKM_WAIT_AND_PAUSE_SYNC();
            //
            WizDocumentOperatorPrivate helper(optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.moveGroupFolderToGroupDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            //
            WIZKM_WAIT_AND_PAUSE_SYNC();
            //
            WizDocumentOperatorPrivate helper(optData);
            helper.moveGroupFolderToGroupDB();
        });
    }
}


void WizDocumentOperator::movePersonalFolderToGroupDB(const QString& sourceFolder, const WIZTAGDATA& targetFolder,
                                                       bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceFolder = sourceFolder;
    optData->targetTag = targetFolder;
    optData->combineFolders = combineFolders;
    int count = documentCount(m_dbMgr->db(), sourceFolder);
    if (showProgressDialog && count > 3)
    {
        WizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Move notes to %1").arg(targetFolder.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            //
            WIZKM_WAIT_AND_PAUSE_SYNC();
            //
            WizDocumentOperatorPrivate helper(optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.movePersonalFolderToGroupDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            //
            WIZKM_WAIT_AND_PAUSE_SYNC();
            //
            WizDocumentOperatorPrivate helper(optData);
            helper.movePersonalFolderToGroupDB();
        });
    }
}

