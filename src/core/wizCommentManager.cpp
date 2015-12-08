#include "wizCommentManager.h"

#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>

#include "rapidjson/document.h"
#include "sync/apientry.h"
#include "sync/token.h"
#include "share/wizEventLoop.h"
#include "share/wizthreads.h"

#define QUERY_DELAY 3

using namespace Core;
CWizCommentQuerier::CWizCommentQuerier(const QString& kbGUID, const QString& GUID, QueryType type, QObject* parent)
    : QObject(parent)
    , m_kbGUID(kbGUID)
    , m_GUID(GUID)
    , m_type(type)
{
}

void CWizCommentQuerier::setCommentsUrl(const QString& url)
{
    emit commentUrlAcquired(m_GUID, url);
}

void CWizCommentQuerier::setCommentsCount(int count)
{
    emit commentCountAcquired(m_GUID, count);
}

void CWizCommentQuerier::errorOccurred()
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

void CWizCommentQuerier::parseReplyData(const QString& reply)
{
    if (reply.isEmpty())
        return;

    rapidjson::Document d;
    d.Parse<0>(reply.toUtf8().constData());

    if (d.HasMember("error_code")) {
        qDebug() << "Failed to get comment count: "
                 << QString::fromUtf8(d.FindMember("error")->value.GetString())
                 << " code: " << d.FindMember("error_code")->value.GetInt();
        setCommentsCount(0);
        return;
    }

    if (d.HasMember("return_code")) {
        int nCode = d.FindMember("return_code")->value.GetInt();
        if (nCode != 200) {
            qDebug() << "Failed to get comment count, need 200, but return "
                     << d.FindMember("return_code")->value.GetInt();
            setCommentsCount(0);
            return;
        }
    }

    if (d.HasMember("comment_count"))
    {
        int count = d.FindMember("comment_count")->value.GetInt();
        setCommentsCount(count);
    }
}

void CWizCommentQuerier::run()
{
    if (m_type == QueryNone)
        return;

    QString token = WizService::Token::token();
    QString commentsUrl =  WizService::CommonApiEntry::commentUrl(token, m_kbGUID, m_GUID);
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

    QString kUrl = WizService::CommonApiEntry::kUrlFromGuid(token, m_kbGUID);
    QString strCountUrl = WizService::CommonApiEntry::commentCountUrl(kUrl, token, m_kbGUID, m_GUID);

    QNetworkAccessManager net;
    QNetworkReply* reply = net.get(QNetworkRequest(strCountUrl));

    CWizAutoTimeOutEventLoop loop(reply);
    loop.exec();

    if (loop.error() != QNetworkReply::NoError)
    {
        setCommentsCount(0);
        return;
    }

    parseReplyData(loop.result());
}



CWizCommentManager::CWizCommentManager(QObject* parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timer_timeOut()));
}

/*
 * 获取url是通过拼接字符串方式处理的，不需要延迟处理
 */
void CWizCommentManager::queryCommentUrl(const QString& kbGUID, const QString& GUID)
{
    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=] {
        CWizCommentQuerier seacher(kbGUID, GUID, CWizCommentQuerier::QueryUrl);
        connect(&seacher, SIGNAL(commentUrlAcquired(const QString&, const QString&)),
                SLOT(on_commentUrlAcquired(QString,QString)));
        seacher.run();
    });
}

/*
 *  获取评论数目是通过网络请求获取的，需要延迟处理请求。防止在快速切换笔记时产生大量的网络请求
 */
void CWizCommentManager::queryCommentCount(const QString& kbGUID, const QString& GUID, bool removeOtherQueryRequest)
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

void CWizCommentManager::on_commentUrlAcquired(QString GUID, QString url)
{
    emit commentUrlAcquired(GUID, url);
}

void CWizCommentManager::on_commentCountAcquired(QString GUID, int count)
{
    emit commentCountAcquired(GUID, count);
}

void CWizCommentManager::on_timer_timeOut()
{
    CountQueryData data;
    pickData(data);

    if (data.strGUID.isEmpty())
        return;

    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=] {
        CWizCommentQuerier seacher(data.strKBGUID, data.strGUID, CWizCommentQuerier::QueryCount);
        connect(&seacher, SIGNAL(commentCountAcquired(QString,int)), SLOT(on_commentCountAcquired(QString,int)));
        seacher.run();
    });
}

void CWizCommentManager::pickData(CountQueryData& data)
{
    QMutexLocker locker(&m_mutext);
    Q_UNUSED(locker)

    if (m_queryList.count() > 0)
    {
        data = m_queryList.first();
        m_queryList.pop_front();
    }
}

