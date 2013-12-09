#ifndef WIZSERVICE_TOKEN_H
#define WIZSERVICE_TOKEN_H

#include <QObject>
#include <QDateTime>

class QString;

struct WIZUSERINFO;

namespace WizService {

namespace Internal {
class TokenPrivate;
}

class Token : public QObject
{
    Q_OBJECT

public:
    static Token* instance();

    Token(const QString& strUserId = 0, const QString& strPasswd = 0);
    ~Token();

    static const WIZUSERINFO& info();

    static void requestToken();
    static void setUserId(const QString& strUserId);
    static void setPasswd(const QString& strPasswd);

Q_SIGNALS:
    void tokenAcquired(QString strToken);

    friend class Internal::TokenPrivate;
};

} // namespace WizService

#endif // WIZSERVICE_TOKEN_H
