#include "wizCommentManager.h"

#include <QMutexLocker>
#include <QRunnable>
#include <QThreadPool>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>

#include "rapidjson/document.h"
#include "sync/apientry.h"
#include "sync/token.h"
#include "share/wizEventLoop.h"

#define QUERY_DELAY 3

using namespace Core;

enum QueryType
{
    QueryNone,
    QueryUrl,
    QueryCount,
    QueryUrlAndCount
};

class CWizQueryCommentRunnable : public QRunnable
{
public:
    CWizQueryCommentRunnable(CWizCommentManager *manager, const QString& kbGUID, const QString& GUID, QueryType type)
        : m_manager(manager)
        , m_kbGUID(kbGUID)
        , m_GUID(GUID)
        , m_type(type)
    {
        setAutoDelete(true);
    }

    void getCommentsUrlFinished(const QString& url)
    {
        QMetaObject::invokeMethod(m_manager, "on_commentUrlAcquired", Qt::QueuedConnection,
                                  Q_ARG(QString, m_GUID), Q_ARG(QString, url));
    }

    void getCommentsCountFinished(int count)
    {
        QMetaObject::invokeMethod(m_manager, "on_commentCountAcquired", Qt::QueuedConnection,
                                  Q_ARG(QString, m_GUID), Q_ARG(int, count));
    }

    void errorOccurred()
    {
        if (QueryUrl == m_type || QueryUrlAndCount == m_type)
        {
            getCommentsUrlFinished("");
        }
        if (QueryCount == m_type || QueryUrlAndCount == m_type)
        {
            getCommentsCountFinished(0);
        }
    }

    void parseReplyData(const QString& reply)
    {
        if (reply.isEmpty())
            return;

        rapidjson::Document d;
        d.Parse<0>(reply.toUtf8().constData());

        if (d.HasMember("error_code")) {
            qDebug() << "Failed to get comment count: "
                     << QString::fromUtf8(d.FindMember("error")->value.GetString())
                     << " code: " << d.FindMember("error_code")->value.GetInt();
            getCommentsCountFinished(0);
            return;
        }

        if (d.HasMember("return_code")) {
            int nCode = d.FindMember("return_code")->value.GetInt();
            if (nCode != 200) {
                qDebug() << "Failed to get comment count, need 200, but return "
                         << d.FindMember("return_code")->value.GetInt();
                getCommentsCountFinished(0);
                return;
            }
        }

        if (d.HasMember("comment_count"))
        {
            int count = d.FindMember("comment_count")->value.GetInt();
            getCommentsCountFinished(count);
        }
    }

    void run()
    {
        if (m_type == QueryNone)
            return;

        QString token = WizService::Token::token();
        QString commentsUrl =  WizService::CommonApiEntry::commentUrl(token, m_kbGUID, m_GUID);
        if (commentsUrl.isEmpty())
        {
            qDebug() << "Can not get comment url by token : " << token;
            errorOccurred();
            return;
        }
        else if (QueryUrl == m_type || QueryUrlAndCount == m_type)
        {
            getCommentsUrlFinished(commentsUrl);
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
            getCommentsCountFinished(0);
        }

        parseReplyData(loop.result());
    }

private:
    QString m_kbGUID;
    QString m_GUID;
    QueryType m_type;
    CWizCommentManager * m_manager;
};


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
    CWizQueryCommentRunnable* runnable = new CWizQueryCommentRunnable(this, kbGUID, GUID, QueryUrl);
    QThreadPool::globalInstance()->start(runnable);
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

    CWizQueryCommentRunnable* runnable = new CWizQueryCommentRunnable(this, data.strKBGUID, data.strGUID, QueryCount);
    QThreadPool::globalInstance()->start(runnable);
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

