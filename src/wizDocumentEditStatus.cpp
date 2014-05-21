#include "wizDocumentEditStatus.h"
#include "sync/apientry.h"
#include "share/wizmisc.h"
#include "rapidjson/document.h"
#include <QtNetWork>
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

wizDocumentEditStatusSyncThread::wizDocumentEditStatusSyncThread(QObject* parent) : QThread(parent)
{
}

void wizDocumentEditStatusSyncThread::addEditingDocument(const QString& strUserAlias, const QString& strKbGUID, const QString& strGUID)
{
    if (strGUID.isEmpty() || strUserAlias.isEmpty() || strKbGUID.isEmpty())
        return;

    QString strObjID = strKbGUID + "/" + strGUID;

    m_mutext.lock();
    m_editingList.insert(strObjID, strUserAlias);
    m_mutext.unlock();

    if (!isRunning())
        start();

    emit sendEditStatusRequest();
}

void wizDocumentEditStatusSyncThread::addDoneDocument(const QString& strKbGUID, const QString& strGUID)
{
    if (strGUID.isEmpty() || strKbGUID.isEmpty())
        return;

    QString strObjID = strKbGUID + "/" + strGUID;

    m_mutext.lock();
    if (m_editingList.contains(strObjID))
    {
        m_doneList.insert(strObjID, m_editingList.value(strObjID));
        m_editingList.remove(strObjID);
    }
    m_mutext.unlock();

    if (!isRunning())
        start();

    emit sendEditStatusRequest();
}

void wizDocumentEditStatusSyncThread::setAllDocumentDone()
{
    m_mutext.lock();
    QMap<QString, QString>::iterator i;
    for (i = m_editingList.begin(); i != m_editingList.end(); i++)
    {
        m_doneList.insert(i.key(), i.value());
    }
    m_editingList.clear();
    m_mutext.unlock();

    if (!isRunning())
        start();

    emit sendEditStatusRequest();
}

void wizDocumentEditStatusSyncThread::run()
{
    while (true)
    {
        sendEditingMessage();
        sendDoneMessage();

        QEventLoop eventLoop;
        connect(this, SIGNAL(sendEditStatusRequest()), &eventLoop, SLOT(quit()));
        QTimer::singleShot(30 * 1000, &eventLoop, SLOT(quit()));
        eventLoop.exec();
    }
}

void wizDocumentEditStatusSyncThread::sendEditingMessage()
{
    m_mutext.lock();
    QMap<QString, QString> editingList = m_editingList;
    m_mutext.unlock();

    while(!editingList.isEmpty())
    {
        QString strGUID = editingList.begin().key();
        QString strUserID = editingList.value(strGUID);
        editingList.remove(strGUID);

        sendEditingMessage(strUserID, strGUID);
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
    QMap<QString, QString> doneList = m_doneList;
    // send done message just once
    m_doneList.clear();
    m_mutext.unlock();

    while(!doneList.isEmpty())
    {
        QString strGUID = doneList.begin().key();
        QString strUserID = doneList.value(strGUID);
        doneList.remove(strGUID);

        sendDoneMessage(strUserID, strGUID);
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
    qDebug() << "wizDocumentEditStatusCheckThread start download data";
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
