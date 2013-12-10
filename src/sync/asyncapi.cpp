#include "asyncapi.h"

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QFuture>

#include "apientry.h"
#include "wizkmxmlrpc.h"

using namespace WizService;


AsyncApi::AsyncApi(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<WIZUSERINFO>("WIZUSERINFO");
}

AsyncApi::~AsyncApi()
{
}

void AsyncApi::login(const QString& strUserId, const QString& strPasswd)
{
    QtConcurrent::run(this, &AsyncApi::login_impl, strUserId, strPasswd);
}

bool AsyncApi::login_impl(const QString& strUserId, const QString& strPasswd)
{
    CWizKMAccountsServer asServer(ApiEntry::syncUrl());
    bool ret = asServer.Login(strUserId, strPasswd);
    if (!ret) {
        m_nErrorCode = asServer.GetLastErrorCode();
        m_strErrorMessage = asServer.GetLastErrorMessage();
    }

    Q_EMIT loginFinished(asServer.GetUserInfo());
    return ret;
}

void AsyncApi::getToken(const QString& strUserId, const QString& strPasswd)
{
    QtConcurrent::run(this, &AsyncApi::getToken_impl, strUserId, strPasswd);
}

bool AsyncApi::getToken_impl(const QString& strUserId, const QString& strPasswd)
{
    QString strToken;

    CWizKMAccountsServer asServer(ApiEntry::syncUrl());
    bool ret = asServer.GetToken(strUserId, strPasswd, strToken);
    if (!ret) {
        m_nErrorCode = asServer.GetLastErrorCode();
        m_strErrorMessage = asServer.GetLastErrorMessage();
    }

    Q_EMIT getTokenFinished(strToken);
    return ret;
}

void AsyncApi::keepAlive(const QString& strToken, const QString& strKbGUID)
{
    QtConcurrent::run(this, &AsyncApi::keepAlive_impl, strToken, strKbGUID);
}

bool AsyncApi::keepAlive_impl(const QString& strToken, const QString& strKbGUID)
{
    CWizKMAccountsServer asServer(ApiEntry::syncUrl());

    WIZUSERINFO info;
    info.strToken = strToken;
    info.strKbGUID = strKbGUID;
    asServer.SetUserInfo(info);

    bool ret = asServer.KeepAlive(strToken);
    if (!ret) {
        m_nErrorCode = asServer.GetLastErrorCode();
        m_strErrorMessage = asServer.GetLastErrorMessage();
    }

    Q_EMIT keepAliveFinished(ret);
    return ret;
}
