#include "token.h"
#include "token_p.h"

#include <QString>
#include <QMutex>

#include "wizkmxmlrpc.h"
#include "asyncapi.h"

using namespace WizService;
using namespace WizService::Internal;

#define TOKEN_TIMEOUT_INTERVAL 60*15

TokenPrivate::TokenPrivate(Token* token)
    : q(token)
    , m_api(new AsyncApi())
    , m_mutex(new QMutex())
{
    connect(m_api, SIGNAL(loginFinished(const WIZUSERINFO&)), SLOT(onLoginFinished(const WIZUSERINFO&)));
    connect(m_api, SIGNAL(getTokenFinished(QString, QString)), SLOT(onGetTokenFinished(QString, QString)));
    connect(m_api, SIGNAL(keepAliveFinished(bool, QString)), SLOT(onKeepAliveFinished(bool, QString)));
}

TokenPrivate::~TokenPrivate()
{
    delete m_mutex;
}

void TokenPrivate::requestToken()
{
    Q_ASSERT(!m_strUserId.isEmpty() && !m_strPasswd.isEmpty());

    m_mutex->lock();

    if (m_info.strToken.isEmpty()) {
        m_api->login(m_strUserId, m_strPasswd);
        return;
    }

    if (QDateTime::currentDateTime() >= m_info.tTokenExpried) {
        m_api->keepAlive(m_info.strToken);
        return;
    }
    //else
    //{
    //    Q_EMIT q->tokenAcquired(m_info.strToken);
    //    m_mutex->unlock();
    //}
}

void TokenPrivate::setUserId(const QString& strUserId)
{
    m_strUserId = strUserId;
}

void TokenPrivate::setPasswd(const QString& strPasswd)
{
    m_strPasswd = strPasswd;
}

const WIZUSERINFO& TokenPrivate::info()
{
    return m_info;
}

void TokenPrivate::onLoginFinished(const WIZUSERINFO& info)
{
    if (!info.strToken.isEmpty()) {
        m_info = info;
        return;
    }

    Q_EMIT q->tokenAcquired(m_info.strToken);
    m_mutex->unlock();
}

void TokenPrivate::onGetTokenFinished(const QString& strToken, const QString& strMsg)
{
    Q_UNUSED(strMsg);

    if (!strToken.isEmpty()) {
        m_info.strToken = strToken;
        m_info.tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
    }

    Q_EMIT q->tokenAcquired(m_info.strToken);
    m_mutex->unlock();
}

void TokenPrivate::onKeepAliveFinished(bool bOk, const QString& strMsg)
{
    Q_UNUSED(strMsg);

    if (bOk) {
        m_info.tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
        Q_EMIT q->tokenAcquired(m_info.strToken);
        m_mutex->unlock();
        return;
    }

    m_api->getToken(m_strUserId, m_strPasswd);
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

void Token::requestToken()
{
    Q_ASSERT(m_instance);

    d->requestToken();
}

void Token::setUserId(const QString& strUserId)
{
    if (!m_instance) {
        m_instance = new Token();
    }

    d->setUserId(strUserId);
}

void Token::setPasswd(const QString& strPasswd)
{
    if (!m_instance) {
        m_instance = new Token();
    }

    d->setPasswd(strPasswd);
}

const WIZUSERINFO& Token::info()
{
    if (!m_instance) {
        m_instance = new Token();
    }

    return d->info();
}
