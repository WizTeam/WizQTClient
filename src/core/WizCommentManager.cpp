#include "WizCommentManager.h"

#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>

#include "share/jsoncpp/json/json.h"
#include "sync/WizApiEntry.h"
#include "sync/WizToken.h"
#include "share/WizEventLoop.h"
#include "share/WizThreads.h"

#define QUERY_DELAY 3

WizCommentQuerier::WizCommentQuerier(const QString& kbGUID, const QString& GUID, QueryType type, QObject* parent)
    : QObject(parent)
    , m_kbGUID(kbGUID)
    , m_GUID(GUID)
    , m_type(type)
{
}

void WizCommentQuerier::setCommentsUrl(const QString& url)
{
    emit commentUrlAcquired(m_GUID, url);
}

void WizCommentQuerier::setCommentsCount(int count)
{
    emit commentCountAcquired(m_GUID, count);
}

void WizCommentQuerier::errorOccurred()
{
    if (QueryUrl == m_type || QueryUrlAndCount == m_type)
    {
        setCommentsUrl("");
    }
    if (QueryCount == m_type || QueryUrlAndCount == m_type)
    {
        setCommentsCount(0);
    }
}

void WizCommentQuerier::parseReplyData(const QString& reply)
{
    if (reply.isEmpty())
        return;

    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(reply.toUtf8().constData(), d))
        return;

    if (d.isMember("error_code")) {
        qDebug() << "Failed to get comment count: "
                 << QString::fromStdString(d["error"].asString())
                 << " code: " << d["error_code"].asInt();
        setCommentsCount(0);
        return;
    }

    if (d.isMember("return_code")) {
        int nCode = d["return_code"].asInt();
        if (nCode != 200) {
            qDebug() << "Failed to get comment count, need 200, but return "
                     << d["return_code"].asInt();
            setCommentsCount(0);
            return;
        }
    }

    if (d.isMember("comment_count"))
    {
        int count = d["comment_count"].asInt();
        setCommentsCount(count);
    }
}

void WizCommentQuerier::run()
{
    if (m_type == QueryNone)
        return;

    QString token = WizToken::token();
    QString commentsUrl =  WizCommonApiEntry::commentUrl(token, m_kbGUID, m_GUID);
    if (commentsUrl.isEmpty() || token.isEmpty())
    {
        qDebug() << "Can not get comment url by token : " << token;
        errorOccurred();
        return;
    }
    else if (QueryUrl == m_type || QueryUrlAndCount == m_type)
    {
        setCommentsUrl(commentsUrl);
    }

    if (QueryCount != m_type && QueryUrlAndCount != m_type)
        return;

    QString kUrl = WizToken::userInfo().strXmlRpcServer;
    QString strCountUrl = WizCommonApiEntry::commentCountUrl(kUrl, token, m_kbGUID, m_GUID);

    QNetworkAccessManager net;
    QNetworkReply* reply = net.get(QNetworkRequest(strCountUrl));

    WizAutoTimeOutEventLoop loop(reply);
    loop.exec();

    if (loop.error() != QNetworkReply::NoError)
    {
        setCommentsCount(0);
        return;
    }

    QString result = QString::fromUtf8(loop.result().constData());
    parseReplyData(result);
}



WizCommentManager::WizCommentManager(QObject* parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timer_timeOut()));
}

/*
 * 获取url是通过拼接字符串方式处理的，不需要延迟处理
 */
void WizCommentManager::queryCommentUrl(const QString& kbGUID, const QString& GUID)
{
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=] {
        WizCommentQuerier seacher(kbGUID, GUID, WizCommentQuerier::QueryUrl);
        connect(&seacher, SIGNAL(commentUrlAcquired(const QString&, const QString&)),
                SLOT(on_commentUrlAcquired(QString,QString)));
        seacher.run();
    });
}

/*
 *  获取评论数目是通过网络请求获取的，需要延迟处理请求。防止在快速切换笔记时产生大量的网络请求
 */
void WizCommentManager::queryCommentCount(const QString& kbGUID, const QString& GUID, bool removeOtherQueryRequest)
{
    QMutexLocker locker(&m_mutext);
    Q_UNUSED(locker)

    if (removeOtherQueryRequest)
    {
        m_timer.stop();
        m_queryList.clear();
    }

    for (CountQueryData query : m_queryList)
    {
        if(query.strGUID == GUID)
            return;
    }

    CountQueryData query;
    query.strKBGUID = kbGUID;
    query.strGUID = GUID;
    m_queryList.append(query);

    if (!m_timer.isActive())
    {
        m_timer.start(QUERY_DELAY * 1000);
    }
}

void WizCommentManager::on_commentUrlAcquired(QString GUID, QString url)
{
    emit commentUrlAcquired(GUID, url);
}

void WizCommentManager::on_commentCountAcquired(QString GUID, int count)
{
    emit commentCountAcquired(GUID, count);
}

void WizCommentManager::on_timer_timeOut()
{
    CountQueryData data;
    pickData(data);

    if (data.strGUID.isEmpty())
        return;

    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=] {
        WizCommentQuerier seacher(data.strKBGUID, data.strGUID, WizCommentQuerier::QueryCount);
        connect(&seacher, SIGNAL(commentCountAcquired(QString,int)), SLOT(on_commentCountAcquired(QString,int)));
        seacher.run();
    });
}

void WizCommentManager::pickData(CountQueryData& data)
{
    QMutexLocker locker(&m_mutext);
    Q_UNUSED(locker)

    if (m_queryList.count() > 0)
    {
        data = m_queryList.first();
        m_queryList.pop_front();
    }
}

