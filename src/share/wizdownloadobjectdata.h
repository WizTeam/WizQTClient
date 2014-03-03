#ifndef WIZDOWNLOADOBJECTDATA_H
#define WIZDOWNLOADOBJECTDATA_H

#include "wizDatabaseManager.h"
#include "../sync/wizXmlRpcServer.h"

class CWizDownloadObjectData
{
    Q_OBJECT

public:
    CWizDownloadObjectData(CWizDatabaseManager& dbMgr);

    void setData(const WIZOBJECTDATA& data);
    void startDownload();

private:
    CWizDatabaseManager& m_dbMgr;
    WIZOBJECTDATA m_data;
    bool m_bInited;

protected:
    virtual void onXmlRpcError(const QString& strMethodName,
                               WizXmlRpcError err,
                               int errorCode,
                               const QString& errorMessage);

    virtual void onClientLogin(const WIZUSERINFO& userInfo);
    virtual void onGetGroupList(const CWizGroupDataArray& arrayGroup);
    virtual void onDownloadObjectDataCompleted(const WIZOBJECTDATA& data);

    void startDownloadObjectData();

Q_SIGNALS:
    void downloadDone(bool succeeded);

public Q_SLOTS:
    void processLog(const QString& strMsg);
};


#endif // WIZDOWNLOADOBJECTDATA_H
