#ifndef WIZSERVICE_APIENTRY_P_H
#define WIZSERVICE_APIENTRY_P_H

/*
#include <QObject>
#include <QMap>

class QString;

namespace WizService {
namespace Internal {

class ApiEntryPrivate: QObject
{
public:
    ApiEntryPrivate();
    ~ApiEntryPrivate();

    void setEnterpriseServerIP(const QString& strIP);
    void setLanguage(const QString& strLocal);
    QString syncUrl();
    QString asServerUrl();
    QString messageServerUrl();
    QString messageVersionUrl();
    QString mailShareUrl(const QString& strKUrl, const QString& strMailInfo);
    QString avatarDownloadUrl(const QString& strUserGUID);
    QString avatarUploadUrl();
    QString commentUrl(const QString& strToken, const QString& strKbGUID,const QString& strGUID);
    QString commentCountUrl(const QString& strKUrl, const QString& strToken,
                            const QString& strKbGUID, const QString& strGUID);
    QString supportUrl();
    QString changeLogUrl();
    QString upgradeUrl();
    QString analyzerUploadUrl();
    QString accountInfoUrl(const QString& strToken);
    QString groupAttributeUrl(const QString& strToken, const QString& strKbGUID);
    QString groupUsersUrl(const QString& strToken, const QString& strBizGUID, const QString& strkbGUID);

    QString kUrlFromGuid(const QString& strToken, const QString& strKbGUID);

    QString createGroupUrl(const QString& strToken);
    QString standardCommandUrl(const QString& strCommand, bool bUseWizServer);
    QString standardCommandUrl(const QString& strCommand, const QString& strToken, bool bUseWizServer);
    QString standardCommandUrl(const QString& strCommand, const QString& strToken,
                               const QString& strExtInfo, bool bUseWizServer);

    QString newStandardCommandUrl(const QString& strCommand, const QString& strToken,
                                  const QString& strExt, bool bUseWizServer);

private:
    QString m_strEnterpriseAPIUrl;
    QString m_strSyncUrl;
    QString m_strMessageServerUrl;
    QString m_strMessageVersionUrl;
    QString m_strAvatarDownloadUrl;
    QString m_strAvatarUploadUrl;
    QString m_strCommentUrl;
    QString m_strCommentCountUrl;
    QMap<QString, QString> m_mapkUrl;

    //
    QString m_strLocal;

    QString urlFromCommand(const QString& strCommand, bool bUseWizServer);
    QString addExtendedInfo(const QString& strUrl, const QString& strExt);
    QString requestUrl(const QString& strCommand, QString& strUrl, bool bUseWizServer);
    QString requestUrl(const QString& strUrl);
};

} // namespace Internal
} // namespace WizService
*/

#endif // WIZSERVICE_APIENTRY_P_H
