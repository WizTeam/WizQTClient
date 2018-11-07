#include "WizDocumentListViewItem.h"

#include <QFile>
#include <QFileInfo>

#include <QPixmapCache>
#include <QPainter>
#include <QStyleOptionViewItem>

#include "WizDocumentListView.h"
#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "share/WizSettings.h"
#include "WizPopupButton.h"

#include "WizThumbCache.h"
#include "sync/WizAvatarHost.h"
#include "utils/WizStyleHelper.h"

#include "share/WizDocumentStyle.h"


WizDocumentListViewDocumentItem::WizDocumentListViewDocumentItem(WizExplorerApp& app,
                                                   const WizDocumentListViewItemData& data)
    : WizDocumentListViewBaseItem(0, WizDocumentListType_Document)
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

void WizDocumentListViewDocumentItem::resetAvatar(const QString& strFileName)
{
    Q_EMIT thumbnailReloaded();
}

bool WizDocumentListViewDocumentItem::isAvatarNeedUpdate(const QString& strFileName)
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

bool WizDocumentListViewDocumentItem::isContainsAttachment() const
{
    //NOTE:不应该从数据中读取附件信息，而是根据当前包含的笔记信息来判断。如果发现笔记信息与实际情况不一致，则问题在笔记信息刷新的部分。
    return m_data.doc.nAttachmentCount > 0;
}

int WizDocumentListViewDocumentItem::badgeType(bool isSummaryView) const
{
    int nType = m_data.doc.nProtected ? (isSummaryView ? DocTypeEncrytedInSummary : DocTypeEncrytedInTitle) : DocTypeNormal;
    nType = m_data.doc.isAlwaysOnTop() ? (DocTypeAlwaysOnTop | nType) : nType;
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

bool WizDocumentListViewDocumentItem::compareWithSectionItem(const WizDocumentListViewSectionItem* secItem) const
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

QString WizDocumentListViewDocumentItem::documentLocation() const
{
    return m_data.location;
}

void WizDocumentListViewDocumentItem::updateDocumentLocationData()
{
    WizDatabase& db = m_app.databaseManager().db(m_data.doc.strKbGUID);
    m_data.location = db.getDocumentLocation(m_data.doc);
}

bool WizDocumentListViewDocumentItem::needDrawDocumentLocation() const
{
    if ((m_nLeadInfoState & DocumentLeadInfo_PersonalRoot) ||
            (m_nLeadInfoState & DocumentLeadInfo_GroupRoot) ||
            (m_nLeadInfoState & DocumentLeadInfo_SearchResult))
    {
        return true;
    }
    return false;
}

void WizDocumentListViewDocumentItem::updateInfoList()
{    
    WizDatabase& db = m_app.databaseManager().db(m_data.doc.strKbGUID);
    m_data.infoList.clear();


    if (m_data.nType == TypeGroupDocument) {
        QString strAuthor = db.getDocumentOwnerAlias(m_data.doc);
        m_strAuthor = strAuthor;
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
        case SortingByLocation:
        case -SortingByLocation:
        {
            WizDatabase& db = m_app.databaseManager().db(m_data.doc.strKbGUID);
            m_data.location = db.getDocumentLocation(m_data.doc);
            m_data.infoList << strAuthor << m_data.location;
        }
            break;
        case SortingBySize:
        case -SortingBySize:
        {
            QString strFileName = db.getDocumentFileName(m_data.doc.strGUID);
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
        case SortingByLocation:
        case -SortingByLocation:
            m_data.location = ::WizLocation2Display(m_data.doc.strLocation);
            m_data.infoList << m_data.location;
            break;
        case SortingBySize:
        case -SortingBySize:
        {
            QString strFileName = db.getDocumentFileName(m_data.doc.strGUID);
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

bool WizDocumentListViewDocumentItem::isSpecialFocus() const
{
    return m_specialFocused;
}

void WizDocumentListViewDocumentItem::setSpecialFocused(bool bSpecialFocus)
{
    m_specialFocused = bSpecialFocus;
}

void WizDocumentListViewDocumentItem::updateDocumentUnreadCount()
{
    if (WizDatabaseManager::instance()->db(m_data.doc.strKbGUID).isGroup())
    {
        m_documentUnread = (m_data.doc.nReadCount == 0);
    }
}

void WizDocumentListViewDocumentItem::resetAbstract(const WIZABSTRACT& abs)
{
    m_data.thumb.strKbGUID = abs.strKbGUID;
    m_data.thumb.guid= abs.guid;
    m_data.thumb.text = abs.text;
    m_data.thumb.image = abs.image;

    Q_EMIT thumbnailReloaded();
}

const QString& WizDocumentListViewDocumentItem::tags()
{
    if (((m_nLeadInfoState & DocumentLeadInfo_PersonalRoot) ||
                                                   (m_nLeadInfoState & DocumentLeadInfo_SearchResult)))
    {
        m_strTags.clear();
        return m_strTags;
    }

    if (m_strTags.isEmpty()) {
        m_strTags = m_app.databaseManager().db(m_data.doc.strKbGUID).getDocumentTagDisplayNameText(m_data.doc.strGUID);
    }

    return m_strTags;
}

const QString& WizDocumentListViewDocumentItem::tagTree()
{
    if (m_strTags.isEmpty()) {
        m_strTags = "/" + m_app.databaseManager().db(m_data.doc.strKbGUID).name();
        m_strTags += m_app.databaseManager().db(m_data.doc.strKbGUID).getDocumentTagTreeDisplayString(m_data.doc.strGUID);
    }

    return m_strTags;
}

void WizDocumentListViewDocumentItem::reload(WizDatabase& db)
{
    Q_ASSERT(db.kbGUID() == m_data.doc.strKbGUID);

    m_strTags.clear();
    m_data.thumb = WIZABSTRACT();
    setSortingType(m_nSortingType); // reset info

    db.documentFromGuid(m_data.doc.strGUID, m_data.doc);
    setText(m_data.doc.strTitle);
    updateDocumentUnreadCount();

    Q_EMIT thumbnailReloaded();
}

void WizDocumentListViewDocumentItem::setSortingType(int type)
{
    m_nSortingType = type;

    updateInfoList();
}

void WizDocumentListViewDocumentItem::setLeadInfoState(int state)
{
    WizDocumentListViewBaseItem::setLeadInfoState(state);

//    updateInfoList();
}

int WizDocumentListViewDocumentItem::documentSize() const
{
    return m_nSize;
}

bool WizDocumentListViewDocumentItem::operator <(const QListWidgetItem &other) const
{
//    qDebug() << "compare by doc Item ; " << text() << " with : " << other.text();

    if (other.type() == WizDocumentListType_Section)
    {
        if(document().isAlwaysOnTop())
            return true;

        const WizDocumentListViewSectionItem* secItem = dynamic_cast<const WizDocumentListViewSectionItem*>(&other);
        return compareWithSectionItem(secItem);
    }

    const WizDocumentListViewDocumentItem* pOther = dynamic_cast<const WizDocumentListViewDocumentItem*>(&other);
    Q_ASSERT(pOther && m_nSortingType == pOther->m_nSortingType);


    if (pOther->m_data.doc.isAlwaysOnTop() || m_data.doc.isAlwaysOnTop())
    {
        if (pOther->m_data.doc.isAlwaysOnTop() && m_data.doc.isAlwaysOnTop())
        {
        }
        else
        {
            bool bTop = m_data.doc.isAlwaysOnTop();
            return bTop;
        }
    }
    //
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

void WizDocumentListViewDocumentItem::on_thumbnailReloaded()
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

void WizDocumentListViewDocumentItem::draw(QPainter* p, const QStyleOptionViewItem* vopt, int nViewType) const
{
    int nItemType = itemType();
    draw_impl(p, vopt, nItemType, nViewType);
    p->setClipRect(vopt->rect);
    drawSyncStatus(p, vopt, nViewType);
}

void WizDocumentListViewDocumentItem::draw_impl(QPainter* p, const QStyleOptionViewItem* vopt, int nItemType, int nViewType) const
{
    if (nItemType == WizDocumentListViewDocumentItem::TypePrivateDocument)
    {
        switch (nViewType) {
        case WizDocumentListView::TypeThumbnail:
            drawPrivateSummaryView_impl(p, vopt);
            return;
        case WizDocumentListView::TypeSearchResult:
            drawPrivateSummaryView_impl(p, vopt);
            return;
        case WizDocumentListView::TypeTwoLine:
            drawPrivateTwoLineView_impl(p, vopt);
            return;
        case WizDocumentListView::TypeOneLine:
            drawOneLineView_impl(p, vopt);
            return;
        default:
            Q_ASSERT(0);
            return;
        }
    }
    else if (nItemType == WizDocumentListViewDocumentItem::TypeGroupDocument)
    {
        switch (nViewType) {
        case WizDocumentListView::TypeThumbnail:
            drawGroupSummaryView_impl(p, vopt);
            return;
        case WizDocumentListView::TypeSearchResult:
            drawGroupSummaryView_impl(p, vopt);
            return;
        case WizDocumentListView::TypeTwoLine:
            drawGroupTwoLineView_impl(p, vopt);
            return;
        case WizDocumentListView::TypeOneLine:
            drawOneLineView_impl(p, vopt);
            return;
        default:
            Q_ASSERT(0);
            return;
        }
    }
    Q_ASSERT(0);
}

void WizDocumentListViewDocumentItem::setNeedUpdate() const
{
    QPixmapCache::remove(cacheKey());
}

QString WizDocumentListViewDocumentItem::cacheKey() const
{
    WizDocumentListView* view = qobject_cast<WizDocumentListView*>(listWidget());
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

    return "ListItem::" + m_data.doc.strGUID + "::" + view->viewType() + "::" + stat;
}

const int nTextTopMargin = 6;

void WizDocumentListViewDocumentItem::drawPrivateSummaryView_impl(QPainter* p, const QStyleOptionViewItem* vopt) const
{
    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();
    //
    WizDocumentListView* view = qobject_cast<WizDocumentListView*>(listWidget());
    Q_ASSERT(view);
    bool searchResult = view->isSearchResult();


    QString title;
    QString text;
    //
    QPixmap pmt;
    //
    QRect rcd = drawItemBackground(p, vopt->rect, bSelected, bFocused);
    rcd.setTop(rcd.top() + nTextTopMargin);
    //
    WIZSTYLEDATA style = WizDocumentStyle::instance().getStyle(document().strStyleGUID);
    //
    WIZABSTRACT thumb;
    if (searchResult) {
        //
        title = m_data.doc.strHighlightTitle;
        text = m_data.doc.strHighlightText;
        //
        if (title.isEmpty()) {
            title = m_data.doc.strTitle;
        }
        //
        QString info = m_strAuthor + " " + m_data.doc.tCreated.toHumanFriendlyString() + " (" + documentLocation() + ")";
        //
        Utils::WizStyleHelper::drawListViewItemSearchResult(p, rcd, title, info,
                                          text, bFocused, bSelected, style.crTextColor);
        //
    } else {
        WizThumbCache::instance()->find(m_data.doc.strKbGUID, m_data.doc.strGUID, thumb);
        if (!thumb.image.isNull()) {
            pmt = QPixmap::fromImage(thumb.image);
        }
        //
        title = m_data.doc.strTitle;
        text = thumb.text;
        //
        int nType = badgeType(true);
        Utils::WizStyleHelper::drawListViewItemThumb(p, rcd, nType, title, m_data.infoList,
                                                   needDrawDocumentLocation() ? documentLocation() : "", text,
                                                  bFocused, bSelected, pmt, style.crTextColor);
    }
}

const int nAvatarRightMargin = 8;
void WizDocumentListViewDocumentItem::drawGroupSummaryView_impl(QPainter* p, const QStyleOptionViewItem* vopt) const
{
    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();
    //
    WizDocumentListView* view = qobject_cast<WizDocumentListView*>(listWidget());
    Q_ASSERT(view);
    bool searchResult = view->isSearchResult();

    QString title;
    QString text;
    //
    QPixmap pmt;
    //
    QRect rcd = drawItemBackground(p, vopt->rect, bSelected, bFocused);
    //
    QPixmap pmAvatar;
    WizAvatarHost::avatar(m_data.strAuthorId, &pmAvatar);
    QRect rcAvatar = rcd.adjusted(8 ,12, 0, 0);
    rcAvatar = Utils::WizStyleHelper::drawAvatar(p, rcAvatar, pmAvatar);
    rcd.setLeft(rcAvatar.right() + nAvatarRightMargin);
    rcd.setTop(rcd.top() + nTextTopMargin);
    //
    if (searchResult)
    {
        title = m_data.doc.strHighlightTitle;
        text = m_data.doc.strHighlightText;
        //
        QString info = m_strAuthor + " " + m_data.doc.tCreated.toHumanFriendlyString() + " (" + documentLocation() + ")";
        //
        Utils::WizStyleHelper::drawListViewItemSearchResult(p, rcd, title, info,
                                          text, bFocused, bSelected);
    }
    else
    {
        WIZABSTRACT thumb;
        WizThumbCache::instance()->find(m_data.doc.strKbGUID, m_data.doc.strGUID, thumb);

        int nType = badgeType(true);
        Utils::WizStyleHelper::drawListViewItemThumb(p, rcd, nType, m_data.doc.strTitle, m_data.infoList,
                                                  needDrawDocumentLocation() ? documentLocation() : "", thumb.text, bFocused, bSelected);

    }

}

void WizDocumentListViewDocumentItem::drawPrivateTwoLineView_impl(QPainter* p, const QStyleOptionViewItem* vopt) const
{
    WIZSTYLEDATA style = WizDocumentStyle::instance().getStyle(document().strStyleGUID);
    //
    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();

    QRect rcd = drawItemBackground(p, vopt->rect, bSelected, bFocused);
    rcd.setTop(rcd.top() + nTextTopMargin);

    int nType = badgeType();
    Utils::WizStyleHelper::drawListViewItemThumb(p, rcd, nType, m_data.doc.strTitle, m_data.infoList,
                                              needDrawDocumentLocation() ? documentLocation() : "", NULL, bFocused, bSelected, QPixmap(), style.crTextColor);
}

void WizDocumentListViewDocumentItem::drawGroupTwoLineView_impl(QPainter* p, const QStyleOptionViewItem* vopt) const
{
    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();

    QRect rcd = drawItemBackground(p, vopt->rect, bSelected, bFocused);

    QPixmap pmAvatar;
    WizAvatarHost::avatar(m_data.strAuthorId, &pmAvatar);
    QRect rcAvatar = rcd.adjusted(8 ,10, 0, 0);
    rcAvatar = Utils::WizStyleHelper::drawAvatar(p, rcAvatar, pmAvatar);
    rcd.setLeft(rcAvatar.right() + nAvatarRightMargin);
    rcd.setTop(rcd.top() + nTextTopMargin);

    int nType = badgeType();
    Utils::WizStyleHelper::drawListViewItemThumb(p, rcd, nType, m_data.doc.strTitle, m_data.infoList,
                                              needDrawDocumentLocation() ? documentLocation() : "", NULL, bFocused, bSelected);
}

void WizDocumentListViewDocumentItem::drawOneLineView_impl(QPainter* p, const  QStyleOptionViewItem* vopt) const
{
    WIZSTYLEDATA style = WizDocumentStyle::instance().getStyle(document().strStyleGUID);
    //
    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();

    QRect rcd = drawItemBackground(p, vopt->rect, bSelected, bFocused);

    int nType = badgeType();
    Utils::WizStyleHelper::drawListViewItemThumb(p, rcd, nType, m_data.doc.strTitle, QStringList(), "", NULL, bFocused, bSelected, QPixmap(), style.crTextColor);
}

void WizDocumentListViewDocumentItem::drawSyncStatus(QPainter* p, const QStyleOptionViewItem* vopt, int nViewType) const
{
    Q_UNUSED(nViewType);

    WizDatabase& db = m_app.databaseManager().db(m_data.doc.strKbGUID);
    //
    QPixmap pix;
    const QSize iconSize(WizSmartScaleUI(16), WizSmartScaleUI(16));
    static QIcon download = WizLoadSkinIcon(m_app.userSettings().skin(), "document_needDownload", iconSize);
    static QIcon upload = WizLoadSkinIcon(m_app.userSettings().skin(), "document_needUpload", iconSize);
    static QPixmap pixmapDownload = download.pixmap(iconSize);
    static QPixmap pixmapUpload = upload.pixmap(iconSize);
    //
    bool attachModified = false;
    if (db.isDocumentModified(m_data.doc.strGUID) || attachModified)
    {
        pix = pixmapUpload;
    }
    else if (!db.isDocumentDownloaded(m_data.doc.strGUID))
    {
        pix = pixmapDownload;
    }
    else
        return;

    p->save();
    int nMargin = -1;
    QSize szPix = iconSize;
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

QRect WizDocumentListViewDocumentItem::drawItemBackground(QPainter* p, const QRect& rect, bool selected, bool focused) const
{
    int index = listWidget()->row(this);
    // if next brother is section item, use full line seperator
    bool useFullLineSeperator = (listWidget()->count() > index + 1) &&
            (listWidget()->item(index + 1)->type() == WizDocumentListType_Section);
    if (selected && focused)
    {
        return Utils::WizStyleHelper::initListViewItemPainter(p, rect,Utils::WizStyleHelper::ListBGTypeActive, useFullLineSeperator);
    }
    else if ((selected && !focused) || m_specialFocused)
    {
        return Utils::WizStyleHelper::initListViewItemPainter(p, rect,  Utils::WizStyleHelper::ListBGTypeHalfActive, useFullLineSeperator);
    }
    else if (m_documentUnread)
    {
        return Utils::WizStyleHelper::initListViewItemPainter(p, rect, Utils::WizStyleHelper::ListBGTypeUnread, useFullLineSeperator);
    }
    //
    WIZSTYLEDATA style = WizDocumentStyle::instance().getStyle(document().strStyleGUID);
    if (style.valid())
    {
        return Utils::WizStyleHelper::initListViewItemPainter(p, rect, Utils::WizStyleHelper::ListBGTypeCustom, useFullLineSeperator, style.crBackColor);
    }
    else
    {
        return Utils::WizStyleHelper::initListViewItemPainter(p, rect, Utils::WizStyleHelper::ListBGTypeNone, useFullLineSeperator);
    }
}


WizDocumentListViewSectionItem::WizDocumentListViewSectionItem(const WizDocumentListViewSectionData& data,
                                                                 const QString& text, int docCount)
    : WizDocumentListViewBaseItem(0, WizDocumentListType_Section)
    , m_data(data)
    , m_text(text)
    , m_documentCount(docCount)
{

}

bool WizDocumentListViewSectionItem::operator<(const QListWidgetItem& other) const
{
//    qDebug() << "compare text : " << text() << "  with ; " << other.text();

    if (other.type() == WizDocumentListType_Document)
    {
        const WizDocumentListViewDocumentItem* docItem = dynamic_cast<const WizDocumentListViewDocumentItem*>(&other);
        if(docItem->document().isAlwaysOnTop())
            return false;

        return compareWithDocumentItem(docItem);
    }
    else
    {
        const WizDocumentListViewSectionItem* secItem = dynamic_cast<const WizDocumentListViewSectionItem*>(&other);
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

void WizDocumentListViewSectionItem::draw(QPainter* p, const QStyleOptionViewItem* vopt, int nViewType) const
{
    p->save();
    p->fillRect(vopt->rect, Utils::WizStyleHelper::listViewSectionItemBackground());

    p->setPen(Utils::WizStyleHelper::listViewSectionItemText());
    QFont font;
    font.setPixelSize(WizSmartScaleUI(12));
    p->setFont(font);
    QRect rc = vopt->rect;
    rc.setLeft(rc.x() + Utils::WizStyleHelper::listViewItemHorizontalPadding());
    p->drawText(rc, Qt::AlignLeft | Qt::AlignVCenter, m_text);

    rc = vopt->rect;
    rc.setRight(rc.right() - Utils::WizStyleHelper::listViewItemHorizontalPadding());
    p->drawText(rc, Qt::AlignRight | Qt::AlignVCenter, QString::number(m_documentCount));

    p->setPen(Utils::WizStyleHelper::listViewItemSeperator());
    p->drawLine(vopt->rect.x(), vopt->rect.bottom(), vopt->rect.right(), vopt->rect.bottom());
    p->restore();
}

bool WizDocumentListViewSectionItem::compareWithDocumentItem(const WizDocumentListViewDocumentItem* docItem) const
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


WizDocumentListViewBaseItem::WizDocumentListViewBaseItem(QObject* parent, WizDocumentListItemType type)
    : QListWidgetItem(0, type)
    , QObject(parent)
    , m_nSortingType(SortingByCreatedTime)
    , m_nLeadInfoState(0)
{

}

void WizDocumentListViewBaseItem::setSortingType(int type)
{
    m_nSortingType = type;
}

void WizDocumentListViewBaseItem::setLeadInfoState(int state)
{
    m_nLeadInfoState = state;
}
