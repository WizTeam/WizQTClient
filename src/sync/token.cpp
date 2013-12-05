#include "token.h"
#include "token_p.h"

#include <QString>
#include <QWaitCondition>
#include <QMutex>

#include "wizkmxmlrpc.h"
#include "asyncapi.h"

using namespace WizService;
using namespace WizService::Internal;

TokenPrivate::TokenPrivate(Token* token)
    : q(token)
    , m_api(new AsyncApi())
    , m_mutex(new QMutex())
    , m_waiter(new QWaitCondition())
{
    connect(m_api, SIGNAL(getTokenFinished(QString, QString)), SLOT(onGetTokenFinished(QString, QString)));
    connect(m_api, SIGNAL(keepAliveFinished(bool, QString)), SLOT(onKeepAliveFinished(bool, QString)));
}

TokenPrivate::~TokenPrivate()
{
    delete m_mutex;
    delete m_waiter;
}

void TokenPrivate::requestToken()
{
    Q_ASSERT(!m_strUserId.isEmpty() && !m_strPasswd.isEmpty());

    m_mutex->lock();

    if (m_strToken.isEmpty()) {
        m_api->getToken(m_strUserId, m_strPasswd);
    } else {
        m_api->keepAlive(m_strToken);
    }
}

void TokenPrivate::setUserId(const QString& strUserId)
{
    m_strUserId = strUserId;
}

void TokenPrivate::setPasswd(const QString& strPasswd)
{
    m_strPasswd = strPasswd;
}

void TokenPrivate::onGetTokenFinished(const QString& strToken, const QString& strMsg)
{
    m_strToken = strToken;
    Q_EMIT q->tokenAcquired(strToken, strMsg);

    m_mutex->unlock();
}

void TokenPrivate::onKeepAliveFinished(bool bOk, const QString& strMsg)
{
    if (bOk) {
        Q_EMIT q->tokenAcquired(m_strToken, 0);
        m_mutex->unlock();
    } else {
        m_api->getToken(m_strUserId, m_strPasswd);
    }
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
