#include "wizObjectOperator.h"
#include <QThread>
#include <QDebug>
#include <QTimer>
#include "wizProgressDialog.h"
#include "wizObjectDataDownloader.h"
#include "wizDatabaseManager.h"
#include "wizDatabase.h"

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
    qDebug() << "prepare to copy in thread : " << QThread::currentThreadId();
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

void CWizDocumentOperator::copyPersonalFolderToPersonalDB(const QString& sourceFolder,
                                                          const QString& targetParentFolder, bool keepDocTime,
                                                          bool keepTag, CWizObjectDataDownloaderHost* downloader)
{
    m_sourceFolder = sourceFolder;
    m_targetFolder = targetParentFolder;
    m_keepDocTime = keepDocTime;
    m_keepTag = keepTag;
    m_downloader = downloader;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(copyPersonalFolderToPersonalDB()));
    m_thread->start();
}

void CWizDocumentOperator::copyPersonalFolderToGroupDB(const QString& sourceFolder,
                                                       const WIZTAGDATA& targetParentTag, bool keepDocTime,
                                                       CWizObjectDataDownloaderHost* downloader)
{
    m_sourceFolder = sourceFolder;
    m_targetTag = targetParentTag;
    m_keepDocTime = keepDocTime;
    m_downloader = downloader;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(copyPersonalFolderToGroupDB()));
    m_thread->start();
}

void CWizDocumentOperator::copyGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder,
                                                       const QString& targetParentFolder, bool keepDocTime,
                                                       CWizObjectDataDownloaderHost* downloader)
{
    m_sourceTag = groupFolder;
    m_targetFolder = targetParentFolder;
    m_keepDocTime = keepDocTime;
    m_downloader = downloader;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(copyGroupFolderToPersonalDB()));
    m_thread->start();
}

void CWizDocumentOperator::copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder,
                                                    const WIZTAGDATA& targetFolder, bool keepDocTime,
                                                    CWizObjectDataDownloaderHost* downloader)
{
    m_sourceTag = sourceFolder;
    m_targetTag = targetFolder;
    m_keepDocTime = keepDocTime;
    m_downloader = downloader;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(copyGroupFolderToGroupDB()));
    m_thread->start();
}

void CWizDocumentOperator::moveGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder,
                                                       const QString& targetParentFolder, CWizObjectDataDownloaderHost* downloader)
{
    m_sourceTag = groupFolder;
    m_targetFolder = targetParentFolder;
    m_downloader = downloader;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(moveGroupFolderToPersonalDB()));
    m_thread->start();
}

void CWizDocumentOperator::moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder,
                                                    const WIZTAGDATA& targetFolder, CWizObjectDataDownloaderHost* downloader)
{
    m_sourceTag = sourceFolder;
    m_targetTag = targetFolder;
    m_downloader = downloader;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(moveGroupFolderToGroupDB()));
    m_thread->start();
}

void CWizDocumentOperator::movePersonalFolderToPersonalDB(const QString& sourceFolder, const QString& targetParentFolder)
{
    m_sourceFolder = sourceFolder;
    m_targetFolder = targetParentFolder;
    //
    m_thread = new QThread();
    moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), SLOT(movePersonalFolderToPersonalDB()));
    m_thread->start();
}

void CWizDocumentOperator::movePersonalFolderToGroupDB(const QString& sourceFolder,
                                                       const WIZTAGDATA& targetFolder, CWizObjectDataDownloaderHost* downloader)
{
    m_sourceFolder = sourceFolder;
    m_targetTag = targetFolder;
    m_downloader = downloader;
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
    qDebug() << "stop document operator in thread : " << QThread::currentThreadId();
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
        qDebug() << "copy finished : " << nCounter << "  need stop : " << m_stop;
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
        qDebug() << "copy finished : " << nCounter << "  need stop : " << m_stop;
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
        qDebug() << "copy finished : " << nCounter << "  need stop : " << m_stop;
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
        qDebug() << "copy finished : " << nCounter << "  need stop : " << m_stop;
        emit progress(m_arrayDocument.size(), nCounter);
    }

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::copyPersonalFolderToPersonalDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(), m_sourceFolder);
    copyPersonalFolderToPersonalDB(m_sourceFolder, m_targetFolder);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::copyPersonalFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(), m_sourceFolder);
    copyPersonalFolderToGroupDB(m_sourceFolder, m_targetTag);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::copyGroupFolderToPersonalDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_sourceTag.strKbGUID), m_sourceTag);
    copyGroupFolderToPersonalDB(m_sourceTag, m_targetFolder);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::copyGroupFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_sourceTag.strKbGUID), m_sourceTag);
    copyGroupFolderToGroupDB(m_sourceTag, m_targetTag);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::moveGroupFolderToPersonalDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_sourceTag.strKbGUID), m_sourceTag);
    moveGroupFolderToPersonalDB(m_sourceTag, m_targetFolder);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::moveGroupFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(m_sourceTag.strKbGUID), m_sourceTag);
    moveGroupFolderToGroupDB(m_sourceTag, m_targetTag);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::movePersonalFolderToPersonalDB()
{    
    m_totoalCount = documentCount(m_dbMgr.db(), m_sourceFolder);
    _movePersonalFolderToPersonalDB(m_sourceFolder, m_targetFolder);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::movePersonalFolderToGroupDB()
{
    m_totoalCount = documentCount(m_dbMgr.db(), m_sourceFolder);
    movePersonalFolderToGroupDB(m_sourceFolder, m_targetTag);

    emit finished();
    //
    deleteLater();
}

void CWizDocumentOperator::copyDocumentToPersonalFolder(const WIZDOCUMENTDATA& doc)
{
    CWizFolder folder(m_dbMgr.db(), m_targetFolder);
    CWizDocument wizDoc(m_dbMgr.db(doc.strKbGUID), doc);
    wizDoc.CopyTo(m_dbMgr.db(), &folder, m_keepDocTime, m_keepTag, m_downloader);
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
                                                          const QString& targetParentFolder)
{
    CWizDatabase& db = m_dbMgr.db();
    CWizDocumentDataArray arrayDocument;
    db.GetDocumentsByLocation(childFolder, arrayDocument, false);
    //
    QString folderName = db.GetLocationName(childFolder);
    QString targetFolder = targetParentFolder + folderName + "/";
    m_targetFolder = targetFolder;

    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Copy note %1").arg(doc.strTitle));
        copyDocumentToPersonalFolder(doc);
        m_counter ++;
        qDebug() << "copy finished : " << m_counter << "  need stop : " << m_stop;
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
            copyPersonalFolderToPersonalDB(newChildFolder, targetFolder);
            if (m_stop)
                return;
        }
    }
}

void CWizDocumentOperator::copyPersonalFolderToGroupDB(const QString& childFolder, const WIZTAGDATA& targetParentTag)
{
    CWizDatabase& db = m_dbMgr.db();
    CWizDatabase& targetDB = m_dbMgr.db(targetParentTag.strKbGUID);
    WIZTAGDATA targetTag;
    targetDB.CreateTag(targetParentTag.strGUID, db.GetLocationName(childFolder), "", targetTag);
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
        qDebug() << "copy finished : " << m_counter << "  need stop : " << m_stop;
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
            copyPersonalFolderToGroupDB(newChildFolder, targetTag);
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
        qDebug() << "copy finished : " << m_counter << "  need stop : " << m_stop;
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
        qDebug() << "copy finished : " << m_counter << "  need stop : " << m_stop;
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
    qDebug() << "moveGroupFolderToPersonalDB, group ;  " << childFolder.strName << "  target : " << m_targetFolder << " documents ; " << arrayDocument.size();
    //
    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end() && !m_stop; it++)
    {
        const WIZDOCUMENTDATA& doc = *it;
        emit newAction(tr("Move note %1").arg(doc.strTitle));
        moveDocumentToPersonalFolder(doc);
        m_counter ++;
        qDebug() << "copy finished : " << m_counter << "  need stop : " << m_stop;
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

void CWizDocumentOperator::moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder)
{
    CWizDatabase& sourceDB = m_dbMgr.db(sourceFolder.strKbGUID);
    if (sourceFolder.strKbGUID == targetFolder.strKbGUID)
    {
        qDebug() << "folder in same db, just modify parent tag guid";
        WIZTAGDATA sourceTag = sourceFolder;
        sourceTag.strParentGUID = targetFolder.strGUID;
        sourceDB.ModifyTag(sourceTag);
    }
    else
    {
        CWizDatabase& targetDB = m_dbMgr.db(targetFolder.strKbGUID);
        WIZTAGDATA targetTag;
        targetDB.CreateTag(targetFolder.strGUID, sourceFolder.strName, sourceFolder.strDescription, targetTag);
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
            qDebug() << "copy finished : " << m_counter << "  need stop : " << m_stop;
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

void CWizDocumentOperator::_movePersonalFolderToPersonalDB(const QString& childFolder, const QString& targetParentFolder)
{
    CWizDatabase& db = m_dbMgr.db();
    CWizDocumentDataArray arrayDocument;
    db.GetDocumentsByLocation(childFolder, arrayDocument, false);
    //
    QString folderName = db.GetLocationName(childFolder);
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
            _movePersonalFolderToPersonalDB(newChildFolder, targetFolder);
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

void CWizDocumentOperator::movePersonalFolderToGroupDB(const QString& sourceFolder, const WIZTAGDATA& targetFolder)
{
    CWizDatabase& db = m_dbMgr.db();
    CWizDatabase& targetDB = m_dbMgr.db(targetFolder.strKbGUID);
    WIZTAGDATA targetTag;
    targetDB.CreateTag(targetFolder.strGUID, db.GetLocationName(sourceFolder), "", targetTag);
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
            movePersonalFolderToGroupDB(childFolder, targetTag);
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
    qDebug() << "get coument count : " << count;
    return count;
}

int CWizDocumentOperator::documentCount(CWizDatabase& db, const WIZTAGDATA& groupFolder)
{
    int count;
    CWizTagDataArray arrayTag;
    db.GetAllChildTags(groupFolder.strGUID, arrayTag);
    arrayTag.push_back(groupFolder);
    qDebug() << "get all child tag count : " << arrayTag.size();
    std::map<CString, int> mapTagDocumentCount;
    db.GetAllTagsDocumentCount(mapTagDocumentCount);
    for (WIZTAGDATA tag : arrayTag)
    {
       std::map<CString, int>::const_iterator pos =  mapTagDocumentCount.find(tag.strGUID);
       if (pos != mapTagDocumentCount.end())
       {
           qDebug() << "get tag document count : tag " << tag.strName << "  count : " << pos->second;
           count += pos->second;
       }
    }

    return count;
}

