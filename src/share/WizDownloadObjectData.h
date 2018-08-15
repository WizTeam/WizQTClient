#ifndef WIZDOWNLOADOBJECTDATA_H
#define WIZDOWNLOADOBJECTDATA_H

#include "WizDatabaseManager.h"

class WizDownloadObjectData
{
    Q_OBJECT

public:
    WizDownloadObjectData(WizDatabaseManager& dbMgr);

    void setData(const WIZOBJECTDATA& data);
    void startDownload();

private:
    WizDatabaseManager& m_dbMgr;
    WIZOBJECTDATA m_data;
    bool m_bInited;

protected:

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
