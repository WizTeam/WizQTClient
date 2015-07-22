#ifndef WIZSERVICE_WIZAPIENTRY_H
#define WIZSERVICE_WIZAPIENTRY_H

class QString;

namespace WizService {

class CommonApiEntry
{
public:
    static void setEnterpriseServerIP(const QString& strIP);
    static void setLanguage(const QString& strLocal);
    static QString syncUrl();
    static QString asServerUrl();
    static QString messageServerUrl();
    static QString messageVersionUrl();
    static QString avatarDownloadUrl(const QString& strUserGUID);
    static QString avatarUploadUrl();
    static QString mailShareUrl(const QString& strKUrl, const QString& strMailInfo);
    static QString commentUrl(const QString& strToken, const QString& strKbGUID,const QString& strGUID);
    static QString commentCountUrl(const QString& strKUrl, const QString& strToken,
                                   const QString& strKbGUID, const QString& strGUID);    
    static QString analyzerUploadUrl();
    static QString accountInfoUrl(const QString& strToken);
    static QString groupAttributeUrl(const QString& strToken, const QString& strKbGUID);
    static QString groupUsersUrl(const QString& strToken, const QString& strBizGUID, const QString& strkbGUID);
    static QString createGroupUrl(const QString& strToken);
    static QString captchaUrl(const QString& strCaptchaID, int nWidth = 120, int nHeight = 40);

    //
    static QString standardCommandUrl(const QString& strCommand, bool bUseWizServer = false);
    static QString standardCommandUrl(const QString& strCommand, const QString& strToken, bool bUseWizServer = false);
    static QString standardCommandUrl(const QString& strCommand, const QString& strToken,
                                      const QString& strExtInfo, bool bUseWizServer = false);


    static QString newStandardCommandUrl(const QString& strCommand, const QString& strToken,
                                         const QString& strExt, bool bUseWizServer = false);

    static QString kUrlFromGuid(const QString& strToken, const QString& strKbGUID);

    static QString appstoreParam(bool useAndSymbol = true);

private:
    QString requestUrl(const QString& strCommand);

private:
    static QString m_server;
    static QString m_strLocal;
};

} // namespace WizService

#endif // WIZSERVICE_WIZAPIENTRY_H
