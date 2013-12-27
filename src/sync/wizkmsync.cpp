#include "wizkmsync.h"

#include <QDebug>

#include "apientry.h"
#include "token.h"

#include "../share/wizDatabase.h"

using namespace WizService;


/* ---------------------------- CWizKMSyncThead ---------------------------- */
void CWizKMSyncEvents::OnSyncProgress(int pos)
{
    qDebug() << "[OnSyncProgress] pos = " << pos;
}

HRESULT CWizKMSyncEvents::OnText(WizKMSyncProgressStatusType type, const QString& strStatus)
{
    qDebug() << "[OnText] type: " << type << " status: " << strStatus;
    TOLOG(strStatus);

    Q_EMIT messageReady(strStatus);
    return 0;
}

void CWizKMSyncEvents::SetDatabaseCount(int count)
{
    qDebug() << "[SetDatabaseCount] count = " << count;
}

void CWizKMSyncEvents::SetCurrentDatabase(int index)
{
    qDebug() << "[SetCurrentDatabase] index = " << index;
}

void CWizKMSyncEvents::OnTrafficLimit(IWizSyncableDatabase* pDatabase)
{
}

void CWizKMSyncEvents::OnStorageLimit(IWizSyncableDatabase* pDatabase)
{

}

void CWizKMSyncEvents::OnUploadDocument(const QString& strDocumentGUID, bool bDone)
{
    qDebug() << "[SetCurrentDatabase] guid: " << strDocumentGUID;
}

void CWizKMSyncEvents::OnBeginKb(const QString& strKbGUID)
{
    qDebug() << "[OnBeginKb] kb_guid: " << strKbGUID;
}

void CWizKMSyncEvents::OnEndKb(const QString& strKbGUID)
{
    qDebug() << "[OnEndKb] kb_guid: " << strKbGUID;
}


/* ---------------------------- CWizKMSyncThead ---------------------------- */
CWizKMSyncThread::CWizKMSyncThread(CWizDatabase& db, QObject* parent)
    : QThread(parent)
    , m_db(db)
    , m_server(::WizService::ApiEntry::syncUrl())
    , m_bNeedSyncAll(true)
    , m_bBusy(false)
    , m_bNeedAccquireToken(false)
{
    m_tLastSyncAll = QDateTime::currentDateTime();
    connect(this, SIGNAL(finished()), SLOT(on_syncFinished()));
}

bool CWizKMSyncThread::checkTokenCore()
{
    if (!m_server.m_userInfo.strToken.isEmpty())
    {
        if (m_server.keepAlive(m_server.m_userInfo.strToken))
            return true;
    }

    if (server.Login(m_db.getUserId(), m_db.GetPassword()))
        return true;
    //
    return false;
}
bool CWizKMSyncThread::checkToken()
{
    if (checkTokenCore())
    {
        onLogin();  //send messages
        return true;
    }
    return false;
}

void CWizKMSyncThread::OnLogin()
{
    //TODO: 发送消息
    //然后清空消息队列
    //send message for accquire token
    Q_EMIT tokenAcquired(Token::lastErrorCode(), Token::lastErrorMessage());
    disconnect(this, IGNAL(tokenAcquired(QString)));
}

void CWizKMSyncThread::run()
{
    while (1)
    {
        if (m_pEvents && m_pEvents->IsStop())
        {
            break;
        }
        //
        //
        if (needSyncAll())
        {
            syncAll();
            m_tLastSyncAll = QDateTime::currentDateTime();
        }
        else if (needQuickSync())
        {
            quickSync();
        }
        else if (needAccquireToken())
        {
            accquireToken();
        }
        else
        {
            onIdle();
        }
    }
}
bool CWizKMSyncThread::needSyncAll()
{
    if (m_bNeedSyncAll)
        return true;
    //
    QDateTime tNow = QDateTime::currentDateTime();
    if (tNow.toTime_t() - m_tLastSyncAll.toTime_t() > 15 * 60)
    {
        m_bNeedSyncAll = true;
    }
    //
    return m_bNeedSyncAll;
}
bool CWizKMSyncThread::needQuickSync()
{
    return false;
}
bool CWizKMSyncThread::needAccquireToken()
{
    return m_bNeedAccquireToken;
}
bool CWizKMSyncThread::onIdle()
{
    sleep(1000);    //sleep 1 seconds
    return true;
}
bool CWizKMSyncThread::syncAll()
{
    class CBusyHelper
    {
        CBusyHelper(bool& b)
        {
            b = true;
        }
        ~CBusyHelper()
        {
            b = false;
        }
    };
    //    //
    m_bNeedSyncAll = false;
    //
    if (!checkToken())
        return false;
    //
    CBusyHelper busy(m_bBusy);
    Q_UNUSED(busy);
    //
    syncUserCert();

    if (!m_pEvents) {
        m_pEvents = new CWizKMSyncEvents();
        connect(m_pEvents, SIGNAL(messageReady(const QString&)), SIGNAL(processLog(const QString&)));
    }

    m_pEvents->SetLastErrorCode(0);
    ::WizSyncDatabase(m_info, m_pEvents, &m_db, true, true);
}
bool CWizKMSyncThread::quickSync()
{
    if (!checkToken())
        return false;
    //
    return true;
}

bool CWizKMSyncThread::accquireToken()
{
    return checkToken();
}

void CWizKMSyncThread::acquireToken(QObject* pObject, const QMethod& slot)
{
    //TODO: 在这里处理请求token的部分,连接事件
    //
    connect(this, SIGNAL(tokenAcquired(QString)), pObject, slot, Qt::QueuedConnection);
    //
    //如果当前正在同步,那么可以直接发送当前的token.
    if (m_bBusy)    //token is valid
    {
        onLogin();
    }
    else
    {
        //否则设置获取token,下个循环的时候,将会自动获取token并发送消息.
        m_bNeedAccquireToken = true;
    }
}

void CWizKMSyncThread::startSync()
{
    if (isRunning())
        return;

    start();
    //connect(Token::instance(), SIGNAL(tokenAcquired(QString)), SLOT(onTokenAcquired(QString)), Qt::QueuedConnection);
    //Token::requestToken();
}

void CWizKMSyncThread::onTokenAcquired(const QString& strToken)
{
    Token::instance()->disconnect(this);

    if (strToken.isEmpty()) {
        Q_EMIT syncFinished(Token::lastErrorCode(), Token::lastErrorMessage());
        return;
    }

    m_info = Token::info();
    start();
}

void CWizKMSyncThread::stopSync()
{
    if (isRunning() && m_pEvents) {
        m_pEvents->SetStop(true);
    }
}

void CWizKMSyncThread::on_syncFinished()
{
    m_pEvents->deleteLater();

    Q_EMIT syncFinished(m_pEvents->GetLastErrorCode(), "");
}

void CWizKMSyncThread::syncUserCert()
{
    QString strN, stre, strd, strHint;

    CWizKMAccountsServer serser(WizService::ApiEntry::syncUrl());
    if (serser.GetCert(m_db.GetUserId(), m_db.GetPassword(), strN, stre, strd, strHint)) {
        m_db.SetUserCert(strN, stre, strd, strHint);
    }
}
