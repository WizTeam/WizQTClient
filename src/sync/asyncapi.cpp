#include "asyncapi.h"

#include <QtGlobal>
#if QT_VERSION > 0x050000
#include <QtConcurrent>
#else
#include <QtConcurrentRun>
#endif

#include <QNetworkAccessManager>
#include <QEventLoop>

#include <rapidjson/document.h>

#include "apientry.h"
#include "wizkmxmlrpc.h"
#include "token.h"

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

void AsyncApi::registerAccount(const QString& strUserId,
                               const QString& strPasswd,
                               const QString& strInviteCode)
{
    QtConcurrent::run(this, &AsyncApi::registerAccount_impl, strUserId, strPasswd, strInviteCode);
}

bool AsyncApi::registerAccount_impl(const QString& strUserId,
                                    const QString& strPasswd,
                                    const QString& strInviteCode)
{
    CWizKMAccountsServer aServer(ApiEntry::syncUrl());

    bool ret = aServer.CreateAccount(strUserId, strPasswd, strInviteCode);
    if (!ret) {
        m_nErrorCode = aServer.GetLastErrorCode();
        m_strErrorMessage = aServer.GetLastErrorMessage();
    }

    Q_EMIT registerAccountFinished(ret);
    return ret;
}

void AsyncApi::getCommentsCount(const QString& strUrl)
{
    QtConcurrent::run(this, &AsyncApi::getCommentsCount_impl, strUrl);
}

void AsyncApi::getCommentsCount_impl(const QString& strUrl)
{
    QNetworkAccessManager net;
    QNetworkReply* reply = net.get(QNetworkRequest(strUrl));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    int nTotalComments = 0;

    if (reply->error()) {
        qDebug() << "[AsyncApi]Failed to get comment count: " << reply->errorString();
        Q_EMIT getCommentsCountFinished(0);
        return;
    }

    QString strReply = QString::fromUtf8(reply->readAll().constData());

    // {"comment_count":0,"return_code":200,"return_message":"success"}
    // {"error":"Token invalid!","error_code":301}
    rapidjson::Document d;
    d.Parse<0>(strReply.toUtf8().constData());

    if (d.HasMember("error_code")) {
        qDebug() << "[AsyncApi]Failed to get comment count: "
                 << QString::fromUtf8(d.FindMember("error")->value.GetString())
                 << " code: " << d.FindMember("error_code")->value.GetInt();
        Q_EMIT getCommentsCountFinished(0);
        return;
    }

    if (d.HasMember("return_code")) {
        int nCode = d.FindMember("return_code")->value.GetInt();
        if (nCode != 200) {
            qDebug() << "[AsyncApi]Failed to get comment count, need 200, but return "
                     << d.FindMember("return_code")->value.GetInt();
            Q_EMIT getCommentsCountFinished(0);
            return;
        }
    }

    nTotalComments = d.FindMember("comment_count")->value.GetInt();
    Q_EMIT getCommentsCountFinished(nTotalComments);
}

void AsyncApi::setMessageStatus(const QString& ids, bool bRead)
{
    QtConcurrent::run(this, &AsyncApi::setMessageStatus_impl, ids, bRead);
}

void AsyncApi::setMessageStatus_impl(const QString& ids, bool bRead)
{
    QString strToken = Token::token();
    qDebug() << "set message status, strken:" << strToken;

    if (strToken.isEmpty()) {
        return;
    }

    CWizKMAccountsServer aServer(ApiEntry::syncUrl());

    WIZUSERINFO info = Token::info();
    info.strToken = strToken;
    aServer.SetUserInfo(info);

    bool ret = aServer.SetMessageReadStatus(ids, bRead);
    if (!ret) {
        m_nErrorCode = aServer.GetLastErrorCode();
        m_strErrorMessage = aServer.GetLastErrorMessage();
    }
}
