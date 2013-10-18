#ifndef WIZCLOUDPOOL_H
#define WIZCLOUDPOOL_H

#include <QObject>

class CWizKMAccountsServer;
class CWizDatabaseManager;


class CWizCloudPool : public QObject
{
    Q_OBJECT

public:
    static CWizCloudPool* instance();
    void init(CWizDatabaseManager* dbMgr);

    Q_INVOKABLE void getToken();
    Q_INVOKABLE void registerAccount(const QString &strUserName, const QString &strPassword, const QString &strInviteCode);

    QString lastErrorMessage() { return m_strLastErrorMsg; }

private:
    CWizCloudPool() : m_dbMgr(0), m_aServer(0), m_bInited(false) {}
    virtual ~CWizCloudPool() {}
    static CWizCloudPool* _instance;
    bool m_bInited;

    void _init(); // internal init for thread

    QString m_strLastErrorMsg;

protected:
    CWizDatabaseManager* m_dbMgr;
    CWizKMAccountsServer* m_aServer;

Q_SIGNALS:
    void tokenAcquired(const QString& strToken);
    void accountRegistered(bool bOk);
};


#endif // WIZCLOUDPOOL_H
