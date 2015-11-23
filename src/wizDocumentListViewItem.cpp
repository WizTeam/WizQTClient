#include "wizDocumentListViewItem.h"

#include <QFile>
#include <QFileInfo>

#include <QPixmapCache>
#include <QPainter>
#include <QStyleOptionViewItemV4>

#include "wizDocumentListView.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "share/wizsettings.h"
#include "wizPopupButton.h"

#include "thumbcache.h"
#include "sync/avatar.h"
#include "utils/stylehelper.h"

using namespace Core;

CWizDocumentListViewDocumentItem::CWizDocumentListViewDocumentItem(CWizExplorerApp& app,
                                                   const WizDocumentListViewItemData& data)
    : CWizDocumentListViewBaseItem(0, WizDocumentListType_Document)
    , m_app(app)
    , m_nSize(0)
    , m_documentUnread(false)
    , m_specialFocused(false)
{
    Q_ASSERT(!data.doc.strGUID.isEmpty());

    m_data.nType = data.nType;
    m_data.doc = data.doc;
    m_data.nMessageId = data.nMessageId;
    m_data.nReadStatus = data.nReadStatus;
    m_data.strAuthorId = data.strAuthorId;

    setText(data.doc.strTitle);

    updateDocumentUnreadCount();

    updateDocumentLocationData();

    connect(this, SIGNAL(thumbnailReloaded()), SLOT(on_thumbnailReloaded()));
}

void CWizDocumentListViewDocumentItem::resetAvatar(const QString& strFileName)
{
    Q_EMIT thumbnailReloaded();
}

bool CWizDocumentListViewDocumentItem::isAvatarNeedUpdate(const QString& strFileName)
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

bool CWizDocumentListViewDocumentItem::isContainsAttachment() const
{
    WIZDOCUMENTDATA docData;
    if (m_app.databaseManager().db(m_data.doc.strKbGUID).DocumentFromGUID(m_data.doc.strGUID, docData))
    {
        return docData.nAttachmentCount > 0;
    }
    return false;
}

int CWizDocumentListViewDocumentItem::badgeType(bool isSummaryView) const
{
    int nType = m_data.doc.nProtected ? (isSummaryView ? DocTypeEncrytedInSummary : DocTypeEncrytedInTitle) : DocTypeNormal;
    nType = m_data.doc.nFlags & wizDocumentAlwaysOnTop ? (DocTypeAlwaysOnTop | nType) : nType;
    nType = isContainsAttachment() ? (DocTypeContainsAttachment | nType) : nType;
    return nType;
}

int compareYearAndMothOfDate(const QDate& dateleft, const QDate& dateRight)
{
   if (dateleft.year() != dateRight.year())
       return dateleft.year() > dateRight.year() ? 1 : -1;

   if (dateleft.month() != dateRight.month())
       return dateleft.month() > dateRight.month() ? 1 : -1;

   return 0;
}

bool CWizDocumentListViewDocumentItem::compareWithSectionItem(const CWizDocumentListViewSectionItem* secItem) const
{
    switch (m_nSortingType) {
    case SortingByCreatedTime:
        // default compare use create time     //There is a bug in Qt sort. if two items have same time, use title to sort.
        return compareYearAndMothOfDate(document().tCreated.date(), secItem->sectionData().date) > 0;
    case -SortingByCreatedTime:
        return compareYearAndMothOfDate(document().tCreated.date(), secItem->sectionData().date) < 0;
    case SortingByModifiedTime:
        return compareYearAndMothOfDate(document().tDataModified.date(), secItem->sectionData().date) > 0;
    case -SortingByModifiedTime:
        return compareYearAndMothOfDate(document().tDataModified.date(), secItem->sectionData().date) < 0;
    case SortingByAccessedTime:
        return compareYearAndMothOfDate(document().tAccessed.date(), secItem->sectionData().date) > 0;
    case -SortingByAccessedTime:
        return compareYearAndMothOfDate(document().tAccessed.date(), secItem->sectionData().date) < 0;
    case SortingByTitle:
        if (document().strTitle.toUpper().trimmed().startsWith(secItem->sectionData().strInfo))
            return false;
        return document().strTitle.localeAwareCompare(secItem->sectionData().strInfo) > 0;
    case -SortingByTitle:
        if (document().strTitle.toUpper().trimmed().startsWith(secItem->sectionData().strInfo))
            return false;
        return document().strTitle.localeAwareCompare(secItem->sectionData().strInfo) < 0;
//    case SortingByLocation:
////        if (m_data.location.startsWith(secItem->sectionData().strInfo))
////            return false;
//    {
//        bool result = m_data.location.localeAwareCompare(secItem->sectionData().strInfo) > 0;
//        qDebug() << "compare doc and sec 1, loc : " << m_data.location << "  other loc : " << secItem->sectionData().strInfo << "result : " << result;
//        return result;
//    }
        //文件夹排序时子文件夹排在父文件夹的后面
    case SortingByLocation:
    case -SortingByLocation:
    {
        if (m_data.location.startsWith(secItem->sectionData().strInfo))
        {
//            qDebug() << "compare doc and sec 2.1, loc : " << m_data.location << "  other loc : " << secItem->sectionData().strInfo << "result : " << false;
            return false;
        }
        bool result = m_data.location.localeAwareCompare(secItem->sectionData().strInfo) < 0;
//        qDebug() << "compare doc and sec 2.2, loc : " << m_data.location << "  other loc : " << secItem->sectionData().strInfo << "result : " << result;
        return result;
    }
    case SortingByTag:
    case -SortingByTag:
        return false;
    case SortingBySize:
    {
        bool result = m_nSize >= secItem->sectionData().sizePair.second;
        return result;
    }
    case -SortingBySize:
    {
        bool result = m_nSize < secItem->sectionData().sizePair.first;
        return result;
    }
    default:
        Q_ASSERT(0);
    }

    return false;
}

QString CWizDocumentListViewDocumentItem::documentLocation() const
{
    return m_data.location;
}

void CWizDocumentListViewDocumentItem::updateDocumentLocationData()
{
    CWizDatabase& db = m_app.databaseManager().db(m_data.doc.strKbGUID);
    m_data.location = db.GetDocumentLocation(m_data.doc);
}

bool CWizDocumentListViewDocumentItem::needDrawDocumentLocation() const
{
    if ((m_nLeadInfoState & DocumentLeadInfo_PersonalRoot) ||
            (m_nLeadInfoState & DocumentLeadInfo_GroupRoot) ||
            (m_nLeadInfoState & DocumentLeadInfo_SearchResult))
    {
        return true;
    }
    return false;
}

void CWizDocumentListViewDocumentItem::updateInfoList()
{    
    CWizDatabase& db = m_app.databaseManager().db(m_data.doc.strKbGUID);
    m_data.infoList.clear();


    if (m_data.nType == TypeGroupDocument) {
        QString strAuthor = db.GetDocumentOwnerAlias(m_data.doc);
//        strAuthor += strAuthor.isEmpty() ? "" : " ";

        switch (m_nSortingType) {
        case SortingByCreatedTime:
        case -SortingByCreatedTime:
            m_data.infoList << strAuthor << m_data.doc.tCreated.toHumanFriendlyString();
            break;
        case SortingByModifiedTime:
        case -SortingByModifiedTime:
            m_data.infoList << strAuthor << m_data.doc.tDataModified.toHumanFriendlyString();
            break;
        case SortingByAccessedTime:
        case -SortingByAccessedTime:
            m_data.infoList << strAuthor << m_data.doc.tAccessed.toHumanFriendlyString();
            break;
        case SortingByTitle:
        case -SortingByTitle:
            m_data.infoList << strAuthor << m_data.doc.tDataModified.toHumanFriendlyString();
            break;
        case SortingByTag:
        case -SortingByTag:
            m_data.infoList << strAuthor << tagTree();
            break;
        case SortingByLocation:
        case -SortingByLocation:
        {
            CWizDatabase& db = m_app.databaseManager().db(m_data.doc.strKbGUID);
            m_data.location = db.GetDocumentLocation(m_data.doc);
            m_data.infoList << strAuthor << m_data.location;
        }
            break;
        case SortingBySize:
        case -SortingBySize:
        {
            QString strFileName = db.GetDocumentFileName(m_data.doc.strGUID);
            QFileInfo fi(strFileName);
            if (!fi.exists()) {
                m_data.infoList << strAuthor << QObject::tr("Unknown");
            } else {
                m_nSize = fi.size();
                m_data.infoList << strAuthor << ::WizGetFileSizeHumanReadalbe(strFileName);
            }
            break;
        }
        default:
            Q_ASSERT(0);
            break;
        }
    } else if (m_data.nType == TypePrivateDocument) {
        switch (m_nSortingType) {
        case SortingByCreatedTime:
        case -SortingByCreatedTime:
            m_data.infoList << m_data.doc.tCreated.toHumanFriendlyString() << tags();
            break;
        case SortingByModifiedTime:
        case -SortingByModifiedTime:
            m_data.infoList << m_data.doc.tDataModified.toHumanFriendlyString() << tags();
            break;
        case SortingByAccessedTime:
        case -SortingByAccessedTime:
            m_data.infoList << m_data.doc.tAccessed.toHumanFriendlyString() << tags();
            break;
        case SortingByTitle:
        case -SortingByTitle:
            m_data.infoList << m_data.doc.tDataModified.toHumanFriendlyString() << tags();
            break;
        case SortingByTag:
        case -SortingByTag:
            m_data.infoList << m_data.doc.tDataModified.toHumanFriendlyString() << tags();
            break;
        case SortingByLocation:
        case -SortingByLocation:
            m_data.location = ::WizLocation2Display(m_data.doc.strLocation);
            m_data.infoList << m_data.location;
            break;
        case SortingBySize:
        case -SortingBySize:
        {
            QString strFileName = db.GetDocumentFileName(m_data.doc.strGUID);
            QFileInfo fi(strFileName);
            if (!fi.exists()) {
                m_data.infoList << QObject::tr("Unknown") << tags();
            } else {
                m_nSize = fi.size();
                m_data.infoList << ::WizGetFileSizeHumanReadalbe(strFileName) << tags();
            }
            break;
        }
        default:
            Q_ASSERT(0);
            break;
        }
    }
}

bool CWizDocumentListViewDocumentItem::isSpecialFocus() const
{
    return m_specialFocused;
}

void CWizDocumentListViewDocumentItem::setSpecialFocused(bool bSpecialFocus)
{
    m_specialFocused = bSpecialFocus;
}

void CWizDocumentListViewDocumentItem::updateDocumentUnreadCount()
{
    if (CWizDatabaseManager::instance()->db(m_data.doc.strKbGUID).IsGroup())
    {
        m_documentUnread = (m_data.doc.nReadCount == 0);
    }
}

void CWizDocumentListViewDocumentItem::resetAbstract(const WIZABSTRACT& abs)
{
    m_data.thumb.strKbGUID = abs.strKbGUID;
    m_data.thumb.guid= abs.guid;
    m_data.thumb.text = abs.text;
    m_data.thumb.image = abs.image;

    Q_EMIT thumbnailReloaded();
}

const QString& CWizDocumentListViewDocumentItem::tags()
{
    if ((qAbs(m_nSortingType) != SortingByTag) && ((m_nLeadInfoState & DocumentLeadInfo_PersonalRoot) ||
                                                   (m_nLeadInfoState & DocumentLeadInfo_SearchResult)))
    {
        m_strTags.clear();
        return m_strTags;
    }

    if (m_strTags.isEmpty()) {
        m_strTags = m_app.databaseManager().db(m_data.doc.strKbGUID).GetDocumentTagDisplayNameText(m_data.doc.strGUID);
    }

    return m_strTags;
}

const QString& CWizDocumentListViewDocumentItem::tagTree()
{
    if (m_strTags.isEmpty()) {
        m_strTags = "/" + m_app.databaseManager().db(m_data.doc.strKbGUID).name();
        m_strTags += m_app.databaseManager().db(m_data.doc.strKbGUID).GetDocumentTagTreeDisplayString(m_data.doc.strGUID);
    }

    return m_strTags;
}

void CWizDocumentListViewDocumentItem::reload(CWizDatabase& db)
{
    Q_ASSERT(db.kbGUID() == m_data.doc.strKbGUID);

    m_strTags.clear();
    m_data.thumb = WIZABSTRACT();
    setSortingType(m_nSortingType); // reset info

    db.DocumentWithExFieldsFromGUID(m_data.doc.strGUID, m_data.doc);
    setText(m_data.doc.strTitle);
    updateDocumentUnreadCount();

    Q_EMIT thumbnailReloaded();
}

void CWizDocumentListViewDocumentItem::setSortingType(int type)
{
    m_nSortingType = type;

    updateInfoList();
}

void CWizDocumentListViewDocumentItem::setLeadInfoState(int state)
{
    CWizDocumentListViewBaseItem::setLeadInfoState(state);

//    updateInfoList();
}

int CWizDocumentListViewDocumentItem::documentSize() const
{
    return m_nSize;
}

bool CWizDocumentListViewDocumentItem::operator <(const QListWidgetItem &other) const
{
//    qDebug() << "compare by doc Item ; " << text() << " with : " << other.text();

    if (other.type() == WizDocumentListType_Section)
    {
        if(document().nFlags & wizDocumentAlwaysOnTop)
            return true;

        const CWizDocumentListViewSectionItem* secItem = dynamic_cast<const CWizDocumentListViewSectionItem*>(&other);
        return compareWithSectionItem(secItem);
    }

    const CWizDocumentListViewDocumentItem* pOther = dynamic_cast<const CWizDocumentListViewDocumentItem*>(&other);
    Q_ASSERT(pOther && m_nSortingType == pOther->m_nSortingType);

    if (pOther->m_data.doc.nFlags & wizDocumentAlwaysOnTop || m_data.doc.nFlags & wizDocumentAlwaysOnTop)
    {
        if (pOther->m_data.doc.nFlags & wizDocumentAlwaysOnTop && m_data.doc.nFlags & wizDocumentAlwaysOnTop)
        {
        }
        else
        {
               bool bTop = m_data.doc.nFlags & wizDocumentAlwaysOnTop;
               return bTop;               
        }
    }


    switch (m_nSortingType) {
    case SortingByCreatedTime:
        // default compare use create time     //There is a bug in Qt sort. if two items have same time, use title to sort.
        if (pOther->m_data.doc.tCreated == m_data.doc.tCreated)
            return pOther->m_data.doc.strTitle.localeAwareCompare(m_data.doc.strTitle) < 0;
        else
            return pOther->m_data.doc.tCreated < m_data.doc.tCreated;
    case -SortingByCreatedTime:
        if (pOther->m_data.doc.tCreated == m_data.doc.tCreated)
            return pOther->m_data.doc.strTitle.localeAwareCompare(m_data.doc.strTitle) > 0;
        else
            return pOther->m_data.doc.tCreated > m_data.doc.tCreated;
    case SortingByModifiedTime:
        if (pOther->m_data.doc.tDataModified == m_data.doc.tDataModified)
            return pOther->m_data.doc.strTitle.localeAwareCompare(m_data.doc.strTitle) < 0;
        else
            return pOther->m_data.doc.tDataModified < m_data.doc.tDataModified;
    case -SortingByModifiedTime:
        if (pOther->m_data.doc.tDataModified == m_data.doc.tDataModified)
            return pOther->m_data.doc.strTitle.localeAwareCompare(m_data.doc.strTitle) > 0;
        else
            return pOther->m_data.doc.tDataModified > m_data.doc.tDataModified;
    case SortingByAccessedTime:
        if (pOther->m_data.doc.tAccessed == m_data.doc.tAccessed)
            return pOther->m_data.doc.strTitle.localeAwareCompare(m_data.doc.strTitle) < 0;
        else
            return pOther->m_data.doc.tAccessed < m_data.doc.tAccessed;
    case -SortingByAccessedTime:
        if (pOther->m_data.doc.tAccessed == m_data.doc.tAccessed)
            return pOther->m_data.doc.strTitle.localeAwareCompare(m_data.doc.strTitle) > 0;
        else
            return pOther->m_data.doc.tAccessed > m_data.doc.tAccessed;
    case SortingByTitle:
        return pOther->m_data.doc.strTitle.localeAwareCompare(m_data.doc.strTitle) < 0;
    case -SortingByTitle:
        return pOther->m_data.doc.strTitle.localeAwareCompare(m_data.doc.strTitle) > 0;
//    case SortingByLocation:
//    {
////        QString otherInfo = pOther->m_data.infoList.join(" ");
////        QString info = pOther->m_data.infoList.join(" ");
////        return otherInfo.localeAwareCompare(info) < 0;
//        bool result = pOther->documentLocation().localeAwareCompare(m_data.location) < 0;
//        qDebug() << "compare doc and doc 1, loc : " << m_data.location << "  other loc : " << pOther->documentLocation() << "result : " << result;
//        return result;
//    }
        //NOTE: 笔记安装路径排序时，子目录的笔记永远排在父目录的后面
    case SortingByLocation:
    case -SortingByLocation:
    {
//        QString otherInfo = pOther->m_data.infoList.join(" ");
//        QString info = pOther->m_data.infoList.join(" ");
//        return otherInfo.localeAwareCompare(info) > 0;
        bool result = pOther->documentLocation().localeAwareCompare(m_data.location) > 0;
//        qDebug() << "compare doc and doc 2, loc : " << m_data.location << "  other loc : " << pOther->documentLocation() << "result : " << result;
        return result;
    }
    case SortingByTag:
        return pOther->m_strTags < m_strTags;
    case -SortingByTag:
        return pOther->m_strTags > m_strTags;
    case SortingBySize:
    {
        bool result = pOther->m_nSize < m_nSize;
        return result;
    }
    case -SortingBySize:
    {
        bool result = pOther->m_nSize > m_nSize;
        return result;
    }
    default:
        Q_ASSERT(0);
    }

    return true;
}

void CWizDocumentListViewDocumentItem::on_thumbnailReloaded()
{
    QPixmapCache::remove(m_data.doc.strGUID + ":normal");
    QPixmapCache::remove(m_data.doc.strGUID + ":focus");
    QPixmapCache::remove(m_data.doc.strGUID + ":nofocus");
}

//void CWizDocumentListViewDocumentItem::onThumbCacheLoaded(const QString& strKbGUID, const QString& strGUID)
//{
//    if (strKbGUID == m_data.doc.strKbGUID && strGUID == m_data.doc.strGUID)
//        setNeedUpdate();
//}

void CWizDocumentListViewDocumentItem::draw(QPainter* p, const QStyleOptionViewItemV4* vopt, int nViewType) const
{
    int nItemType = itemType();
    draw_impl(p, vopt, nItemType, nViewType);
    p->setClipRect(vopt->rect);
    drawSyncStatus(p, vopt, nViewType);
}

void CWizDocumentListViewDocumentItem::draw_impl(QPainter* p, const QStyleOptionViewItemV4* vopt, int nItemType, int nViewType) const
{
    if (nItemType == CWizDocumentListViewDocumentItem::TypePrivateDocument)
    {
        switch (nViewType) {
        case CWizDocumentListView::TypeThumbnail:
            drawPrivateSummaryView_impl(p, vopt);
            return;
        case CWizDocumentListView::TypeTwoLine:
            drawPrivateTwoLineView_impl(p, vopt);
            return;
        case CWizDocumentListView::TypeOneLine:
            drawOneLineView_impl(p, vopt);
            return;
        default:
            Q_ASSERT(0);
            return;
        }
    }
    else if (nItemType == CWizDocumentListViewDocumentItem::TypeGroupDocument)
    {
        switch (nViewType) {
        case CWizDocumentListView::TypeThumbnail:
            drawGroupSummaryView_impl(p, vopt);
            return;
        case CWizDocumentListView::TypeTwoLine:
            drawGroupTwoLineView_impl(p, vopt);
            return;
        case CWizDocumentListView::TypeOneLine:
            drawOneLineView_impl(p, vopt);
            return;
        default:
            Q_ASSERT(0);
            return;
        }
    }
    Q_ASSERT(0);
}

void CWizDocumentListViewDocumentItem::setNeedUpdate() const
{
    QPixmapCache::remove(cacheKey());
}

QString CWizDocumentListViewDocumentItem::cacheKey() const
{
    CWizDocumentListView* view = qobject_cast<CWizDocumentListView*>(listWidget());
    Q_ASSERT(view);

    QString stat;
    if (isSelected()) {
        if (view->hasFocus())
            stat = "Focus";
        else
            stat = "LoseFocus";
    } else {
        stat = "Normal";
    }

    return "Core::ListItem::" + m_data.doc.strGUID + "::" + view->viewType() + "::" + stat;
}

const int nTextTopMargin = 6;

void CWizDocumentListViewDocumentItem::drawPrivateSummaryView_impl(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();

    WIZABSTRACT thumb;
    ThumbCache::instance()->find(m_data.doc.strKbGUID, m_data.doc.strGUID, thumb);

    QRect rcd = drawItemBackground(p, vopt->rect, bSelected, bFocused);

    QPixmap pmt;
    if (!thumb.image.isNull()) {
        pmt = QPixmap::fromImage(thumb.image);
    }

    rcd.setTop(rcd.top() + nTextTopMargin);
    int nType = badgeType(true);
    Utils::StyleHelper::drawListViewItemThumb(p, rcd, nType, m_data.doc.strTitle, m_data.infoList,
                                               needDrawDocumentLocation() ? documentLocation() : "", thumb.text,
                                              bFocused, bSelected, pmt);
}

const int nAvatarRightMargin = 8;
void CWizDocumentListViewDocumentItem::drawGroupSummaryView_impl(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();

    WIZABSTRACT thumb;
    ThumbCache::instance()->find(m_data.doc.strKbGUID, m_data.doc.strGUID, thumb);

    QRect rcd = drawItemBackground(p, vopt->rect, bSelected, bFocused);

    QPixmap pmAvatar;
    WizService::AvatarHost::avatar(m_data.strAuthorId, &pmAvatar);
    QRect rcAvatar = rcd.adjusted(8 ,12, 0, 0);
    rcAvatar = Utils::StyleHelper::drawAvatar(p, rcAvatar, pmAvatar);
    rcd.setLeft(rcAvatar.right() + nAvatarRightMargin);
    rcd.setTop(rcd.top() + nTextTopMargin);

    int nType = badgeType(true);
    Utils::StyleHelper::drawListViewItemThumb(p, rcd, nType, m_data.doc.strTitle, m_data.infoList,
                                              needDrawDocumentLocation() ? documentLocation() : "", thumb.text, bFocused, bSelected);
}

void CWizDocumentListViewDocumentItem::drawPrivateTwoLineView_impl(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();

    QRect rcd = drawItemBackground(p, vopt->rect, bSelected, bFocused);
    rcd.setTop(rcd.top() + nTextTopMargin);

    int nType = badgeType();
    Utils::StyleHelper::drawListViewItemThumb(p, rcd, nType, m_data.doc.strTitle, m_data.infoList,
                                              needDrawDocumentLocation() ? documentLocation() : "", NULL, bFocused, bSelected);
}

void CWizDocumentListViewDocumentItem::drawGroupTwoLineView_impl(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();

    QRect rcd = drawItemBackground(p, vopt->rect, bSelected, bFocused);

    QPixmap pmAvatar;
    WizService::AvatarHost::avatar(m_data.strAuthorId, &pmAvatar);
    QRect rcAvatar = rcd.adjusted(8 ,10, 0, 0);
    rcAvatar = Utils::StyleHelper::drawAvatar(p, rcAvatar, pmAvatar);
    rcd.setLeft(rcAvatar.right() + nAvatarRightMargin);
    rcd.setTop(rcd.top() + nTextTopMargin);

    int nType = badgeType();
    Utils::StyleHelper::drawListViewItemThumb(p, rcd, nType, m_data.doc.strTitle, m_data.infoList,
                                              needDrawDocumentLocation() ? documentLocation() : "", NULL, bFocused, bSelected);
}

void CWizDocumentListViewDocumentItem::drawOneLineView_impl(QPainter* p, const  QStyleOptionViewItemV4* vopt) const
{
    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();

    QRect rcd = drawItemBackground(p, vopt->rect, bSelected, bFocused);

    int nType = badgeType();
    Utils::StyleHelper::drawListViewItemThumb(p, rcd, nType, m_data.doc.strTitle, QStringList(), "", NULL, bFocused, bSelected);
}

void CWizDocumentListViewDocumentItem::drawSyncStatus(QPainter* p, const QStyleOptionViewItemV4* vopt, int nViewType) const
{
    Q_UNUSED(nViewType);

    QString strIconPath;
    CWizDatabase& db = m_app.databaseManager().db(m_data.doc.strKbGUID);
    bool isRetina = WizIsHighPixel();
    strIconPath = ::WizGetSkinResourcePath(m_app.userSettings().skin());
    CWizDocumentAttachmentDataArray arrayAttachment;
    db.GetDocumentAttachments(m_data.doc.strGUID, arrayAttachment);
    bool attachModified = false;
    for (WIZDOCUMENTATTACHMENTDATAEX attachment : arrayAttachment)
    {
        if (db.IsAttachmentModified(attachment.strGUID))
            attachModified = true;        
    }
    if (db.IsDocumentModified(m_data.doc.strGUID) || attachModified)
    {
        strIconPath += isRetina ? "document_needUpload@2x.png" : "document_needUpload.png";
    }
    else if (!db.IsDocumentDownloaded(m_data.doc.strGUID))
    {
        strIconPath += isRetina ? "document_needDownload@2x.png" : "document_needDownload.png";
    }
    else
        return;

    p->save();
    int nMargin = -1;
    QPixmap pix(strIconPath);
    QSize szPix = pix.size();
    WizScaleIconSizeForRetina(szPix);
    QRect rcSync(vopt->rect.right() - szPix.width() - nMargin, vopt->rect.bottom() - szPix.height() - nMargin,
                 szPix.width(), szPix.height());
    if (vopt->state & QStyle::State_Selected)
    {
        QRect rcClip(vopt->rect.right() - szPix.width() - nMargin, vopt->rect.bottom() - szPix.height(),
                     szPix.width(), szPix.height());
        p->setClipRect(rcClip);
    }
    p->drawPixmap(rcSync, pix);
    p->restore();

    return;
}

QRect CWizDocumentListViewDocumentItem::drawItemBackground(QPainter* p, const QRect& rect, bool selected, bool focused) const
{
    int index = listWidget()->row(this);
    // if next brother is section item, use full line seperator
    bool useFullLineSeperator = (listWidget()->count() > index + 1) &&
            (listWidget()->item(index + 1)->type() == WizDocumentListType_Section);
    if (selected && focused)
    {
        return Utils::StyleHelper::initListViewItemPainter(p, rect,Utils::StyleHelper::ListBGTypeActive, useFullLineSeperator);
    }
    else if ((selected && !focused) || m_specialFocused)
    {
        return Utils::StyleHelper::initListViewItemPainter(p, rect,  Utils::StyleHelper::ListBGTypeHalfActive, useFullLineSeperator);
    }
    else if (m_documentUnread)
    {
        return Utils::StyleHelper::initListViewItemPainter(p, rect, Utils::StyleHelper::ListBGTypeUnread, useFullLineSeperator);
    }

    return Utils::StyleHelper::initListViewItemPainter(p, rect, Utils::StyleHelper::ListBGTypeNone, useFullLineSeperator);
}


CWizDocumentListViewSectionItem::CWizDocumentListViewSectionItem(const WizDocumentListViewSectionData& data,
                                                                 const QString& text, int docCount)
    : CWizDocumentListViewBaseItem(0, WizDocumentListType_Section)
    , m_data(data)
    , m_text(text)
    , m_documentCount(docCount)
{

}

bool CWizDocumentListViewSectionItem::operator<(const QListWidgetItem& other) const
{
//    qDebug() << "compare text : " << text() << "  with ; " << other.text();

    if (other.type() == WizDocumentListType_Document)
    {
        const CWizDocumentListViewDocumentItem* docItem = dynamic_cast<const CWizDocumentListViewDocumentItem*>(&other);
        if(docItem->document().nFlags & wizDocumentAlwaysOnTop)
            return false;

        return compareWithDocumentItem(docItem);
    }
    else
    {
        const CWizDocumentListViewSectionItem* secItem = dynamic_cast<const CWizDocumentListViewSectionItem*>(&other);
        switch (m_nSortingType) {
        case SortingByCreatedTime:
        case SortingByModifiedTime:
        case SortingByAccessedTime:
            return m_data.date > secItem->sectionData().date;
        case -SortingByCreatedTime:
        case -SortingByModifiedTime:
        case -SortingByAccessedTime:
            return m_data.date < secItem->sectionData().date;
        case SortingByTitle:
//        case SortingByLocation:
        {
            if (m_data.strInfo.startsWith(secItem->sectionData().strInfo) &&
                    m_data.strInfo.length() > secItem->sectionData().strInfo.length())
            {
//                qDebug() << "compare sec and sec 1.1 , loc : " << m_data.strInfo << "  other loc : " << secItem->sectionData().strInfo << "result : " << false;
                return false;
            }
            bool result = (m_data.strInfo.localeAwareCompare(secItem->sectionData().strInfo) > 0);
//            qDebug() << "compare sec and sec 1.1 , loc : " << m_data.strInfo << "  other loc : " << secItem->sectionData().strInfo << "result : " << result;
            return result;
        }
        case -SortingByTitle:
        case -SortingByLocation:
        case SortingByLocation:     //NOTE: 按文件夹排序目前只提供一种排序方向，降序排列需要重新整理算法
        {
//            if (m_data.strInfo.startsWith(secItem->sectionData().strInfo) &&
//                    m_data.strInfo.length() > secItem->sectionData().strInfo.length())
//            {
//                qDebug() << "compare sec and sec 2 , loc : " << m_data.strInfo << "  other loc : " << secItem->sectionData().strInfo << "result : " << 1;
//                return true;
//            }

            bool result = (m_data.strInfo.localeAwareCompare(secItem->sectionData().strInfo) < 0);
//            qDebug() << "compare sec and sec 2 , loc : " << m_data.strInfo << "  other loc : " << secItem->sectionData().strInfo << "result : " << result;
            return result;
        }
        case SortingByTag:
        case -SortingByTag:
            return 0;
        case SortingBySize:
        {
            bool result = sectionData().sizePair.second > secItem->sectionData().sizePair.second;
            return result;
        }
        case -SortingBySize:
        {
            bool result = sectionData().sizePair.first < secItem->sectionData().sizePair.first;
            return result;
        }
        default:
            Q_ASSERT(0);
        }
    }

    return true;
}

void CWizDocumentListViewSectionItem::draw(QPainter* p, const QStyleOptionViewItemV4* vopt, int nViewType) const
{
    p->save();
    p->fillRect(vopt->rect, Utils::StyleHelper::listViewSectionItemBackground());

    p->setPen(Utils::StyleHelper::listViewSectionItemText());
    QFont font;
    font.setPixelSize(12);
    p->setFont(font);
    QRect rc = vopt->rect;
    rc.setLeft(rc.x() + Utils::StyleHelper::listViewItemHorizontalPadding());
    p->drawText(rc, Qt::AlignLeft | Qt::AlignVCenter, m_text);

    rc = vopt->rect;
    rc.setRight(rc.right() - Utils::StyleHelper::listViewItemHorizontalPadding());
    p->drawText(rc, Qt::AlignRight | Qt::AlignVCenter, QString::number(m_documentCount));

    p->setPen(Utils::StyleHelper::listViewItemSeperator());
    p->drawLine(vopt->rect.x(), vopt->rect.bottom(), vopt->rect.right(), vopt->rect.bottom());
    p->restore();
}

bool CWizDocumentListViewSectionItem::compareWithDocumentItem(const CWizDocumentListViewDocumentItem* docItem) const
{
    switch (m_nSortingType) {
    case SortingByCreatedTime:
        // default compare use create time     //There is a bug in Qt sort. if two items have same time, use title to sort.
        return compareYearAndMothOfDate(docItem->document().tCreated.date(), sectionData().date) <= 0;
    case -SortingByCreatedTime:
        return compareYearAndMothOfDate(docItem->document().tCreated.date(), sectionData().date) >= 0;
    case SortingByModifiedTime:
        return compareYearAndMothOfDate(docItem->document().tDataModified.date(), sectionData().date) <= 0;
    case -SortingByModifiedTime:
        return compareYearAndMothOfDate(docItem->document().tDataModified.date(), sectionData().date) >= 0;
    case SortingByAccessedTime:
        return compareYearAndMothOfDate(docItem->document().tAccessed.date(), sectionData().date) <= 0;
    case -SortingByAccessedTime:
        return compareYearAndMothOfDate(docItem->document().tAccessed.date(), sectionData().date) >= 0;
    case SortingByTitle:
        if (docItem->document().strTitle.toUpper().trimmed().startsWith(sectionData().strInfo))
            return true;
        return docItem->document().strTitle.localeAwareCompare(sectionData().strInfo) <= 0;
    case -SortingByTitle:
        if (docItem->document().strTitle.toUpper().trimmed().startsWith(sectionData().strInfo))
            return true;
        return docItem->document().strTitle.localeAwareCompare(sectionData().strInfo) >= 0;
//    case SortingByLocation:
//    {
//        if (docItem->documentLocation().startsWith(sectionData().strInfo) &&
//                docItem->documentLocation().length() > sectionData().strInfo.length())
//        {
//            qDebug() << "compare sec and doc 1.1 , loc : " << m_data.strInfo << "  other loc : " << docItem->documentLocation() << "result : " << true;
//            return false;
//        }
//        bool result = docItem->documentLocation().localeAwareCompare(sectionData().strInfo) <= 0;
//        qDebug() << "compare sec and doc 1.2 , loc : " << m_data.strInfo << "  other loc : " << docItem->documentLocation() << "result : " << result;
//        return result;
//    }
        //按照路径排序的时候子文件夹排在父文件夹的后面
    case SortingByLocation:
    case -SortingByLocation:
    {
//        if (docItem->documentLocation().startsWith(sectionData().strInfo))
//        {
//            qDebug() << "compare sec and doc 2 , loc : " << m_data.strInfo << "  other loc : " << docItem->documentLocation() << "result : " << -1;
//            return false;
//        }
        bool result = docItem->documentLocation().localeAwareCompare(sectionData().strInfo) >= 0;
//        qDebug() << "compare sec and doc 2 , loc : " << m_data.strInfo << "  other loc : " << docItem->documentLocation() << "result : " << result;
        return result;
    }
    case SortingByTag:
    case -SortingByTag:
        return true;
    case SortingBySize:
    {
        bool result = docItem->documentSize() <= sectionData().sizePair.second;
        return result;
    }
    case -SortingBySize:
    {
        bool result = docItem->documentSize() > sectionData().sizePair.first;
        return result;
    }
    default:
        Q_ASSERT(0);
    }

    return true;
}


CWizDocumentListViewBaseItem::CWizDocumentListViewBaseItem(QObject* parent, WizDocumentListItemType type)
    : QListWidgetItem(0, type)
    , QObject(parent)
    , m_nSortingType(SortingByCreatedTime)
    , m_nLeadInfoState(0)
{

}

void CWizDocumentListViewBaseItem::setSortingType(int type)
{
    m_nSortingType = type;
}

void CWizDocumentListViewBaseItem::setLeadInfoState(int state)
{
    m_nLeadInfoState = state;
}
