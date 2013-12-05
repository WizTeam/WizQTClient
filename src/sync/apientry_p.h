#ifndef WIZSERVICE_APIENTRY_P_H
#define WIZSERVICE_APIENTRY_P_H

#include <QObject>

class QString;

namespace WizService {
namespace Internal {

class ApiEntryPrivate: QObject
{
public:
    ApiEntryPrivate();
    ~ApiEntryPrivate();

    QString syncUrl();
    QString messageVersionUrl();
    QString avatarDownloadUrl();
    QString avatarUploadUrl();
    QString commentUrl();
    QString commentCountUrl();
    QString feedbackUrl();
    QString accountInfoUrl(const QString& strToken);
    QString groupAttributeUrl(const QString& strToken, const QString& strKbGUID);

private:
    QString m_strSyncUrl;
    QString m_strMessageVersionUrl;
    QString m_strAvatarDownloadUrl;
    QString m_strAvatarUploadUrl;
    QString m_strCommentUrl;
    QString m_strCommentCountUrl;
    QString m_strFeedbackUrl;

    QString urlFromCommand(const QString& strCommand);
    QString addExtendedInfo(const QString& strUrl, const QString& strExt);
    QString requestUrl(const QString& strCommand, QString& strUrl);
    QString requestUrl(const QString& strUrl);
};

} // namespace Internal
} // namespace WizService


#endif // WIZSERVICE_APIENTRY_P_H
