#ifndef CWIZJSONSERVERBASE_H
#define CWIZJSONSERVERBASE_H

#include <memory>
#include <QNetworkAccessManager>

#define JSON_RETURNCODE_OK    200

class CWizJSONServerBase
{
public:
    CWizJSONServerBase();
    ~CWizJSONServerBase();

    int returnCode();
    QString returnMessage();
    QString jsonResult();

    void getRequest(const QString& strUrl);
    void deleteRequest(const QString& strUrl);

protected:
    std::shared_ptr<QNetworkAccessManager> m_net;
    int m_nReturnCode;
    QString m_strReturnMessage;
    QString m_strJSONResult;
};

#endif // CWIZJSONSERVERBASE_H
