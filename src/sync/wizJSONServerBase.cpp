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

QString CWizJSONServerBase::returnMessage()
{
    return m_strReturnMessage;
}

QString CWizJSONServerBase::jsonResult()
{
    return m_strJSONResult;
}

void CWizJSONServerBase::getRequest(const QString& strUrl)
{
    m_nReturnCode = 0;
    m_strReturnMessage.clear();
    m_strJSONResult.clear();

    QNetworkReply* reply = m_net->get(QNetworkRequest(strUrl));
    CWizAutoTimeOutEventLoop loop(reply);
    loop.exec();
    //
    if (loop.error() != QNetworkReply::NoError)
    {
        qDebug() << "[JSONRequest]Send get request failed.";
        return;
    }

    m_strJSONResult = loop.result();
    rapidjson::Document d;
    d.Parse<0>(loop.result().toUtf8().constData());

    if (!d.HasMember("return_code"))
    {
        qDebug() << "[JSONRequest]Send get request do not get return code ";
        return;
    }

    m_nReturnCode = d.FindMember("return_code")->value.GetInt();
    m_strReturnMessage = d.FindMember("return_message")->value.GetString();
    if (m_nReturnCode != JSON_RETURNCODE_OK )
    {
        qDebug() << "[JSONRequest]Send get request error :  " << m_nReturnCode << loop.result();
    }
    else
    {
        qDebug() << "[JSONRequest]Send get request OK";
    }
}

void CWizJSONServerBase::deleteRequest(const QString& strUrl)
{
    m_nReturnCode = 0;
    m_strReturnMessage.clear();
    m_strJSONResult.clear();

    QNetworkReply* reply = m_net->deleteResource(QNetworkRequest(strUrl));
    CWizAutoTimeOutEventLoop loop(reply);
    loop.exec();
    //
    if (loop.error() != QNetworkReply::NoError)
    {
        qDebug() << "[JSONDelete]Send delete request failed. result : " << loop.result();
        return;
    }

    m_strJSONResult = loop.result();
    rapidjson::Document d;
    d.Parse<0>(loop.result().toUtf8().constData());

    if (!d.HasMember("return_code"))
    {
        qDebug() << "[JSONDelete]Send delete request do not get return code ";
        return;
    }

    m_nReturnCode = d.FindMember("return_code")->value.GetInt();
    m_strReturnMessage = d.FindMember("return_message")->value.GetString();
    if (m_nReturnCode != JSON_RETURNCODE_OK )
    {
        qDebug() << "[JSONDelete]Send delete request status error :  " << m_nReturnCode << loop.result();
    }
    else
    {
        qDebug() << "[JSONDelete]Send delete request OK";
    }
}

