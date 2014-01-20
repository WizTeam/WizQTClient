#include "token.h"
#include "token_p.h"

#include <QMutexLocker>
#include <QString>
#include <QDebug>

#include "wizkmxmlrpc.h"
#include "asyncapi.h"
#include "apientry.h"

using namespace WizService;
using namespace WizService::Internal;

// use 15 minutes locally, server use 20 minutes
#define TOKEN_TIMEOUT_INTERVAL 60*15

TokenPrivate::TokenPrivate(Token* token)
    : q(token)
    , m_api(new AsyncApi())
    , m_mutex(new QMutex())
    , m_bProcess(false)
{
    connect(m_api, SIGNAL(loginFinished(const WIZUSERINFO&)), SLOT(onLoginFinished(const WIZUSERINFO&)));
    connect(m_api, SIGNAL(getTokenFinished(QString)), SLOT(onGetTokenFinished(QString)));
    connect(m_api, SIGNAL(keepAliveFinished(bool)), SLOT(onKeepAliveFinished(bool)));
}

TokenPrivate::~TokenPrivate()
{
    delete m_mutex;
}

QString TokenPrivate::token()
{
    Q_ASSERT(!m_strUserId.isEmpty() && !m_strPasswd.isEmpty());

    if (m_bProcess)
        return m_info.strToken;

    CWizKMAccountsServer asServer(ApiEntry::syncUrl());
    if (m_info.strToken.isEmpty()) {
        if (asServer.Login(m_strUserId, m_strPasswd)) {
            m_info = asServer.GetUserInfo();
            m_info.tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
            return m_info.strToken;
        } else {
            return QString();
        }
    }

    if (m_info.tTokenExpried >= QDateTime::currentDateTime()) {
        return m_info.strToken;
    } else {
        WIZUSERINFO info;
        info.strToken = m_info.strToken;
        info.strKbGUID = m_info.strKbGUID;
        asServer.SetUserInfo(info);

        if (asServer.KeepAlive(m_info.strToken)) {
            m_info.tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
            return m_info.strToken;
        } else {
            QString strToken;
            if (asServer.GetToken(m_strUserId, m_strPasswd, strToken)) {
                m_info.strToken = strToken;
                m_info.tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
                return m_info.strToken;
            } else {
                return QString();
            }
        }
    }

    Q_ASSERT(0);
}

void TokenPrivate::requestToken()
{
    Q_ASSERT(!m_strUserId.isEmpty() && !m_strPasswd.isEmpty());

    QMutexLocker locker(m_mutex);

    // return the old one even it will be failed if still waiting for reply
    if (m_bProcess) {
        Q_EMIT q->tokenAcquired(m_info.strToken);
        return;
    }

    if (m_info.strToken.isEmpty()) {
        m_bProcess = true;
        m_api->login(m_strUserId, m_strPasswd);
        return;
    }

    if (m_info.tTokenExpried >= QDateTime::currentDateTime()) {
        Q_EMIT q->tokenAcquired(m_info.strToken);
    } else {
        m_bProcess = true;
        m_api->keepAlive(m_info.strToken, m_info.strKbGUID);
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

const WIZUSERINFO& TokenPrivate::info()
{
    return m_info;
}

int TokenPrivate::lastErrorCode() const
{
    return m_api->lastErrorCode();
}

QString TokenPrivate::lastErrorMessage() const
{
    return m_api->lastErrorMessage();
}

void TokenPrivate::onLoginFinished(const WIZUSERINFO &info)
{
    qDebug() << "[Token]: Login Done...";

    if (info.strToken.isEmpty()) {
        Q_EMIT q->tokenAcquired(0);
    } else {
        m_info = info;
        m_info.tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
        Q_EMIT q->tokenAcquired(info.strToken);
    }

    m_bProcess = false;
}

void TokenPrivate::onGetTokenFinished(const QString& strToken)
{
    qDebug() << "[Token]: GetToken Done...";

    if (strToken.isEmpty()) {
        Q_EMIT q->tokenAcquired(0);
    } else {
        m_info.strToken = strToken;
        m_info.tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
        Q_EMIT q->tokenAcquired(strToken);
    }

    m_bProcess = false;
}

void TokenPrivate::onKeepAliveFinished(bool bOk)
{
    qDebug() << "[Token]: KeepAlive Done...";

    if (bOk) {
        m_info.tTokenExpried = QDateTime::currentDateTime().addSecs(TOKEN_TIMEOUT_INTERVAL);
        Q_EMIT q->tokenAcquired(m_info.strToken);
        m_bProcess = false;
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

const WIZUSERINFO& Token::info()
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
