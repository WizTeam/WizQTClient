#pragma once
#include <QtGlobal>
#include <QString>

#include "jsoncpp/json/json-forwards.h"

struct WIZSTANDARDRESULT
{
    int returnCode;
    QString returnMessage;
    QString externCode;
    //
    enum ERRORTYPE
    {
        network = -1,
        json = -2,
        server = -3,
        format = -4
    };

    //
    WIZSTANDARDRESULT()
    {
        *this = noError();
    }

    //
    WIZSTANDARDRESULT(int code, QString message, QString extCode)
        : returnCode(code)
        , returnMessage(message)
        , externCode(extCode)
    {

    }
    WIZSTANDARDRESULT(int code, std::string message, std::string extCode)
        : returnCode(code)
        , returnMessage(QString::fromStdString(message))
        , externCode(QString::fromStdString(extCode))
    {

    }
    //
    WIZSTANDARDRESULT(ERRORTYPE error, QString message = "", QString extCode = "");
    //
    static WIZSTANDARDRESULT noError() { return WIZSTANDARDRESULT(int(200), QString("OK"), QString("")); }
    //
    operator bool () {
        return returnCode == 200;
    }
    //
    QString toString() const {
        return QString("code=%1, message=%2, externCode=%3").arg(QString::number(returnCode), returnMessage, externCode);
    }

    //
    bool isNetworkError() const {
        return returnCode < 0;
    }
};

class WizRequest
{
private:
    static bool execJsonRequest(const QString& url, QString method, const QByteArray& reqBody, QByteArray& resBody);
    static bool execJsonRequest(const QString &url, const QString& method, const Json::Value &reqBody, QByteArray &resBody);
    static bool execJsonRequest(const QString &url, QByteArray &resBody);
    //
public:
    static WIZSTANDARDRESULT isSucceededStandardJsonRequest(const QByteArray& resBody, Json::Value& res);
    static WIZSTANDARDRESULT isSucceededStandardJsonRequest(const QByteArray& resBody);
    static WIZSTANDARDRESULT isSucceededStandardJsonRequest(Json::Value& res);
    //
    static WIZSTANDARDRESULT execStandardJsonRequest(const QString &url, const QString& method, const QByteArray &reqBody, Json::Value& res);
    static WIZSTANDARDRESULT execStandardJsonRequest(const QString &url, const QString& method, const Json::Value &reqBody, Json::Value& res);
    static WIZSTANDARDRESULT execStandardJsonRequest(const QString &url, const QString& method);
    static WIZSTANDARDRESULT execStandardJsonRequest(const QString &url, Json::Value& res);
    static WIZSTANDARDRESULT execStandardJsonRequest(const QString &url);
};


