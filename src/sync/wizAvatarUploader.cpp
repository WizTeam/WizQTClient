#include "wizAvatarUploader.h"

#include "wizdef.h"
#include "share/wizApiEntry.h"
#include "sync/wizkmxmlrpc.h"
#include "share/wizDatabaseManager.h"

#include <QtNetwork>
#include <QImage>


CWizAvatarUploader::CWizAvatarUploader(CWizExplorerApp& app, QObject* parent)
    : QObject(parent)
    , m_db(app.databaseManager().db())
    , m_net(new QNetworkAccessManager(this))
{
    connect(m_net, SIGNAL(finished(QNetworkReply*)), SLOT(on_upload_finished(QNetworkReply*)));
}

QString CWizAvatarUploader::convert2Avatar(const QString& strFileName)
{
    QImage image(strFileName);
    if (image.isNull())
        return NULL;

    // FIXME: avoid image is too large
    if (image.width() > 100 || image.height() > 100) {
        image = image.scaled(100, 100, Qt::IgnoreAspectRatio);
    }

    QString strTempAvatar = ::WizGlobal()->GetTempPath() + QString::number(qrand()) + ".png";
    if (!image.save(strTempAvatar))
        return NULL;

    return strTempAvatar;
}

void CWizAvatarUploader::upload(const QString& strFileName)
{
    m_strFileName = convert2Avatar(strFileName);
    if (m_strFileName.isEmpty())
        return;

    CWizApiEntry* apiEntry = new CWizApiEntry(this);
    connect(apiEntry, SIGNAL(acquireEntryFinished(const QString&)), SLOT(on_acquireEntry_finished(const QString&)));
    apiEntry->getAvatarUploadUrl();
}

void CWizAvatarUploader::on_acquireEntry_finished(const QString& strUrl)
{
    sender()->deleteLater();

    if (strUrl.isEmpty()) {
        qDebug() << "[CWizAvatarUploader] Failed to acquire avatar upload url";
        return;
    }

    upload_impl(strUrl, m_strFileName);
}

void CWizAvatarUploader::upload_impl(const QString& strUrl, const QString& strFileName)
{
    qDebug() << "[CWizAvatarUploader] Filename: " << strFileName << " UploadUrl: " << strUrl;

    QFile file(strFileName);
    if (!file.open(QIODevice::ReadOnly))
        return;
    QByteArray data = file.readAll();
    file.close();

    QString strToken;
    CWizKMAccountsServer server(::WizKMGetAccountsServerURL(true));
    if (!server.GetToken(m_db.GetUserId(), m_db.GetPassword(), strToken)) {
        qDebug() << "[CWizAvatarUploader] Failed to get token";
        return;
    }

    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain; charset=utf-8"));
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"token\""));
    textPart.setBody(strToken.toUtf8());

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"avatar\"; filename=\""+ strFileName +"\""));
    imagePart.setBody(data);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->append(textPart);
    multiPart->append(imagePart);

    QNetworkRequest request(strUrl);
    QNetworkReply *reply = m_net->post(request, multiPart);
    multiPart->setParent(reply); // delete the multiPart with the reply
}

void CWizAvatarUploader::on_upload_finished(QNetworkReply* reply)
{
    reply->deleteLater();

    if (int e = reply->error()) {
        Q_EMIT uploaded(false);
        return;
    }

    if (!reply->open(QIODevice::ReadOnly)) {
        Q_EMIT uploaded(false);
        return;
    }

    QString strReply = reply->readAll();
    reply->close();

    qDebug() << "[CWizAvatarUploader] reply:" << strReply;

    // FIXME
    if (!strReply.contains("200")) {
        Q_EMIT uploaded(false);
        return;
    }

    Q_EMIT uploaded(true);
}
