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
{
}

CWizDocumentOperator::~CWizDocumentOperator()
{
    if (m_thread)
    {
        qDebug() << "copy destractor in thread : " << QThread::currentThreadId();
        connect(m_thread, &QThread::destroyed, [](){
           qDebug() << "document opreator destroyed";
        });

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

