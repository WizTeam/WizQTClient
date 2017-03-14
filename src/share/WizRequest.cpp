#include "WizRequest.h"
#include "WizEventLoop.h"

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "jsoncpp/json/json.h"


WIZSTANDARDRESULT::WIZSTANDARDRESULT(ERRORTYPE error, QString message)
    : returnCode(error)
{
    switch (error) {
    case network: returnMessage = QObject::tr("Network error"); break;
    case json: returnMessage = QObject::tr("Response is not an valid json"); break;
    case server: returnMessage = QObject::tr("Server error"); break;
    }
    //
    if (!message.isEmpty()) {
        returnMessage = message;
    }
}

bool WizRequest::execJsonRequest(const QString& url, QString method, const QByteArray& reqBody, QByteArray& resBody)
{
    method = method.toUpper();

    QNetworkAccessManager net;
    QNetworkRequest request;
    request.setUrl(url);
    //
    QNetworkReply* reply = NULL;
    if (method == "POST")
    {
        if (reqBody.isEmpty())
        {
            request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
            reply = net.post(request, reqBody);
        }
    }
    else if (method == "GET")
    {
        reply = net.get(request);
    }
    else if (method == "PUT")
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
        reply = net.put(request, reqBody);
    }
    else if (method == "DELETE")
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
        reply = net.deleteResource(request);
    }
    else
    {
        return false;
    }

    WizAutoTimeOutEventLoop loop(reply);
    loop.setTimeoutWaitSeconds(60 * 60 * 1000);
    loop.exec();
    //
    if (loop.error() != QNetworkReply::NoError)
    {
        qDebug() << "Failed to exec json request, error=" << loop.error() << ", message=" << loop.errorString();
        return false;
    }
    //
    resBody = loop.result();
    //
    return true;
}

bool WizRequest::execJsonRequest(const QString &url, const QString& method, const Json::Value &reqBody, QByteArray &resBody)
{
    Json::FastWriter writer;
    std::string body = writer.write(reqBody);
    QByteArray buffer(body.c_str());
    //
    return execJsonRequest(url, method, buffer, resBody);
}

bool WizRequest::execJsonRequest(const QString &url, QByteArray &resBody)
{
    return execJsonRequest(url, "GET", QByteArray(), resBody);
}

WIZSTANDARDRESULT WizRequest::isSucceededStandardJsonRequest(const QByteArray& resBody, Json::Value& res)
{
    std::string resultString = resBody.toStdString();
    try {
        Json::Reader reader;
        if (!reader.parse(resultString, res))
        {
            qDebug() << "Can't parse result, ret: \n" << QString::fromUtf8(resultString.c_str());
            return WIZSTANDARDRESULT(WIZSTANDARDRESULT::json);
        }
        //
        return isSucceededStandardJsonRequest(res);
    }
    catch (std::exception& err)
    {
        qDebug() << "josn error: " << err.what();
        return WIZSTANDARDRESULT(WIZSTANDARDRESULT::json, QString::fromUtf8(err.what()));
    }

}

WIZSTANDARDRESULT WizRequest::isSucceededStandardJsonRequest(const QByteArray& resBody)
{
    try {
        Json::Value res;
        return isSucceededStandardJsonRequest(resBody, res);
    }
    catch (std::exception& err)
    {
        qDebug() << "josn error: " << err.what();
        return WIZSTANDARDRESULT(WIZSTANDARDRESULT::json, QString::fromUtf8(err.what()));
    }
}

WIZSTANDARDRESULT WizRequest::isSucceededStandardJsonRequest(Json::Value& res)
{
    try {
        //
        Json::Value returnCode = res["returnCode"];
        if (returnCode.asInt() != 200)
        {
            Json::Value returnMessage = res["returnMessage"];
            qDebug() << "Can't upload note data, ret code=" << returnCode.asInt() << ", message=" << QString::fromUtf8(returnMessage.asString().c_str());
            return WIZSTANDARDRESULT(returnCode.asInt(), returnMessage.asString());
        }
        //
        return WIZSTANDARDRESULT(200, QString("OK"));
    }
    catch (std::exception& err)
    {
        qDebug() << "josn error: " << err.what();
        return WIZSTANDARDRESULT(WIZSTANDARDRESULT::json, QString::fromUtf8(err.what()));
    }
}

WIZSTANDARDRESULT WizRequest::execStandardJsonRequest(const QString &url, const QString& method, const QByteArray &reqBody, Json::Value& res)
{
    QByteArray resData;
    if (!execJsonRequest(url, method, reqBody, resData))
        return WIZSTANDARDRESULT(WIZSTANDARDRESULT::network);
    //
    return isSucceededStandardJsonRequest(resData);
}

WIZSTANDARDRESULT WizRequest::execStandardJsonRequest(const QString &url, const QString& method, const Json::Value &reqBody, Json::Value& res)
{
    QByteArray resData;
    if (!execJsonRequest(url, method, reqBody, resData))
        return WIZSTANDARDRESULT(WIZSTANDARDRESULT::network);
    //
    return isSucceededStandardJsonRequest(resData);
}

WIZSTANDARDRESULT WizRequest::execStandardJsonRequest(const QString &url, const QString& method)
{
    Json::Value res;
    return execStandardJsonRequest(url, method, QByteArray(), res);
}


WIZSTANDARDRESULT WizRequest::execStandardJsonRequest(const QString &url, Json::Value& res)
{
    return execStandardJsonRequest(url, "GET", QByteArray(), res);
}

WIZSTANDARDRESULT WizRequest::execStandardJsonRequest(const QString &url)
{
    Json::Value ret;
    return execStandardJsonRequest(url, ret);
}
