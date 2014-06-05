#include "wizDocumentEditStatus.h"
#include "sync/apientry.h"
#include "share/wizmisc.h"
#include "rapidjson/document.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTextCodec>

#include <QDebug>


QString WizKMGetDocumentEditStatusURL()
{
    static QString strUrl = 0;
    if (strUrl.isEmpty())
    {
        QString strCmd = "note_edit_status_url";
        QString strRequestUrl = WizService::ApiEntry::standardCommandUrl(strCmd);

        QNetworkAccessManager* net = new QNetworkAccessManager();
        QNetworkReply* reply = net->get(QNetworkRequest(strRequestUrl));

        QEventLoop loop;
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        if (reply->error()) {
            return 0;
        }

        strUrl = QString::fromUtf8(reply->readAll().constData());

        net->deleteLater();
    }

    return strUrl;
}

CWizDocumentEditStatusSyncThread::CWizDocumentEditStatusSyncThread(QObject* parent)
    : QThread(parent)
    , m_mutext(QMutex::Recursive)
    , m_stop(false)
    , m_sendNow(false)
{
}

void CWizDocumentEditStatusSyncThread::stopEditingDocument()
{
    setCurrentEditingDocument("", "", "");
}

void CWizDocumentEditStatusSyncThread::setCurrentEditingDocument(const QString& strUserAlias, const QString& strKbGUID, const QString& strGUID)
{
    QString strObjID = "";
    if (!strKbGUID.isEmpty())
    {
        strObjID = strKbGUID + "/" + strGUID;
    }

    m_mutext.lock();
    //
    if (m_editingObj.strObjID != strObjID)
    {
        if (!m_editingObj.strObjID.isEmpty())
        {
            m_oldObj = m_editingObj;
        }
        //
        m_editingObj.strObjID = strObjID;
        m_editingObj.strUserName = strUserAlias;
        //
        m_sendNow = true;
    }
    //
    m_mutext.unlock();

    if (!isRunning())
        start();
}

void CWizDocumentEditStatusSyncThread::stop()
{
    //If thread wasn't running, no need to stop. Otherwise will start running ,and cause crash at destructor of program.
    if (!isRunning())
        return;

    m_stop = true;
    //
    stopEditingDocument();
}

void CWizDocumentEditStatusSyncThread::waitForDone()
{
    stop();
    //
    WizWaitForThread(this);
}

void CWizDocumentEditStatusSyncThread::run()
{
    int idleCounter = 0;
    while (1)
    {
        if (idleCounter >= 30 || m_sendNow || m_stop)
        {
            sendEditingMessage();
            sendDoneMessage();
            //
            idleCounter = 0;
            m_sendNow = false;
            //
            if (m_stop)
                return;
        }
        else
        {
            idleCounter++;
            msleep(1000);
        }
    }
}



void CWizDocumentEditStatusSyncThread::sendEditingMessage()
{
    m_mutext.lock();
    EditStatusObj editingObj = m_editingObj;
    m_mutext.unlock();

    if (!editingObj.strObjID.isEmpty() && !editingObj.strUserName.isEmpty())
    {
        sendEditingMessage(editingObj.strUserName, editingObj.strObjID);
    }
}

void CWizDocumentEditStatusSyncThread::sendEditingMessage(const QString& strUserAlias, const QString& strObjID)
{
    QString strUrl = ::WizFormatString4(_T("%1/add?obj_id=%2&user_id=%3&t=%4"),
                                        WizKMGetDocumentEditStatusURL(),
                                        strObjID,
                                        strUserAlias,
                                        ::WizIntToStr(GetTickCount()));

    if (!m_netManager)
    {
        m_netManager = new QNetworkAccessManager();
    }
    QNetworkReply* reply = m_netManager->get(QNetworkRequest(strUrl));

    qDebug() << "sendEditingMessage called " <<strObjID;

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}

void CWizDocumentEditStatusSyncThread::sendDoneMessage()
{
    m_mutext.lock();
    EditStatusObj doneObj = m_oldObj;
    // send done message just once
    m_oldObj.clear();
    m_mutext.unlock();

    if (!doneObj.strObjID.isEmpty() && !doneObj.strUserName.isEmpty())
    {
        sendDoneMessage(doneObj.strUserName, doneObj.strObjID);
    }
}

void CWizDocumentEditStatusSyncThread::sendDoneMessage(const QString& strUserAlias, const QString& strObjID)
{
    QString strUrl = WizFormatString4(_T("%1/delete?obj_id=%2&user_id=%3&t=%4"),
                                      WizKMGetDocumentEditStatusURL(),
                                      strObjID,
                                      strUserAlias,
                                      ::WizIntToStr(GetTickCount()));

    if (!m_netManager)
    {
        m_netManager = new QNetworkAccessManager();
    }
    QNetworkReply* reply = m_netManager->get(QNetworkRequest(strUrl));
    qDebug() << "sendDoneMessage called " <<strObjID;

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}


CWizDocumentEditStatusCheckThread::CWizDocumentEditStatusCheckThread(QObject* parent)
    : QThread(parent)
    , m_stop(false)
    , m_mutexWait(QMutex::NonRecursive)
{
}

void CWizDocumentEditStatusCheckThread::waitForDone()
{
    stop();
    //
    WizWaitForThread(this);
}

void CWizDocumentEditStatusCheckThread::checkEditStatus(const QString& strKbGUID, const QString& strGUID)
{
    setDocmentGUID(strKbGUID, strGUID);
    if (!isRunning())
    {
        start(HighPriority);
    }
}

void CWizDocumentEditStatusCheckThread::downloadData(const QString& strUrl)
{
    QNetworkAccessManager net;
    QNetworkReply* reply = net.get(QNetworkRequest(strUrl));

    QEventLoop loop;
    loop.connect(reply, SIGNAL(finished()), SLOT(quit()));
    loop.exec();

    if (reply->error()) {
        Q_EMIT checkFinished(QString(), QStringList());
        reply->deleteLater();
        return;
    }

    rapidjson::Document d;
    d.Parse<0>(reply->readAll().constData());
    if (d.IsArray())
    {
        QStringList strList;
        QTextCodec* codec = QTextCodec::codecForName("UTF-8");
        QTextDecoder* encoder = codec->makeDecoder();
        for (rapidjson::SizeType i = 0; i < d.Size(); i++)
        {
            const rapidjson::Value& u = d[i];
            strList.append(encoder->toUnicode(u.GetString(), u.GetStringLength()));
        }
        emit checkFinished(m_strGUID, strList);
        reply->deleteLater();
        return;
    }
    Q_EMIT checkFinished(QString(), QStringList());
    reply->deleteLater();
}

void CWizDocumentEditStatusCheckThread::run()
{
    QString kbGUID;
    QString guid;
    //
    while (!m_stop)
    {
        //////
        {
            QMutexLocker lock(&m_mutexWait);
            m_wait.wait(&m_mutexWait);
            if (m_stop)
                return;
            kbGUID = m_strKbGUID;
            guid = m_strGUID;
        }
        //
        //
        QString strRequestUrl = WizFormatString4(_T("%1/get?obj_id=%2/%3&t=%4"),
                                                 WizKMGetDocumentEditStatusURL(),
                                                 kbGUID,
                                                 guid,
                                                 ::WizIntToStr(GetTickCount()));

        downloadData(strRequestUrl);
    }
}


void CWizDocumentEditStatusCheckThread::setDocmentGUID(const QString& strKbGUID, const QString& strGUID)
{
    m_mutexWait.lock();
    m_strKbGUID = strKbGUID;
    m_strGUID = strGUID;
    m_wait.wakeAll();
    m_mutexWait.unlock();
}
