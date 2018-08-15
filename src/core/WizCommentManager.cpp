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
#include "sync/WizKMServer.h"
#include "share/WizDatabase.h"
#include "share/WizDatabaseManager.h"

#define QUERY_DELAY 3

WizCommentQuerier::WizCommentQuerier(WizCommentManager* manager, const QString& kbGUID, const QString& GUID, QObject* parent)
    : QObject(parent)
    , m_manager(manager)
    , m_kbGUID(kbGUID)
    , m_GUID(GUID)
{
}


void WizCommentQuerier::setCommentsCount(int count)
{
    emit commentCountAcquired(m_GUID, count);
}

void WizCommentQuerier::run()
{
    QString commentUrl = m_manager->getCommentUrlTemplate();
    if (commentUrl.isEmpty())
    {
        commentUrl = WizCommonApiEntry::commentUrlTemplate();
        m_manager->setCommentUrlTemplate(commentUrl);
    }
    //
    QString token = WizToken::token();
    emit tokenAcquired(token);
    //
    if (token.isEmpty())
        return;
    //
    WIZUSERINFO info = WizToken::userInfo();
    //
    info.strKbGUID = m_kbGUID;
    //
    WIZGROUPDATA group;
    if (WizDatabaseManager::instance()->db().getGroupData(m_kbGUID, group))
    {
        info = WIZUSERINFO(info, group);
    }
    //
    WizKMDatabaseServer server(info);
    int count = 0;
    server.getCommentCount(m_GUID, count);
    //
    setCommentsCount(count);
}


WizCommentManager::WizCommentManager(QObject* parent)
    : QObject(parent)
    , m_busy(false)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timer_timeOut()));
}

QString WizCommentManager::getCommentUrlTemplate()
{
    QMutexLocker locker(&m_mutext);
    Q_UNUSED(locker)

    return m_commentUrlTemplate;
}

void WizCommentManager::setCommentUrlTemplate(QString url)
{
    QMutexLocker locker(&m_mutext);
    Q_UNUSED(locker)

    m_commentUrlTemplate = url;
}

/*
 *  获取评论数目是通过网络请求获取的，需要延迟处理请求。防止在快速切换笔记时产生大量的网络请求
 */
void WizCommentManager::queryCommentCount(const QString& kbGUID, const QString& GUID, bool removeOtherQueryRequest)
{
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
        //
        CountQueryData query;
        query.strKBGUID = kbGUID;
        query.strGUID = GUID;
        m_queryList.append(query);

        if (!m_timer.isActive())
        {
            m_timer.start(QUERY_DELAY * 1000);
        }
    }
    //
    if (m_queryList.size() == 1 && !m_busy)
    {
        on_timer_timeOut();
    }
}


void WizCommentManager::on_timer_timeOut()
{
    CountQueryData data;
    pickData(data);

    if (data.strGUID.isEmpty())
        return;

    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=] {
        m_busy = true;
        WizCommentQuerier query(this, data.strKBGUID, data.strGUID);
        connect(&query, SIGNAL(tokenAcquired(QString)), SIGNAL(tokenAcquired(QString)));
        connect(&query, SIGNAL(commentCountAcquired(QString,int)), SIGNAL(commentCountAcquired(QString,int)));
        query.run();
        m_busy = false;
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

