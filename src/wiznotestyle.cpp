#include "wiznotestyle.h"

#include <QProxyStyle>

#include "wizCategoryView.h"
#include "wizDocumentListView.h"
#include "wizDocumentListViewItem.h"
#include "share/wizdrawtexthelper.h"
#include "share/wizqthelper.h"
#include "share/wizsettings.h"
#include "share/wizuihelper.h"
#include "share/wizui.h"
#include "share/wizmultilinelistwidget.h"
//#include "share/wizimagepushbutton.h"

#include "messagelistview.h"

#include "utils/stylehelper.h"
#include "sync/avatar.h"

#ifdef Q_OS_MAC
#include "mac/wizmachelper.h"
#endif


class CWizCategoryBaseView;
class CWizDocumentListView;

using namespace WizService::Internal;

typedef QProxyStyle CWizNoteBaseStyle;

class CWizNoteStyle : public CWizNoteBaseStyle
{
public:
    CWizNoteStyle(const QString& skinName);

private:
    QImage m_expandedImage;
    QImage m_collapsedImage;
    QImage m_imgDocumentUnread;
    QImage m_imgDefaultAvatar;

    CWizSkin9GridImage m_multiLineListSelectedItemBackground;
    CWizSkin9GridImage m_multiLineListSelectedItemBackgroundHot;
    CWizSkin9GridImage m_imagePushButton;
    CWizSkin9GridImage m_imagePushButtonHot;
    CWizSkin9GridImage m_imagePushButtonPressed;
    CWizSkin9GridImage m_imagePushButtonDisabled;
    CWizSkin9GridImage m_imagePushButtonLabel;
    CWizSkin9GridImage m_imagePushButtonLabelRed;

    // category view
    QColor m_colorCategoryBackground;
    QColor m_colorCategoryTextNormal;
    QColor m_colorCategoryTextSelected;
    QColor m_colorCategoryTextCategoryNormal;
    QColor m_colorCategorySelectedBackground;
    QColor m_colorCategorySelctedBackgroundNoFocus;

    // document list view
    QIcon m_iconDocumentsBadge;
    QIcon m_iconDocumentsBadgeEncrypted;
    QIcon m_iconDocumentsAttachment;
    QColor m_colorDocumentsBackground;
    QColor m_colorDocumentsItemFocusBackground;
    QColor m_colorDocumentsItemLoseFocusBackground;
    QColor m_colorDocumentsTitle;
    QColor m_colorDocumentsDate;
    QColor m_colorDocumentsSummary;
    QColor m_colorDocumentsTitleFocus;
    QColor m_colorDocumentsDateFocus;
    QColor m_colorDocumentsSummaryFocus;
    QColor m_colorDocumentsTitleLoseFocus;
    QColor m_colorDocumentsDateLoseFocus;
    QColor m_colorDocumentsSummaryLoseFocus;
    QColor m_colorDocumentsLine;

    QColor m_colorMultiLineListFirstLine;
    QColor m_colorMultiLineListFirstLineSelected;
    QColor m_colorMultiLineListOtherLine;
    QColor m_colorMultiLineListOtherLineSelected;

    QFont m_fontImagePushButtonLabel;

protected:
    virtual void drawCategoryViewItem(const QStyleOptionViewItemV4 *option, QPainter *painter, const CWizCategoryBaseView *widget) const;
    virtual void drawMultiLineListWidgetItem(const QStyleOptionViewItemV4 *option, QPainter *painter, const CWizMultiLineListWidget *widget) const;
    //virtual void drawImagePushButton(const QStyleOptionButton *option, QPainter *painter, const CWizImagePushButton *button) const;
    //virtual void drawSplitter(const QStyleOption *option, QPainter *painter, const QSplitter *splitter) const;

    void drawDocumentListViewItem(const QStyleOptionViewItemV4* option,
                                  QPainter* painter,
                                  const CWizDocumentListView* view) const;

    QPixmap genThumbnailPixmap(const QStyleOptionViewItemV4* vopt, const CWizDocumentListView* view) const;

    // different view type for different document type: user, group, message
    void drawItemPrivateThumbnail(const QStyleOptionViewItemV4* vopt,
                                  QPainter* p,
                                  const CWizDocumentListView* view) const;

    void drawItemPrivateTwoLine(const QStyleOptionViewItemV4* vopt,
                                QPainter* p,
                                const CWizDocumentListView* view) const;

    //void drawItemGroupThumbnail(const QStyleOptionViewItemV4* vopt,
    //                            QPainter* p,
    //                            const CWizDocumentListView* view) const;

    void drawItemGroupTwoLine(const QStyleOptionViewItemV4* vopt,
                              QPainter* p,
                              const CWizDocumentListView* view) const;

    void drawItemOneLine(const QStyleOptionViewItemV4* vopt,
                         QPainter* p,
                         const CWizDocumentListView* view) const;

    //void drawItemMessage(const QStyleOptionViewItemV4* option,
    //                     QPainter* painter,
    //                     const CWizDocumentListView* view) const;

    //void drawDocumentListViewItemMessage(const QStyleOptionViewItemV4 *option,
    //                                     QPainter *painter,
    //                                     const CWizDocumentListView *view) const;

    void drawcenterImage(QPainter* p, const QImage& image, const QRect& rc) const;

public:
    virtual void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    virtual void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const;
    virtual int	pixelMetric(PixelMetric metric, const QStyleOption* option = 0, const QWidget* widget = 0 ) const;

public:
    QColor categoryBackground() { return m_colorCategoryBackground; }
    QColor documentsBackground() { return m_colorDocumentsBackground; }

public:
    static CWizNoteStyle* noteStyle(const QString& skinName);
};


const int IMAGE_WIDTH = 80;

CWizNoteStyle::CWizNoteStyle(const QString& strSkinName)
{
    QString strSkinPath = ::WizGetSkinResourcePath(strSkinName);

    m_expandedImage.load(strSkinPath + "branch_expanded.png");
    m_collapsedImage.load(strSkinPath + "branch_collapsed.png");
    m_imgDocumentUnread.load(strSkinPath + "read_btn_unread.png");
    m_imgDefaultAvatar.load(strSkinPath + "avatar_default.png");

    m_iconDocumentsBadge = ::WizLoadSkinIcon(strSkinName, "document_badge");
    m_iconDocumentsBadgeEncrypted = ::WizLoadSkinIcon(strSkinName, "document_badge_encrypted");

    m_multiLineListSelectedItemBackground.SetImage(strSkinPath + "multilinelist_selected_background.png", QPoint(4, 4));
    m_multiLineListSelectedItemBackgroundHot.SetImage(strSkinPath + "multilinelist_selected_background_hot.png", QPoint(4, 4));
    m_imagePushButton.SetImage(strSkinPath + "imagepushbutton.png", QPoint(4, 4));
    m_imagePushButtonHot.SetImage(strSkinPath + "imagepushbutton_hot.png", QPoint(4, 4));
    m_imagePushButtonPressed.SetImage(strSkinPath + "imagepushbutton_pressed.png", QPoint(4, 4));
    m_imagePushButtonDisabled.SetImage(strSkinPath + "imagepushbutton_disabled.png", QPoint(4, 4));
    m_imagePushButtonLabel.SetImage(strSkinPath + "imagepushbutton_label.png", QPoint(8, 8));
    m_imagePushButtonLabelRed.SetImage(strSkinPath + "imagepushbutton_label_red.png", QPoint(8, 8));

    CWizSettings settings(strSkinPath + "skin.ini");

    // category view
    m_colorCategoryBackground = settings.GetColor("Category", "Background", "#808080");
    m_colorCategoryTextNormal = settings.GetColor("Category", "Text", "#000000");
    m_colorCategoryTextSelected = settings.GetColor("Category", "TextSelected", "#ffffff");
    m_colorCategoryTextCategoryNormal = settings.GetColor("Category", "ItemCategoryText", "#ffffff");
    m_colorCategorySelectedBackground = settings.GetColor("Category", "ItemSelected", "#808080");
    m_colorCategorySelctedBackgroundNoFocus = settings.GetColor("Category", "ItemSelectedNoFocus", "#808080");

    // document list view
    m_colorDocumentsBackground = settings.GetColor("Documents", "Background", "#ffffff");
    m_colorDocumentsItemFocusBackground = settings.GetColor("Documents", "ItemFocusBackground", "#0c8eec");
    m_colorDocumentsItemLoseFocusBackground = settings.GetColor("Documents", "ItemLoseFocusBackground", "#e1e1e1");
    m_colorDocumentsTitle = settings.GetColor("Documents", "Title", "#464646");
    m_colorDocumentsDate = settings.GetColor("Documents", "Date", "#0000FF");
    m_colorDocumentsSummary = settings.GetColor("Documents", "Summary", "#8C8C8C");
    m_colorDocumentsTitleFocus = settings.GetColor("Documents", "TitleFocus", "#FFFFFF");
    m_colorDocumentsDateFocus = settings.GetColor("Documents", "DateFocus", "#FFFFFF");
    m_colorDocumentsSummaryFocus = settings.GetColor("Documents", "SummaryFocus", "#FFFFFF");
    m_colorDocumentsTitleLoseFocus = settings.GetColor("Documents", "TitleLoseFocus", "6A6A6A");
    m_colorDocumentsDateLoseFocus = settings.GetColor("Documents", "DateLoseFocus", "6A6A6A");
    m_colorDocumentsSummaryLoseFocus = settings.GetColor("Documents", "SummaryLoseFocus", "6A6A6A");
    m_colorDocumentsLine = settings.GetColor("Documents", "Line", "#d9dcdd");

    m_colorMultiLineListFirstLine = settings.GetColor("MultiLineList", "First", "#000000");
    m_colorMultiLineListFirstLineSelected = settings.GetColor("MultiLineList", "FirstSelected", "#000000");
    m_colorMultiLineListOtherLine = settings.GetColor("MultiLineList", "Other", "#666666");
    m_colorMultiLineListOtherLineSelected = settings.GetColor("MultiLineList", "OtherSelected", "#666666");

#ifdef Q_OS_MAC
    m_fontImagePushButtonLabel = QFont("Arial Black", 9);
#else
    m_fontImagePushButtonLabel = QFont("Arial Black", 8);
#endif
    m_fontImagePushButtonLabel.setBold(true);
}


void CWizNoteStyle::drawCategoryViewItem(const QStyleOptionViewItemV4 *vopt,
                                         QPainter *painter, const CWizCategoryBaseView *view) const
{
    CWizCategoryViewItemBase* pItem = view->categoryItemFromIndex(vopt->index);

    if (view->isHelperItemByIndex(vopt->index)) {
        if (NULL != dynamic_cast<const CWizCategoryViewCategoryItem*>(pItem)) {
            painter->setPen(m_colorCategoryTextCategoryNormal);
            painter->drawText(vopt->rect, Qt::AlignLeft | Qt::AlignVCenter, vopt->text);
        }

        return;
    }

    QPainter* p = painter;
    p->save();
    p->setClipRect(vopt->rect);

    // draw the icon
    if (!vopt->icon.isNull()) {
        // FIXME: draw icon little bigger than the default rect qt give us
        QRect iconRect = subElementRect(SE_ItemViewItemDecoration, vopt, view);

        if (vopt->state.testFlag(State_Selected)) {
            vopt->icon.paint(p, iconRect, Qt::AlignCenter, QIcon::Selected);
        } else {
            vopt->icon.paint(p, iconRect, Qt::AlignCenter, QIcon::Normal);
        }
    }

    // text should not empty
    if (vopt->text.isEmpty()) {
        Q_ASSERT(0);
        return;
    }

    // draw text little far from icon than the default
    QRect textRect = subElementRect(SE_ItemViewItemText, vopt, view);
    //textRect.adjust(8, 0, 0, 0);

    // draw the text
    QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled
            ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
        cg = QPalette::Inactive;

    if (vopt->state & QStyle::State_Selected) {
        p->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
    } else {
        p->setPen(vopt->palette.color(cg, QPalette::Text));
    }

    if (vopt->state & QStyle::State_Editing) {
        p->setPen(vopt->palette.color(cg, QPalette::Text));
        p->drawRect(textRect.adjusted(0, 0, -1, -1));
    }

    // compute document count string length and leave enough space for drawing
    QString strCount = pItem->countString;
    int nCountWidthMax;
    int nMargin = 3;
    QFont fontCount = p->font();
    fontCount.setPointSize(10);

    if (!strCount.isEmpty()) {
        QFont fontOld = p->font();
        p->setFont(fontCount);
        nCountWidthMax = p->fontMetrics().width(strCount) + nMargin;
        textRect.adjust(0, 0, -nCountWidthMax, 0);
        p->setFont(fontOld);
    }

    QFont f = p->font();
    f.setPixelSize(12);
    p->setFont(f);

    QColor colorText = vopt->state.testFlag(State_Selected) ?
                m_colorCategoryTextSelected : m_colorCategoryTextNormal;

    CString strText = vopt->text;
    int nWidth = ::WizDrawTextSingleLine(p, textRect, strText,
                                         Qt::TextSingleLine | Qt::AlignVCenter,
                                         colorText, true);

    // only draw document count if count string is not empty
    if (!strCount.isEmpty()) {
        p->setFont(fontCount);
        textRect.adjust(nWidth + nMargin, 0, nCountWidthMax, 0);
        QColor colorCount = vopt->state.testFlag(State_Selected) ? QColor(230, 230, 230) : QColor(150, 150, 150);
        CString strCount2(strCount);
        ::WizDrawTextSingleLine(p, textRect, strCount2,  Qt::TextSingleLine | Qt::AlignVCenter, colorCount, false);
    }

    p->restore();
}

void CWizNoteStyle::drawDocumentListViewItem(const QStyleOptionViewItemV4 *option,
                                             QPainter *painter,
                                             const CWizDocumentListView* view) const
{
    CWizDocumentListViewItem* pItem = view->documentItemFromIndex(option->index);
    int nItemType = pItem->itemType();
    int nViewType = view->viewType();

    if (nItemType == CWizDocumentListViewItem::TypePrivateDocument)
    {
        switch (nViewType) {
        case CWizDocumentListView::TypeThumbnail:
            //drawItemPrivateThumbnail(option, painter, view);
            view->drawItem(painter, option);
            break;
        case CWizDocumentListView::TypeTwoLine:
            drawItemPrivateTwoLine(option, painter, view);
            break;
        case CWizDocumentListView::TypeOneLine:
            drawItemOneLine(option, painter, view);
            break;
        default:
            Q_ASSERT(0);
            break;
        }
    }
    else if (nItemType == CWizDocumentListViewItem::TypeGroupDocument)
    {
        switch (nViewType) {
        case CWizDocumentListView::TypeThumbnail:
            //drawItemGroupThumbnail(option, painter, view);
            view->drawItem(painter, option);
            break;
        case CWizDocumentListView::TypeTwoLine:
            drawItemGroupTwoLine(option, painter, view);
            break;
        case CWizDocumentListView::TypeOneLine:
            drawItemOneLine(option, painter, view);
            break;
        default:
            Q_ASSERT(0);
            break;
        }
    }
    //else if (nItemType == CWizDocumentListViewItem::TypeMessage)
    //{
    //    drawItemMessage(option, painter, view);
    //}
    else
    {
        Q_ASSERT(0);
    }
}


QPixmap CWizNoteStyle::genThumbnailPixmap(const QStyleOptionViewItemV4* vopt, const CWizDocumentListView* view) const
{
    QRect rc(0, 0, vopt->rect.width(), vopt->rect.height());

    QPixmap pixmap = Utils::StyleHelper::pixmapFromDevice(rc.size());
    pixmap.fill(m_colorDocumentsBackground);

    QPainter p(&pixmap);
    Utils::StyleHelper::initPainterByDevice(&p);

    // indirect access
    const WizDocumentListViewItemData& data = view->documentItemDataFromIndex(vopt->index);
    const WIZABSTRACT& thumb = view->documentAbstractFromIndex(vopt->index);
    bool bFocused = view->hasFocus();

    Utils::StyleHelper::drawListViewItemSeperator(&p, rc);
    Utils::StyleHelper::drawListViewItemBackground(&p, rc, view->hasFocus(), vopt->state & QStyle::State_Selected);

    QRect textRect(rc);

    if (!thumb.image.isNull()) {
        QPixmap pm = QPixmap::fromImage(thumb.image);
        QRect rcp = Utils::StyleHelper::drawThumbnailPixmap(&p, textRect, pm);

        textRect.setRight(rcp.left());
    }

    // draw the text
    if (!vopt->text.isEmpty()) {
        textRect.adjust(6, 6, -6, -6);

        QColor colorTitle, colorDate, colorSummary;
        if (vopt->state & QStyle::State_Selected) {
            if (bFocused) {
                colorTitle = m_colorDocumentsTitleFocus;
                colorDate  = m_colorDocumentsDateFocus;
                colorSummary = m_colorDocumentsSummaryFocus;
            } else {
                colorTitle = m_colorDocumentsTitleLoseFocus;
                colorDate = m_colorDocumentsDateLoseFocus;
                colorSummary = m_colorDocumentsSummaryLoseFocus;
            }
        } else {
            colorTitle = m_colorDocumentsTitle;
            colorDate = m_colorDocumentsDate;
            colorSummary = m_colorDocumentsSummary;
        }

        // draw title
        QFont fontTitle = p.font();
        fontTitle.setPixelSize(13);
        fontTitle.setBold(true);
        p.setFont(fontTitle);
        int nFontHeight = p.fontMetrics().height();

        // badge icon first
        QRect rcBadge = textRect;
        rcBadge.setSize(QSize(nFontHeight, nFontHeight));
        const QIcon& badge = data.doc.nProtected ? m_iconDocumentsBadgeEncrypted : m_iconDocumentsBadge;
        if (vopt->state & QStyle::State_Selected && bFocused) {
            badge.paint(&p, rcBadge, Qt::AlignCenter, QIcon::Active, QIcon::Off);
        } else {
            badge.paint(&p, rcBadge, Qt::AlignCenter, QIcon::Normal, QIcon::Off);
        }

        QRect rcTitle(QPoint(rcBadge.right(), textRect.top()), QPoint(textRect.right(), rcBadge.bottom()));
        QString strTitle = data.doc.strTitle;
        //::WizDrawTextSingleLine(&p, rcTitle, strTitle,  Qt::TextSingleLine | Qt::AlignVCenter, colorTitle, true);
        Utils::StyleHelper::drawText(&p, rcTitle, strTitle, 1, Qt::AlignVCenter, colorTitle, fontTitle);

        // draw date and tags, use 12px font size
        QFont fontAbs = p.font();
        fontAbs.setPixelSize(12);
        fontAbs.setBold(false);
        p.setFont(fontAbs);
        nFontHeight = p.fontMetrics().height();

        QRect rcInfo(textRect.left(), rcTitle.bottom() + 6, textRect.width(), nFontHeight);
        QString strInfo = data.strInfo;
        int infoWidth = ::WizDrawTextSingleLine(&p, rcInfo, strInfo,  Qt::TextSingleLine | Qt::AlignVCenter, colorDate, true);

        // there lines document summary
        QString strAbstract = thumb.text;

        int nLineSpacing = Utils::StyleHelper::lineSpacing();

        QRect rcAbstract1(QPoint(textRect.left() + infoWidth + 4, rcInfo.top()), QPoint(textRect.right(), rcInfo.bottom()));
        ::WizDrawTextSingleLine(&p, rcAbstract1, strAbstract, Qt::TextSingleLine | Qt::AlignVCenter, colorSummary, false);

        QRect rcAbstract2(textRect.left(), rcAbstract1.bottom() + nLineSpacing, textRect.width(), nFontHeight);
        ::WizDrawTextSingleLine(&p, rcAbstract2, strAbstract, Qt::TextSingleLine | Qt::AlignVCenter, colorSummary, false);

        QRect rcAbstract3(textRect.left(), rcAbstract2.bottom() + nLineSpacing, textRect.width(), nFontHeight);
        ::WizDrawTextSingleLine(&p, rcAbstract3, strAbstract, Qt::TextSingleLine | Qt::AlignVCenter, colorSummary, true);
    }

    return pixmap;
}

void CWizNoteStyle::drawItemPrivateThumbnail(const QStyleOptionViewItemV4* vopt,
                                             QPainter* p,
                                             const CWizDocumentListView* view) const
{
    const WizDocumentListViewItemData& data = view->documentItemDataFromIndex(vopt->index);

    QString state;
    if (vopt->state & QStyle::State_Selected) {
        if (view->hasFocus()) {
            state = "focus";
        } else {
            state = "nofocus";
        }
    } else {
        state = "normal";
    }

    QString strKey = QString("%1:%2").arg(data.doc.strGUID).arg(state);
    QPixmap pm;
    if (!QPixmapCache::find(strKey, &pm)) {
        pm = genThumbnailPixmap(vopt, view);
        if (!QPixmapCache::insert(strKey, pm)) {
            qDebug() << "Failed insert thumbnail to QPixmapCache while drawing document list";
        }
    }

    p->save();
    p->setClipRect(vopt->rect);
    p->drawPixmap(vopt->rect, pm);
    p->restore();
}

void CWizNoteStyle::drawItemPrivateTwoLine(const QStyleOptionViewItemV4* vopt,
                                           QPainter* p,
                                           const CWizDocumentListView* view) const
{
    const WizDocumentListViewItemData& data = view->documentItemDataFromIndex(vopt->index);
    bool bFocused = view->hasFocus();

    p->save();
    p->setClipRect(vopt->rect);

    // seperator line
    QRect textLine = vopt->rect;
    textLine.adjust(5, 0, -5, 0);
    p->setPen(m_colorDocumentsLine);
    p->drawLine(textLine.bottomLeft(), textLine.bottomRight());

    // draw background behaviour
    if (vopt->state & QStyle::State_Selected) {
        if (bFocused) {
            p->fillRect(vopt->rect, m_colorDocumentsItemFocusBackground);
        } else {
            p->fillRect(vopt->rect, m_colorDocumentsItemLoseFocusBackground);
        }
    }

    QRect textRect = vopt->rect;

    // draw the text
    if (!vopt->text.isEmpty()) {
        textRect.adjust(6, 6, -6, -6);

        QColor colorTitle, colorDate;
        if (vopt->state & QStyle::State_Selected) {
            if (bFocused) {
                colorTitle = m_colorDocumentsTitleFocus;
                colorDate  = m_colorDocumentsDateFocus;
            } else {
                colorTitle = m_colorDocumentsTitleLoseFocus;
                colorDate = m_colorDocumentsDateLoseFocus;
            }
        } else {
            colorTitle = m_colorDocumentsTitle;
            colorDate = m_colorDocumentsDate;
        }

        // draw title
        QFont fontTitle = p->font();
        fontTitle.setPixelSize(13);
        p->setFont(fontTitle);
        int nFontHeight = p->fontMetrics().height();

        // badge icon first
        QRect rcBadge = textRect;
        rcBadge.setSize(QSize(nFontHeight, nFontHeight));
        const QIcon& badge = data.doc.nProtected ? m_iconDocumentsBadgeEncrypted : m_iconDocumentsBadge;
        if (vopt->state & QStyle::State_Selected && bFocused) {
            badge.paint(p, rcBadge, Qt::AlignCenter, QIcon::Active, QIcon::Off);
        } else {
            badge.paint(p, rcBadge, Qt::AlignCenter, QIcon::Normal, QIcon::Off);
        }

        QRect rcTitle(QPoint(rcBadge.right() + 5, rcBadge.top()), QPoint(textRect.right(), rcBadge.bottom()));
        QString strTitle = data.doc.strTitle;
        ::WizDrawTextSingleLine(p, rcTitle, strTitle, Qt::TextSingleLine | Qt::AlignVCenter, colorTitle, true);

        // draw date and tags, use 12px font size
        QFont fontAbs = p->font();
        fontAbs.setPixelSize(12);
        p->setFont(fontAbs);
        nFontHeight = p->fontMetrics().height();

        QRect rcInfo(textRect.left(), rcTitle.bottom() + 6, textRect.width(), nFontHeight);
        QString strInfo = data.strInfo;
        ::WizDrawTextSingleLine(p, rcInfo, strInfo,  Qt::TextSingleLine | Qt::AlignVCenter, colorDate, true);
    }

    p->restore();
}

//void CWizNoteStyle::drawItemGroupThumbnail(const QStyleOptionViewItemV4* vopt,
//                                           QPainter* p,
//                                           const CWizDocumentListView* view) const
//{
//    // indirect access
//    const WizDocumentListViewItemData& data = view->documentItemDataFromIndex(vopt->index);
//    const WIZABSTRACT& abstract = view->documentAbstractFromIndex(vopt->index);
//    //const QImage& imgAuthorAvatar = view->messageSenderAvatarFromIndex(vopt->index);
//    WIZDOCUMENTDATA document = view->documentFromIndex(vopt->index);
//    bool bFocused =  view->hasFocus();
//
//    p->save();
//    p->setClipRect(vopt->rect);
//
//    // seperator line
//    QRect textLine = vopt->rect;
//    textLine.adjust(5, 0, -5, 0);
//    p->setPen(m_colorDocumentsLine);
//    p->drawLine(textLine.bottomLeft(), textLine.bottomRight());
//
//    // draw background behaviour
//    if (vopt->state & QStyle::State_Selected) {
//        if (bFocused) {
//            p->fillRect(vopt->rect, m_colorDocumentsItemFocusBackground);
//        } else {
//            p->fillRect(vopt->rect, m_colorDocumentsItemLoseFocusBackground);
//        }
//    }
//
//    QRect textRect = vopt->rect.adjusted(5, 5, 0, 0);
//
//    // draw author avatar
//    int nAvatarHeight = Utils::StyleHelper::avatarHeight();
//    QRect rectAvatar = textRect;
//    rectAvatar.setSize(QSize(nAvatarHeight, nAvatarHeight));
//
//    p->save();
//    p->setRenderHint(QPainter::Antialiasing);
//    QPixmap pmAvatar;
//    WizService::Internal::AvatarHost::avatar(data.strAuthorId, &pmAvatar);
//    p->drawPixmap(rectAvatar, pmAvatar);
//
//    //if (!imgAuthorAvatar.isNull()) {
//    //    p->drawImage(rectAvatar, imgAuthorAvatar);
//    //} else {
//    //    p->drawImage(rectAvatar, m_imgDefaultAvatar);
//    //}
//    p->restore();
//
//    textRect.setLeft(rectAvatar.right());
//
//    // draw the text
//    if (!vopt->text.isEmpty()) {
//        textRect.adjust(5, 5, -5, -5);
//
//        QColor colorTitle, colorDate, colorSummary;
//        if (vopt->state & QStyle::State_Selected) {
//            if (bFocused) {
//                colorTitle = m_colorDocumentsTitleFocus;
//                colorDate  = m_colorDocumentsDateFocus;
//                colorSummary = m_colorDocumentsSummaryFocus;
//            } else {
//                colorTitle = m_colorDocumentsTitleLoseFocus;
//                colorDate = m_colorDocumentsDateLoseFocus;
//                colorSummary = m_colorDocumentsSummaryLoseFocus;
//            }
//        } else {
//            colorTitle = m_colorDocumentsTitle;
//            colorDate = m_colorDocumentsDate;
//            colorSummary = m_colorDocumentsSummary;
//        }
//
//        // draw title
//        QFont fontTitle = p->font();
//        fontTitle.setPixelSize(13);
//        fontTitle.setBold(true);
//        p->setFont(fontTitle);
//        int nFontHeight = p->fontMetrics().height();
//
//        // badge icon first
//        QRect rcBadge = textRect;
//        rcBadge.setSize(QSize(nFontHeight, nFontHeight));
//        if (vopt->state & QStyle::State_Selected && bFocused) {
//            m_iconDocumentsBadge.paint(p, rcBadge, Qt::AlignCenter, QIcon::Active, QIcon::Off);
//        } else {
//            m_iconDocumentsBadge.paint(p, rcBadge, Qt::AlignCenter, QIcon::Normal, QIcon::Off);
//        }
//
//        QRect rcTitle(QPoint(rcBadge.right() + 5, rcBadge.top()), QPoint(textRect.right(), rcBadge.bottom()));
//        ::WizDrawTextSingleLine(p, rcTitle, document.strTitle,  Qt::TextSingleLine | Qt::AlignVCenter, colorTitle, true);
//
//        // draw date and tags, use 12px font size
//        QFont fontAbs = p->font();
//        fontAbs.setPixelSize(12);
//        fontAbs.setBold(false);
//        p->setFont(fontAbs);
//        nFontHeight = p->fontMetrics().height();
//
//        QRect rcInfo(textRect.left(), rectAvatar.bottom() - nFontHeight, textRect.width(), nFontHeight);
//        QString strInfo = data.strInfo;
//        int infoWidth = ::WizDrawTextSingleLine(p, rcInfo, strInfo,  Qt::TextSingleLine | Qt::AlignVCenter, colorDate, true);
//
//        // there lines document summary
//        QString strAbstract = abstract.text;
//
//        int nLineSpacing = Utils::StyleHelper::lineSpacing();
//
//        QRect rcAbstract1(QPoint(textRect.left() + infoWidth + 4, rcInfo.top()), QPoint(textRect.right(), rcInfo.bottom()));
//        ::WizDrawTextSingleLine(p, rcAbstract1, strAbstract, Qt::TextSingleLine | Qt::AlignVCenter, colorSummary, false);
//
//        QRect rcAbstract2(vopt->rect.left() + 6, rcAbstract1.bottom() + nLineSpacing, vopt->rect.width() - 12, nFontHeight);
//        ::WizDrawTextSingleLine(p, rcAbstract2, strAbstract, Qt::TextSingleLine | Qt::AlignVCenter, colorSummary, false);
//
//        QRect rcAbstract3(vopt->rect.left() + 6, rcAbstract2.bottom() + nLineSpacing, vopt->rect.width() - 12, nFontHeight);
//        ::WizDrawTextSingleLine(p, rcAbstract3, strAbstract, Qt::TextSingleLine | Qt::AlignVCenter, colorSummary, true);
//    }
//
//    p->restore();
//}

void CWizNoteStyle::drawItemGroupTwoLine(const QStyleOptionViewItemV4* vopt,
                                         QPainter* p,
                                         const CWizDocumentListView* view) const
{
    // indirect access
    const WizDocumentListViewItemData& data = view->documentItemDataFromIndex(vopt->index);
    //const QImage& imgAuthorAvatar = view->messageSenderAvatarFromIndex(vopt->index);
    bool bFocused = view->hasFocus();

    p->save();
    p->setClipRect(vopt->rect);

    // seperator line
    QRect textLine = vopt->rect;
    textLine.adjust(5, 0, -5, 0);
    p->setPen(m_colorDocumentsLine);
    p->drawLine(textLine.bottomLeft(), textLine.bottomRight());

    // draw background behaviour
    if (vopt->state & QStyle::State_Selected) {
        if (bFocused) {
            p->fillRect(vopt->rect, m_colorDocumentsItemFocusBackground);
        } else {
            p->fillRect(vopt->rect, m_colorDocumentsItemLoseFocusBackground);
        }
    }

    QRect textRect = vopt->rect;

    // draw author avatar
    QRect rectAvatar = textRect;
    rectAvatar.setSize(QSize(rectAvatar.height(), rectAvatar.height()));
    rectAvatar.adjust(6, 6 , -6, -6);

    p->save();
    p->setRenderHint(QPainter::HighQualityAntialiasing);
    QPixmap pmAvatar;
    WizService::Internal::AvatarHost::avatar(data.strAuthorId, &pmAvatar);
    p->drawPixmap(rectAvatar, pmAvatar);

    //if (!imgAuthorAvatar.isNull()) {
    //    p->drawImage(rectAvatar, imgAuthorAvatar);
    //} else {
    //    p->drawImage(rectAvatar, m_imgDefaultAvatar);
    //}
    p->restore();

    textRect.setLeft(rectAvatar.right());

    // draw the text
    if (!vopt->text.isEmpty()) {
        textRect.adjust(6, 6, -6, -6);

        QColor colorTitle, colorDate;
        if (vopt->state & QStyle::State_Selected) {
            if (bFocused) {
                colorTitle = m_colorDocumentsTitleFocus;
                colorDate  = m_colorDocumentsDateFocus;
            } else {
                colorTitle = m_colorDocumentsTitleLoseFocus;
                colorDate = m_colorDocumentsDateLoseFocus;
            }
        } else {
            colorTitle = m_colorDocumentsTitle;
            colorDate = m_colorDocumentsDate;
        }

        // draw title
        QFont fontTitle = p->font();
        fontTitle.setPixelSize(13);
        p->setFont(fontTitle);
        int nFontHeight = p->fontMetrics().height();

        // badge icon first
        QRect rcBadge = textRect;
        rcBadge.setSize(QSize(nFontHeight, nFontHeight));
        if (vopt->state & QStyle::State_Selected && bFocused) {
            m_iconDocumentsBadge.paint(p, rcBadge, Qt::AlignCenter, QIcon::Active, QIcon::Off);
        } else {
            m_iconDocumentsBadge.paint(p, rcBadge, Qt::AlignCenter, QIcon::Normal, QIcon::Off);
        }

        QRect rcTitle(QPoint(rcBadge.right() + 5, rcBadge.top()), QPoint(textRect.right(), rcBadge.bottom()));
        QString strTitle = data.doc.strTitle;
        ::WizDrawTextSingleLine(p, rcTitle, strTitle,  Qt::TextSingleLine | Qt::AlignVCenter, colorTitle, true);

        // draw date and tags, use 12px font size
        QFont fontAbs = p->font();
        fontAbs.setPixelSize(12);
        p->setFont(fontAbs);
        nFontHeight = p->fontMetrics().height();

        QRect rcInfo(textRect.left(), rcTitle.bottom() + 6, textRect.width(), nFontHeight);
        QString strInfo =  data.strInfo;
        ::WizDrawTextSingleLine(p, rcInfo, strInfo,  Qt::TextSingleLine | Qt::AlignVCenter, colorDate, true);
    }

    p->restore();
}

void CWizNoteStyle::drawItemOneLine(const QStyleOptionViewItemV4* vopt,
                                    QPainter* p,
                                    const CWizDocumentListView* view) const
{
    // indirect access
    WIZDOCUMENTDATA document = view->documentFromIndex(vopt->index);
    bool bFocused = view->hasFocus();

    p->save();
    p->setClipRect(vopt->rect);

    Utils::StyleHelper::drawListViewItemSeperator(p, vopt->rect);
    Utils::StyleHelper::drawListViewItemBackground(p, vopt->rect, bFocused, vopt->state & State_Selected);

    QRect textRect = vopt->rect;

    // draw the text
    if (!vopt->text.isEmpty()) {
        //textRect.adjust(6, 6, -6, -6);

        QColor colorTitle;
        if (vopt->state & QStyle::State_Selected) {
            if (bFocused) {
                colorTitle = m_colorDocumentsTitleFocus;
            } else {
                colorTitle = m_colorDocumentsTitleLoseFocus;
            }
        } else {
            colorTitle = m_colorDocumentsTitle;
        }

        // draw title
        QFont fontTitle = p->font();
        fontTitle.setPixelSize(13);
        p->setFont(fontTitle);
        int nFontHeight = p->fontMetrics().height();

        int nType = document.nProtected ? Utils::StyleHelper::BadgeEncryted : Utils::StyleHelper::BadgeNormal;
        QRect rcBadge = Utils::StyleHelper::drawBadgeIcon(p, textRect, nFontHeight, nType, bFocused, vopt->state & State_Selected);

        int nMargin = Utils::StyleHelper::margin();
        textRect.adjust(rcBadge.width() + nMargin, nMargin, 0, 0);
        QString strTitle = document.strTitle;
        Utils::StyleHelper::drawText(p, textRect, strTitle, 1, Qt::TextSingleLine | Qt::AlignVCenter, colorTitle, p->font());
    }

    p->restore();
}

void CWizNoteStyle::drawMultiLineListWidgetItem(const QStyleOptionViewItemV4 *vopt, QPainter *p, const CWizMultiLineListWidget *view) const
{
    bool imageAlignLeft = view->imageAlignLeft();
    int imageWidth = view->imageWidth();
    int lineCount = view->lineCount();
    int wrapTextLineText = view->wrapTextLineIndex();
    const QPixmap img = view->itemImage(vopt->index);

    p->save();
    p->setClipRect(vopt->rect);

    QRect textLine = vopt->rect;
    textLine.adjust(4, 0, -4, 0);
    p->setPen(m_colorDocumentsLine);
    p->drawLine(textLine.bottomLeft(), textLine.bottomRight());

    QRect textRect = vopt->rect;
    //QRect textRect = subElementRect(SE_ItemViewItemText, vopt, view);

    // draw the background
    if (vopt->state.testFlag(State_Selected))
    {
        m_multiLineListSelectedItemBackground.Draw(p, vopt->rect, 0);
    }

    if (!img.isNull() && img.width() > 0 && img.height() > 0)
    {
        QRect imageRect = textRect;
        if (imageAlignLeft)
        {
            imageRect.setRight(imageRect.left() + imageWidth + 16);
            imageRect.adjust(4, 4, -4, -4);
            textRect.setLeft(imageRect.right());
        }
        else
        {
            imageRect.setLeft(imageRect.right() - imageWidth + 16);
            imageRect.adjust(4, 4, -4, -4);
            textRect.setRight(imageRect.left());
        }

        if (img.width() > imageRect.width() || img.height() > imageRect.height())
        {
            double fRate = std::min<double>(double(imageRect.width()) / img.width(), double(imageRect.height()) / img.height());
            int newWidth = int(img.width() * fRate);
            int newHeight = int(img.height() * fRate);
            //
            int adjustX = (imageRect.width() - newWidth) / 2;
            int adjustY = (imageRect.height() - newHeight) / 2;
            imageRect.adjust(adjustX, adjustY, -adjustX, -adjustY);
        }
        else
        {
            int adjustX = (imageRect.width() - img.width()) / 2;
            int adjustY = (imageRect.height() - img.height()) / 2;
            imageRect.adjust(adjustX, adjustY, -adjustX, -adjustY);
        }
        p->drawPixmap(imageRect, img);
    }

    // draw the text
    QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled
                              ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
        cg = QPalette::Inactive;

    if (vopt->state & QStyle::State_Selected) {
        p->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
    } else {
        p->setPen(vopt->palette.color(cg, QPalette::Text));
    }
    if (vopt->state & QStyle::State_Editing) {
        p->setPen(vopt->palette.color(cg, QPalette::Text));
        p->drawRect(textRect.adjusted(0, 0, -1, -1));
    }

    textRect.adjust(8, 8, -8, -8);
    bool selected = vopt->state.testFlag(State_Selected);
    int lineHeight = vopt->fontMetrics.height() + 2;

    for (int line = 0; line < wrapTextLineText && line < lineCount; line++)
    {
        QColor color = (0 == line) ? (selected ? m_colorMultiLineListFirstLineSelected : m_colorMultiLineListFirstLine)
            : (selected ? m_colorMultiLineListOtherLineSelected : m_colorMultiLineListOtherLine);
        //
        CString strText = view->itemText(vopt->index, line);
        color = view->itemTextColor(vopt->index, line, selected, color);
        QRect rc = textRect;
        rc.setTop(rc.top() + line * lineHeight);
        rc.setHeight(lineHeight);
        ::WizDrawTextSingleLine(p, rc, strText,  Qt::TextSingleLine | Qt::AlignVCenter, color, true);
    }

    int line = wrapTextLineText;
    if (line < lineCount)
    {
        CString strText = view->itemText(vopt->index, line);
        for (; line < lineCount; line++)
        {
            QColor color = selected ? m_colorMultiLineListOtherLineSelected : m_colorMultiLineListOtherLine;
            //
            color = view->itemTextColor(vopt->index, line, selected, color);
            QRect rc = textRect;
            rc.setTop(rc.top() + line * lineHeight);
            rc.setHeight(lineHeight);
            bool elidedText = (line == lineCount - 1);
            ::WizDrawTextSingleLine(p, rc, strText,  Qt::TextSingleLine | Qt::AlignVCenter, color, elidedText);
        }
    }

    // draw the focus rect
    if (vopt->state & QStyle::State_HasFocus) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(*vopt);
        o.rect = subElementRect(SE_ItemViewItemFocusRect, vopt, view);
        o.state |= QStyle::State_KeyboardFocusChange;
        o.state |= QStyle::State_Item;
        QPalette::ColorGroup cg = (vopt->state & QStyle::State_Enabled)
                                  ? QPalette::Normal : QPalette::Disabled;
        o.backgroundColor = vopt->palette.color(cg, (vopt->state & QStyle::State_Selected)
                                                ? QPalette::Highlight : QPalette::Window);
        proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, view);
    }

    p->restore();
}


//void CWizNoteStyle::drawImagePushButton(const QStyleOptionButton *option, QPainter *painter, const CWizImagePushButton *button) const
//{
//    bool disabled = false;
//    bool pressed = false;
//    bool hot = false;
//    //
//    State flags = option->state;
//    if (!(flags & State_Enabled))
//        disabled = true;
//    else if (flags & (State_Sunken | State_On))
//        pressed = true;
//    else if (flags & State_MouseOver)
//        hot = true;
//    //
//    QRect rect(option->rect);
//    QRect rectImage = rect;
//    //
//    CString label = button->text();
//    if (!label.isEmpty())
//    {
//        rectImage.setWidth(std::min<int>(16 + 16, rect.width()));
//    }
//    //
//    if (disabled)
//    {
//        m_imagePushButtonDisabled.Draw(painter, rectImage, 0);
//    }
//    else if (pressed)
//    {
//        m_imagePushButtonPressed.Draw(painter, rectImage, 0);
//    }
//    else if (hot)
//    {
//        m_imagePushButtonHot.Draw(painter, rectImage, 0);
//    }
//    else
//    {
//        m_imagePushButton.Draw(painter, rectImage, 0);
//    }
//    //
//    QIcon::Mode mode = disabled ? QIcon::Disabled : QIcon::Normal;
//    QPixmap pixmap = button->icon().pixmap(16, 16, mode);
//    if (!pixmap.isNull())
//    {
//        QPoint pt = rectImage.center();
//        pt.setX(pt.x() - pixmap.width() / 2 + 1);
//        pt.setY(pt.y() - pixmap.height() / 2 + 1);
//        //
//        painter->drawPixmap(pt, pixmap);
//    }
//    //
//    if (!label.isEmpty())
//    {
//        QRect rectLabel = rect;
//        rectLabel.setLeft(rectImage.right() - 8);
//        rectLabel.setHeight(m_imagePushButtonLabel.actualSize().height());
//        //
//        if (button->redFlag())
//        {
//            m_imagePushButtonLabelRed.Draw(painter, rectLabel, 0);
//        }
//        else
//        {
//            m_imagePushButtonLabel.Draw(painter, rectLabel, 0);
//        }
//        //
//        QRect rectLabelText = rectLabel;
//        rectLabelText.setRight(rectLabelText.right() - 2);    //shadow
//        rectLabelText.setBottom(rectLabelText.bottom() - 4);    //shadow
//        painter->save();
//        painter->setFont(m_fontImagePushButtonLabel);
//        ::WizDrawTextSingleLine(painter, rectLabelText, label,  Qt::TextSingleLine | Qt::AlignVCenter | Qt::AlignCenter, QColor(0xff, 0xff, 0xff), false);
//        painter->restore();
//    }
//}

//void CWizNoteStyle::drawSplitter(const QStyleOption *option, QPainter *painter, const QSplitter *splitter) const
//{
//    QRect rectSplitter = option->rect;
//    qDebug() << rectSplitter.x() << ":" << rectSplitter.y() << ":" << rectSplitter.width() << ":" << rectSplitter.height();
//    //m_splitterShadowImage.Draw(painter, , 255);
//}

void CWizNoteStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element)
    {
    case CE_ItemViewItem:
        {
            const QStyleOptionViewItemV4 *vopt = qstyleoption_cast<const QStyleOptionViewItemV4 *>(option);
            Q_ASSERT(vopt);

            if (const MessageListView* view = dynamic_cast<const MessageListView*>(widget))
            {
                view->drawItem(painter, vopt);
                //drawMessageListViewItem(vopt, painter, view);
            }
            else if (const CWizDocumentListView *view = dynamic_cast<const CWizDocumentListView *>(widget))
            {
                drawDocumentListViewItem(vopt, painter, view);
            }
            else if (const CWizMultiLineListWidget *view = dynamic_cast<const CWizMultiLineListWidget *>(widget))
            {
                drawMultiLineListWidgetItem(vopt, painter, view);
            }
            else if (const CWizCategoryBaseView *view = dynamic_cast<const CWizCategoryBaseView *>(widget))
            {
                if (view->isDragHovered() && view->validateDropDestination(view->dragHoveredPos())) {
                    QRect rect = view->visualItemRect(view->itemAt(view->dragHoveredPos()));
                    QPen pen;
                    pen.setStyle(Qt::SolidLine);
                    pen.setCapStyle(Qt::RoundCap);
                    pen.setColor(Qt::blue);
                    pen.setWidth(2);
                    painter->setPen(pen);
                    rect.setWidth(rect.width() - 2);
                    painter->drawRect(rect);
                }

                drawCategoryViewItem(vopt, painter, view);
            }
            else
            {
                CWizNoteBaseStyle::drawControl(element, option, painter, widget);
            }
            break;
        }
    //case CE_PushButton:
    //    {
    //        const QStyleOptionButton* vopt = qstyleoption_cast<const QStyleOptionButton *>(option);
    //        ATLASSERT(vopt);
    //        //
    //        if (const CWizImagePushButton *button = dynamic_cast<const CWizImagePushButton *>(widget))
    //        {
    //            drawImagePushButton(vopt, painter, button);
    //        }
    //        else
    //        {
    //            CWizNoteBaseStyle::drawControl(element, option, painter, widget);
    //        }
    //    }
    //    break;
    //case CE_Splitter:
    //{
    //    if (const QSplitter* splitter = dynamic_cast<const QSplitter *>(widget))
    //    {
    //        drawSplitter(option, painter, splitter);
    //    } else {
    //        CWizNoteBaseStyle::drawControl(element, option, painter, widget);
    //    }
    //}

    default:
        CWizNoteBaseStyle::drawControl(element, option, painter, widget);
        break;
    }
}



void CWizNoteStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                                  const QWidget *w) const
{
    switch (pe)
    {
    case PE_IndicatorBranch:
        {
            if (const CWizCategoryBaseView *view = dynamic_cast<const CWizCategoryBaseView *>(w))
            {
                Q_UNUSED(view);

                if (opt->state & QStyle::State_Children) {
                    bool bExpanded = (opt->state & QStyle::State_Open) ? true : false;
                    drawcenterImage(p, bExpanded ? m_expandedImage : m_collapsedImage, opt->rect.adjusted(8, 0, 0, 0));
                }
                return;
            }
        }
        break;
    case PE_PanelItemViewRow:
        {
            if (const CWizCategoryBaseView *view = dynamic_cast<const CWizCategoryBaseView *>(w))
            {
                const QStyleOptionViewItemV4 *vopt = qstyleoption_cast<const QStyleOptionViewItemV4 *>(opt);
                Q_ASSERT(vopt);

                const QTreeWidgetItem* pItemBase = view->itemAt(vopt->rect.center());
                if (NULL != dynamic_cast<const CWizCategoryViewCategoryItem *>(pItemBase)) {
                    p->fillRect(vopt->rect, QColor(255, 255, 255, 15)); //FIXME
                    //drawCategoryViewItemCategoryItem(pItem, vopt, p);
                    return;
                }

                if (opt->state & QStyle::State_Selected)
                {
                    QRect rect = opt->rect;
                    rect.setWidth(p->window().width());
                    if (opt->state & QStyle::State_HasFocus) {
                        p->fillRect(rect, m_colorCategorySelectedBackground);
                    } else {
                        p->fillRect(rect, m_colorCategorySelctedBackgroundNoFocus);
                        rect.setWidth(5); // FIXME
                        p->fillRect(rect, m_colorCategorySelectedBackground);
                    }
                }
            }

            return;
        }
        break;
    default:
        break;
    }
    CWizNoteBaseStyle::drawPrimitive(pe, opt, p, w);
}

int	CWizNoteStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    switch (metric)
    {
        case PM_SplitterWidth:
            return 20;
        case PM_ScrollBarExtent:
            return 3;
        default:
            return CWizNoteBaseStyle::pixelMetric(metric, option, widget);
    }
}

void CWizNoteStyle::drawcenterImage(QPainter* p, const QImage& image, const QRect& rc) const
{
    int width = image.width();
    int height = image.height();

    int x = rc.left() + (rc.width() - width) / 2;
    int y = rc.top() + (rc.height() - height) / 2;

    p->drawImage(x, y, image);
}

CWizNoteStyle* CWizNoteStyle::noteStyle(const QString& skinName)
{
    static CWizNoteStyle style(skinName);
    return &style;
}

QStyle* WizGetStyle(const QString& skinName)
{
    return CWizNoteStyle::noteStyle(skinName);
}

QColor WizGetCategoryBackroundColor(const QString& skinName)
{
    return CWizNoteStyle::noteStyle(skinName)->categoryBackground();
}

QColor WizGetDocumentsBackroundColor(const QString& skinName)
{
    return CWizNoteStyle::noteStyle(skinName)->documentsBackground();
}

QColor WizGetClientBackgroundColor(const QString& strSkinName)
{
    CWizSettings settings(::WizGetSkinResourcePath(strSkinName) + "skin.ini");
    return settings.GetColor("Client", "Background", QColor(0x80, 0x80, 0x80));
}
