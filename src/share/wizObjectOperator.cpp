#include "wizObjectOperator_p.h"
#include "wizObjectOperator.h"
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include "share/wizMessageBox.h"
#include "share/wizsettings.h"
#include "share/wizthreads.h"
#include "wizProgressDialog.h"
#include "wizObjectDataDownloader.h"
#include "wizDatabaseManager.h"
#include "wizDatabase.h"
#include "wizLineInputDialog.h"

CWizDocumentOperatorPrivate::CWizDocumentOperatorPrivate(CWizDatabaseManager& dbMgr, OperatorData* data, QObject* parent)
    : QObject(parent)
    , m_dbMgr(dbMgr)
    , m_data(data)
    , m_stop(false)
    , m_totoalCount(0)
    , m_counter(0)
{
}

CWizDocumentOperatorPrivate::~CWizDocumentOperatorPrivate()
{
    if (m_data)
    {
        delete m_data;
        m_data = nullptr;
    }
}

void CWizDocumentOperatorPrivate::bindSignalsToProgressDialog(CWizProgressDialog* progress)
{
    QObject::connect(this, SIGNAL(progress(int,int)), progress, SLOT(setProgress(int,int)));
    QObject::connect(this, SIGNAL(newAction(QString)), progress, SLOT(setActionString(QString)));
    QObject::connect(this, SIGNAL(finished()), progress, SLOT(accept()));
    QObject::connect(progress, SIGNAL(stopRequest()), this, SLOT(stop()), Qt::DirectConnection);
}

void CWizDocumentOperatorPrivate::stop()
{
    m_stop = true;
}

void CWizDocumentOperatorPrivate::copyDocumentToPersonalFolder()
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

void CWizDocumentOperatorPrivate::copyDocumentToGroupFolder()
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

void CWizDocumentOperatorPrivate::moveDocumentToPersonalFolder()
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

void CWizDocumentOperatorPrivate::moveDocumentToGroupFolder()
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

void CWizDocumentOperatorPrivate::deleteDocuments()
{
    m_totoalCount = m_data->arrayDocument.size();
    m_counter = 0;
    for (WIZDOCUMENTDATA doc : m_data->arrayDocument)
    {
        emit newAction(tr("Delete note %1").arg(doc.strTitle));
        CWizDatabase& db = m_dbMgr.db(doc.strKbGUID);
        CWizDocument document(db, doc);
        document.Delete();
        emit progress(m_totoalCount, m_counter);

        if (m_stop)
            break;
    }

    emit finished();
}

void CWizDocumentOperatorPrivate::copyPersonalFolderToPersonalDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(), m_data->sourceFolder);

    //
    QString sourceFolderName = CWizDatabase::GetLocationName(m_data->sourceFolder);
    QString uniqueFolder = getUniqueFolderName(m_data->targetFolder, sourceFolderName);
    if (m_data->combineFolders)
    {
        uniqueFolder.clear();
    }

    copyPersonalFolderToPersonalDB(m_data->sourceFolder, m_data->targetFolder, uniqueFolder);

    emit finished();
}

void CWizDocumentOperatorPrivate::copyPersonalFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(), m_data->sourceFolder);

    //    
    QString targetTagName = getUniqueFolderName(m_data->targetTag, m_data->sourceFolder, m_data->combineFolders);
    copyPersonalFolderToGroupDB(m_data->sourceFolder, m_data->targetTag, targetTagName);

    emit finished();
}

void CWizDocumentOperatorPrivate::copyGroupFolderToPersonalDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_data->sourceTag.strKbGUID), m_data->sourceTag);
    //
    QString uniqueFolder = getUniqueFolderName(m_data->targetFolder, m_data->sourceTag.strName);
    if (uniqueFolder != m_data->sourceTag.strName && !m_data->combineFolders)
    {
        m_data->sourceTag.strName = uniqueFolder;
    }

    copyGroupFolderToPersonalDB(m_data->sourceTag, m_data->targetFolder);

    emit finished();
}

void CWizDocumentOperatorPrivate::copyGroupFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_data->sourceTag.strKbGUID), m_data->sourceTag);

    m_data->sourceTag.strName = getUniqueFolderName(m_data->targetTag, m_data->sourceTag, m_data->combineFolders);
    copyGroupFolderToGroupDB(m_data->sourceTag, m_data->targetTag);

    emit finished();
}

void CWizDocumentOperatorPrivate::moveGroupFolderToPersonalDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_data->sourceTag.strKbGUID), m_data->sourceTag);

    //
    QString uniqueFolder = getUniqueFolderName(m_data->targetFolder, m_data->sourceTag.strName);
    if (uniqueFolder != m_data->sourceTag.strName && !m_data->combineFolders)
    {
        m_data->sourceTag.strName = uniqueFolder;
    }

    moveGroupFolderToPersonalDB(m_data->sourceTag, m_data->targetFolder);

    emit finished();
}

void CWizDocumentOperatorPrivate::moveGroupFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_data->sourceTag.strKbGUID), m_data->sourceTag);

    m_data->sourceTag.strName = getUniqueFolderName(m_data->targetTag, m_data->sourceTag, m_data->combineFolders, m_data->sourceTag.strGUID);
    moveGroupFolderToGroupDB(m_data->sourceTag, m_data->targetTag);

    emit finished();
}

void CWizDocumentOperatorPrivate::movePersonalFolderToPersonalDB()
{    
    m_totoalCount = documentCount(m_dbMgr.db(), m_data->sourceFolder);

    //
    QString sourceFolderName = CWizDatabase::GetLocationName(m_data->sourceFolder);
    QString uniqueFolder = getUniqueFolderName(m_data->targetFolder, sourceFolderName);
    if (m_data->combineFolders)
    {
        uniqueFolder.clear();
    }

    _movePersonalFolderToPersonalDB(m_data->sourceFolder, m_data->targetFolder, uniqueFolder);

    emit finished();
}

void CWizDocumentOperatorPrivate::movePersonalFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(), m_data->sourceFolder);

    //    
    QString targetTagName = getUniqueFolderName(m_data->targetTag, m_data->sourceFolder, m_data->combineFolders);
    movePersonalFolderToGroupDB(m_data->sourceFolder, m_data->targetTag, targetTagName);

    emit finished();
}

void CWizDocumentOperatorPrivate::copyDocumentToPersonalFolder(const WIZDOCUMENTDATA& doc)
{
    CWizDatabase& sourceDb = m_dbMgr.db(doc.strKbGUID);
    CWizDatabase& targetDb = m_dbMgr.db();
    CWizFolder folder(targetDb, m_data->targetFolder);
    CWizDocument wizDoc(sourceDb, doc);
    QString newDocGUID;
    if (!wizDoc.CopyTo(targetDb, &folder, m_data->keepDocTime, m_data->keepTag, newDocGUID))
        return;

    if (!sourceDb.IsGroup() && doc.nProtected == 1)
    {
        WIZDOCUMENTDATA newDoc;
        if (targetDb.DocumentFromGUID(newDocGUID, newDoc))
        {
            sourceDb.EncryptDocument(newDoc);
        }
    }
}

void CWizDocumentOperatorPrivate::copyDocumentToGroupFolder(const WIZDOCUMENTDATA& doc)
{
    CWizDocument wizDoc(m_dbMgr.db(doc.strKbGUID), doc);
    wizDoc.CopyTo(m_dbMgr.db(m_data->targetTag.strKbGUID), m_data->targetTag, m_data->keepDocTime);
}

void CWizDocumentOperatorPrivate::moveDocumentToPersonalFolder(const WIZDOCUMENTDATA& doc)
{
    CWizFolder folder(m_dbMgr.db(), m_data->targetFolder);
    CWizDocument wizDoc(m_dbMgr.db(doc.strKbGUID), doc);
    wizDoc.MoveTo(m_dbMgr.db(), &folder);
}

void CWizDocumentOperatorPrivate::moveDocumentToGroupFolder(const WIZDOCUMENTDATA& doc)
{
    CWizDocument wizDoc(m_dbMgr.db(doc.strKbGUID), doc);
    wizDoc.MoveTo(m_dbMgr.db(m_data->targetTag.strKbGUID), m_data->targetTag);
}


void CWizDocumentOperatorPrivate::copyPersonalFolderToPersonalDB(const QString& childFolder,
                                                          const QString& targetParentFolder, const QString& targetFolderName)
{
    CWizDatabase& db = m_dbMgr.db();
    CWizDocumentDataArray arrayDocument;
    db.GetDocumentsByLocation(childFolder, arrayDocument, false);
    //
    QString folderName = targetFolderName.isEmpty() ? db.GetLocationName(childFolder) : targetFolderName;
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
    db.GetAllLocations(arrayFolder);
    CWizStdStringArray arrayChild;
    if (db.GetChildLocations(arrayFolder, childFolder, arrayChild))
    {
        for (QString newChildFolder : arrayChild)
        {
            copyPersonalFolderToPersonalDB(newChildFolder, targetFolder, "");
            if (m_stop)
                return;
        }
    }
}

void CWizDocumentOperatorPrivate::copyPersonalFolderToGroupDB(const QString& childFolder,
                                                       const WIZTAGDATA& targetParentTag, const QString& targetTagName)
{
    CWizDatabase& db = m_dbMgr.db();
    CWizDatabase& targetDB = m_dbMgr.db(targetParentTag.strKbGUID);
    QString tagName = targetTagName.isEmpty() ? db.GetLocationName(childFolder) : targetTagName;
    WIZTAGDATA targetTag;
    targetDB.CreateTag(targetParentTag.strGUID, tagName, "", targetTag);
    m_data->targetTag = targetTag;
    //
    CWizDocumentDataArray arrayDocument;
    db.GetDocumentsByLocation(childFolder, arrayDocument, false);

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
    db.GetAllLocations(arrayFolder);
    CWizStdStringArray arrayChild;
    if (db.GetChildLocations(arrayFolder, childFolder, arrayChild))
    {
        for (QString newChildFolder : arrayChild)
        {
            copyPersonalFolderToGroupDB(newChildFolder, targetTag, "");
            if (m_stop)
                return;
        }
    }
}

void CWizDocumentOperatorPrivate::copyGroupFolderToPersonalDB(const WIZTAGDATA& childFolder, const QString& targetParentFolder)
{
    CWizDatabase& groupDB = m_dbMgr.db(childFolder.strKbGUID);
    CWizDocumentDataArray arrayDocument;
    groupDB.GetDocumentsByTag(childFolder, arrayDocument);
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
    groupDB.GetChildTags(childFolder.strGUID, arrayTag);
    for (WIZTAGDATA childTag : arrayTag)
    {
        copyGroupFolderToPersonalDB(childTag, targetFolder);
    }
}

void CWizDocumentOperatorPrivate::copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder)
{
    CWizDatabase& targetDB = m_dbMgr.db(targetFolder.strKbGUID);
    WIZTAGDATA targetTag;
    targetDB.CreateTag(targetFolder.strGUID, sourceFolder.strName, sourceFolder.strDescription, targetTag);
    m_data->targetTag = targetTag;

    //
    CWizDatabase& sourceDB = m_dbMgr.db(sourceFolder.strKbGUID);
    CWizDocumentDataArray arrayDocument;
    sourceDB.GetDocumentsByTag(sourceFolder, arrayDocument);
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
    sourceDB.GetChildTags(sourceFolder.strGUID, arrayTag);
    for (WIZTAGDATA childTag : arrayTag)
    {
        copyGroupFolderToGroupDB(childTag, targetTag);
    }
}

void CWizDocumentOperatorPrivate::moveGroupFolderToPersonalDB(const WIZTAGDATA& childFolder, const QString& targetParentFolder)
{
    CWizDatabase& groupDB = m_dbMgr.db(childFolder.strKbGUID);
    CWizDocumentDataArray arrayDocument;
    groupDB.GetDocumentsByTag(childFolder, arrayDocument);
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
    groupDB.GetChildTags(childFolder.strGUID, arrayTag);
    for (WIZTAGDATA childTag : arrayTag)
    {
        moveGroupFolderToPersonalDB(childTag, targetFolder);
    }

    // remove tag at last  if tag is empty
    arrayDocument.clear();
    groupDB.GetDocumentsByTag(childFolder, arrayDocument);
    arrayTag.clear();
    groupDB.GetChildTags(childFolder.strGUID, arrayTag);
    if (arrayDocument.size() == 0 && arrayTag.size() == 0)
    {
        groupDB.DeleteTag(childFolder, true);
        emit newAction(tr("Delete folder %1").arg(childFolder.strName));
    }
}

void CWizDocumentOperatorPrivate::moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetParentTag)
{
    CWizDatabase& sourceDB = m_dbMgr.db(sourceFolder.strKbGUID);
    if (sourceFolder.strKbGUID == targetParentTag.strKbGUID)
    {
        qDebug() << "folder in same db, just modify parent tag guid";
        WIZTAGDATA sourceTag = sourceFolder;
        sourceTag.strParentGUID = targetParentTag.strGUID;
        sourceDB.ModifyTag(sourceTag);
    }
    else
    {
        CWizDatabase& targetDB = m_dbMgr.db(targetParentTag.strKbGUID);
        WIZTAGDATA targetTag;
        targetDB.CreateTag(targetParentTag.strGUID, sourceFolder.strName, sourceFolder.strDescription, targetTag);
        m_data->targetTag = targetTag;
        //
        CWizDocumentDataArray arrayDocument;
        sourceDB.GetDocumentsByTag(sourceFolder, arrayDocument);
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
        sourceDB.GetChildTags(sourceFolder.strGUID, arrayTag);
        for (WIZTAGDATA childTag : arrayTag)
        {
            moveGroupFolderToGroupDB(childTag, targetTag);
        }

        // remove tag at last  if tag is empty
        arrayDocument.clear();
        sourceDB.GetDocumentsByTag(sourceFolder, arrayDocument);
        arrayTag.clear();
        sourceDB.GetChildTags(sourceFolder.strGUID, arrayTag);
        if (arrayDocument.size() == 0 && arrayTag.size() == 0)
        {
            sourceDB.DeleteTag(sourceFolder, true);
        }
    }
}

void CWizDocumentOperatorPrivate::_movePersonalFolderToPersonalDB(const QString& childFolder, const QString& targetParentFolder,
                                                           const QString& targetFolderName)
{
    CWizDatabase& db = m_dbMgr.db();
    CWizDocumentDataArray arrayDocument;
    db.GetDocumentsByLocation(childFolder, arrayDocument, false);
    //
    QString folderName = targetFolderName.isEmpty() ? db.GetLocationName(childFolder) : targetFolderName;
    QString targetFolder = targetParentFolder + folderName + "/";

    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end() && !m_stop; it++)
    {
        WIZDOCUMENTDATA doc = *it;
        emit newAction(tr("Move note %1").arg(doc.strTitle));
        doc.strLocation = targetFolder;
        db.ModifyDocumentInfo(doc);
        m_counter ++;
        emit progress(m_totoalCount, m_counter);
    }

    if (m_stop)
        return;

    CWizStdStringArray arrayFolder;
    db.GetAllLocations(arrayFolder);
    CWizStdStringArray arrayChild;
    if (db.GetChildLocations(arrayFolder, childFolder, arrayChild))
    {
        for (QString newChildFolder : arrayChild)
        {
            _movePersonalFolderToPersonalDB(newChildFolder, targetFolder, "");
        }
    }

    //
    arrayDocument.clear();
    db.GetDocumentsByLocation(childFolder, arrayDocument, true);
    if (arrayDocument.size() == 0)
    {
        emit newAction(tr("Delete folder %1").arg(childFolder));
        db.DeleteExtraFolder(childFolder);
    }
}

void CWizDocumentOperatorPrivate::movePersonalFolderToGroupDB(const QString& sourceFolder,
                                                       const WIZTAGDATA& targetParentTag, const QString& targetTagName)
{
    CWizDatabase& db = m_dbMgr.db();
    CWizDatabase& targetDB = m_dbMgr.db(targetParentTag.strKbGUID);
    QString tagName = targetTagName.isEmpty() ? db.GetLocationName(sourceFolder) : targetTagName;
    WIZTAGDATA targetTag;
    targetDB.CreateTag(targetParentTag.strGUID, tagName, "", targetTag);
    m_data->targetTag = targetTag;
    //
    CWizDocumentDataArray arrayDocument;
    db.GetDocumentsByLocation(sourceFolder, arrayDocument, false);
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
    db.GetAllLocations(arrayFolder);
    CWizStdStringArray arrayChild;
    if (db.GetChildLocations(arrayFolder, sourceFolder, arrayChild))
    {
        for (QString childFolder : arrayChild)
        {
            movePersonalFolderToGroupDB(childFolder, targetTag, "");
        }
    }

    //  check if move sucessfull
    arrayDocument.clear();
    db.GetDocumentsByLocation(sourceFolder, arrayDocument, true);
    if (arrayDocument.size() == 0)
    {
        db.DeleteExtraFolder(sourceFolder);
    }
}

int CWizDocumentOperatorPrivate::documentCount(CWizDatabase& db, const QString& personalFolder)
{
    int count = 0;
    db.GetDocumentsCountByLocation(personalFolder, count, true);
    return count;
}

int CWizDocumentOperatorPrivate::documentCount(CWizDatabase& db, const WIZTAGDATA& groupFolder)
{
    int count = 0;
    CWizTagDataArray arrayTag;
    db.GetAllChildTags(groupFolder.strGUID, arrayTag);
    arrayTag.push_back(groupFolder);
    std::map<CString, int> mapTagDocumentCount;
    db.GetAllTagsDocumentCount(mapTagDocumentCount);
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

void CWizDocumentOperatorPrivate::combineSameNameGroupFolder(const WIZTAGDATA& parentTag, const WIZTAGDATA& childTag)
{
    CWizDatabase& targetDB = m_dbMgr.db(parentTag.strKbGUID);
    CWizTagDataArray arrayTag;
    WIZTAGDATA sameNameBrother;
    if (targetDB.GetChildTags(parentTag.strGUID, arrayTag))
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
    if (!sameNameBrother.strGUID.IsEmpty())
    {
        if (m_data->combineFolders)
        {
            CWizDocumentDataArray arrayDocument;
            targetDB.GetDocumentsByTag(childTag, arrayDocument);
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
            targetDB.GetChildTags(childTag.strGUID, arrayChild);
            for (WIZTAGDATA tag : arrayChild)
            {
                tag.strParentGUID = sameNameBrother.strGUID;
                targetDB.ModifyTag(tag);
            }
            targetDB.DeleteTag(childTag, false);
        }
        else
        {
            WIZTAGDATA tag = childTag;
            tag.strName = getUniqueTagName(parentTag, childTag);
            targetDB.ModifyTag(tag);
        }
    }
}

QString CWizDocumentOperatorPrivate::getUniqueTagName(const WIZTAGDATA& parentTag, const WIZTAGDATA& tag)
{
    QString name = tag.strName;
    int nRepeat = 0;
    CWizTagDataArray arrayTag;
    if(!m_dbMgr.db(parentTag.strKbGUID).GetChildTags(parentTag.strGUID, arrayTag))
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

QString CWizDocumentOperatorPrivate::getUniqueFolderName(const QString& parentLocation, const QString& locationName)
{
    QString name = locationName;
    int nRepeat = 0;
    CWizDatabase& db = m_dbMgr.db();
    CWizStdStringArray arrayAllLocations;
    if (!db.GetAllLocations(arrayAllLocations))
        return "";
    CWizStdStringArray arrayChild;
    if (!db.GetChildLocations(arrayAllLocations, parentLocation, arrayChild))
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
            if (CWizDatabase::GetLocationName(brotherLocation) == name)
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

QString CWizDocumentOperatorPrivate::getUniqueFolderName(const WIZTAGDATA& parentTag, const WIZTAGDATA& sourceTag, bool combineFolder, const QString& exceptGUID)
{
    CWizDatabase& targetDB = m_dbMgr.db(parentTag.strKbGUID);
    CWizTagDataArray arrayTag;
    WIZTAGDATA sameNameBrother;
    if (targetDB.GetChildTags(parentTag.strGUID, arrayTag))
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
    if (!sameNameBrother.strGUID.IsEmpty())
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

QString CWizDocumentOperatorPrivate::getUniqueFolderName(const WIZTAGDATA& parentTag, const QString& sourceFolder, bool combineFolder)
{
    CWizDatabase& targetDB = m_dbMgr.db(parentTag.strKbGUID);
    CWizTagDataArray arrayTag;
    WIZTAGDATA sameNameBrother;
    QString locationName = CWizDatabase::GetLocationName(sourceFolder);
    if (targetDB.GetChildTags(parentTag.strGUID, arrayTag))
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
    if (!sameNameBrother.strGUID.IsEmpty())
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


bool getUserCipher(CWizDatabase& db, const WIZDOCUMENTDATA& encryptedDoc)
{
    if (encryptedDoc.strGUID.isEmpty())
    {
        CWizMessageBox::warning(qApp->activeWindow(), QObject::tr("Info"), QObject::tr("Encrypted notes are not downloaded to local, "
                                                                   "can not verify password!"));
        return false;
    }

    CWizLineInputDialog dlg(QObject::tr("Please input note password"),
                            QObject::tr("Password :"), "", 0, QLineEdit::Password);
    if (dlg.exec() == QDialog::Rejected)
        return false;

    db.loadUserCert();
    db.setUserCipher(dlg.input());

    if (db.IsDocumentDownloaded(encryptedDoc.strGUID))
    {
        if (!db.IsFileAccessible(encryptedDoc))
        {
            CWizMessageBox::warning(qApp->activeWindow(), QObject::tr("Info"), QObject::tr("Password error!"));
            return false;
        }
    }
    db.setSaveUserCipher(true);
    return true;
}

bool WizAskUserCipherToOperateEncryptedNotes(const QString& sourceFolder, CWizDatabase& db)
{
    CWizDocumentDataArray arrayDoc;
    db.GetDocumentsByLocation(sourceFolder, arrayDoc, true);

    bool includeEncrpyted = false;
    WIZDOCUMENTDATA encryptedDoc;
    for (WIZDOCUMENTDATA doc : arrayDoc)
    {
        if (doc.nProtected == 1)
        {
            includeEncrpyted = true;
            if (db.IsDocumentDownloaded(doc.strGUID))
            {
                encryptedDoc = doc;
                break;
            }
        }
    }

    if (includeEncrpyted)
    {
        if (CWizMessageBox::question(qApp->activeWindow(), QObject::tr("Info"), QObject::tr("Source folders contains "
                                "encrypted notes, are you sure to operate these notes?")) != QMessageBox::Yes)
            return false;

        return getUserCipher(db, encryptedDoc);
    }

    return true;
}

bool WizAskUserCipherToOperateEncryptedNote(const CWizDocumentDataArray& arrayDocument, CWizDatabase& db)
{
    bool includeEncrpyted = false;
    WIZDOCUMENTDATA encryptedDoc;
    for (WIZDOCUMENTDATA doc : arrayDocument)
    {
        if (doc.nProtected == 1)
        {
            includeEncrpyted = true;
            if (db.IsDocumentDownloaded(doc.strGUID))
            {
                encryptedDoc = doc;
                break;
            }
        }
    }

    if (includeEncrpyted)
    {
        if (CWizMessageBox::question(qApp->activeWindow(), QObject::tr("Info"), QObject::tr("Source notes list contains "
                                "encrypted notes, are you sure to operate these notes?")) != QMessageBox::Yes)
            return false;

        return getUserCipher(db, encryptedDoc);
    }

    return true;
}


void WizClearUserCipher(CWizDatabase& db, CWizUserSettings& settings)
{
    if (!settings.isRememberNotePasswordForSession())
    {
        db.setUserCipher(QString());
        db.setSaveUserCipher(false);
    }
}


CWizDocumentOperator::CWizDocumentOperator(CWizDatabaseManager& dbMgr, QObject* parent)
    : QObject(parent)
    , m_dbMgr(dbMgr)
{
}

void CWizDocumentOperator::copyDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument,
                                                         const QString& targetFolder, bool keepDocTime,
                                                         bool keepTag, bool showProgressDialog)
{    
    OperatorData* optData = new OperatorData();
    optData->arrayDocument =  arrayDocument;
    optData->targetFolder = targetFolder;
    optData->keepDocTime = keepDocTime;
    optData->keepTag = keepTag;
    if (showProgressDialog)
    {
        CWizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetFolder));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyDocumentToPersonalFolder();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.copyDocumentToPersonalFolder();
        });
    }
}

void CWizDocumentOperator::copyDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument,
                                                      const WIZTAGDATA& targetTag, bool keepDocTime,
                                                      bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->arrayDocument =  arrayDocument;
    optData->targetTag = targetTag;
    optData->keepDocTime = keepDocTime;
    if (showProgressDialog)
    {
        CWizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetTag.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyDocumentToGroupFolder();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.copyDocumentToGroupFolder();
        });
    }
}

void CWizDocumentOperator::moveDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument,
                                                         const QString& targetFolder, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->arrayDocument =  arrayDocument;
    optData->targetFolder = targetFolder;
    if (showProgressDialog)
    {
        CWizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Move notes to %1").arg(targetFolder));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.moveDocumentToPersonalFolder();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.moveDocumentToPersonalFolder();
        });
    }
}

void CWizDocumentOperator::moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument,
                                                      const WIZTAGDATA& targetTag, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->arrayDocument =  arrayDocument;
    optData->targetTag = targetTag;
    if (showProgressDialog)
    {
        CWizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Move notes to %1").arg(targetTag.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.moveDocumentToGroupFolder();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.moveDocumentToGroupFolder();
        });
    }
}

void CWizDocumentOperator::deleteDocuments(const CWizDocumentDataArray& arrayDocument)
{
    OperatorData* optData = new OperatorData();
    optData->arrayDocument =  arrayDocument;
    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
        CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
        helper.deleteDocuments();
    });
}

void CWizDocumentOperator::copyPersonalFolderToPersonalDB(const QString& sourceFolder, const QString& targetParentFolder,
                                                          bool keepDocTime, bool keepTag, bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceFolder = sourceFolder;
    optData->targetFolder = targetParentFolder;
    optData->keepDocTime = keepDocTime;
    optData->keepTag = keepTag;
    optData->combineFolders = combineFolders;
    if (showProgressDialog)
    {
        CWizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetParentFolder));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyPersonalFolderToPersonalDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.copyPersonalFolderToPersonalDB();
        });
    }
}

void CWizDocumentOperator::copyPersonalFolderToGroupDB(const QString& sourceFolder, const WIZTAGDATA& targetParentTag,
                                                       bool keepDocTime, bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceFolder = sourceFolder;
    optData->targetTag = targetParentTag;
    optData->keepDocTime = keepDocTime;
    optData->combineFolders = combineFolders;
    if (showProgressDialog)
    {
        CWizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetParentTag.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyPersonalFolderToGroupDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.copyPersonalFolderToGroupDB();
        });
    }
}

void CWizDocumentOperator::copyGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder, const QString& targetParentFolder,
                                                       bool keepDocTime, bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceTag = groupFolder;
    optData->targetFolder = targetParentFolder;
    optData->keepDocTime = keepDocTime;
    optData->combineFolders = combineFolders;
    if (showProgressDialog)
    {
        CWizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetParentFolder));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyGroupFolderToPersonalDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.copyGroupFolderToPersonalDB();
        });
    }
}

void CWizDocumentOperator::copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                                    bool keepDocTime, bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceTag = sourceFolder;
    optData->targetTag = targetFolder;
    optData->keepDocTime = keepDocTime;
    optData->combineFolders = combineFolders;
    if (showProgressDialog)
    {
        CWizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Copy notes to %1").arg(targetFolder.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.copyGroupFolderToGroupDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.copyGroupFolderToGroupDB();
        });
    }
}

void CWizDocumentOperator::moveGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder, const QString& targetParentFolder,
                                                       bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceTag = groupFolder;
    optData->targetFolder = targetParentFolder;
    optData->combineFolders = combineFolders;
    if (showProgressDialog)
    {
        CWizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Move notes to %1").arg(targetParentFolder));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.moveGroupFolderToPersonalDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.moveGroupFolderToPersonalDB();
        });
    }
}

void CWizDocumentOperator::moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                                    bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceTag = sourceFolder;
    optData->targetTag = targetFolder;
    optData->combineFolders = combineFolders;
    if (showProgressDialog)
    {
        CWizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Move notes to %1").arg(targetFolder.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.moveGroupFolderToGroupDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.moveGroupFolderToGroupDB();
        });
    }
}

void CWizDocumentOperator::movePersonalFolderToPersonalDB(const QString& sourceFolder, const QString& targetParentFolder,
                                                          bool combineFolder)
{
    OperatorData* optData = new OperatorData();
    optData->sourceFolder = sourceFolder;
    optData->targetFolder = targetParentFolder;
    optData->combineFolders = combineFolder;

    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
        CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
        helper.movePersonalFolderToPersonalDB();
    });
}

void CWizDocumentOperator::movePersonalFolderToGroupDB(const QString& sourceFolder, const WIZTAGDATA& targetFolder,
                                                       bool combineFolders, bool showProgressDialog)
{
    OperatorData* optData = new OperatorData();
    optData->sourceFolder = sourceFolder;
    optData->targetTag = targetFolder;
    optData->combineFolders = combineFolders;
    if (showProgressDialog)
    {
        CWizProgressDialog dlg(qApp->activeWindow(), true);
        dlg.setWindowTitle(QObject::tr("Move notes to %1").arg(targetFolder.strName));

        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=, &dlg]() {
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.bindSignalsToProgressDialog(&dlg);
            helper.movePersonalFolderToGroupDB();
        });

        dlg.exec();
    }
    else
    {
        WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
            CWizDocumentOperatorPrivate helper(m_dbMgr, optData);
            helper.movePersonalFolderToGroupDB();
        });
    }
}

