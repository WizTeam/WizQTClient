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

wizDocumentEditStatusSyncThread::wizDocumentEditStatusSyncThread(QObject* parent)
    : QThread(parent)
    , m_mutext(QMutex::Recursive)
    , m_stop(false)
    , m_sendNow(false)
{
}

wizDocumentEditStatusSyncThread::~wizDocumentEditStatusSyncThread()
{
}

void wizDocumentEditStatusSyncThread::stopEditingDocument()
{
    setCurrentEditingDocument("", "", "");
}

void wizDocumentEditStatusSyncThread::setCurrentEditingDocument(const QString& strUserAlias, const QString& strKbGUID, const QString& strGUID)
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

void wizDocumentEditStatusSyncThread::stop()
{
    m_stop = true;
    //
    stopEditingDocument();
}

void wizDocumentEditStatusSyncThread::run()
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



void wizDocumentEditStatusSyncThread::sendEditingMessage()
{
    m_mutext.lock();
    EditStatusObj editingObj = m_editingObj;
    m_mutext.unlock();

    if (!editingObj.strObjID.isEmpty() && !editingObj.strUserName.isEmpty())
    {
        sendEditingMessage(editingObj.strUserName, editingObj.strObjID);
    }
}

void wizDocumentEditStatusSyncThread::sendEditingMessage(const QString& strUserAlias, const QString& strObjID)
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
    m_netManager->get(QNetworkRequest(strUrl));

    qDebug() << "sendEditingMessage called " <<strObjID;
}

void wizDocumentEditStatusSyncThread::sendDoneMessage()
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

void wizDocumentEditStatusSyncThread::sendDoneMessage(const QString& strUserAlias, const QString& strObjID)
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
    m_netManager->get(QNetworkRequest(strUrl));
    qDebug() << "sendDoneMessage called " <<strObjID;
}


wizDocumentEditStatusCheckThread::wizDocumentEditStatusCheckThread(QObject* parent) : QThread(parent)
{
}

void wizDocumentEditStatusCheckThread::checkEditStatus(const QString& strKbGUID, const QString& strGUID)
{
    setDocmentGUID(strKbGUID, strGUID);
    if (!isRunning())
    {
        start(HighPriority);
    }
}

void wizDocumentEditStatusCheckThread::downloadData(const QString& strUrl)
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

void wizDocumentEditStatusCheckThread::run()
{
    QString strRequestUrl = WizFormatString4(_T("%1/get?obj_id=%2/%3&t=%4"),
                                             WizKMGetDocumentEditStatusURL(),
                                             m_strKbGUID,
                                             m_strGUID,
                                             ::WizIntToStr(GetTickCount()));

    downloadData(strRequestUrl);

}


void wizDocumentEditStatusCheckThread::setDocmentGUID(const QString& strKbGUID, const QString& strGUID)
{
    m_strKbGUID = strKbGUID;
    m_strGUID = strGUID;
}
