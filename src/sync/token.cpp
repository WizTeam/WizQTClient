#include "token.h"
#include "token_p.h"

#include <QMutexLocker>
#include <QString>
#include <QDebug>

#include "wizKMServer.h"
#include "asyncapi.h"
#include "apientry.h"

using namespace WizService;
using namespace WizService::Internal;

// use 5 minutes locally, server use 20 minutes
#define TOKEN_TIMEOUT_INTERVAL 60 * 5

TokenPrivate::TokenPrivate(Token* token)
    : q(token)
    , m_mutex(new QMutex(QMutex::Recursive))
    , m_bProcessing(false)
{
}

TokenPrivate::~TokenPrivate()
{
    delete m_mutex;
}

QString TokenPrivate::token()
{
    QMutexLocker locker(m_mutex);
    Q_UNUSED(locker);
    //
//    Q_ASSERT(!m_strUserId.isEmpty() && !m_strPasswd.isEmpty());

    if (m_info.strToken.isEmpty())
    {
        CWizKMAccountsServer asServer(CommonApiEntry::syncUrl());
        if (asServer.Login(m_strUserId, m_strPasswd))
        {
            m_info = asServer.GetUserInfo();
            m_info.tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
            return m_info.strToken;
        }
        else
        {
            m_lastErrorCode = asServer.GetLastErrorCode();
            m_lastErrorMessage = asServer.GetLastErrorMessage();
            return QString();
        }
    }
    //
    if (m_info.tTokenExpried >= QDateTime::currentDateTime())
    {
        return m_info.strToken;
    }
    else
    {
        WIZUSERINFO info;
        info.strToken = m_info.strToken;
        info.strKbGUID = m_info.strKbGUID;
        CWizKMAccountsServer asServer(CommonApiEntry::syncUrl());
        asServer.SetUserInfo(info);

        if (asServer.KeepAlive(m_info.strToken))
        {
            m_info.tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
            return m_info.strToken;
        }
        else
        {
            QString strToken;
            if (asServer.GetToken(m_strUserId, m_strPasswd, strToken))
            {
                m_info.strToken = strToken;
                m_info.tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
                return m_info.strToken;
            }
            else
            {
                m_lastErrorCode = asServer.GetLastErrorCode();
                m_lastErrorMessage = asServer.GetLastErrorMessage();
                return QString();
            }
        }
    }

    Q_ASSERT(0);
}

void TokenPrivate::requestToken()
{
    if (m_bProcessing)
    {
        qDebug() << "Querying token...";
        return;
    }
    //
    class GetTokenRunnable : public QRunnable
    {
        TokenPrivate* m_p;
    public:
        GetTokenRunnable(TokenPrivate* p)
            : m_p(p)
        {
        }

        void run()
        {
            m_p->m_bProcessing = true;
            QString token = m_p->token();
            m_p->m_bProcessing = false;
            Q_EMIT m_p->q->tokenAcquired(token);
        }
    };
    //
    QThreadPool::globalInstance()->start(new GetTokenRunnable(this));
}

void TokenPrivate::clearToken()
{
    m_info.strToken.clear();
}

void TokenPrivate::clearLastError()
{
    m_lastErrorCode = 0;
    m_lastErrorMessage.clear();
}

void TokenPrivate::setUserId(const QString& strUserId)
{
    m_strUserId = strUserId;
}

void TokenPrivate::setPasswd(const QString& strPasswd)
{
    m_strPasswd = strPasswd;
}

WIZUSERINFO TokenPrivate::info()
{
    QMutexLocker locker(m_mutex);
    Q_UNUSED(locker);
    //
    return m_info;
}

int TokenPrivate::lastErrorCode() const
{
    QMutexLocker locker(m_mutex);
    Q_UNUSED(locker);
    //
    return m_lastErrorCode;
}

QString TokenPrivate::lastErrorMessage() const
{
    QMutexLocker locker(m_mutex);
    Q_UNUSED(locker);
    //
    return m_lastErrorMessage;
}


static TokenPrivate* d = 0;
static Token* m_instance = 0;

Token::Token(const QString& strUserId, const QString& strPasswd)
{
    m_instance = this;
    d = new TokenPrivate(this);

    d->setUserId(strUserId);
    d->setPasswd(strPasswd);
}

Token::~Token()
{
    delete d;
    d = 0;
}

Token* Token::instance()
{
    return m_instance;
}

QString Token::token()
{
    Q_ASSERT(m_instance);
    return d->token();
}

void Token::requestToken()
{
    Q_ASSERT(m_instance);

    d->requestToken();
}

void Token::clearToken()
{
    d->clearToken();
}

void Token::clearLastError()
{
    d->clearLastError();
}

void Token::setUserId(const QString& strUserId)
{
    Q_ASSERT(m_instance);

    d->setUserId(strUserId);
}

void Token::setPasswd(const QString& strPasswd)
{
    Q_ASSERT(m_instance);

    d->setPasswd(strPasswd);
}

WIZUSERINFO Token::info()
{
    Q_ASSERT(m_instance);

    return d->info();
}

QString Token::lastErrorMessage()
{
    Q_ASSERT(m_instance);

    return d->lastErrorMessage();
}

int Token::lastErrorCode()
{
    Q_ASSERT(m_instance);

    return d->lastErrorCode();
}
