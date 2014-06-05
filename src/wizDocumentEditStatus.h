#ifndef WIZDOCUMENTEDITSTATUS_H
#define WIZDOCUMENTEDITSTATUS_H

#include <QThread>
#include <QMutex>
#include <QWeakPointer>
#include <QMap>
#include <QPointer>
#include <QTimer>

class QNetworkAccessManager;



class CWizDocumentEditStatusSyncThread : public QThread
{
    Q_OBJECT
public:
    CWizDocumentEditStatusSyncThread(QObject* parent = 0);
    //
    void stopEditingDocument();
    void setCurrentEditingDocument(const QString& strUserAlias,const QString& strKbGUID ,const QString& strGUID);
    void stop();
protected:
    void run();
private:
    void sendEditingMessage();
    void sendDoneMessage();
    void sendEditingMessage(const QString& strUserAlias, const QString& strObjID);
    void sendDoneMessage(const QString& strUserAlias, const QString& strObjID);
private:
    struct EditStatusObj{
        QString strObjID;
        QString strUserName;
        //
        void clear(){
            strObjID.clear();
            strUserName.clear();
        }
    };
    //
    bool m_stop;
    bool m_sendNow;

    EditStatusObj m_editingObj;
    EditStatusObj m_oldObj;

    QMutex m_mutext;
    QPointer<QNetworkAccessManager> m_netManager;
};

class CWizDocumentEditStatusCheckThread : public QThread
{
    Q_OBJECT
public:
    CWizDocumentEditStatusCheckThread(QObject* parent = 0);
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
