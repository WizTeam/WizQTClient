#include "asyncapi.h"

#include <QtGlobal>
#include <QNetworkAccessManager>
#include <QEventLoop>

#include <rapidjson/document.h>
#include "share/wizEventLoop.h"
#include "share/wizthreads.h"
#include "apientry.h"
#include "wizKMServer.h"
#include "token.h"

using namespace WizService;


AsyncApi::AsyncApi(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    qRegisterMetaType<WIZUSERINFO>("WIZUSERINFO");
}

AsyncApi::~AsyncApi()
{
}

void AsyncApi::login(const QString& strUserId, const QString& strPasswd)
{
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        login_impl(strUserId, strPasswd);
    });
}

bool AsyncApi::login_impl(const QString& strUserId, const QString& strPasswd)
{
    CWizKMAccountsServer asServer(CommonApiEntry::syncUrl());
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
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        getToken_impl(strUserId, strPasswd);
    });
}

bool AsyncApi::getToken_impl(const QString& strUserId, const QString& strPasswd)
{
    QString strToken;

    CWizKMAccountsServer asServer(CommonApiEntry::syncUrl());
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
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        keepAlive_impl(strToken, strKbGUID);
    });
}

bool AsyncApi::keepAlive_impl(const QString& strToken, const QString& strKbGUID)
{
    CWizKMAccountsServer asServer(CommonApiEntry::syncUrl());

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

void AsyncApi::registerAccount(const QString& strUserId, const QString& strPasswd,
                               const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha)
{
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        registerAccount_impl(strUserId, strPasswd, strInviteCode, strCaptchaID, strCaptcha);
    });
}

bool AsyncApi::registerAccount_impl(const QString& strUserId, const QString& strPasswd,
                                    const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha)
{
    CWizKMAccountsServer aServer(CommonApiEntry::syncUrl());

    bool ret = aServer.CreateAccount(strUserId, strPasswd, strInviteCode, strCaptchaID, strCaptcha);
    if (!ret) {
        m_nErrorCode = aServer.GetLastErrorCode();
        m_strErrorMessage = aServer.GetLastErrorMessage();
    }

    Q_EMIT registerAccountFinished(ret);
    return ret;
}

void AsyncApi::setMessageReadStatus(const QString& ids, bool bRead)
{
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        setMessageReadStatus_impl(ids, bRead);
    });
}

void AsyncApi::setMessageDeleteStatus(const QString& ids, bool bDelete)
{
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        setMessageDeleteStatus_impl(ids, bDelete);
    });
}

void AsyncApi::setMessageReadStatus_impl(const QString& ids, bool bRead)
{
    QString strToken = Token::token();
    qDebug() << "set message read status, strken:" << strToken;

    if (strToken.isEmpty()) {
        return;
    }

    CWizKMAccountsServer aServer(CommonApiEntry::syncUrl());

    WIZUSERINFO info = Token::info();
    info.strToken = strToken;
    aServer.SetUserInfo(info);

    bool ret = aServer.SetMessageReadStatus(ids, bRead);
    qDebug() << "set message read status : " << ret;
    if (!ret)
    {
        m_nErrorCode = aServer.GetLastErrorCode();
        m_strErrorMessage = aServer.GetLastErrorMessage();
    }
    else
    {
        emit uploadMessageReadStatusFinished(ids);
    }
}

void AsyncApi::setMessageDeleteStatus_impl(const QString& ids, bool bDelete)
{
    QString strToken = Token::token();
    if (strToken.isEmpty()) {
        return;
    }

    CWizKMAccountsServer aServer(CommonApiEntry::syncUrl());

    WIZUSERINFO info = Token::info();
    info.strToken = strToken;
    aServer.SetUserInfo(info);

    bool ret = aServer.SetMessageDeleteStatus(ids, bDelete);
    if (ret)
    {
        qDebug() << "[MessageStatus]Upload message delete status OK";
        emit uploadMessageDeleteStatusFinished(ids);
    }
    else
    {
        m_nErrorCode = aServer.GetLastErrorCode();
        m_strErrorMessage = aServer.GetLastErrorMessage();
        qDebug() << "[MessageStatus]Upload message delete status error :  " << m_nErrorCode << m_strErrorMessage;
    }
    //

}
