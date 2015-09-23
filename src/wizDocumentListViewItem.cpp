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
    int nType = m_data.doc.nProtected ? (isSummaryView ? DocTypeEncrytedInTitle : DocTypeEncrytedInTitle) : DocTypeNormal;
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
        return compareYearAndMothOfDate(document().tModified.date(), secItem->sectionData().date) > 0;
    case -SortingByModifiedTime:
        return compareYearAndMothOfDate(document().tModified.date(), secItem->sectionData().date) < 0;
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
    case SortingByLocation:
        if (document().strLocation.startsWith(secItem->sectionData().strInfo))
            return false;
        return document().strLocation.localeAwareCompare(secItem->sectionData().strInfo) > 0;
    case -SortingByLocation:
        if (document().strLocation.startsWith(secItem->sectionData().strInfo))
            return false;
        return document().strLocation.localeAwareCompare(secItem->sectionData().strInfo) < 0;
    case SortingByTag:
    case -SortingByTag:
        return false;
    case SortingBySize:
    case -SortingBySize:
        return false;
    default:
        Q_ASSERT(0);
    }

    return false;
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
    if (m_data.doc.strKbGUID != CWizDatabaseManager::instance()->db().kbGUID())
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
            m_data.infoList << strAuthor << m_data.doc.tModified.toHumanFriendlyString();
            break;
        case SortingByAccessedTime:
        case -SortingByAccessedTime:
            m_data.infoList << strAuthor << m_data.doc.tAccessed.toHumanFriendlyString();
            break;
        case SortingByTitle:
        case -SortingByTitle:
            m_data.infoList << strAuthor << m_data.doc.tModified.toHumanFriendlyString();
            break;
        case SortingByTag:
        case -SortingByTag:
            m_data.infoList << strAuthor << tagTree();
            break;
        case SortingByLocation:
        case -SortingByLocation:
            m_data.infoList << strAuthor << tagTree();
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
            m_data.infoList << m_data.doc.tModified.toHumanFriendlyString() << tags();
            break;
        case SortingByAccessedTime:
        case -SortingByAccessedTime:
            m_data.infoList << m_data.doc.tAccessed.toHumanFriendlyString() << tags();
            break;
        case SortingByTitle:
        case -SortingByTitle:
            m_data.infoList << m_data.doc.tModified.toHumanFriendlyString() << tags();
            break;
        case SortingByTag:
        case -SortingByTag:
            m_data.infoList << m_data.doc.tModified.toHumanFriendlyString() << tags();
            break;
        case SortingByLocation:
        case -SortingByLocation:
            m_data.infoList << ::WizLocation2Display(m_data.doc.strLocation);
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
        if (pOther->m_data.doc.tModified == m_data.doc.tModified)
            return pOther->m_data.doc.strTitle.localeAwareCompare(m_data.doc.strTitle) < 0;
        else
            return pOther->m_data.doc.tModified < m_data.doc.tModified;
    case -SortingByModifiedTime:
        if (pOther->m_data.doc.tModified == m_data.doc.tModified)
            return pOther->m_data.doc.strTitle.localeAwareCompare(m_data.doc.strTitle) > 0;
        else
            return pOther->m_data.doc.tModified > m_data.doc.tModified;
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
    case SortingByLocation:
    {
        QString otherInfo = pOther->m_data.infoList.join(' ');
        QString info = pOther->m_data.infoList.join(' ');
        return otherInfo.localeAwareCompare(info) < 0;
    }
    case -SortingByLocation:
    {
        QString otherInfo = pOther->m_data.infoList.join(' ');
        QString info = pOther->m_data.infoList.join(' ');
        return otherInfo.localeAwareCompare(info) > 0;
    }
    case SortingByTag:
        return pOther->m_strTags < m_strTags;
    case -SortingByTag:
        return pOther->m_strTags > m_strTags;
    case SortingBySize:
        return pOther->m_nSize < m_nSize;
    case -SortingBySize:
        return pOther->m_nSize > m_nSize;
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

    if (!thumb.image.isNull()) {
        QPixmap pmt = QPixmap::fromImage(thumb.image);
        QRect rcp = Utils::StyleHelper::drawThumbnailPixmap(p, rcd, pmt);
        rcd.setRight(rcp.left() - 8);
    }

    rcd.setTop(rcd.top() + nTextTopMargin);
    int nType = badgeType(true);
    Utils::StyleHelper::drawListViewItemThumb(p, rcd, nType, m_data.doc.strTitle, m_data.infoList,
                                              m_data.doc.strLocation, thumb.text, bFocused, bSelected);
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
                                              m_data.doc.strLocation, thumb.text, bFocused, bSelected);
}

void CWizDocumentListViewDocumentItem::drawPrivateTwoLineView_impl(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();

    QRect rcd = drawItemBackground(p, vopt->rect, bSelected, bFocused);
    rcd.setTop(rcd.top() + nTextTopMargin);

    int nType = badgeType();
    Utils::StyleHelper::drawListViewItemThumb(p, rcd, nType, m_data.doc.strTitle, m_data.infoList,
                                              m_data.doc.strLocation, NULL, bFocused, bSelected);
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
                                              m_data.doc.strLocation, NULL, bFocused, bSelected);
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
        strIconPath += isRetina ? "uploading@2x.png" : "uploading.png";
    }
    else if (!db.IsDocumentDownloaded(m_data.doc.strGUID))
    {
        strIconPath += isRetina ? "downloading@2x.png" : "downloading.png";
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
        case SortingByLocation:
            return (m_data.strInfo.localeAwareCompare(secItem->sectionData().strInfo) > 0);
        case -SortingByTitle:
        case -SortingByLocation:
            return (m_data.strInfo.localeAwareCompare(secItem->sectionData().strInfo) < 0);

        case SortingByTag:
        case -SortingByTag:
            return 0;
        case SortingBySize:
        case -SortingBySize:
            return 0;
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
    font.setPixelSize(14);
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
        return compareYearAndMothOfDate(docItem->document().tModified.date(), sectionData().date) <= 0;
    case -SortingByModifiedTime:
        return compareYearAndMothOfDate(docItem->document().tModified.date(), sectionData().date) >= 0;
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
    case SortingByLocation:
        if (docItem->document().strLocation.startsWith(sectionData().strInfo))
            return true;
        return docItem->document().strLocation.localeAwareCompare(sectionData().strInfo) <= 0;
    case -SortingByLocation:
        if (docItem->document().strLocation.startsWith(sectionData().strInfo))
            return true;
        return docItem->document().strLocation.localeAwareCompare(sectionData().strInfo) >= 0;
    case SortingByTag:
    case -SortingByTag:
        return true;
    case SortingBySize:
    case -SortingBySize:
        return true;
    default:
        Q_ASSERT(0);
    }

    return true;
}


CWizDocumentListViewBaseItem::CWizDocumentListViewBaseItem(QObject* parent, WizDocumentListItemType type)
    : QListWidgetItem(0, type)
    , QObject(parent)
    , m_nSortingType(0)
{

}

void CWizDocumentListViewBaseItem::setSortingType(int type)
{
    m_nSortingType = type;
}
