#include "wizDocumentListViewItem.h"

//#include <QtWidgets>

#include <QFile>
#include <QFileInfo>
#include <QPixmapCache>

#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "share/wizThumbIndexCache.h"
#include "wizPopupButton.h"

#include "sync/avatar.h"

CWizDocumentListViewItem::CWizDocumentListViewItem(CWizExplorerApp& app,
                                                   const WizDocumentListViewItemData& data)
    : QListWidgetItem(0, UserType)
    , QObject(0)
    , m_app(app)
    , m_nSortingType(0)
    , m_nSize(0)
{
    Q_ASSERT(!data.doc.strKbGUID.isEmpty());
    Q_ASSERT(!data.doc.strGUID.isEmpty());

    m_data.nType = data.nType;
    m_data.doc = data.doc;
    m_data.nMessageId = data.nMessageId;
    m_data.nReadStatus = data.nReadStatus;
    m_data.strAuthorId = data.strAuthorId;

    setText(data.doc.strTitle);

    connect(this, SIGNAL(thumbnailReloaded()), SLOT(on_thumbnailReloaded()));
}

const QImage& CWizDocumentListViewItem::avatar(const CWizDatabase& db)
{
    Q_ASSERT(!m_data.strAuthorId.isEmpty());

    if (m_data.imgAuthorAvatar.isNull()) {
        // load avatar or request downloader to download
        //QString strFileName = db.GetAvatarPath() + m_data.strAuthorId + ".png";
        //if (isAvatarNeedUpdate(strFileName)) {
        WizService::Internal::AvatarHost::load(m_data.strAuthorId);
        //} else {
        //    m_data.imgAuthorAvatar.load(strFileName);
        //}
    }

    return m_data.imgAuthorAvatar;
}

void CWizDocumentListViewItem::resetAvatar(const QString& strFileName)
{
    m_data.imgAuthorAvatar.load(strFileName);

    Q_EMIT thumbnailReloaded();
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

    Q_EMIT thumbnailReloaded();
}

const QString& CWizDocumentListViewItem::tags()
{
    if (m_strTags.isEmpty()) {
        m_strTags = m_app.databaseManager().db(m_data.doc.strKbGUID).GetDocumentTagDisplayNameText(m_data.doc.strGUID);
    }

    return m_strTags;
}

const QString& CWizDocumentListViewItem::tagTree()
{
    if (m_strTags.isEmpty()) {
        m_strTags = "/" + m_app.databaseManager().db(m_data.doc.strKbGUID).name();
        m_strTags += m_app.databaseManager().db(m_data.doc.strKbGUID).GetDocumentTagTreeDisplayString(m_data.doc.strGUID);
    }

    return m_strTags;
}

void CWizDocumentListViewItem::reload(CWizDatabase& db)
{
    Q_ASSERT(db.kbGUID() == m_data.doc.strKbGUID);

    m_strTags.clear();
    m_data.thumb = WIZABSTRACT();
    setSortingType(m_nSortingType); // reset info

    db.DocumentFromGUID(m_data.doc.strGUID, m_data.doc);
    setText(m_data.doc.strTitle);

    Q_EMIT thumbnailReloaded();
}

void CWizDocumentListViewItem::setSortingType(int type)
{
    m_nSortingType = type;

    QString strFileName = m_app.databaseManager().db(m_data.doc.strKbGUID).GetDocumentFileName(m_data.doc.strGUID);
    QFileInfo fi(strFileName);

    if (m_data.nType == TypeGroupDocument) {
        switch (m_nSortingType) {
        case CWizSortingPopupButton::SortingCreateTime:
            m_data.strInfo = m_data.doc.tCreated.toHumanFriendlyString();
            break;
        case CWizSortingPopupButton::SortingUpdateTime:
            m_data.strInfo = m_data.doc.tModified.toHumanFriendlyString();
            break;
        case CWizSortingPopupButton::SortingTitle:
            m_data.strInfo = m_data.doc.tModified.toHumanFriendlyString();
            break;
        case CWizSortingPopupButton::SortingTag:
            m_data.strInfo = tagTree();
            break;
        case CWizSortingPopupButton::SortingLocation:
            m_data.strInfo = tagTree();
            break;
        case CWizSortingPopupButton::SortingSize:
            if (!fi.exists()) {
                m_data.strInfo = QObject::tr("Unknown");
            } else {
                m_nSize = fi.size();
                m_data.strInfo = ::WizGetFileSizeHumanReadalbe(strFileName);
            }
            break;
        default:
            Q_ASSERT(0);
            break;
        }
    } else if (m_data.nType == TypePrivateDocument) {
        switch (m_nSortingType) {
        case CWizSortingPopupButton::SortingCreateTime:
            m_data.strInfo = m_data.doc.tCreated.toHumanFriendlyString() + " " + tags();
            break;
        case CWizSortingPopupButton::SortingUpdateTime:
            m_data.strInfo = m_data.doc.tModified.toHumanFriendlyString() + " " + tags();
            break;
        case CWizSortingPopupButton::SortingTitle:
            m_data.strInfo = m_data.doc.tModified.toHumanFriendlyString() + " " + tags();
            break;
        case CWizSortingPopupButton::SortingTag:
            m_data.strInfo = m_data.doc.tModified.toHumanFriendlyString() + " " + tags();
            break;
        case CWizSortingPopupButton::SortingLocation:
            m_data.strInfo = ::WizLocation2Display(m_data.doc.strLocation);
            break;
        case CWizSortingPopupButton::SortingSize:
            if (!fi.exists()) {
                m_data.strInfo = QObject::tr("Unknown") + " " + tags();
            } else {
                m_nSize = fi.size();
                m_data.strInfo = ::WizGetFileSizeHumanReadalbe(strFileName) + " "  + tags();
            }
            break;
        default:
            Q_ASSERT(0);
            break;
        }
    }
}

bool CWizDocumentListViewItem::operator <(const QListWidgetItem &other) const
{
    const CWizDocumentListViewItem* pOther = dynamic_cast<const CWizDocumentListViewItem*>(&other);
    Q_ASSERT(pOther && m_nSortingType == pOther->m_nSortingType);

    switch (m_nSortingType) {
    case CWizSortingPopupButton::SortingCreateTime:
        // default compare use create time
        return pOther->m_data.doc.tCreated < m_data.doc.tCreated;
    case CWizSortingPopupButton::SortingUpdateTime:
        return pOther->m_data.doc.tModified < m_data.doc.tModified;
    case CWizSortingPopupButton::SortingTitle:
        return pOther->m_data.doc.strTitle < m_data.doc.strTitle;
    case CWizSortingPopupButton::SortingLocation:
        return pOther->m_data.strInfo < m_data.strInfo;
    case CWizSortingPopupButton::SortingTag:
        return pOther->m_strTags < m_strTags;
    case CWizSortingPopupButton::SortingSize:
        return pOther->m_nSize < m_nSize;
    default:
        Q_ASSERT(0);
    }

    return true;
}

void CWizDocumentListViewItem::on_thumbnailReloaded()
{
    QPixmapCache::remove(m_data.doc.strGUID + ":normal");
    QPixmapCache::remove(m_data.doc.strGUID + ":focus");
    QPixmapCache::remove(m_data.doc.strGUID + ":nofocus");
}
