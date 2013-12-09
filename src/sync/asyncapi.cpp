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
    QFuture<bool> future = QtConcurrent::run(this, &AsyncApi::login_impl, strUserId, strPasswd);
    //QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>();
    //watcher->setFuture(future);
    //connect(watcher, SIGNAL(finished()), SLOT(on_getToken_finished()));
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
    QFuture<bool> future = QtConcurrent::run(this, &AsyncApi::getToken_impl, strUserId, strPasswd);
    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>();
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()), SLOT(on_getToken_finished()));
}

bool AsyncApi::getToken_impl(const QString& strUserId, const QString& strPasswd)
{
    CWizKMAccountsServer asServer(ApiEntry::syncUrl());
    bool ret = asServer.GetToken(strUserId, strPasswd, m_strToken);
    if (!ret) {
        m_nErrorCode = asServer.GetLastErrorCode();
        m_strErrorMessage = asServer.GetLastErrorMessage();
    }

    return ret;
}

void AsyncApi::on_getToken_finished()
{
    QFutureWatcher<bool>* watcher = dynamic_cast<QFutureWatcher<bool>*>(sender());
    watcher->deleteLater();

    if (watcher->result()) {
        Q_EMIT getTokenFinished(m_strToken, 0);
    } else {
        Q_EMIT getTokenFinished(0, m_strErrorMessage);
    }
}

void AsyncApi::keepAlive(const QString& strToken)
{
    QFuture<bool> future = QtConcurrent::run(this, &AsyncApi::keepAlive_impl, strToken);
    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>();
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()), SLOT(on_keepAlive_finished()));
}

bool AsyncApi::keepAlive_impl(const QString& strToken)
{
    CWizKMAccountsServer asServer(ApiEntry::syncUrl());
    bool ret = asServer.KeepAlive(strToken);
    if (!ret) {
        m_nErrorCode = asServer.GetLastErrorCode();
        m_strErrorMessage = asServer.GetLastErrorMessage();
    }

    return ret;
}

void AsyncApi::on_keepAlive_finished()
{
    QFutureWatcher<bool>* watcher = dynamic_cast<QFutureWatcher<bool>*>(sender());
    watcher->deleteLater();

    if (watcher->result()) {
        Q_EMIT keepAliveFinished(true, 0);
    } else {
        Q_EMIT keepAliveFinished(false, m_strErrorMessage);
    }
}
