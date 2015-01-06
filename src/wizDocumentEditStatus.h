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



class CWizDocumentEditStatusSyncThread : public QThread
{
    Q_OBJECT
public:
    CWizDocumentEditStatusSyncThread(QObject* parent = 0);
    //
    void startEditingDocument(const QString& strUserAlias, const QString& strKbGUID, const QString& strGUID);
    void stopEditingDocument(const QString& strKbGUID ,const QString& strGUID, bool bModified);
    void documentSaved(const QString& strUserAlias, const QString& strKbGUID ,const QString& strGUID);

    void stop();
    //
    void waitForDone();

public slots:
    void documentUploaded(const QString& strKbGUID ,const QString& strGUID);

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

    QMutex m_mutext;
    QPointer<QNetworkAccessManager> m_netManager;
};

class CWizDocumentStatusCheckThread : public QThread
{
    Q_OBJECT
public:
    CWizDocumentStatusCheckThread(QObject* parent = 0);
    ~CWizDocumentStatusCheckThread();
    void checkEditStatus(const QString& strKbGUID,const QString& strGUID);
    void downloadData(const QString& strUrl);
    //
    void stop() { m_stop = true; m_wait.wakeAll(); }
    //
    void waitForDone();

public slots:
    void needRecheck();
    void onTimeOut();

signals:
    void checkTimeOut(QString strGUID);
    void checkFinished(QString strGUID,QStringList editors);
    void checkDocumentChangedFinished(const QString& strGUID, bool bChanged, int lastVersion);
    void syncDatabaseRequest(const QString& strKbGUID);

protected:
    virtual void run();

private:
    void setDocmentGUID(const QString& strKbGUID,const QString& strGUID);

    bool checkDocumentChangedOnServer(const QString& strKbGUID, const QString& strGUID, int& versionOnServer);
    bool checkDocumentEditStatus(const QString& strKbGUID, const QString& strGUID);

private:
    QString m_strKbGUID;
    QString m_strGUID;
    bool m_stop;
    QMutex m_mutexWait;
    QWaitCondition m_wait;
    bool m_needRecheck;

    QTimer* m_timer;
    QString m_strCurGUID;
    QString m_strCurKbGUID;
};

#endif // WIZDOCUMENTEDITSTATUS_H
