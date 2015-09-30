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

    static WIZUSERINFO info();

    static QString token(); // sync
    static void requestToken(); // async
    static void clearToken();
    static void clearLastError();
    static void setUserId(const QString& strUserId);
    static void setPasswd(const QString& strPasswd);

    static QString lastErrorMessage();
    static int lastErrorCode();

Q_SIGNALS:
    void tokenAcquired(QString strToken);

    friend class Internal::TokenPrivate;
};

} // namespace WizService

#endif // WIZSERVICE_TOKEN_H
