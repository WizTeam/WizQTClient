#include "WizAsyncApi.h"

#include <QtGlobal>
#include <QNetworkAccessManager>
#include <QEventLoop>

#include "share/WizEventLoop.h"
#include "share/WizThreads.h"
#include "WizApiEntry.h"
#include "WizKMServer.h"
#include "WizToken.h"


WizAsyncApi::WizAsyncApi(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    qRegisterMetaType<WIZUSERINFO>("WIZUSERINFO");
}

WizAsyncApi::~WizAsyncApi()
{
}

void WizAsyncApi::registerAccount(const QString& strUserId, const QString& strPasswd,
                               const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha)
{
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        registerAccount_impl(strUserId, strPasswd, strInviteCode, strCaptchaID, strCaptcha);
    });
}

bool WizAsyncApi::registerAccount_impl(const QString& strUserId, const QString& strPasswd,
                                    const QString& strInviteCode, const QString& strCaptchaID, const QString& strCaptcha)
{
    WizKMAccountsServer aServer;

    bool ret = aServer.createAccount(strUserId, strPasswd, strInviteCode, strCaptchaID, strCaptcha);
    if (!ret) {
        m_nErrorCode = aServer.getLastErrorCode();
        m_strErrorMessage = aServer.getLastErrorMessage();
    }

    Q_EMIT registerAccountFinished(ret);
    return ret;
}

void WizAsyncApi::setMessageReadStatus(const QString& ids, bool bRead)
{
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        setMessageReadStatus_impl(ids, bRead);
    });
}

void WizAsyncApi::setMessageDeleteStatus(const QString& ids, bool bDelete)
{
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        setMessageDeleteStatus_impl(ids, bDelete);
    });
}

void WizAsyncApi::setMessageReadStatus_impl(const QString& ids, bool bRead)
{
    QString strToken = WizToken::token();
    qDebug() << "set message read status, strken:" << strToken;

    if (strToken.isEmpty()) {
        return;
    }

    WizKMAccountsServer aServer;

    WIZUSERINFO info = WizToken::userInfo();
    info.strToken = strToken;
    aServer.setUserInfo(info);

    bool ret = aServer.setMessageReadStatus(ids, bRead);
    qDebug() << "set message read status : " << ret;
    if (!ret)
    {
        m_nErrorCode = aServer.getLastErrorCode();
        m_strErrorMessage = aServer.getLastErrorMessage();
    }
    else
    {
        emit uploadMessageReadStatusFinished(ids);
    }
}

void WizAsyncApi::setMessageDeleteStatus_impl(const QString& ids, bool bDelete)
{
    QString strToken = WizToken::token();
    if (strToken.isEmpty()) {
        return;
    }

    WizKMAccountsServer aServer;

    WIZUSERINFO info = WizToken::userInfo();
    info.strToken = strToken;
    aServer.setUserInfo(info);

    bool ret = aServer.setMessageDeleteStatus(ids, bDelete);
    if (ret)
    {
        qDebug() << "[MessageStatus]Upload message delete status OK";
        emit uploadMessageDeleteStatusFinished(ids);
    }
    else
    {
        m_nErrorCode = aServer.getLastErrorCode();
        m_strErrorMessage = aServer.getLastErrorMessage();
        qDebug() << "[MessageStatus]Upload message delete status error :  " << m_nErrorCode << m_strErrorMessage;
    }
    //
}
