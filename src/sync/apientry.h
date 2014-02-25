#ifndef WIZSERVICE_WIZAPIENTRY_H
#define WIZSERVICE_WIZAPIENTRY_H

class QString;

namespace WizService {

class ApiEntry
{
public:
    static QString syncUrl();
    static QString messageVersionUrl();
    static QString avatarDownloadUrl(const QString& strUserGUID);
    static QString avatarUploadUrl();
    static QString commentUrl(const QString& strToken, const QString& strKbGUID,const QString& strGUID);
    static QString commentCountUrl(const QString& strServer, const QString& strToken,
                                   const QString& strKbGUID, const QString& strGUID);
    static QString feedbackUrl();
    static QString accountInfoUrl(const QString& strToken);
    static QString groupAttributeUrl(const QString& strToken, const QString& strKbGUID);
    static QString groupUsersUrl(const QString& strToken, const QString& strBizGUID, const QString& strkbGUID);
    static QString createGroupUrl(const QString& strToken);
    //
    static QString standardCommandUrl(const QString& strCommand);
    static QString standardCommandUrl(const QString& strCommand, const QString& strToken);
    static QString standardCommandUrl(const QString& strCommand, const QString& strToken, const QString& strExtInfo);

    static QString kUrlFromGuid(const QString& strToken, const QString& strKbGUID);
};

} // namespace WizService

#endif // WIZSERVICE_WIZAPIENTRY_H
