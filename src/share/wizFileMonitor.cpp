#include "wizFileMonitor.h"
#include "wizmisc.h"
#include "../utils/logger.h"
#include <QFileInfo>

CWizFileMonitor::CWizFileMonitor(QObject *parent) :
    QThread(parent)
  , m_stop(false)
{
}

CWizFileMonitor::~CWizFileMonitor()
{
    stop();
}

CWizFileMonitor&CWizFileMonitor::instance()
{
    static CWizFileMonitor instance;
    return instance;
}

void CWizFileMonitor::addFile(const QString strKbGUID, const QString& strGUID,
                              const QString& strFileName, const QString& strMD5, const QDateTime& dtLastModified)
{
    Q_ASSERT(strFileName.isEmpty());

    FMData fileData;
    fileData.strKbGUID = strKbGUID;
    fileData.strGUID = strGUID;
    fileData.strFileName = strFileName;
    fileData.strMD5 = strMD5;
    fileData.dtLastModified = dtLastModified;

    m_fileList.append(fileData);

    if (!isRunning())
    {
        start();
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
    foreach (FMData fileData, m_fileList) {
        if (m_stop)
            break;

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

            emit fileModified(fileData.strKbGUID, fileData.strGUID, fileData.strFileName,
                              fileData.strMD5, fileData.dtLastModified);
        }
    }
}
