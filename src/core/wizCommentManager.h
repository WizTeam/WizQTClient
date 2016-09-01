#ifndef CWIZCOMMENTDOWNLOADER_H
#define CWIZCOMMENTDOWNLOADER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QMutex>

/*
 *  为防止快速切换笔记时创建大量评论数目查询带来的问题，获取评论数目的请求会延迟执行
 */


class CWizCommentManager : public QObject
{
    Q_OBJECT
public:
    CWizCommentManager(QObject* parent = 0);

    void queryCommentUrl(const QString& kbGUID, const QString& GUID);
    void queryCommentCount(const QString& kbGUID, const QString& GUID, bool removeOtherQueryRequest);

public slots:
    void on_commentUrlAcquired(QString GUID, QString url);
    void on_commentCountAcquired(QString GUID, int count);

signals:
    void commentUrlAcquired(const QString& GUID, const QString& url);
    void commentCountAcquired(const QString& GUID, int count);

private slots:
    void on_timer_timeOut();

private:
    struct CountQueryData
    {
        QString strGUID;
        QString strKBGUID;
    };

    QList<CountQueryData> m_queryList;
    QTimer m_timer;
    QMutex m_mutext;

private:
    void pickData(CountQueryData& data);
};

class CWizCommentQuerier : public QObject
{
    Q_OBJECT
public:
    enum QueryType
    {
        QueryNone,
        QueryUrl,
        QueryCount,
        QueryUrlAndCount
    };

    CWizCommentQuerier(const QString& kbGUID, const QString& GUID, QueryType type, QObject*parent = 0);

    void run();

signals:
    void commentUrlAcquired(const QString& GUID, const QString& url);
    void commentCountAcquired(const QString& GUID, int count);

private:
    void setCommentsUrl(const QString& url);
    void setCommentsCount(int count);

    void errorOccurred();

    void parseReplyData(const QString& reply);


private:
    QString m_kbGUID;
    QString m_GUID;
    QueryType m_type;
};

#endif // CWIZCOMMENTDOWNLOADER_H
