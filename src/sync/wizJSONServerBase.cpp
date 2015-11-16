#include "wizJSONServerBase.h"
#include "share/wizEventLoop.h"
#include "rapidjson/document.h"
#include <QDebug>

CWizJSONServerBase::CWizJSONServerBase()
    : m_net(std::make_shared<QNetworkAccessManager>())
{

}

CWizJSONServerBase::~CWizJSONServerBase()
{

}

int CWizJSONServerBase::returnCode()
{
    return m_nReturnCode;
}

bool CWizJSONServerBase::get(const QString& strUrl, QString& strResult)
{
    QNetworkReply* reply = m_net->get(QNetworkRequest(strUrl));
    CWizAutoTimeOutEventLoop loop(reply);
    loop.exec();
    //
    if (loop.error() != QNetworkReply::NoError)
    {
        qDebug() << "[JSONRequest]Send get request failed.";
        return false;
    }

    strResult = loop.result();
    return !strResult.isEmpty();
}

bool CWizJSONServerBase::deleteResource(const QString& strUrl)
{
    QNetworkReply* reply = m_net->deleteResource(QNetworkRequest(strUrl));
    CWizAutoTimeOutEventLoop loop(reply);
    loop.exec();
    //
    if (loop.error() != QNetworkReply::NoError)
    {
        qDebug() << "[JSONDelete]Send delete request failed. result : " << loop.result();
        return false;
    }

    QString result = loop.result();

    int returnCode = 0;
    QString returnMessage;
    if (!getReturnCodeAndMessageFromJSON(result, returnCode, returnMessage))
        return false;

    if (returnCode != JSON_RETURNCODE_OK )
    {
        qDebug() << "[JSONDelete]Send delete request status error, code : " << returnCode << " message : " << returnMessage;
    }
    else
    {
        qDebug() << "[JSONDelete]Send delete request OK";
    }

    return returnCode == JSON_RETURNCODE_OK;
}

bool CWizJSONServerBase::getReturnCodeAndMessageFromJSON(const QString& strJSON,
                                                         int& returnCode, QString& returnMessage)
{
    rapidjson::Document d;
    d.Parse<0>(strJSON.toUtf8().constData());

    if (!d.HasMember("return_code"))
    {
        qDebug() << "can not found return code from json data";
        return false;
    }

    returnCode = d.FindMember("return_code")->value.GetInt();

    if (!d.HasMember("return_message"))
    {
        qDebug() << "can not found return message from json data";
        return false;
    }

    returnMessage = d.FindMember("return_message")->value.GetString();

    return true;
}

