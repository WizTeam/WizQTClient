#include "WizDocumentEditStatus.h"
#include "sync/WizApiEntry.h"
#include "sync/WizToken.h"
#include "share/WizMisc.h"
#include "share/jsoncpp/json/json.h"
#include "share/WizDatabase.h"
#include "share/WizDatabaseManager.h"
#include "sync/WizToken.h"
#include "sync/WizKMServer.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTextCodec>

#include <QDebug>


QString WizKMGetDocumentEditStatusURL()
{
    return WizCommonApiEntry::editStatusUrl();
}

WizDocumentEditStatusSyncThread::WizDocumentEditStatusSyncThread(QObject* parent)
    : QThread(parent)
    , m_stop(false)
    , m_sendNow(false)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timerOut()));
    connect(this, SIGNAL(startTimer(int)), &m_timer, SLOT(start(int)));
    connect(this, SIGNAL(stopTimer()), &m_timer, SLOT(stop()));
}


void WizDocumentEditStatusSyncThread::startEditingDocument(const QString& strUserAlias, const QString& strKbGUID, const QString& strGUID)
{
    QString strObjID = combineObjID(strKbGUID, strGUID);
    if (strObjID.isEmpty())
        return;

    m_mutex.lock();
    if (m_doneMap.contains(strObjID))
    {
        m_doneMap.remove(strObjID);
    }
    m_editingMap.insert(strObjID, strUserAlias);
    m_wait.wakeAll();
    m_mutex.unlock();
}

void WizDocumentEditStatusSyncThread::stopEditingDocument(const QString& strKbGUID, \
                                                           const QString& strGUID, bool bModified)
{
//    qDebug() << "stop editing document , guid " << strGUID << "  modified : " << bModified;
    QString strObjID = combineObjID(strKbGUID, strGUID);
    if (strObjID.isEmpty())
        return;

    m_mutex.lock();
    if (m_editingMap.contains(strObjID))
    {
        if (bModified)
        {
            m_modifiedMap.insert(strObjID, m_editingMap.value(strObjID));
        }
        else if (!m_modifiedMap.contains(strObjID))
        {
            m_doneMap.insert(strObjID, m_editingMap.value(strObjID));
        }
        m_editingMap.remove(strObjID);
    }
    m_wait.wakeAll();
    m_mutex.unlock();
}

void WizDocumentEditStatusSyncThread::documentSaved(const QString& strUserAlias, const QString& strKbGUID, const QString& strGUID)
{
//    qDebug() << "EditStatusSyncThread document saved : kbguid : " << strKbGUID << "  guid : " << strGUID;
    QString strObjID = combineObjID(strKbGUID, strGUID);
    if (strObjID.isEmpty())
        return;

    m_mutex.lock();
    if (m_doneMap.contains(strObjID))
    {
        m_doneMap.remove(strObjID);
    }
    m_modifiedMap.insert(strObjID, strUserAlias);
    m_wait.wakeAll();
    m_mutex.unlock();
}

void WizDocumentEditStatusSyncThread::documentUploaded(const QString& strKbGUID, const QString& strGUID)
{
    QString strObjID = combineObjID(strKbGUID, strGUID);
    if (strObjID.isEmpty())
        return;

    m_mutex.lock();
    if (m_modifiedMap.contains(strObjID))
    {
        if (!m_editingMap.contains(strObjID))
        {
            m_doneMap.insert(strObjID, m_modifiedMap.value(strObjID));
        }
        m_modifiedMap.remove(strObjID);
    }
    m_wait.wakeAll();
    m_mutex.unlock();
}

void WizDocumentEditStatusSyncThread::on_timerOut()
{
    m_mutex.lock();
    m_wait.wakeAll();
    m_mutex.unlock();
}

void WizDocumentEditStatusSyncThread::stop()
{
    //If thread wasn't running, no need to stop. Otherwise will start running ,and cause crash at destructor of program.
    if (!isRunning())
        return;

    m_mutex.lock();
    m_stop = true;
    m_wait.wakeAll();
    m_mutex.unlock();
}

void WizDocumentEditStatusSyncThread::waitForDone()
{
    stop();
    //
    WizWaitForThread(this);
}

void WizDocumentEditStatusSyncThread::run()
{
    while (!m_stop)
    {
        m_mutex.lock();
        m_wait.wait(&m_mutex);

        emit stopTimer();

        if (m_stop)
        {
            m_mutex.unlock();
            return;
        }
        m_mutex.unlock();
        //
        sendEditingMessage();
        sendDoneMessage();
    }
}

QString WizDocumentEditStatusSyncThread::combineObjID(const QString& strKbGUID, const QString& strGUID)
{
    QString strObjID = "";
    if (!strKbGUID.isEmpty())
    {
        strObjID = strKbGUID + "/" + strGUID;
    }
    return strObjID;
}

void WizDocumentEditStatusSyncThread::sendEditingMessage()
{
    m_mutex.lock();
    QMap<QString, QString> editingMap(m_editingMap);
    QMap<QString, QString>::const_iterator it;
    for ( it = m_modifiedMap.begin(); it != m_modifiedMap.end(); ++it )
    {
        editingMap.insert(it.key(), it.value());
    }
    m_mutex.unlock();

    for (it = editingMap.begin(); it != editingMap.end(); ++it)
    {
//        qDebug() << "try to send editing message , objId : " << it.key() << "  userAlias : " << it.value();
        if (!it.key().isEmpty() && !it.value().isEmpty())
        {
            sendEditingMessage(it.value(), it.key());
        }
    }

    // send again after 30s
    if (editingMap.size() > 0)
    {
        emit startTimer(30 * 1000);
    }
}

bool WizDocumentEditStatusSyncThread::sendEditingMessage(const QString& strUserAlias, const QString& strObjID)
{
    QString strUrl = ::WizFormatString5("%1/add?obj_id=%2&user_id=%3&t=%4&token=%5",
                                        WizKMGetDocumentEditStatusURL(),
                                        strObjID,
                                        strUserAlias,
                                        ::WizIntToStr(WizGetTickCount()),
                                        WizToken::token());

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

void WizDocumentEditStatusSyncThread::sendDoneMessage()
{
    m_mutex.lock();
    QMap<QString, QString> doneList(m_doneMap);
    m_mutex.unlock();

    QMap<QString, QString>::const_iterator it;
    for ( it = doneList.begin(); it != doneList.end(); ++it )
    {
        if (!it.key().isEmpty() && !it.value().isEmpty())
        {
            if (sendDoneMessage(it.value(), it.key()))
            {
                m_mutex.lock();
                m_doneMap.remove(it.key());
                m_mutex.unlock();
            }
        }
    }
}

bool WizDocumentEditStatusSyncThread::sendDoneMessage(const QString& strUserAlias, const QString& strObjID)
{
    QString strUrl = WizFormatString5("%1/delete?obj_id=%2&user_id=%3&t=%4&token=%5",
                                      WizKMGetDocumentEditStatusURL(),
                                      strObjID,
                                      strUserAlias,
                                      ::WizIntToStr(WizGetTickCount()),
                                      WizToken::token());

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


WizDocumentStatusChecker::WizDocumentStatusChecker(QObject* parent)
    : m_timeOutTimer(0)
    //, m_loopCheckTimer(0)
    , m_stop(false)
{

}

WizDocumentStatusChecker::~WizDocumentStatusChecker()
{
    //if (m_timeOutTimer)
    //    delete m_timeOutTimer;

//    if (m_loopCheckTimer)
//        delete m_loopCheckTimer;
}

void WizDocumentStatusChecker::checkEditStatus(const QString& strKbGUID, const QString& strGUID)
{
//    qDebug() << "CWizDocumentStatusChecker start to check guid : " << strGUID;
    setDocmentGUID(strKbGUID, strGUID);
    m_stop = false;
    startCheck();
}

void WizDocumentStatusChecker::stopCheckStatus(const QString& strKbGUID, const QString& strGUID)
{
    m_timeOutTimer->stop();
//    m_loopCheckTimer->stop();
    m_stop = true;

    m_mutexWait.lock();
    m_strKbGUID.clear();
    m_strGUID.clear();
    m_mutexWait.unlock();
}

void WizDocumentStatusChecker::setDocmentGUID(const QString& strKbGUID, const QString& strGUID)
{
    m_mutexWait.lock();
    m_strKbGUID = strKbGUID;
    m_strGUID = strGUID;
    m_mutexWait.unlock();
}

void WizDocumentStatusChecker::peekDocumentGUID(QString& strKbGUID, QString& strGUID)
{
    m_mutexWait.lock();
    strKbGUID = m_strKbGUID;
    strGUID = m_strGUID;
    m_mutexWait.unlock();
}

void WizDocumentStatusChecker::onTimeOut()
{
    qDebug() << "CWizDocumentStatusChecker time out";
    m_timeOutTimer->stop();
    m_stop = true;
    emit checkTimeOut(m_strCurGUID);
}

void WizDocumentStatusChecker::recheck()
{
//    qDebug() << "CWizDocumentStatusChecker  recheck called";
    startRecheck();
}

void WizDocumentStatusChecker::initialise()
{
//    qDebug() << "CWizDocumentStatusChecker thread id : ";
    m_timeOutTimer = new QTimer(this);
    connect(m_timeOutTimer, SIGNAL(timeout()), SLOT(onTimeOut()));
    m_timeOutTimer->setSingleShot(true);
//    m_loopCheckTimer = new QTimer(this);
//    connect(m_loopCheckTimer, SIGNAL(timeout()), SLOT(recheck()));
}

void WizDocumentStatusChecker::clearTimers()
{
    m_timeOutTimer->stop();
//    m_loopCheckTimer->stop();
}

void WizDocumentStatusChecker::startRecheck()
{
//    qDebug() << "CWizDocumentStatusChecker  start recheck";    
    startCheck();
}

void WizDocumentStatusChecker::startCheck()
{
//    qDebug() << "----------    CWizDocumentStatusChecker::startCheck start";
    m_networkError = false;
    peekDocumentGUID(m_strCurKbGUID, m_strCurGUID);

    // start timer
    m_timeOutTimer->start(5000);

    bool isGroup = false;
    bool changed = checkDocumentChangedOnServer(m_strCurKbGUID, m_strCurGUID, isGroup);
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

    bool editingByOthers = false;
    if (isGroup)
    {
        editingByOthers = checkDocumentEditStatus(m_strCurKbGUID, m_strCurGUID);
    }
    //
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

bool WizDocumentStatusChecker::checkDocumentChangedOnServer(const QString& strKbGUID, const QString& strGUID, bool& isGroup)
{
    WizDatabase& db = WizDatabaseManager::instance()->db(strKbGUID);
    WIZDOCUMENTDATA doc;
    if (!db.documentFromGuid(strGUID, doc))
        return false;

    if (doc.nVersion == -1)
    {
        return !db.canEditDocument(doc);
    }

    isGroup = false;
    WIZUSERINFO userInfo = WizToken::userInfo();
    if (db.isGroup())
    {
        isGroup = true;
        WIZGROUPDATA group;
        if (!WizDatabaseManager::instance()->db().getGroupData(strKbGUID, group))
            return false;
        userInfo = WIZUSERINFO(userInfo, group);
    }
    //
    WizKMDatabaseServer server(userInfo);
    //
    if (!server.kb_getInfo())
        return false;
    //
    WIZKBINFO info = server.kbInfo();
    //
    if (info.nDocumentVersion <= db.getObjectVersion("document"))
        return false;

    WIZDOCUMENTDATAEX docOnServer;
    if (!server.document_getInfo(strGUID, docOnServer))
        return false;

    if ((docOnServer.strGUID == doc.strGUID)
            && (docOnServer.nVersion > doc.nVersion)
            && doc.nVersion > 0) //刚刚上传成功，
    {
        qDebug() << "[Status]New version of note detected , note : " << docOnServer.strTitle << "  local version : " << doc.nVersion << " server version : " << docOnServer.nVersion;
        return true;
    }
    return false;
}

bool WizDocumentStatusChecker::checkDocumentEditStatus(const QString& strKbGUID, const QString& strGUID)
{
    QString strRequestUrl = WizFormatString4("%1/get?obj_id=%2/%3&t=%4",
                                             WizKMGetDocumentEditStatusURL(),
                                             strKbGUID,
                                             strGUID,
                                             ::WizIntToStr(WizGetTickCount()));

    return checkDocumentEditStatus(strRequestUrl);
}

bool WizDocumentStatusChecker::checkDocumentEditStatus(const QString& strUrl)
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

    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(reply->readAll().constData(), d))
        return false;

    if (d.isArray())
    {
        QStringList strList;
        //
        for (Json::ArrayIndex i = 0; i < d.size(); i++)
        {
            const Json::Value& u = d[i];
            strList.append(QString::fromStdString(u.asString()));
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
