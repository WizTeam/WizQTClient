#ifndef WIZSERVICE_WIZAPIENTRY_H
#define WIZSERVICE_WIZAPIENTRY_H

#define WIZNOTE_API_SERVER      "http://api.wiz.cn/"

#include <QString>
#include <QMap>

namespace WizService {

class CommonApiEntry
{
public:
    static void setEnterpriseServerIP(const QString& strIP);
    static void setLanguage(const QString& strLocal);
    static QString syncUrl();
    static QString asServerUrl();
    static QString messageServerUrl();
    static QString avatarDownloadUrl(const QString& strUserGUID);
    static QString avatarUploadUrl();
    static QString mailShareUrl(const QString& strKUrl, const QString& strMailInfo);
    static QString commentUrl(const QString& strToken, const QString& strKbGUID,const QString& strGUID);
    static QString commentCountUrl(const QString& strKUrl, const QString& strToken,
                                   const QString& strKbGUID, const QString& strGUID);
    static QString accountInfoUrl(const QString& strToken);
    static QString groupAttributeUrl(const QString& strToken, const QString& strKbGUID);
    static QString groupUsersUrl(const QString& strToken, const QString& strBizGUID, const QString& strkbGUID);
    static QString createGroupUrl(const QString& strToken);
    static QString captchaUrl(const QString& strCaptchaID, int nWidth = 120, int nHeight = 40);
    static QString editStatusUrl();

    //
    static QString makeUpUrlFromCommand(const QString& strCommand);
    static QString makeUpUrlFromCommand(const QString& strCommand, const QString& strToken);
    static QString makeUpUrlFromCommand(const QString& strCommand, const QString& strToken,
                                      const QString& strExtInfo);

    // new standard command url for new server
    static QString newStandardCommandUrl(const QString& strCommand, const QString& strToken,
                                         const QString& strExt);

    static QString getUrlByCommand(const QString& strCommand);

    static QString kUrlFromGuid(const QString& strToken, const QString& strKbGUID);

    static QString appstoreParam(bool useAndSymbol = true);

private:
    static QString requestUrl(const QString& strCommand);
    static void getEndPoints();
    static void updateUrlCache(const QString& strCommand, const QString& url);
    static QString getUrlFromCache(const QString& strCommand);

private:
    static QString m_server;
    static QMap<QString, QString> m_cacheMap;
    static QMap<QString, QString> m_mapkUrl;
};

class WizApiEntry
{
public:
    WizApiEntry();

    static QString analyzerUploadUrl();
    static QString crashReportUrl();
    static QString standardCommandUrl(const QString& strCommand);
    static QString standardCommandUrl(const QString& strCommand, const QString& strToken);
    static QString standardCommandUrl(const QString& strCommand, const QString& strToken,
                                      const QString& strExtInfo);

private:
    static QString requestUrl(const QString& strCommand);
    static QString urlFromCommand(const QString& strCommand);
};

} // namespace WizService

#endif // WIZSERVICE_WIZAPIENTRY_H
