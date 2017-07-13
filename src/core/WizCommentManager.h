#ifndef CWIZCOMMENTDOWNLOADER_H
#define CWIZCOMMENTDOWNLOADER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QMutex>

/*
 *  为防止快速切换笔记时创建大量评论数目查询带来的问题，获取评论数目的请求会延迟执行
 */


class WizCommentManager : public QObject
{
    Q_OBJECT
public:
    WizCommentManager(QObject* parent = 0);

    QString getCommentUrlTemplate();
    void queryCommentCount(const QString& kbGUID, const QString& GUID, bool removeOtherQueryRequest);

signals:
    void tokenAcquired(QString token);
    void commentCountAcquired(QString documentGUID, int count);

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
    bool m_busy;
    QString m_commentUrlTemplate;

private:
    void pickData(CountQueryData& data);
    void setCommentUrlTemplate(QString url);
    //
    friend class WizCommentQuerier;
};

class WizCommentQuerier : public QObject
{
    Q_OBJECT
public:
    WizCommentQuerier(WizCommentManager* manager, const QString& kbGUID, const QString& GUID, QObject*parent = 0);

    void run();

signals:
    void tokenAcquired(QString token);
    void commentCountAcquired(QString documentGUID, int count);

private:
    void setCommentsCount(int count);

private:
    WizCommentManager* m_manager;
    QString m_kbGUID;
    QString m_GUID;
};

#endif // CWIZCOMMENTDOWNLOADER_H
