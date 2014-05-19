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
    }
    Q_EMIT checkFinished(QString(), QStringList());
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
