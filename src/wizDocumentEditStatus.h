#ifndef WIZDOCUMENTEDITSTATUS_H
#define WIZDOCUMENTEDITSTATUS_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QWeakPointer>


class wizDocumentEditStatusSyncThread : public QThread
{
public:
    wizDocumentEditStatusSyncThread(QObject* parent = 0);

private:
    QList<QString> m_editingList;
    QList<QString> m_doneList;
};

class wizDocumentEditStatusCheckThread : public QThread
{
    Q_OBJECT
public:
    wizDocumentEditStatusCheckThread(QObject* parent = 0);
    void checkEditStatus(const QString& strKbGUID,const QString& strGUID);
    void downloadData(const QString& strUrl);

signals:
    void checkFinished(QString strGUID,QStringList editors);

protected:
    virtual void run();

private:
    void setDocmentGUID(const QString& strKbGUID,const QString& strGUID);

private:
    QString m_strKbGUID;
    QString m_strGUID;
};

#endif // WIZDOCUMENTEDITSTATUS_H
