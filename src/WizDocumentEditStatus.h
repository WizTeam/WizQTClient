#ifndef WIZDOCUMENTEDITSTATUS_H
#define WIZDOCUMENTEDITSTATUS_H

#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QWeakPointer>
#include <QMap>
#include <QPointer>
#include <QWaitCondition>

class QNetworkAccessManager;



class WizDocumentEditStatusSyncThread : public QThread
{
    Q_OBJECT
public:
    WizDocumentEditStatusSyncThread(QObject* parent = 0);
    //
    void startEditingDocument(const QString& strUserAlias, const QString& strKbGUID, const QString& strGUID);
    void stopEditingDocument(const QString& strKbGUID ,const QString& strGUID, bool bModified);
    void documentSaved(const QString& strUserAlias, const QString& strKbGUID ,const QString& strGUID);

    void stop();
    //
    void waitForDone();

public slots:
    void documentUploaded(const QString& strKbGUID ,const QString& strGUID);
    void on_timerOut();

signals:
    void startTimer(int interval);
    void stopTimer();

protected:
    void run();
private:
    QString combineObjID(const QString& strKbGUID, const QString& strGUID);
    void sendEditingMessage();
    void sendDoneMessage();
    bool sendEditingMessage(const QString& strUserAlias, const QString& strObjID);
    bool sendDoneMessage(const QString& strUserAlias, const QString& strObjID);
private:
    //
    bool m_stop;
    bool m_sendNow;

    QMap<QString, QString> m_editingMap;
    QMap<QString, QString> m_modifiedMap;
    QMap<QString, QString> m_doneMap;

    QMutex m_mutex;
    QWaitCondition m_wait;
    QPointer<QNetworkAccessManager> m_netManager;
    QTimer m_timer;
};

class WizDocumentStatusChecker : public QObject
{
    Q_OBJECT
public:
    WizDocumentStatusChecker(QObject* parent = 0);
    ~WizDocumentStatusChecker();

public slots:
    void onTimeOut();
    void recheck();
    void initialise();
    void clearTimers();
    void checkEditStatus(const QString& strKbGUID, const QString& strGUID);
    void stopCheckStatus(const QString& strKbGUID, const QString& strGUID);

signals:
    void checkTimeOut(QString strGUID);
    void documentEditingByOthers(QString strGUID,QStringList editors);
    void checkDocumentChangedFinished(const QString& strGUID, bool bChanged);
    void checkEditStatusFinished(const QString& strGUID, bool eidtable);

private:
    void setDocmentGUID(const QString& strKbGUID,const QString& strGUID);
    void peekDocumentGUID(QString& strKbGUID, QString& strGUID);
    void startRecheck();
    void startCheck();
    bool checkDocumentChangedOnServer(const QString& strKbGUID, const QString& strGUID, bool& isGroup);
    bool checkDocumentEditStatus(const QString& strKbGUID, const QString& strGUID);
    bool checkDocumentEditStatus(const QString& strUrl);

private:
    QTimer* m_timeOutTimer;
//    QTimer* m_loopCheckTimer;
    bool m_stop;
    bool m_networkError;

    QString m_strKbGUID;
    QString m_strGUID;
    QMutex m_mutexWait;

    QString m_strCurGUID;
    QString m_strCurKbGUID;
};


#endif // WIZDOCUMENTEDITSTATUS_H
