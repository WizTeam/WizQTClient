#ifndef WIZGROUPMESSAGE_H
#define WIZGROUPMESSAGE_H

#include "wizapi.h"

class WizDatabase;
class CWizApiEntry;
class QNetworkAccessManager;

class WizGroupMessage : public CWizApiBase
{
    Q_OBJECT

public:
    WizGroupMessage(WizDatabase& db, QObject* parent = 0);

private:
    WizDatabase& m_db;
    QNetworkAccessManager* m_net;
    QTimer m_timer;

    QString m_strVersionRequestUrl;

    // store my user guid, used for acquire message version number
    QString m_strUserGUID;

    // GUID-name map of biz info, used for fetching biz users list
    QMap<QString, QString> m_bizInfo;

    // message upload status variables
    CWizMessageDataArray m_arrayMsgNeedUpload;
    QList<qint64> m_listMsgUploading;
    bool m_bMsgUploadStatus;

    // 1. upload first, only upload read status
    void uploadMessages();
    void uploadMessageNext();
    void uploadMessageEnd();

    // 2. query message version
    void acquireMessageVersionEntry();
    void queryMessageVersion();

    // 3. download messages
    void downloadMessages();

    // 4. download biz users, for every time new messages arrives
    void fetchBizUsers();
    void fetchBizUsersNext();
    void fetchBizUsersEnd(); // stub

    bool loadBizUsersFromJson(const QString& strJsonUsers,
                              CWizBizUserDataArray& arrayUser);

    // 5. end
    void syncEnd(); // stub

protected:

    virtual void onClientLogin(const WIZUSERINFO& userInfo);
    virtual void onSetMessageStatus();
    virtual void onGetMessages(const CWizMessageDataArray& messages);
    virtual void onGetBizUsers(const QString& strJsonUsers);

public Q_SLOTS:
    void on_timer_timeout();

private Q_SLOTS:
    void on_acquireMessageVersionEntry_finished(const QString& strReply);
    void on_queryMessageVersion_finished();

Q_SIGNALS:
    void messageReady();
};

#endif // WIZGROUPMESSAGE_H
