#include "wizDocumentListViewItem.h"

#include <QFile>
#include <QFileInfo>

#include "share/wizDatabase.h"
#include "share/wizThumbIndexCache.h"
#include "share/wizUserAvatar.h"

CWizDocumentListViewItem::CWizDocumentListViewItem(const WizDocumentListViewItemData& data)
    : QListWidgetItem(0, UserType)
{
    Q_ASSERT(!data.doc.strKbGUID.isEmpty());
    Q_ASSERT(!data.doc.strGUID.isEmpty());

    m_data.nType = data.nType;
    m_data.doc = data.doc;
    m_data.nMessageId = data.nMessageId;
    m_data.nReadStatus = data.nReadStatus;
    m_data.strAuthorGUID = data.strAuthorGUID;

    setText(data.doc.strTitle);
}

//CWizDocumentListViewItem::CWizDocumentListViewItem(const WIZDOCUMENTDATA& data)
//    : QListWidgetItem(0, UserType)
//    , m_data(data)
//{
//    nType = TypePrivateDocument;
//    setText(m_data.strTitle);
//}
//
//CWizDocumentListViewItem::CWizDocumentListViewItem(const WIZMESSAGEDATA& msg)
//    : QListWidgetItem(0, UserType)
//    , m_message(msg)
//{
//    nType = TypeMessage;
//    setText(msg.title);
//}

const QImage& CWizDocumentListViewItem::avatar(const CWizDatabase& db,
                                               CWizUserAvatarDownloaderHost& downloader)
{
    //Q_ASSERT(!m_data.strAuthorGUID.isEmpty());
    if (m_data.strAuthorGUID.isEmpty())
        return m_data.imgAuthorAvatar;

    if (m_data.imgAuthorAvatar.isNull()) {
        // load avatar or request downloader to download
        QString strFileName = db.GetAvatarPath() + m_data.strAuthorGUID + ".png";
        if (isAvatarNeedUpdate(strFileName)) {
            downloader.download(m_data.strAuthorGUID);
        } else {
            m_data.imgAuthorAvatar.load(strFileName);
        }
    }

    return m_data.imgAuthorAvatar;
}

bool CWizDocumentListViewItem::isAvatarNeedUpdate(const QString& strFileName)
{
    if (!QFile::exists(strFileName)) {
        return true;
    }

    QFileInfo info(strFileName);

    QDateTime tCreated = info.created();
    QDateTime tNow = QDateTime::currentDateTime();
    if (tCreated.daysTo(tNow) >= 1) { // download avatar before yesterday
        return true;
    }

    return false;
}

const WIZABSTRACT& CWizDocumentListViewItem::abstract(CWizThumbIndexCache& thumbCache)
{
    if (m_data.thumb.strKbGUID.isEmpty()) {
        // ask thumbCache to load abstract to pool
        thumbCache.load(m_data.doc.strKbGUID, m_data.doc.strGUID);
    }

    return m_data.thumb;
}

void CWizDocumentListViewItem::resetAbstract(const WIZABSTRACT& abs)
{
    m_data.thumb.strKbGUID = abs.strKbGUID;
    m_data.thumb.guid= abs.guid;
    m_data.thumb.text = abs.text;
    m_data.thumb.image = abs.image;
}

void CWizDocumentListViewItem::resetAvatar(const QString& strFileName)
{
    m_data.imgAuthorAvatar.load(strFileName);
}

const QString& CWizDocumentListViewItem::tags(CWizDatabase& db)
{
    Q_ASSERT(db.kbGUID() == m_data.doc.strKbGUID);

    if (m_data.strTags.isEmpty()) {
        m_data.strTags = db.GetDocumentTagDisplayNameText(m_data.doc.strGUID);
        //m_tags = " " + m_tags;
    }

    return m_data.strTags;
}

void CWizDocumentListViewItem::reload(CWizDatabase& db)
{
    Q_ASSERT(db.kbGUID() == m_data.doc.strKbGUID);

    m_data.thumb = WIZABSTRACT();
    m_data.strTags.clear();

    db.DocumentFromGUID(m_data.doc.strGUID, m_data.doc);
    setText(m_data.doc.strTitle);
}

bool CWizDocumentListViewItem::operator <(const QListWidgetItem &other) const
{
    const CWizDocumentListViewItem* pOther = dynamic_cast<const CWizDocumentListViewItem*>(&other);
    Q_ASSERT(pOther);

    // default compare use create time
    if (pOther->m_data.doc.tCreated == m_data.doc.tCreated) {
        return text().compare(other.text(), Qt::CaseInsensitive) < 0;
    } else {
        return pOther->m_data.doc.tCreated < m_data.doc.tCreated;
    }

    //if (!m_data.doc.strGUID.isEmpty()) {
    //    // use document type
    //    if (pOther->m_data.tCreated == m_data.tCreated) {
    //        return text().compare(other.text(), Qt::CaseInsensitive) < 0;
    //    } else {
    //        return pOther->m_data.tCreated < m_data.tCreated;
    //    }

    //} else if (!m_message.documentGUID.isEmpty()) {
    //    // use message type
    //    if (pOther->m_message.tCreated == m_message.tCreated) {
    //        return text().compare(other.text(), Qt::CaseInsensitive) < 0;
    //    } else {
    //        return pOther->m_message.tCreated < m_message.tCreated;
    //    }
    //}

    Q_ASSERT(0);
    return true;
}
