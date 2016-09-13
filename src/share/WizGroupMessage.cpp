#include "WizGroupMessage.h"

#include "WizDatabase.h"
#include "WizApiEntry.h"
#include "rapidjson/document.h"

WizGroupMessage::WizGroupMessage(WizDatabase& db, QObject* parent /* = 0 */)
    : m_db(db)
    , CWizApiBase(WIZ_API_URL, parent)
    , m_bMsgUploadStatus(false)
{
    m_net = new QNetworkAccessManager(this);
    connect(&m_timer, SIGNAL(timeout()), SLOT(on_timer_timeout()));

    m_timer.start(120 * 1000);   // default message fetching interval, 2 minutes
}

void WizGroupMessage::on_timer_timeout()
{
    m_timer.stop();

    // clean up
    m_arrayMsgNeedUpload.clear();
    m_listMsgUploading.clear();
    m_bizInfo.clear();

    // start login
    QString strUserId = m_db.getUserId();
    QString strPasswd = m_db.getPassword();
    callClientLogin(strUserId, strPasswd);
}

void WizGroupMessage::onClientLogin(const WIZUSERINFO& userInfo)
{
    qDebug() << "\n[Message Sync]logined...";

    m_strUserGUID = userInfo.strUserGUID;
    uploadMessages();
}

void WizGroupMessage::uploadMessages()
{
    if (!m_db.getModifiedMessages(m_arrayMsgNeedUpload)) {
        qDebug() << "[Message Sync]failed: unable to get modified messages";
        syncEnd();
        return;
    }

    uploadMessageNext();
}

void WizGroupMessage::uploadMessageNext()
{
    if (!m_arrayMsgNeedUpload.size()) {
        uploadMessageEnd();
        return;
    }

    // actually, we don't have to care current status cause of only two status.
    m_bMsgUploadStatus = !m_bMsgUploadStatus;

    CWizMessageDataArray::iterator it;
    for (it = m_arrayMsgNeedUpload.begin(); it != m_arrayMsgNeedUpload.end();) {
        WIZMESSAGEDATA& msg = *it;

        if (msg.nReadStatus == m_bMsgUploadStatus) {
            m_listMsgUploading.push_back(msg.nId);
            it = m_arrayMsgNeedUpload.erase(it);
        } else {
            it++;
        }
    }

    if (!m_listMsgUploading.size()) {
        uploadMessageNext();
        return;
    }

    qDebug() << "[Message Sync]upload messages, total: "
             << m_listMsgUploading.size() << " status: " << m_bMsgUploadStatus;

    callSetMessageStatus(m_listMsgUploading, m_bMsgUploadStatus);
}

void WizGroupMessage::onSetMessageStatus()
{
    QList<qint64>::iterator it;
    for (it = m_listMsgUploading.begin(); it != m_listMsgUploading.end();) {
        QString strId = QString::number(*it);
        m_db.modifyObjectVersion(strId, WIZMESSAGEDATA::objectName(), 0);

        it = m_listMsgUploading.erase(it);
    }

    uploadMessageNext();
}

void WizGroupMessage::uploadMessageEnd()
{
    // query message version from light-weight api
    acquireMessageVersionEntry();
}

void WizGroupMessage::acquireMessageVersionEntry()
{
    if (!m_strVersionRequestUrl.isEmpty()) {
        queryMessageVersion();
        return;
    }

    qDebug() << "[Message Sync]message version entry is empty, acquire entry...";

    // 2.1 if version request url not exist, acquire from CWizApiEntry
    CWizApiEntry* entry = new CWizApiEntry(this);
    connect(entry, SIGNAL(acquireEntryFinished(const QString&)),
            SLOT(on_acquireMessageVersionEntry_finished(const QString&)));
    entry->getMessageUrl();
}

void WizGroupMessage::on_acquireMessageVersionEntry_finished(const QString& strReply)
{
    sender()->deleteLater();

    if (strReply.isEmpty()) {
        TOLOG("[Message Sync]failed: unable to acquire message version entry!");
        syncEnd();
        return;
    }

    m_strVersionRequestUrl = strReply;

    qDebug() << "[Message Sync]acquire entry finished, url: " << strReply;

    // 2.2 chain back, query version from server
    queryMessageVersion();
}

void WizGroupMessage::queryMessageVersion()
{
    Q_ASSERT(!m_strVersionRequestUrl.isEmpty() && !m_strUserGUID.isEmpty());

    qDebug() << "[Message Sync]query remote message version...";

    // remote return: http://message.wiz.cn/wizmessage/messages/version?user_guid={userGuid}
    // just do substitution
    QString requestUrl = m_strVersionRequestUrl;
    requestUrl.replace(QRegExp("\\{.*\\}"), m_strUserGUID);

    QNetworkReply* reply = m_net->get(QNetworkRequest(requestUrl));
    connect(reply, SIGNAL(finished()), SLOT(on_queryMessageVersion_finished()));
}

void WizGroupMessage::on_queryMessageVersion_finished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(sender());

    if (reply->error()) {
        reply->deleteLater();
        syncEnd();
        return;
    }

    // return: "{"result": version,"return_code":200,"return_message":"success"}"
    QString strReply = reply->readAll();
    reply->deleteLater();

    qDebug() << "[Message Sync]query version finished, reply: " << strReply;

    rapidjson::Document document;
    document.Parse<0>(strReply.toUtf8().constData());

    if (!document.IsObject()) {
        TOLOG("Error occured when try to parse json of message version");
        syncEnd();
        return;
    }

    QString nCode;
    if (document["return_code"].IsString()) {
        nCode = document["return_code"].getString();
    } else {
        nCode = QString::number(document["return_code"].getInt());
    }

    QString strMsg = document["return_message"].getString();
    if (nCode != WIZAPI_RETURN_SUCCESS) {
        TOLOG2("Error occured when get message version, code: %1, message: %2",
               nCode, strMsg);
        syncEnd();
        return;
    }

    qint64 nVersionRemote = document["result"].getInt64();
    qint64 nVersionLocal = m_db.getObjectVersion(WIZMESSAGEDATA::objectName());

    qDebug() << "[Message Sync]local message version: " << nVersionLocal;

    // 3. compare version, determine fetch needed or not
    if (nVersionLocal < nVersionRemote) {
        callGetMessages(nVersionLocal);
    } else {
        syncEnd();
    }
}

void WizGroupMessage::onGetMessages(const CWizMessageDataArray& messages)
{
    qDebug() << "[Message Sync]fetch message finished, total: " << messages.size();

    // update messages info
    m_db.updateMessages(messages);

    // 4. fetch user list
    fetchBizUsers();
}

void WizGroupMessage::fetchBizUsers()
{
    // biz group info is set when call getGroupKbList api
    if (!m_db.GetBizGroupInfo(m_bizInfo)) {
        qDebug() << "[Message Sync]failed: unable to get biz info!";
        TOLOG("[Message Sync]failed: unable to get biz info!");
        syncEnd();
        return;
    }

    // no biz users list have to fetch
    if (m_bizInfo.empty()) {
        syncEnd();
        return;
    }

    // 5. fetch users by bizGUID
    fetchBizUsersNext();
}

void WizGroupMessage::fetchBizUsersNext()
{
    QMap<QString, QString>::const_iterator it = m_bizInfo.begin();
    QString bizGUID = it.key();

    qDebug() << "[Message Sync]fetch biz users, guid: " << bizGUID;

    callGetBizUsers(bizGUID);
}

void WizGroupMessage::onGetBizUsers(const QString& strJsonUsers)
{
    CWizBizUserDataArray arrayUser;

    if (!loadBizUsersFromJson(strJsonUsers, arrayUser)) {
        qDebug() << "[Message Sync]failed: unable to load users from json!";
        syncEnd();
        return;
    }

    if (!m_db.updateBizUsers(arrayUser)) {
        qDebug() << "[Message Sync]failed: unable to update users!";
        syncEnd();
        return;
    }

    // save unique users for next fetching user avatar
//    CWizBizUserDataArray::const_iterator it;
//    for (it = arrayUser.begin(); it != arrayUser.end(); it++) {
//        const WIZBIZUSER& user = *it;
//        if (!m_strListUserGUID.contains(user.userGUID))
//            m_strListUserGUID.append(user.userGUID);
//    }

    qDebug() << "[Message Sync]fetch biz users finished, total: "
             << arrayUser.size();

    QMap<QString, QString>::const_iterator itMap = m_bizInfo.begin();
    m_bizInfo.remove(itMap.key());

    if (m_bizInfo.empty()) {
        fetchBizUsersEnd();
        return;
    }

    fetchBizUsersNext();
}

bool WizGroupMessage::loadBizUsersFromJson(const QString& strJsonUsers,
                                            CWizBizUserDataArray& arrayUser)
{
    // QString assumes Lantin-1 when convert to and from const char* and QByteArrays
    // set to UTF-8 as default converting to avoid messy code
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    rapidjson::Document document;
    document.Parse<0>(strJsonUsers.toUtf8().constData());

    if (!document.IsArray()) {
        TOLOG("Error occured when try to parse json of biz users");
        return false;
    }

    for (rapidjson::SizeType i = 0; i < document.Size(); i++) {
        const rapidjson::Value& u = document[i];
        if (!u.IsObject()) {
            TOLOG("Error occured when parse json of biz users");
            return false;
        }

        WIZBIZUSER user;
        user.alias = u["alias"].getString();
        user.pinyin = u["pinyin"].getString();
        user.userGUID = u["user_guid"].getString();
        user.userId = u["user_id"].getString();

        QMap<QString, QString>::const_iterator it = m_bizInfo.begin();
        user.bizGUID = it.key();

        arrayUser.push_back(user);
    }

    return true;
}

void WizGroupMessage::fetchBizUsersEnd()
{
    syncEnd();
    //acquireUserAvatarEntry();
}

void WizGroupMessage::syncEnd()
{
    m_timer.start();
}

void WizGroupMessage::onXmlRpcError(const QString& strMethodName,
                                     WizXmlRpcError err,
                                     int errorCode,
                                     const QString& errorMessage)
{
    CWizApiBase::onXmlRpcError(strMethodName, err, errorCode, errorMessage);
}
