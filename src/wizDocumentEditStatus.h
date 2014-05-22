#ifndef WIZDOCUMENTEDITSTATUS_H
#define WIZDOCUMENTEDITSTATUS_H

#include <QThread>
#include <QMutex>
#include <QWeakPointer>
#include <QMap>
#include <QPointer>
#include <QTimer>

class QNetworkAccessManager;



class wizDocumentEditStatusSyncThread : public QThread
{
    Q_OBJECT
public:
    wizDocumentEditStatusSyncThread(QObject* parent = 0);
    ~wizDocumentEditStatusSyncThread();
    void addEditingDocument(const QString& strUserAlias,const QString& strKbGUID ,const QString& strGUID);
    void addDoneDocument(const QString& strKbGUID, const QString& strGUID);
    void setAllDocumentDone();
    void stop();

signals:
    //void sendEditStatusRequest();

protected:
    void run();

private:
    //
    void sendAllDoneMessage();
    void sendEditingMessage();
    void sendEditingMessage(const QString& strUserAlias, const QString& strObjID);
    void sendDoneMessage();
    void sendDoneMessage(const QString& strUserAlias, const QString& strObjID);
private:
    struct EditStatusObj{
    //key : ObjID, value : UserID
        QString strObjID;
        QString strUserName;

        void clear(){
            strObjID.clear();
            strUserName.clear();
        }
    };

    EditStatusObj m_editingObj;
    EditStatusObj m_doneObj;
    QMutex m_mutext;
    QPointer<QNetworkAccessManager> m_netManager;
    QTimer m_timer;
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
