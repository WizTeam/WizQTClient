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
    qDebug() << "EditStatusSyncThread document saved : kbguid : " << strKbGUID << "  guid : " << strGUID;
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
    qDebug() << "edit status sync thread on document uploaded , kbGuid :  " << strKbGUID << "   strGuid  :  " << strGUID;

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
        qDebug() << "try to send editing message , objId : " << it.key() << "  userAlias : " << it.value();
        if (!it.key().isEmpty() && !it.value().isEmpty())
        {
            sendEditingMessage(it.value(), it.key());
        }
    }
}

bool CWizDocumentEditStatusSyncThread::sendEditingMessage(const QString& strUserAlias, const QString& strObjID)
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

    return reply->error() == QNetworkReply::NoError;
}


CWizDocumentStatusCheckThread::CWizDocumentStatusCheckThread(QObject* parent)
    : QThread(parent)
    , m_stop(false)
    , m_mutexWait(QMutex::NonRecursive)
    , m_needRecheck(false)
    , m_timer(0)
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
    m_timer->stop();

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
            m_timer = new QTimer();
            connect(m_timer, SIGNAL(timeout()), SLOT(onTimeOut()));
        }

        m_timer->start(5000);
        //
        int lastVersion;
        bool changed = checkDocumentChangedOnServer(m_strCurKbGUID, m_strCurGUID, lastVersion);
        emit checkDocumentChangedFinished(m_strCurGUID, changed, lastVersion);

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
