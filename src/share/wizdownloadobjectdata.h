#ifndef WIZDOWNLOADOBJECTDATA_H
#define WIZDOWNLOADOBJECTDATA_H

#include "wizapi.h"
#include "wizdatabase.h"

class CWizDownloadObjectData : public CWizApi
{
    Q_OBJECT

public:
    CWizDownloadObjectData(CWizDatabase& db);

    void setData(const WIZOBJECTDATA& data);
    void startDownload();

private:
    WIZOBJECTDATA m_data;
    bool m_bInited;

protected:
    virtual void onXmlRpcError(const QString& strMethodName, \
                             WizXmlRpcError err, int errorCode, \
                             const QString& errorMessage);
    virtual void onClientLogin(const WIZUSERINFO& userInfo);
    virtual void onDownloadObjectDataCompleted(const WIZOBJECTDATA& data);
    virtual void onClientLogout();

Q_SIGNALS:
    void downloadDone(bool succeeded);

public Q_SLOTS:
    void processLog(const QString& strMsg);
};


#endif // WIZDOWNLOADOBJECTDATA_H
