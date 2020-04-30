#include "WizRequest.h"
#include "WizEventLoop.h"

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "jsoncpp/json/json.h"
#include "share/WizMisc.h"
#include "utils/WizLogger.h"


WIZSTANDARDRESULT::WIZSTANDARDRESULT(ERRORTYPE error, QString message, QString extCode)
    : returnCode(error)
{
    switch (error) {
    case network: returnMessage = QObject::tr("Network error"); break;
    case json: returnMessage = QObject::tr("Response is not an valid json"); break;
    case server: returnMessage = QObject::tr("Server error"); break;
    case format: returnMessage = QObject::tr("Json format error"); break;
    }
    //
    if (!message.isEmpty()) {
        returnMessage = message;
    }
    //
    externCode = extCode;
}

bool WizRequest::execJsonRequest(const QString& url, QString method, const QByteArray& reqBody, QByteArray& resBody)
{
#ifdef QT_DEBUG
    //qDebug() << url;
#endif
    //
    method = method.toUpper();

    QNetworkAccessManager net;
    QNetworkRequest request;
    request.setUrl(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    //
    QNetworkReply* reply = NULL;
    if (method == "POST")
    {
        if (!reqBody.isEmpty())
        {
            request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
        }
        reply = net.post(request, reqBody);
    }
    else if (method == "GET")
    {
        reply = net.get(request);
    }
    else if (method == "PUT")
    {
        if (!reqBody.isEmpty())
        {
            request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
        }
        //
        reply = net.put(request, reqBody);
    }
    else if (method == "DELETE")
    {
        reply = net.deleteResource(request);
    }
    else
    {
        return false;
    }

    WizAutoTimeOutEventLoop loop(reply);
    loop.setTimeoutWaitSeconds(60 * 60);
    loop.exec();
    //
    QNetworkReply::NetworkError err = loop.error();
    if (err != QNetworkReply::NoError)
    {
        TOLOG3("Failed to exec json request, network error=%1, message=%2, url=%3", WizIntToStr(err), loop.errorString(), url);
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
        if (returnCode.isNull())
        {
            returnCode = res["return_code"];
        }
        //
        if (returnCode.asInt() != 200)
        {
            Json::Value returnMessage = res["returnMessage"];
            Json::Value externCodeValue = res["externCode"];
            //
            if (returnMessage.isNull()) {
                returnMessage = res["return_message"];
            }
            //
            std::string externCode;
            if (externCodeValue.isString()) {
                externCode = externCodeValue.asString();
            } else if (externCodeValue.isInt()) {
                externCode = WizIntToStr(externCodeValue.asInt()).toUtf8().constData();
            }
            //
            TOLOG3("Can't exec request, ret code=%1, message=%2, externCode=%3",
                   WizIntToStr(returnCode.asInt()),
                   QString::fromStdString(returnMessage.asString()),
                   QString::fromStdString(externCode));
            //
            return WIZSTANDARDRESULT(returnCode.asInt(), returnMessage.asString(), externCode);
        }
        //
        return WIZSTANDARDRESULT(200, QString("OK"), QString());
    }
    catch (std::exception& err)
    {
        qDebug() << "josn error: " << err.what();
        return WIZSTANDARDRESULT(WIZSTANDARDRESULT::json, QString::fromUtf8(err.what()), "");
    }
}

WIZSTANDARDRESULT WizRequest::execStandardJsonRequest(const QString &url, const QString& method, const QByteArray &reqBody, Json::Value& res)
{
    QByteArray resData;
    if (!execJsonRequest(url, method, reqBody, resData))
        return WIZSTANDARDRESULT(WIZSTANDARDRESULT::network);
    //
    return isSucceededStandardJsonRequest(resData, res);
}

WIZSTANDARDRESULT WizRequest::execStandardJsonRequest(const QString &url, const QString& method, const Json::Value &reqBody, Json::Value& res)
{
    QByteArray resData;
    if (!execJsonRequest(url, method, reqBody, resData))
        return WIZSTANDARDRESULT(WIZSTANDARDRESULT::network);
    //
    return isSucceededStandardJsonRequest(resData, res);
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
