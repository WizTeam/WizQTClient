#include "wizFileMonitor.h"
#include "wizmisc.h"
#include "../utils/logger.h"
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include "wizthreads.h"

#define WIZ_THREAD_FILE_MONITOR     1024

CWizFileMonitor::CWizFileMonitor(QObject *parent) :
    QObject(parent)
{    
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timerOut()));
    m_timer.start(5 * 1000);
}

CWizFileMonitor&CWizFileMonitor::instance()
{
    static CWizFileMonitor instance;
    return instance;
}

void CWizFileMonitor::addFile(const QString& strKbGUID, const QString& strGUID,
                              const QString& strFileName, const QString& strMD5)
{
    Q_ASSERT(!strFileName.isEmpty());

    QFileInfo fileInfo(strFileName);

    FMData fileData;
    fileData.strKbGUID = strKbGUID;
    fileData.strGUID = strGUID;
    fileData.strFileName = strFileName;
    fileData.strMD5 = strMD5;
    fileData.dtLastModified = fileInfo.lastModified();

    ::WizExecuteOnThread(WIZ_THREAD_FILE_MONITOR, [=]{

        for (const FMData& fmData: m_fileList) {
            if (fmData.strFileName == fileData.strFileName)
                return;
        }

        m_fileList.append(fileData);
        //
    });
}

void CWizFileMonitor::on_timerOut()
{
    long pid = (long)QCoreApplication::applicationPid();
    //
    bool isForeground = false;
    QList<WizWindowInfo> windowInfos = WizGetActiveWindows();
    for (const WizWindowInfo& info : windowInfos)
    {
        if (info.pid == pid)
        {
            isForeground = true;
            break;
        }
    }
    if (!isForeground)
        return;
    //
    //
    ::WizExecuteOnThread(WIZ_THREAD_FILE_MONITOR, [=]{

        checkFiles();
        //
    });
}

void CWizFileMonitor::checkFiles()
{
    for (FMData& fileData: m_fileList)
    {
        QFileInfo info(fileData.strFileName);
        if (info.lastModified() > fileData.dtLastModified)
        {
            QString strMD5 = WizMd5FileString(fileData.strFileName);
            if (strMD5 == fileData.strMD5)
            {
                TOLOG("[FileMoniter] file modified, but md5 keep same");
                continue;
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
