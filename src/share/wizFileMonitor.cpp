#include "wizFileMonitor.h"
#include "wizmisc.h"
#include "../utils/logger.h"
#include <QFileInfo>
#include <QDebug>

CWizFileMonitor::CWizFileMonitor(QObject *parent) :
    QThread(parent)
  , m_stop(false)
{    
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timerOut()));
    m_timer.start(5 * 1000);
}

CWizFileMonitor::~CWizFileMonitor()
{
    stop();
    WizWaitForThread(this);
}

CWizFileMonitor&CWizFileMonitor::instance()
{
    static CWizFileMonitor instance;
    return instance;
}

void CWizFileMonitor::addFile(const QString strKbGUID, const QString& strGUID,
                              const QString& strFileName, const QString& strMD5, const QDateTime& dtLastModified)
{
    Q_ASSERT(!strFileName.isEmpty());

    {
        QMutexLocker locker(&m_mutex);
        foreach (FMData fmData, m_fileList) {
            if (fmData.strFileName == strFileName)
                return;
        }

        FMData fileData;
        fileData.strKbGUID = strKbGUID;
        fileData.strGUID = strGUID;
        fileData.strFileName = strFileName;
        fileData.strMD5 = strMD5;
        fileData.dtLastModified = dtLastModified;

        m_fileList.append(fileData);
    }

    if (!isRunning())
    {
        start();
    }
}

void CWizFileMonitor::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stop  = true;
    m_wait.wakeAll();
}

void CWizFileMonitor::on_timerOut()
{
    QMutexLocker locker(&m_mutex);
    m_wait.wakeAll();
}

void CWizFileMonitor::run()
{
    while (!m_stop)
    {
        checkFiles();
        //
        QMutexLocker locker(&m_mutex);
        m_wait.wait(&m_mutex);
    }
}

void CWizFileMonitor::checkFiles()
{
    m_mutex.lock();
    QList<FMData> fileList = m_fileList;
    m_mutex.unlock();


    QList<FMData>::iterator fmIter;
    for (fmIter = fileList.begin(); fmIter != fileList.end(); fmIter++)
    {
        if (m_stop)
            break;

        FMData& fileData = *fmIter;

        QFileInfo info(fileData.strFileName);
        if (info.lastModified() > fileData.dtLastModified)
        {
            QString strMD5 = WizMd5FileString(fileData.strFileName);
            if (strMD5 == fileData.strMD5)
            {
                TOLOG("[FileMoniter] file modified, but md5 keep same");
            }
            else
            {
                fileData.strMD5 = strMD5;
            }

            fileData.dtLastModified = info.lastModified();
            //
            emit fileModified(fileData.strKbGUID, fileData.strGUID, fileData.strFileName,
                              fileData.strMD5, fileData.dtLastModified);
        }
    }
}
