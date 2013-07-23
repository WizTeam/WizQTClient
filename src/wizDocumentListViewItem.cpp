#include "wizDocumentListViewItem.h"

#include <QFile>
#include <QFileInfo>

#include "share/wizDatabase.h"
#include "share/wizThumbIndexCache.h"
#include "share/wizUserAvatar.h"


CWizDocumentListViewItem::CWizDocumentListViewItem(const WIZDOCUMENTDATA& data)
    : QListWidgetItem(0, UserType)
    , m_data(data)
{
    nType = PrivateDocument;
    setText(m_data.strTitle);
}

CWizDocumentListViewItem::CWizDocumentListViewItem(const WIZMESSAGEDATA& msg)
    : QListWidgetItem(0, UserType)
    , m_message(msg)
{
    nType = MessageDocument;
    setText(msg.title);
}

const QImage& CWizDocumentListViewItem::avatar(const CWizDatabase& db,
                                               CWizUserAvatarDownloaderHost& downloader)
{
    Q_ASSERT(m_message.nId);

    if (m_imgAvatar.isNull()) {
        // load avatar or request downloader to download
        QString strFileName = db.GetAvatarPath() + m_message.senderGUID + ".png";
        if (isAvatarNeedUpdate(strFileName)) {
            downloader.download(m_message.senderGUID);
        } else {
            resetAvatar(strFileName);
        }
    }

    return m_imgAvatar;
}

bool CWizDocumentListViewItem::isAvatarNeedUpdate(const QString& strFileName)
{
    if (!QFile::exists(strFileName)) {
        return true;
    }

    QFileInfo info(strFileName);

    QDateTime tCreated = info.created();
    QDateTime tNow = QDateTime::currentDateTime();
    if (tCreated.daysTo(tNow) >= 1) {
        return true;
    }

    return false;
}

const WIZABSTRACT& CWizDocumentListViewItem::abstract(CWizThumbIndexCache& thumbCache)
{
    // ask thumbCache to load abstract to pool
    if (m_abstract.strKbGUID.isEmpty()) {
        if (!m_data.strKbGUID.isEmpty() && !m_data.strGUID.isEmpty()) {
            thumbCache.load(m_data.strKbGUID, m_data.strGUID);
        } else if (!m_message.kbGUID.isEmpty() && !m_message.documentGUID.isEmpty()) {
            thumbCache.load(m_message.kbGUID, m_message.documentGUID);
        } else {
            Q_ASSERT(0);
        }
    }

    // just return it
    return m_abstract;
}

void CWizDocumentListViewItem::resetAbstract(const WIZABSTRACT& abs)
{
    // called by CWizDocumentListView when thumbCache pool is ready for reading
    m_abstract.strKbGUID = abs.strKbGUID;
    m_abstract.guid= abs.guid;
    m_abstract.text = abs.text;
    m_abstract.image = abs.image;
}

void CWizDocumentListViewItem::resetAvatar(const QString& strFileName)
{
    m_imgAvatar.load(strFileName);
}

const QString& CWizDocumentListViewItem::tags(CWizDatabase& db)
{
    if (m_tags.isEmpty()) {
        m_tags = db.GetDocumentTagDisplayNameText(m_data.strGUID);
        m_tags = " " + m_tags;
    }

    return m_tags;
}

void CWizDocumentListViewItem::reload(CWizDatabase& db)
{
    m_abstract = WIZABSTRACT();
    m_tags.clear();
    setText("");    //force repaint

    if (nType == PrivateDocument) {
        db.DocumentFromGUID(m_data.strGUID, m_data);
        setText(m_data.strTitle);
    } else if (nType == MessageDocument) {
        db.messageFromId(m_message.nId, m_message);
        setText(m_message.title);
    }
}

bool CWizDocumentListViewItem::operator <(const QListWidgetItem &other) const
{
    const CWizDocumentListViewItem* pOther = dynamic_cast<const CWizDocumentListViewItem*>(&other);
    Q_ASSERT(pOther);

    // default compare use create time

    if (!m_data.strGUID.isEmpty()) {
        // use document type
        if (pOther->m_data.tCreated == m_data.tCreated) {
            return text().compare(other.text(), Qt::CaseInsensitive) < 0;
        } else {
            return pOther->m_data.tCreated < m_data.tCreated;
        }

    } else if (!m_message.documentGUID.isEmpty()) {
        // use message type
        if (pOther->m_message.tCreated == m_message.tCreated) {
            return text().compare(other.text(), Qt::CaseInsensitive) < 0;
        } else {
            return pOther->m_message.tCreated < m_message.tCreated;
        }
    }

    Q_ASSERT(0);
    return true;
}
