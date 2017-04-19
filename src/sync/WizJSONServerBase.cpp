#include "WizJSONServerBase.h"
#include "share/WizEventLoop.h"
#include "share/jsoncpp/json/json.h"
#include <QDebug>

WizJSONServerBase::WizJSONServerBase()
    : m_net(std::make_shared<QNetworkAccessManager>())
{

}

WizJSONServerBase::~WizJSONServerBase()
{

}

int WizJSONServerBase::returnCode()
{
    return m_nReturnCode;
}

bool WizJSONServerBase::get(const QString& strUrl, QString& strResult)
{
    QNetworkReply* reply = m_net->get(QNetworkRequest(strUrl));
    WizAutoTimeOutEventLoop loop(reply);
    loop.exec();
    //
    if (loop.error() != QNetworkReply::NoError)
    {
        qDebug() << "[JSONRequest]Send get request failed.";
        return false;
    }

    strResult = QString::fromUtf8(loop.result().constData());
    return !strResult.isEmpty();
}

bool WizJSONServerBase::deleteResource(const QString& strUrl)
{
    QNetworkReply* reply = m_net->deleteResource(QNetworkRequest(strUrl));
    WizAutoTimeOutEventLoop loop(reply);
    loop.exec();
    //
    if (loop.error() != QNetworkReply::NoError)
    {
        qDebug() << "[JSONDelete]Send delete request failed. result : " << loop.result();
        return false;
    }

    QString result = QString::fromUtf8(loop.result().constData());

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

bool WizJSONServerBase::getReturnCodeAndMessageFromJSON(const QString& strJSON,
                                                         int& returnCode, QString& returnMessage)
{
    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(strJSON.toUtf8().constData(), d))
        return false;

    if (!d.isMember("return_code"))
    {
        qDebug() << "can not found return code from json data";
        return false;
    }

    returnCode = d["return_code"].asInt();

    if (!d.isMember("return_message"))
    {
        qDebug() << "can not found return message from json data";
        return false;
    }

    returnMessage = QString::fromStdString(d["return_message"].asString());

    return true;
}

