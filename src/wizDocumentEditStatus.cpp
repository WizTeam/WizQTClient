#include "wizDocumentEditStatus.h"
#include "sync/apientry.h"
#include "share/wizmisc.h"
#include "rapidjson/document.h"
#include "share/wizDatabase.h"
#include "share/wizDatabaseManager.h"
#include "sync/token.h"
#include "sync/wizkmxmlrpc.h"

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


CWizDocumentStatusCheckThread::CWizDocumentStatusCheckThread(QObject* parent)
    : QThread(parent)
    , m_stop(false)
    , m_mutexWait(QMutex::NonRecursive)
    , m_needRecheck(false)
    , m_jumpToNext(false)
    , m_timer(new QTimer(parent))
{
    connect(m_timer, SIGNAL(timeout()), SLOT(onTimeOut()));
}

void CWizDocumentStatusCheckThread::waitForDone()
{
    stop();
    //
    WizWaitForThread(this);
}

void CWizDocumentStatusCheckThread::needRecheck()
{
    m_needRecheck = true;
}

void CWizDocumentStatusCheckThread::onTimeOut()
{
    m_timer->stop();

    m_jumpToNext = true;
    emit checkTimeOut(m_strCurGUID);
}

void CWizDocumentStatusCheckThread::checkEditStatus(const QString& strKbGUID, const QString& strGUID)
{
    setDocmentGUID(strKbGUID, strGUID);
}

void CWizDocumentStatusCheckThread::downloadData(const QString& strUrl)
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
        //
        {
            QMutexLocker lock(&m_mutexWait);
            if (strUrl.indexOf(m_strGUID) != -1)
            {
                emit checkFinished(m_strGUID, strList);
            }
            else
            {
                needRecheck();
            }
        }
        reply->deleteLater();
        return;
    }
    Q_EMIT checkFinished(QString(), QStringList());
    reply->deleteLater();
}

void CWizDocumentStatusCheckThread::run()
{
    //
    while (!m_stop)
    {
        //////
        {
            QMutexLocker lock(&m_mutexWait);
            if (!m_needRecheck)
            {
                m_wait.wait(&m_mutexWait);
            }
            else
            {
                m_needRecheck = false;
                m_jumpToNext = false;
            }
            //
            if (m_stop)
                return;
            m_strCurKbGUID = m_strKbGUID;
            m_strCurGUID = m_strGUID;
        }

        //

        m_timer->start(5000);

        //
        int lastVersion;
        bool changed = checkDocumentChangedOnServer(m_strCurKbGUID, m_strCurGUID, lastVersion);
        emit checkDocumentChangedFinished(m_strCurGUID, changed, lastVersion);

        if (m_jumpToNext)
            continue;

        checkDocumentEditStatus(m_strCurKbGUID, m_strCurGUID);
        m_timer->stop();
    }
}


void CWizDocumentStatusCheckThread::setDocmentGUID(const QString& strKbGUID, const QString& strGUID)
{
    m_mutexWait.lock();
    m_strKbGUID = strKbGUID;
    m_strGUID = strGUID;
    m_wait.wakeAll();
    m_jumpToNext = true;
    m_mutexWait.unlock();
}

bool CWizDocumentStatusCheckThread::checkDocumentChangedOnServer(const QString& strKbGUID, const QString& strGUID, int& versionOnServer)
{
    CWizDatabase& db = CWizDatabaseManager::instance()->db(strKbGUID);
    WIZDOCUMENTDATA doc;
    if (!db.DocumentFromGUID(strGUID, doc))
        return false;

    if (doc.nVersion == -1)
        return false;

    WIZUSERINFO userInfo = WizService::Token::info();
    if (db.IsGroup())
    {
        WIZGROUPDATA group;
        if (!CWizDatabaseManager::instance()->db().GetGroupData(strKbGUID, group))
            return false;
        userInfo.strKbGUID = group.strGroupGUID;
        userInfo.strDatabaseServer = group.strDatabaseServer;
        if (userInfo.strDatabaseServer.isEmpty())
        {
            userInfo.strDatabaseServer = WizService::ApiEntry::kUrlFromGuid(userInfo.strToken, userInfo.strKbGUID);
        }
    }
    CWizKMDatabaseServer server(userInfo, NULL);
    WIZOBJECTVERSION versionServer;
    if (!server.wiz_getVersion(versionServer))
        return false;

    if (versionServer.nDocumentVersion <= db.GetObjectVersion("document"))
        return false;

    //emit syncDatabaseRequest(strKbGUID);

    int nPart = 0;
    nPart |= WIZKM_XMKRPC_DOCUMENT_PART_INFO;
    WIZDOCUMENTDATAEX docOnServer;
    if (!server.document_getData(strGUID, nPart, docOnServer))
        return false;

    qDebug() << "compare document version , server  :  " << docOnServer.nVersion << " doc in local  :  " << doc.nVersion;

    versionOnServer = docOnServer.nVersion;
    return docOnServer.nVersion > doc.nVersion;
}

bool CWizDocumentStatusCheckThread::checkDocumentEditStatus(const QString& strKbGUID, const QString& strGUID)
{
    QString strRequestUrl = WizFormatString4(_T("%1/get?obj_id=%2/%3&t=%4"),
                                             WizKMGetDocumentEditStatusURL(),
                                             strKbGUID,
                                             strGUID,
                                             ::WizIntToStr(GetTickCount()));

    downloadData(strRequestUrl);
    return true;
}
