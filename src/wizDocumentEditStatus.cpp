#include "wizDocumentEditStatus.h"
#include "sync/apientry.h"
#include "sync/token.h"
#include "share/wizmisc.h"
#include "rapidjson/document.h"
#include "share/wizDatabase.h"
#include "share/wizDatabaseManager.h"
#include "sync/token.h"
#include "sync/wizKMServer.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTextCodec>

#include <QDebug>


QString WizKMGetDocumentEditStatusURL()
{
    return WizService::CommonApiEntry::editStatusUrl();
}

CWizDocumentEditStatusSyncThread::CWizDocumentEditStatusSyncThread(QObject* parent)
    : QThread(parent)
    , m_stop(false)
    , m_mutext(QMutex::Recursive)
    , m_sendNow(false)
{
}


void CWizDocumentEditStatusSyncThread::startEditingDocument(const QString& strUserAlias, const QString& strKbGUID, const QString& strGUID)
{
    QString strObjID = combineObjID(strKbGUID, strGUID);
    if (strObjID.isEmpty())
        return;

    m_mutext.lock();
    if (m_doneMap.contains(strObjID))
    {
        m_doneMap.remove(strObjID);
    }
    m_editingMap.insert(strObjID, strUserAlias);
    m_mutext.unlock();

    m_sendNow = true;
}

void CWizDocumentEditStatusSyncThread::stopEditingDocument(const QString& strKbGUID, \
                                                           const QString& strGUID, bool bModified)
{
//    qDebug() << "stop editing document , guid " << strGUID << "  modified : " << bModified;
    QString strObjID = combineObjID(strKbGUID, strGUID);
    if (strObjID.isEmpty())
        return;

    m_mutext.lock();
    if (m_editingMap.contains(strObjID))
    {
        if (bModified)
        {
            m_modifiedMap.insert(strObjID, m_editingMap.value(strObjID));
        }
        else if (!m_modifiedMap.contains(strObjID))
        {
            m_doneMap.insert(strObjID, m_editingMap.value(strObjID));
            m_sendNow = true;
        }
        m_editingMap.remove(strObjID);
    }
    m_mutext.unlock();
}

void CWizDocumentEditStatusSyncThread::documentSaved(const QString& strUserAlias, const QString& strKbGUID, const QString& strGUID)
{
//    qDebug() << "EditStatusSyncThread document saved : kbguid : " << strKbGUID << "  guid : " << strGUID;
    QString strObjID = combineObjID(strKbGUID, strGUID);
    if (strObjID.isEmpty())
        return;

    m_mutext.lock();
    if (m_doneMap.contains(strObjID))
    {
        m_doneMap.remove(strObjID);
    }
    m_modifiedMap.insert(strObjID, strUserAlias);
    m_mutext.unlock();
}

void CWizDocumentEditStatusSyncThread::documentUploaded(const QString& strKbGUID, const QString& strGUID)
{
    QString strObjID = combineObjID(strKbGUID, strGUID);
    if (strObjID.isEmpty())
        return;

    m_mutext.lock();
    if (m_modifiedMap.contains(strObjID))
    {
        if (!m_editingMap.contains(strObjID))
        {
            m_doneMap.insert(strObjID, m_modifiedMap.value(strObjID));
            m_sendNow = true;
        }
        m_modifiedMap.remove(strObjID);
    }
    m_mutext.unlock();
}

void CWizDocumentEditStatusSyncThread::stop()
{
    //If thread wasn't running, no need to stop. Otherwise will start running ,and cause crash at destructor of program.
    if (!isRunning())
        return;

    m_stop = true;
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

QString CWizDocumentEditStatusSyncThread::combineObjID(const QString& strKbGUID, const QString& strGUID)
{
    QString strObjID = "";
    if (!strKbGUID.isEmpty())
    {
        strObjID = strKbGUID + "/" + strGUID;
    }
    return strObjID;
}

void CWizDocumentEditStatusSyncThread::sendEditingMessage()
{
    m_mutext.lock();
    QMap<QString, QString> editingMap(m_editingMap);
    QMap<QString, QString>::const_iterator it;
    for ( it = m_modifiedMap.begin(); it != m_modifiedMap.end(); ++it )
    {
        editingMap.insert(it.key(), it.value());
    }
    m_mutext.unlock();

    for (it = editingMap.begin(); it != editingMap.end(); ++it)
    {
//        qDebug() << "try to send editing message , objId : " << it.key() << "  userAlias : " << it.value();
        if (!it.key().isEmpty() && !it.value().isEmpty())
        {
            sendEditingMessage(it.value(), it.key());
        }
    }
}

bool CWizDocumentEditStatusSyncThread::sendEditingMessage(const QString& strUserAlias, const QString& strObjID)
{
    QString strUrl = ::WizFormatString5(_T("%1/add?obj_id=%2&user_id=%3&t=%4&token=%5"),
                                        WizKMGetDocumentEditStatusURL(),
                                        strObjID,
                                        strUserAlias,
                                        ::WizIntToStr(GetTickCount()),
                                        WizService::Token::token());

    if (!m_netManager)
    {
        m_netManager = new QNetworkAccessManager();
    }
    QNetworkReply* reply = m_netManager->get(QNetworkRequest(strUrl));

    //qDebug() << "sendEditingMessage called " <<strUrl;
    qDebug() << "[EditStatus]:Send editing status : " << strObjID;

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    return reply->error() == QNetworkReply::NoError;
}

void CWizDocumentEditStatusSyncThread::sendDoneMessage()
{
    m_mutext.lock();
    QMap<QString, QString> doneList(m_doneMap);
    m_mutext.unlock();

    QMap<QString, QString>::const_iterator it;
    for ( it = doneList.begin(); it != doneList.end(); ++it )
    {
        if (!it.key().isEmpty() && !it.value().isEmpty())
        {
            if (sendDoneMessage(it.value(), it.key()))
            {
                m_mutext.lock();
                m_doneMap.remove(it.key());
                m_mutext.unlock();
            }
        }
    }
}

bool CWizDocumentEditStatusSyncThread::sendDoneMessage(const QString& strUserAlias, const QString& strObjID)
{
    QString strUrl = WizFormatString5(_T("%1/delete?obj_id=%2&user_id=%3&t=%4&token=%5"),
                                      WizKMGetDocumentEditStatusURL(),
                                      strObjID,
                                      strUserAlias,
                                      ::WizIntToStr(GetTickCount()),
                                      WizService::Token::token());

    if (!m_netManager)
    {
        m_netManager = new QNetworkAccessManager();
    }
    QNetworkReply* reply = m_netManager->get(QNetworkRequest(strUrl));
//    qDebug() << "sendDoneMessage called " <<strUrl;
    qDebug() << "[EditStatus]:Send done status : " << strObjID;

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    return reply->error() == QNetworkReply::NoError;
}


CWizDocumentStatusCheckThread::CWizDocumentStatusCheckThread(QObject* parent)
    : QThread(parent)
    , m_stop(false)
    , m_mutexWait(QMutex::NonRecursive)
    , m_needRecheck(false)
    , m_timer(0)
    , m_checkNow(false)
{

}

CWizDocumentStatusCheckThread::~CWizDocumentStatusCheckThread()
{
    if (m_timer)
        delete m_timer;
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
    qDebug() << "document check thread time out " << m_strCurGUID;
    emit checkTimeOut(m_strCurGUID);
}

void CWizDocumentStatusCheckThread::checkEditStatus(const QString& strKbGUID, const QString& strGUID)
{
    setDocmentGUID(strKbGUID, strGUID);
    m_checkNow = true;
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
    int idleCounter = 0;
    while (1)
    {
        //////
        {
            QMutexLocker lock(&m_mutexWait);
            if (!m_needRecheck)
            {
//                m_wait.wait(&m_mutexWait);
            }
            else
            {
                m_needRecheck = false;
            }
            //
            if (m_stop)
                return;

            m_strCurKbGUID = m_strKbGUID;
            m_strCurGUID = m_strGUID;
        }

        //
        if (!m_timer)
        {
            m_timer = new QTimer(0);
            m_timer->setSingleShot(true);
            m_timer->moveToThread(this);
            connect(m_timer, SIGNAL(timeout()), SLOT(onTimeOut()));
        }

//        QTimer::singleShot(5000, this, SLOT(onTimeOut()));
        //

        if (idleCounter >= 60 || m_checkNow || m_stop)
        {
            m_checkNow = false;
            idleCounter = 0;
            if (m_stop)
                return;

            m_timer->start(5000);
//            qDebug() << "after qtimer started.";
//            qDebug() << "start to check document changed on server , guid : " << m_strCurGUID;
//            bool changed = checkDocumentChangedOnServer(m_strCurKbGUID, m_strCurGUID);
//            qDebug() << "check finished, document changed : " << changed;
//            emit checkDocumentChangedFinished(m_strCurGUID, changed);

//            qDebug() << "start to check document edit status";
//            checkDocumentEditStatus(m_strCurKbGUID, m_strCurGUID);

//            m_timer->stop();
        }
        else
        {
            sleep(1);
            idleCounter ++;
        }
    }
}


void CWizDocumentStatusCheckThread::setDocmentGUID(const QString& strKbGUID, const QString& strGUID)
{
    m_mutexWait.lock();
    m_strKbGUID = strKbGUID;
    m_strGUID = strGUID;
    m_wait.wakeAll();
    m_mutexWait.unlock();
}

bool CWizDocumentStatusCheckThread::checkDocumentChangedOnServer(const QString& strKbGUID, const QString& strGUID)
{
    CWizDatabase& db = CWizDatabaseManager::instance()->db(strKbGUID);
    WIZDOCUMENTDATA doc;
    if (!db.DocumentFromGUID(strGUID, doc))
        return false;

    if (doc.nVersion == -1)
    {
        return !db.CanEditDocument(doc);
    }

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
            userInfo.strDatabaseServer = WizService::CommonApiEntry::kUrlFromGuid(userInfo.strToken, userInfo.strKbGUID);
        }
    }
    CWizKMDatabaseServer server(userInfo, NULL);
    WIZOBJECTVERSION versionServer;
    if (!server.wiz_getVersion(versionServer))
        return false;

    if (versionServer.nDocumentVersion <= db.GetObjectVersion("document"))
        return false;

    int nPart = 0;
    nPart |= WIZKM_XMKRPC_DOCUMENT_PART_INFO;
    WIZDOCUMENTDATAEX docOnServer;
    if (!server.document_getData(strGUID, nPart, docOnServer))
        return false;

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


CWizDocumentStatusChecker::CWizDocumentStatusChecker(QObject* parent)
    : m_timeOutTimer(0)
    //, m_loopCheckTimer(0)
    , m_stop(false)
{

}

CWizDocumentStatusChecker::~CWizDocumentStatusChecker()
{
    if (m_timeOutTimer)
        delete m_timeOutTimer;

//    if (m_loopCheckTimer)
//        delete m_loopCheckTimer;
}

void CWizDocumentStatusChecker::checkEditStatus(const QString& strKbGUID, const QString& strGUID)
{
//    qDebug() << "CWizDocumentStatusChecker start to check guid : " << strGUID;
    setDocmentGUID(strKbGUID, strGUID);
    m_timeOutTimer->start(5 * 1000);
//    m_loopCheckTimer->start(1 * 60 * 1000);
    m_stop = false;
    startCheck();
}

void CWizDocumentStatusChecker::stopCheckStatus(const QString& strKbGUID, const QString& strGUID)
{
    m_timeOutTimer->stop();
//    m_loopCheckTimer->stop();
    m_stop = true;

    m_mutexWait.lock();
    m_strKbGUID.clear();
    m_strGUID.clear();
    m_mutexWait.unlock();
}

void CWizDocumentStatusChecker::setDocmentGUID(const QString& strKbGUID, const QString& strGUID)
{
    m_mutexWait.lock();
    m_strKbGUID = strKbGUID;
    m_strGUID = strGUID;
    m_mutexWait.unlock();
}

void CWizDocumentStatusChecker::peekDocumentGUID(QString& strKbGUID, QString& strGUID)
{
    m_mutexWait.lock();
    strKbGUID = m_strKbGUID;
    strGUID = m_strGUID;
    m_mutexWait.unlock();
}

void CWizDocumentStatusChecker::onTimeOut()
{
    qDebug() << "CWizDocumentStatusChecker time out";
    m_timeOutTimer->stop();
    m_stop = true;
    emit checkTimeOut(m_strCurGUID);
}

void CWizDocumentStatusChecker::recheck()
{
//    qDebug() << "CWizDocumentStatusChecker  recheck called";
    startRecheck();
}

void CWizDocumentStatusChecker::initialise()
{
//    qDebug() << "CWizDocumentStatusChecker thread id : ";
    m_timeOutTimer = new QTimer(this);
    connect(m_timeOutTimer, SIGNAL(timeout()), SLOT(onTimeOut()));
//    m_loopCheckTimer = new QTimer(this);
//    connect(m_loopCheckTimer, SIGNAL(timeout()), SLOT(recheck()));
}

void CWizDocumentStatusChecker::clearTimers()
{
    m_timeOutTimer->stop();
//    m_loopCheckTimer->stop();
}

void CWizDocumentStatusChecker::startRecheck()
{
//    qDebug() << "CWizDocumentStatusChecker  start recheck";
    m_timeOutTimer->start(5000);
    startCheck();
}

void CWizDocumentStatusChecker::startCheck()
{
//    qDebug() << "----------    CWizDocumentStatusChecker::startCheck start";
    m_networkError = false;
    peekDocumentGUID(m_strCurKbGUID, m_strCurGUID);

    bool changed = checkDocumentChangedOnServer(m_strCurKbGUID, m_strCurGUID);
    if (m_stop)
    {
        emit checkEditStatusFinished(m_strCurGUID, false);
        return;
    }
    emit checkDocumentChangedFinished(m_strCurGUID, changed);

    if (changed)
    {
        emit checkEditStatusFinished(m_strCurGUID, false);
        m_timeOutTimer->stop();
        return;
    }

    bool editingByOthers = checkDocumentEditStatus(m_strCurKbGUID, m_strCurGUID);
    if (m_stop)
    {
        emit checkEditStatusFinished(m_strCurGUID, false);
        return;
    }

    m_timeOutTimer->stop();


//    qDebug() << "------------   CWizDocumentStatusChecker::startCheck  finished";

    if (m_networkError)
    {
        //NOTE: 网络错误时不一定会导致超时，此处发送超时消息进行离线编辑的提示
        emit checkTimeOut(m_strCurGUID);
    }
    else
    {
        emit checkEditStatusFinished(m_strCurGUID, !editingByOthers);
    }
}

bool CWizDocumentStatusChecker::checkDocumentChangedOnServer(const QString& strKbGUID, const QString& strGUID)
{
    CWizDatabase& db = CWizDatabaseManager::instance()->db(strKbGUID);
    WIZDOCUMENTDATA doc;
    if (!db.DocumentFromGUID(strGUID, doc))
        return false;

    if (doc.nVersion == -1)
    {
        return !db.CanEditDocument(doc);
    }

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
            userInfo.strDatabaseServer = WizService::CommonApiEntry::kUrlFromGuid(userInfo.strToken, userInfo.strKbGUID);
        }
    }
    CWizKMDatabaseServer server(userInfo, NULL);
    WIZOBJECTVERSION versionServer;
    if (!server.wiz_getVersion(versionServer))
        return false;

    if (versionServer.nDocumentVersion <= db.GetObjectVersion("document"))
        return false;

    int nPart = 0;
    nPart |= WIZKM_XMKRPC_DOCUMENT_PART_INFO;
    WIZDOCUMENTDATAEX docOnServer;
    if (!server.document_getData(strGUID, nPart, docOnServer))
        return false;

    if ((docOnServer.strGUID == doc.strGUID) && (docOnServer.nVersion > doc.nVersion))
    {
        qDebug() << "[Status]New version of note detected , note : " << docOnServer.strTitle << "  local version : " << doc.nVersion << " server version : " << docOnServer.nVersion;
        return true;
    }
    return false;
}

bool CWizDocumentStatusChecker::checkDocumentEditStatus(const QString& strKbGUID, const QString& strGUID)
{
    QString strRequestUrl = WizFormatString4(_T("%1/get?obj_id=%2/%3&t=%4"),
                                             WizKMGetDocumentEditStatusURL(),
                                             strKbGUID,
                                             strGUID,
                                             ::WizIntToStr(GetTickCount()));

    return checkDocumentEditStatus(strRequestUrl);
}

bool CWizDocumentStatusChecker::checkDocumentEditStatus(const QString& strUrl)
{
    QNetworkAccessManager net;
    QNetworkReply* reply = net.get(QNetworkRequest(strUrl));

    QEventLoop loop;
    loop.connect(reply, SIGNAL(finished()), SLOT(quit()));
    loop.exec();

    if (reply->error()) {
        m_networkError = true;
        reply->deleteLater();
        return false;
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
                emit  documentEditingByOthers(m_strGUID, strList);
            }
        }
        reply->deleteLater();
        return strList.count() > 0;
    }
    Q_EMIT documentEditingByOthers(QString(), QStringList());
    reply->deleteLater();
    return false;
}
