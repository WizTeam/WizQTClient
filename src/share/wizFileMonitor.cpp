#include "wizFileMonitor.h"
#include "wizmisc.h"
#include "../utils/logger.h"
#include <QFileInfo>
#include <QDebug>

CWizFileMonitor::CWizFileMonitor(QObject *parent) :
    QThread(parent)
  , m_stop(false)
{
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

    return;
    Q_ASSERT(!strFileName.isEmpty());

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

    if (!isRunning())
    {
//        start();
    }
}

void CWizFileMonitor::stop()
{
    m_stop  = true;
}

void CWizFileMonitor::run()
{
    while (!m_stop)
    {
        sleep(1);

        checkFiles();
    }
}

void CWizFileMonitor::checkFiles()
{
    QList<FMData>::iterator fmIter;
    for (fmIter = m_fileList.begin(); fmIter != m_fileList.end(); fmIter++)
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
