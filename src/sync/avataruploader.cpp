#include "avataruploader.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpPart>
#include <QImage>
#include <QString>
#include <QDir>
#include <QDebug>

#include "apientry.h"
#include "token.h"

using namespace WizService;

AvatarUploader::AvatarUploader(QObject* parent)
    : QObject(parent)
    , m_net(new QNetworkAccessManager(this))
{
    connect(m_net, SIGNAL(finished(QNetworkReply*)), SLOT(onUploadFinished(QNetworkReply*)));
}

QString AvatarUploader::convert2Avatar(const QString& strFileName)
{
    QImage image(strFileName);
    if (image.isNull())
        return NULL;

    if (image.width() > 100 || image.height() > 100) {
        image = image.scaled(100, 100, Qt::IgnoreAspectRatio);
    }

    QString strTempAvatar = QDir::tempPath() + "/" + QString::number(qrand()) + ".png";
    if (!image.save(strTempAvatar))
        return NULL;

    return strTempAvatar;
}

void AvatarUploader::upload(const QString& strFileName)
{
    m_strFileName = convert2Avatar(strFileName);
    if (m_strFileName.isEmpty()) {
        qDebug() << "[avatarUploader] failed to convter image to avatar!";
        m_strError = "failed to convter image to avatar!";
        Q_EMIT uploaded(false);
        return;
    }

    m_strUrl = WizService::CommonApiEntry::avatarUploadUrl();
    if (m_strUrl.isEmpty()) {
        qDebug() << "[avatarUploader] failed to get url for uploading avatar!";
        m_strError = "failed to get url for uploading avatar!";
        Q_EMIT uploaded(false);
        return;
    }

    connect(Token::instance(), SIGNAL(tokenAcquired(QString)),
            SLOT(onTokenAcquired(QString)), Qt::QueuedConnection);
    Token::instance()->requestToken();
}

void AvatarUploader::onTokenAcquired(const QString& strToken)
{
    Token::instance()->disconnect(this);

    if (strToken.isEmpty()) {
        qDebug() << "[avatarUploader] failed to get token while upload avatar!";
        m_strError = "failed to get token while upload avatar!";
        Q_EMIT uploaded(false);
        return;
    }

    upload_impl(m_strUrl, strToken, m_strFileName);
}

void AvatarUploader::upload_impl(const QString& strUrl,
                                     const QString& strToken,
                                     const QString& strFileName)
{
    qDebug() << "[avatarUploader] File: " << strFileName << " Upload: " << strUrl;

    QFile file(strFileName);
    if (!file.open(QIODevice::ReadOnly))
        return;
    QByteArray data = file.readAll();
    file.close();

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

void AvatarUploader::onUploadFinished(QNetworkReply* reply)
{
    reply->deleteLater();

    if (reply->error()) {
        m_strError = "network error! code = " + QString(reply->error());
        Q_EMIT uploaded(false);
        return;
    }

    if (!reply->open(QIODevice::ReadOnly)) {
        m_strError = "failed to open reply!";
        Q_EMIT uploaded(false);
        return;
    }

    QString strReply = reply->readAll();
    reply->close();

    // FIXME
    if (!strReply.contains("200")) {
        m_strError = "server rejected! detail:" + strReply;
        Q_EMIT uploaded(false);
        return;
    }

    Q_EMIT uploaded(true);
}
