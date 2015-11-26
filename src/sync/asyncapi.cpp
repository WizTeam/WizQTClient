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
#include "share/wizEventLoop.h"
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
    QtConcurrent::run(this, &AsyncApi::login_impl, strUserId, strPasswd);
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
    QtConcurrent::run(this, &AsyncApi::getToken_impl, strUserId, strPasswd);
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
    QtConcurrent::run(this, &AsyncApi::keepAlive_impl, strToken, strKbGUID);
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
    QtConcurrent::run(this, &AsyncApi::registerAccount_impl, strUserId, strPasswd, strInviteCode, strCaptchaID, strCaptcha);
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


//
//使用QtConcurrent的时候,如果线程没有返回,例如网络阻塞等,会导致应用程序无法退出
//因此使用异步api处理
//QtConcurrent::run(this, &AsyncApi::getCommentsCount_impl, strUrl);
//
//

void AsyncApi::getCommentsCount(const QString& strUrl)
{
    QNetworkReply* reply = m_networkManager->get(QNetworkRequest(strUrl));
    
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), this, SLOT(on_comments_finished()));
    loop.exec();
}

void AsyncApi::on_comments_finished()
{
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    //

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


void AsyncApi::setMessageReadStatus(const QString& ids, bool bRead)
{
    QtConcurrent::run(this, &AsyncApi::setMessageReadStatus_impl, ids, bRead);
}

void AsyncApi::setMessageDeleteStatus(const QString& ids, bool bDelete)
{
    QtConcurrent::run(this, &AsyncApi::setMessageDeleteStatus_impl, ids, bDelete);
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
