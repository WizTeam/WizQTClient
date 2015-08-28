#include "wizObjectOperator.h"
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include "wizProgressDialog.h"
#include "wizObjectDataDownloader.h"
#include "wizDatabaseManager.h"
#include "wizDatabase.h"
#include "wizLineInputDialog.h"
#include "share/wizMessageBox.h"
#include "share/wizsettings.h"

CWizDocumentOperator::CWizDocumentOperator(CWizDatabaseManager& dbMgr, QObject* parent)
    : m_dbMgr(dbMgr)
    , QObject(parent)
    , m_downloader(nullptr)
    , m_keepDocTime(false)
    , m_keepTag(false)
    , m_stop(false)
    , m_thread(nullptr)
    , m_totoalCount(0)
    , m_counter(0)
    , m_combineFolders(false)
{
}

CWizDocumentOperator::~CWizDocumentOperator()
{
    if (m_thread)
    {
        connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
        m_thread->quit();
    }
}

void CWizDocumentOperator::copyDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument,
                                                     const QString& targetFolder, bool keepDocTime, bool keepTag,
                                                     CWizObjectDataDownloaderHost* downloader)
{
    m_arrayDocument = arrayDocument;
    m_targetFolder = targetFolder;
    m_keepDocTime = keepDocTime;
    m_keepTag = keepTag;
    m_downloader = downloader;

    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(copyDocumentToPersonalFolder()));
    m_thread->start();
}

void CWizDocumentOperator::copyDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument,
                                                      const WIZTAGDATA& targetTag, bool keepDocTime, CWizObjectDataDownloaderHost* downloader)
{
    m_arrayDocument = arrayDocument;
    m_targetTag = targetTag;
    m_keepDocTime = keepDocTime;
    m_downloader = downloader;

    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(copyDocumentToGroupFolder()));
    m_thread->start();
}

void CWizDocumentOperator::moveDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument,
                                                         const QString& targetFolder, CWizObjectDataDownloaderHost* downloader)
{
    m_arrayDocument = arrayDocument;
    m_targetFolder = targetFolder;
    m_downloader = downloader;

    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(moveDocumentToPersonalFolder()));
    m_thread->start();
}

void CWizDocumentOperator::moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument,
                                                      const WIZTAGDATA& targetTag, CWizObjectDataDownloaderHost* downloader)
{
    m_arrayDocument = arrayDocument;
    m_targetTag = targetTag;
    m_downloader = downloader;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(moveDocumentToGroupFolder()));
    m_thread->start();
}

void CWizDocumentOperator::deleteDocuments(const CWizDocumentDataArray& arrayDocument)
{
    m_arrayDocument = arrayDocument;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(deleteDocuments()));
    m_thread->start();
}

void CWizDocumentOperator::copyPersonalFolderToPersonalDB(const QString& sourceFolder,
                                                          const QString& targetParentFolder, bool keepDocTime,
                                                          bool keepTag, bool combineFolders, CWizObjectDataDownloaderHost* downloader)
{
    m_sourceFolder = sourceFolder;
    m_targetFolder = targetParentFolder;
    m_keepDocTime = keepDocTime;
    m_keepTag = keepTag;
    m_downloader = downloader;
    m_combineFolders = combineFolders;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(copyPersonalFolderToPersonalDB()));
    m_thread->start();
}

void CWizDocumentOperator::copyPersonalFolderToGroupDB(const QString& sourceFolder,
                                                       const WIZTAGDATA& targetParentTag, bool keepDocTime, bool combineFolders,
                                                       CWizObjectDataDownloaderHost* downloader)
{
    m_sourceFolder = sourceFolder;
    m_targetTag = targetParentTag;
    m_keepDocTime = keepDocTime;
    m_downloader = downloader;
    m_combineFolders = combineFolders;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(copyPersonalFolderToGroupDB()));
    m_thread->start();
}

void CWizDocumentOperator::copyGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder,
                                                       const QString& targetParentFolder, bool keepDocTime, bool combineFolders,
                                                       CWizObjectDataDownloaderHost* downloader)
{
    m_sourceTag = groupFolder;
    m_targetFolder = targetParentFolder;
    m_keepDocTime = keepDocTime;
    m_downloader = downloader;
    m_combineFolders = combineFolders;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(copyGroupFolderToPersonalDB()));
    m_thread->start();
}

void CWizDocumentOperator::copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder,
                                                    const WIZTAGDATA& targetFolder, bool keepDocTime, bool combineFolders,
                                                    CWizObjectDataDownloaderHost* downloader)
{
    m_sourceTag = sourceFolder;
    m_targetTag = targetFolder;
    m_keepDocTime = keepDocTime;
    m_downloader = downloader;
    m_combineFolders = combineFolders;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(copyGroupFolderToGroupDB()));
    m_thread->start();
}

void CWizDocumentOperator::moveGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder,
                                                       const QString& targetParentFolder, bool combineFolders,
                                                       CWizObjectDataDownloaderHost* downloader)
{
    m_sourceTag = groupFolder;
    m_targetFolder = targetParentFolder;
    m_downloader = downloader;
    m_combineFolders = combineFolders;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(moveGroupFolderToPersonalDB()));
    m_thread->start();
}

void CWizDocumentOperator::moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                                     bool combineFolders, CWizObjectDataDownloaderHost* downloader)
{
    m_sourceTag = sourceFolder;
    m_targetTag = targetFolder;
    m_downloader = downloader;
    m_combineFolders = combineFolders;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(moveGroupFolderToGroupDB()));
    m_thread->start();
}

void CWizDocumentOperator::movePersonalFolderToPersonalDB(const QString& sourceFolder, const QString& targetParentFolder, bool combineFolder)
{
    m_sourceFolder = sourceFolder;
    m_targetFolder = targetParentFolder;
    m_combineFolders = combineFolder;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(movePersonalFolderToPersonalDB()));
    m_thread->start();
}

void CWizDocumentOperator::movePersonalFolderToGroupDB(const QString& sourceFolder, const WIZTAGDATA& targetFolder,
                                                       bool combineFolders, CWizObjectDataDownloaderHost* downloader)
{
    m_sourceFolder = sourceFolder;
    m_targetTag = targetFolder;
    m_downloader = downloader;
    m_combineFolders = combineFolders;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(movePersonalFolderToGroupDB()));
    m_thread->start();
}

void CWizDocumentOperator::bindSignalsToProgressDialog(CWizProgressDialog* progress)
{
    QObject::connect(this, SIGNAL(progress(int,int)), progress, SLOT(setProgress(int,int)));
    QObject::connect(this, SIGNAL(newAction(QString)), progress, SLOT(setActionString(QString)));
    QObject::connect(this, SIGNAL(finished()), progress, SLOT(accept()));
    QObject::connect(progress, SIGNAL(stopRequest()), this, SLOT(stop()), Qt::DirectConnection);
}

void CWizDocumentOperator::stop()
{
    m_stop = true;
}

void CWizDocumentOperator::copyDocumentToPersonalFolder()
{
    int nCounter = 0;
    CWizDocumentDataArray::iterator it;
    for (it = m_arrayDocument.begin(); it != m_arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        copyDocumentToPersonalFolder(doc);
        nCounter ++;
        emit progress(m_arrayDocument.size(), nCounter);
    }

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::copyDocumentToGroupFolder()
{
    int nCounter = 0;
    CWizDocumentDataArray::iterator it;
    for (it = m_arrayDocument.begin(); it != m_arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        copyDocumentToGroupFolder(doc);
        nCounter ++;
        emit progress(m_arrayDocument.size(), nCounter);
    }

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::moveDocumentToPersonalFolder()
{
    int nCounter = 0;
    CWizDocumentDataArray::iterator it;
    for (it = m_arrayDocument.begin(); it != m_arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        moveDocumentToPersonalFolder(doc);
        nCounter ++;
        emit progress(m_arrayDocument.size(), nCounter);
    }

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::moveDocumentToGroupFolder()
{
    int nCounter = 0;
    CWizDocumentDataArray::iterator it;
    for (it = m_arrayDocument.begin(); it != m_arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        moveDocumentToGroupFolder(doc);
        nCounter ++;
        emit progress(m_arrayDocument.size(), nCounter);
    }

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::deleteDocuments()
{
    m_totoalCount = m_arrayDocument.size();
    m_counter = 0;
    for (WIZDOCUMENTDATA doc : m_arrayDocument)
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
    //
    deleteLater();
}

void CWizDocumentOperator::copyPersonalFolderToPersonalDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(), m_sourceFolder);

    //
    QString sourceFolderName = CWizDatabase::GetLocationName(m_sourceFolder);
    QString uniqueFolder = getUniqueFolderName(m_targetFolder, sourceFolderName);
    if (m_combineFolders)
    {
        uniqueFolder.clear();
    }

    copyPersonalFolderToPersonalDB(m_sourceFolder, m_targetFolder, uniqueFolder);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::copyPersonalFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(), m_sourceFolder);

    //    
    QString targetTagName = getUniqueFolderName(m_targetTag, m_sourceFolder, m_combineFolders);
    copyPersonalFolderToGroupDB(m_sourceFolder, m_targetTag, targetTagName);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::copyGroupFolderToPersonalDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_sourceTag.strKbGUID), m_sourceTag);
    //
    QString uniqueFolder = getUniqueFolderName(m_targetFolder, m_sourceTag.strName);
    if (uniqueFolder != m_sourceTag.strName && !m_combineFolders)
    {
        m_sourceTag.strName = uniqueFolder;
    }

    copyGroupFolderToPersonalDB(m_sourceTag, m_targetFolder);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::copyGroupFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_sourceTag.strKbGUID), m_sourceTag);    

    m_sourceTag.strName = getUniqueFolderName(m_targetTag, m_sourceTag, m_combineFolders);
    copyGroupFolderToGroupDB(m_sourceTag, m_targetTag);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::moveGroupFolderToPersonalDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_sourceTag.strKbGUID), m_sourceTag);

    //
    QString uniqueFolder = getUniqueFolderName(m_targetFolder, m_sourceTag.strName);
    if (uniqueFolder != m_sourceTag.strName && !m_combineFolders)
    {
        m_sourceTag.strName = uniqueFolder;
    }

    moveGroupFolderToPersonalDB(m_sourceTag, m_targetFolder);    

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::moveGroupFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_sourceTag.strKbGUID), m_sourceTag);

    m_sourceTag.strName = getUniqueFolderName(m_targetTag, m_sourceTag, m_combineFolders, m_sourceTag.strGUID);
    moveGroupFolderToGroupDB(m_sourceTag, m_targetTag);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::movePersonalFolderToPersonalDB()
{    
    m_totoalCount = documentCount(m_dbMgr.db(), m_sourceFolder);

    //
    QString sourceFolderName = CWizDatabase::GetLocationName(m_sourceFolder);
    QString uniqueFolder = getUniqueFolderName(m_targetFolder, sourceFolderName);
    if (m_combineFolders)
    {
        uniqueFolder.clear();
    }

    _movePersonalFolderToPersonalDB(m_sourceFolder, m_targetFolder, uniqueFolder);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::movePersonalFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(), m_sourceFolder);

    //    
    QString targetTagName = getUniqueFolderName(m_targetTag, m_sourceFolder, m_combineFolders);
    movePersonalFolderToGroupDB(m_sourceFolder, m_targetTag, targetTagName);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::copyDocumentToPersonalFolder(const WIZDOCUMENTDATA& doc)
{
    CWizDatabase& sourceDb = m_dbMgr.db(doc.strKbGUID);
    CWizDatabase& targetDb = m_dbMgr.db();
    CWizFolder folder(targetDb, m_targetFolder);
    CWizDocument wizDoc(sourceDb, doc);
    QString newDocGUID;
    if (!wizDoc.CopyTo(targetDb, &folder, m_keepDocTime, m_keepTag, newDocGUID, m_downloader))
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

void CWizDocumentOperator::copyDocumentToGroupFolder(const WIZDOCUMENTDATA& doc)
{
    CWizDocument wizDoc(m_dbMgr.db(doc.strKbGUID), doc);
    wizDoc.CopyTo(m_dbMgr.db(m_targetTag.strKbGUID), m_targetTag, m_keepDocTime, m_downloader);
}

void CWizDocumentOperator::moveDocumentToPersonalFolder(const WIZDOCUMENTDATA& doc)
{
    CWizFolder folder(m_dbMgr.db(), m_targetFolder);
    CWizDocument wizDoc(m_dbMgr.db(doc.strKbGUID), doc);
    wizDoc.MoveTo(m_dbMgr.db(), &folder, m_downloader);
}

void CWizDocumentOperator::moveDocumentToGroupFolder(const WIZDOCUMENTDATA& doc)
{
    CWizDocument wizDoc(m_dbMgr.db(doc.strKbGUID), doc);
    wizDoc.MoveTo(m_dbMgr.db(m_targetTag.strKbGUID), m_targetTag, m_downloader);
}


void CWizDocumentOperator::copyPersonalFolderToPersonalDB(const QString& childFolder,
                                                          const QString& targetParentFolder, const QString& targetFolderName)
{
    CWizDatabase& db = m_dbMgr.db();
    CWizDocumentDataArray arrayDocument;
    db.GetDocumentsByLocation(childFolder, arrayDocument, false);
    //
    QString folderName = targetFolderName.isEmpty() ? db.GetLocationName(childFolder) : targetFolderName;
    QString targetFolder = targetParentFolder + folderName + "/";
    m_targetFolder = targetFolder;

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

void CWizDocumentOperator::copyPersonalFolderToGroupDB(const QString& childFolder,
                                                       const WIZTAGDATA& targetParentTag, const QString& targetTagName)
{
    CWizDatabase& db = m_dbMgr.db();
    CWizDatabase& targetDB = m_dbMgr.db(targetParentTag.strKbGUID);
    QString tagName = targetTagName.isEmpty() ? db.GetLocationName(childFolder) : targetTagName;
    WIZTAGDATA targetTag;
    targetDB.CreateTag(targetParentTag.strGUID, tagName, "", targetTag);
    m_targetTag = targetTag;
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

void CWizDocumentOperator::copyGroupFolderToPersonalDB(const WIZTAGDATA& childFolder, const QString& targetParentFolder)
{
    CWizDatabase& groupDB = m_dbMgr.db(childFolder.strKbGUID);
    CWizDocumentDataArray arrayDocument;
    groupDB.GetDocumentsByTag(childFolder, arrayDocument);
    QString targetFolder = targetParentFolder + childFolder.strName + "/";
    m_targetFolder = targetFolder;

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

void CWizDocumentOperator::copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder)
{
    CWizDatabase& targetDB = m_dbMgr.db(targetFolder.strKbGUID);
    WIZTAGDATA targetTag;
    targetDB.CreateTag(targetFolder.strGUID, sourceFolder.strName, sourceFolder.strDescription, targetTag);
    m_targetTag = targetTag;

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

void CWizDocumentOperator::moveGroupFolderToPersonalDB(const WIZTAGDATA& childFolder, const QString& targetParentFolder)
{
    CWizDatabase& groupDB = m_dbMgr.db(childFolder.strKbGUID);
    CWizDocumentDataArray arrayDocument;
    groupDB.GetDocumentsByTag(childFolder, arrayDocument);
    QString targetFolder = targetParentFolder + childFolder.strName + "/";
    m_targetFolder = targetFolder;
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

void CWizDocumentOperator::moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetParentTag)
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
        m_targetTag = targetTag;
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

void CWizDocumentOperator::_movePersonalFolderToPersonalDB(const QString& childFolder, const QString& targetParentFolder,
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

void CWizDocumentOperator::movePersonalFolderToGroupDB(const QString& sourceFolder,
                                                       const WIZTAGDATA& targetParentTag, const QString& targetTagName)
{
    CWizDatabase& db = m_dbMgr.db();
    CWizDatabase& targetDB = m_dbMgr.db(targetParentTag.strKbGUID);
    QString tagName = targetTagName.isEmpty() ? db.GetLocationName(sourceFolder) : targetTagName;
    WIZTAGDATA targetTag;
    targetDB.CreateTag(targetParentTag.strGUID, tagName, "", targetTag);
    m_targetTag = targetTag;
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

int CWizDocumentOperator::documentCount(CWizDatabase& db, const QString& personalFolder)
{
    int count;
    db.GetDocumentsCountByLocation(personalFolder, count, true);
    return count;
}

int CWizDocumentOperator::documentCount(CWizDatabase& db, const WIZTAGDATA& groupFolder)
{
    int count;
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

void CWizDocumentOperator::combineSameNameGroupFolder(const WIZTAGDATA& parentTag, const WIZTAGDATA& childTag)
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
        if (m_combineFolders)
        {
            CWizDocumentDataArray arrayDocument;
            targetDB.GetDocumentsByTag(childTag, arrayDocument);
            //
            m_targetTag = sameNameBrother;
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

QString CWizDocumentOperator::getUniqueTagName(const WIZTAGDATA& parentTag, const WIZTAGDATA& tag)
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

QString CWizDocumentOperator::getUniqueFolderName(const QString& parentLocation, const QString& locationName)
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

QString CWizDocumentOperator::getUniqueFolderName(const WIZTAGDATA& parentTag, const WIZTAGDATA& sourceTag, bool combineFolder, const QString& exceptGUID)
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

    return m_sourceTag.strName;
}

QString CWizDocumentOperator::getUniqueFolderName(const WIZTAGDATA& parentTag, const QString& sourceFolder, bool combineFolder)
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
