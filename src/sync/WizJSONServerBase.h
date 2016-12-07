#ifndef CWIZJSONSERVERBASE_H
#define CWIZJSONSERVERBASE_H

#include <memory>
#include <QNetworkAccessManager>

#define JSON_RETURNCODE_OK    200

class WizJSONServerBase
{
public:
    WizJSONServerBase();
    ~WizJSONServerBase();

    int returnCode();

    bool get(const QString& strUrl, QString& strResult);
    bool deleteResource(const QString& strUrl);

    static bool getReturnCodeAndMessageFromJSON(const QString& strJSON, int& returnCode, QString& returnMessage);

protected:
    std::shared_ptr<QNetworkAccessManager> m_net;
    int m_nReturnCode;
    QString m_strReturnMessage;
    QString m_strJSONResult;
};

#endif // CWIZJSONSERVERBASE_H
