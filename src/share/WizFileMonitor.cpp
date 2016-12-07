#include "WizFileMonitor.h"
#include "WizMisc.h"
#include "../utils/WizLogger.h"
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include "WizThreads.h"

#define WIZ_THREAD_FILE_MONITOR     1024

WizFileMonitor::WizFileMonitor(QObject *parent) :
    QObject(parent)
{    
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timerOut()));
    m_timer.start(5 * 1000);
}

WizFileMonitor&WizFileMonitor::instance()
{
    static WizFileMonitor instance;
    return instance;
}

void WizFileMonitor::addFile(const QString& strKbGUID, const QString& strGUID,
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

void WizFileMonitor::on_timerOut()
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

void WizFileMonitor::checkFiles()
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
