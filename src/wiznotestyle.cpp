#include "wiznotestyle.h"

#include <QProxyStyle>
#include <QPainter>

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
    QImage m_expandedImageSelected;
    QImage m_collapsedImageSelected;
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

    QFont m_fontImagePushButtonLabel;
    QFont m_fontLink;

protected:
    virtual void drawCategoryViewItem(const QStyleOptionViewItemV4 *option, QPainter *painter, const CWizCategoryBaseView *widget) const;
    virtual void drawMultiLineListWidgetItem(const QStyleOptionViewItemV4 *option, QPainter *painter, const CWizMultiLineListWidget *widget) const;
    void drawcenterImage(QPainter* p, const QImage& image, const QRect& rc) const;

public:
    virtual void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    virtual void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const;
    virtual int	pixelMetric(PixelMetric metric, const QStyleOption* option = 0, const QWidget* widget = 0 ) const;

public:
    static CWizNoteStyle* noteStyle(const QString& skinName);
};


const int IMAGE_WIDTH = 80;

CWizNoteStyle::CWizNoteStyle(const QString& strSkinName)
{
    if (!strSkinName.isEmpty())
    {
        QString strSkinPath = ::WizGetSkinResourcePath(strSkinName);

        m_expandedImage.load(strSkinPath + "branch_expanded.png");
        m_collapsedImage.load(strSkinPath + "branch_collapsed.png");
        m_expandedImageSelected.load(strSkinPath + "branch_expandedSelected.png");
        m_collapsedImageSelected.load(strSkinPath + "branch_collapsedSelected.png");
        m_imgDocumentUnread.load(strSkinPath + "read_btn_unread.png");
        m_imgDefaultAvatar.load(strSkinPath + "avatar_default.png");

        //m_iconDocumentsBadge = ::WizLoadSkinIcon(strSkinName, "document_badge");
        //m_iconDocumentsBadgeEncrypted = ::WizLoadSkinIcon(strSkinName, "document_badge_encrypted");

        m_multiLineListSelectedItemBackground.SetImage(strSkinPath + "multilinelist_selected_background.png", QPoint(4, 4));
        m_multiLineListSelectedItemBackgroundHot.SetImage(strSkinPath + "multilinelist_selected_background_hot.png", QPoint(4, 4));
        m_imagePushButton.SetImage(strSkinPath + "imagepushbutton.png", QPoint(4, 4));
        m_imagePushButtonHot.SetImage(strSkinPath + "imagepushbutton_hot.png", QPoint(4, 4));
        m_imagePushButtonPressed.SetImage(strSkinPath + "imagepushbutton_pressed.png", QPoint(4, 4));
        m_imagePushButtonDisabled.SetImage(strSkinPath + "imagepushbutton_disabled.png", QPoint(4, 4));
        m_imagePushButtonLabel.SetImage(strSkinPath + "imagepushbutton_label.png", QPoint(8, 8));
        m_imagePushButtonLabelRed.SetImage(strSkinPath + "imagepushbutton_label_red.png", QPoint(8, 8));
    }

#ifdef Q_OS_MAC
    m_fontImagePushButtonLabel = QFont("Arial Black", 9);
#else
    m_fontImagePushButtonLabel = QFont("Arial Black", 8);
#endif
    m_fontImagePushButtonLabel.setBold(true);
    //
    m_fontLink.setItalic(true);
    //m_fontLink.setUnderline(true);
    m_fontLink.setPixelSize(m_fontLink.pixelSize() - 4);
}



void CWizNoteStyle::drawCategoryViewItem(const QStyleOptionViewItemV4 *vopt,
                                         QPainter *p, const CWizCategoryBaseView *view) const
{
    if (view->isDragHovered() && view->validateDropDestination(view->dragHoveredPos())) {
        QRect rect = view->visualItemRect(view->itemAt(view->dragHoveredPos()));
        QPen pen;
        pen.setStyle(Qt::SolidLine);
        pen.setCapStyle(Qt::RoundCap);
        pen.setColor(QColor("#3498DB"));
        pen.setWidth(1);
        p->setPen(pen);
        rect.setWidth(rect.width() - 2);
        p->drawRect(rect);
    }

    CWizCategoryViewItemBase* pItem = view->categoryItemFromIndex(vopt->index);

    if (view->isHelperItemByIndex(vopt->index)) {
        if (NULL != dynamic_cast<const CWizCategoryViewSectionItem*>(pItem)) {
            QString str = vopt->text;
            QRect rc(vopt->rect);
            rc.setTop(rc.top() + 12);
            Utils::StyleHelper::drawSingleLineText(p, rc, str, Qt::AlignVCenter, Utils::StyleHelper::treeViewItemCategoryText(), p->font());
        }
        else if (NULL != dynamic_cast<const CWizCategoryViewLinkItem*>(pItem)) {
            QString str = vopt->text;
            QRect rc(vopt->rect);
            rc.setLeft(rc.left() + 16);
            Utils::StyleHelper::drawSingleLineText(p, rc, str, Qt::AlignVCenter, Utils::StyleHelper::treeViewItemLinkText(), m_fontLink);
        }

        return;
    }

    p->save();

    bool bSelected = vopt->state.testFlag(State_Selected);
    int nLeftMargin = 4;
    if (!vopt->icon.isNull()) {
        QRect iconRect = subElementRect(SE_ItemViewItemDecoration, vopt, view);
        iconRect.adjust(nLeftMargin, 0, nLeftMargin, 0);
        Utils::StyleHelper::drawTreeViewItemIcon(p, iconRect, vopt->icon, bSelected && (vopt->state & State_HasFocus));
    }

    QFont f;
    Utils::StyleHelper::fontNormal(f);

    QFont fontCount;
    Utils::StyleHelper::fontExtend(fontCount);

    QRect rcText = subElementRect(SE_ItemViewItemText, vopt, view);
    rcText.adjust(nLeftMargin * 2, 0, nLeftMargin * 2, 0);
    QString strCount = pItem->countString();

    QString strText = vopt->text;
    if (!strText.isEmpty()) {
        QColor colorText = Utils::StyleHelper::treeViewItemText(bSelected && (vopt->state & State_HasFocus));

        p->setPen(colorText);
        int right = Utils::StyleHelper::drawSingleLineText(p, rcText, strText, Qt::AlignVCenter, colorText, f);
        //
        rcText.setLeft(right + 4);
    }

    if (!strCount.isEmpty()) {
        QRect rcCount = rcText;
        rcCount.setTop(rcCount.top() + 1);  //add extra 1 pixel for vcenter / 2
        QColor colorCount = Utils::StyleHelper::treeViewItemTextExtend(bSelected && (vopt->state & State_HasFocus));
        Utils::StyleHelper::drawSingleLineText(p, rcCount, strCount, Qt::AlignVCenter, colorCount, fontCount);
    }

    p->restore();

    // FIXME: this is used for drawing additional badge, please merge it.
    view->categoryItemFromIndex(vopt->index)->draw(p, vopt);
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
    p->setPen(Utils::StyleHelper::listViewItemSeperator());
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
        QColor color = (0 == line) ? Utils::StyleHelper::listViewMultiLineFirstLine(selected)
            : Utils::StyleHelper::listViewMultiLineOtherLine(selected);
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
            QColor color = Utils::StyleHelper::listViewMultiLineOtherLine(selected);
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

    //draw extra image
    QRect rcExtra;
    QPixmap pixExtra;
    if (view->itemExtraImage(vopt->index, vopt->rect, rcExtra, pixExtra))
    {
        p->drawPixmap(rcExtra, pixExtra);
    }

    p->restore();
}

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
                view->drawItem(painter, vopt);
                //drawDocumentListViewItem(vopt, painter, view);
            }
            else if (const CWizMultiLineListWidget *view = dynamic_cast<const CWizMultiLineListWidget *>(widget))
            {
                drawMultiLineListWidgetItem(vopt, painter, view);
            }
            else if (const CWizCategoryBaseView *view = dynamic_cast<const CWizCategoryBaseView *>(widget))
            {
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
                    if ((opt->state & QStyle::State_Selected) && view->hasFocus()) {        //(opt->state & State_HasFocus)
                        drawcenterImage(p, bExpanded ? m_expandedImageSelected : m_collapsedImageSelected, opt->rect.adjusted(8, 0, 0, 0));
                    } else {
                        drawcenterImage(p, bExpanded ? m_expandedImage : m_collapsedImage, opt->rect.adjusted(8, 0, 0, 0));
                    }
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
                const CWizCategoryViewSectionItem *secItem = dynamic_cast<const CWizCategoryViewSectionItem *>(pItemBase);
                if (NULL != secItem) {
                    secItem->draw(p,vopt);
                    return;
                }

                if (opt->state & State_Selected) {
                    QRect rc(vopt->rect);
                    rc.setWidth(p->window().width());
                    Utils::StyleHelper::drawTreeViewItemBackground(p, rc, opt->state & State_HasFocus);
                }

                //if (opt->state & QStyle::State_Selected)
                //{
                //    QRect rect = opt->rect;
                //    rect.setWidth(p->window().width());
                //    if (opt->state & QStyle::State_HasFocus) {
                //        p->fillRect(rect, m_colorCategorySelectedBackground);
                //    } else {
                //        p->fillRect(rect, m_colorCategorySelctedBackgroundNoFocus);
                //        rect.setWidth(5); // FIXME
                //        p->fillRect(rect, m_colorCategorySelectedBackground);
                //    }
                //}
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

class CWizImageButtonStyle : public CWizNoteBaseStyle
{
public:
    CWizImageButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
                         const QString& downBackgroundFileName, const QString& disabledBackgroundFileName,
                         const QColor& normalTextColor, const QColor& activeTextColor, const QColor& disableTextColor)
    {
        m_imagePushButton.SetImage(normalBackgroundFileName, QPoint(4, 4));
        m_imagePushButtonHot.SetImage(hotBackgroundFileName, QPoint(4, 4));
        m_imagePushButtonPressed.SetImage(downBackgroundFileName, QPoint(4, 4));
        m_imagePushButtonDisabled.SetImage(disabledBackgroundFileName, QPoint(4, 4));
        m_colorTextNormal = normalTextColor;
        m_colorTextActive = activeTextColor;
        m_colorTextDisable = disableTextColor;
    }
private:
    CWizSkin9GridImage m_imagePushButton;
    CWizSkin9GridImage m_imagePushButtonHot;
    CWizSkin9GridImage m_imagePushButtonPressed;
    CWizSkin9GridImage m_imagePushButtonDisabled;
    QColor m_colorTextNormal;
    QColor m_colorTextActive;
    QColor m_colorTextDisable;
protected:
    virtual void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        switch (element)
        {
        case CE_PushButton:
            {
                const QStyleOptionButton* vopt = qstyleoption_cast<const QStyleOptionButton *>(option);
                ATLASSERT(vopt);
                //
                if (!vopt->state.testFlag(QStyle::State_Enabled))
                {
                    m_imagePushButtonDisabled.Draw(painter, vopt->rect, 0);
                    painter->setPen(m_colorTextDisable);
                }
                else if (!vopt->state.testFlag(QStyle::State_Raised))
                {
                    m_imagePushButtonPressed.Draw(painter, vopt->rect, 0);
                    painter->setPen(m_colorTextActive);
                }
                else if (vopt->state.testFlag(QStyle::State_MouseOver))
                {
                    m_imagePushButtonHot.Draw(painter, vopt->rect, 0);
                    painter->setPen(m_colorTextActive);
                }
                else
                {
                    m_imagePushButton.Draw(painter, vopt->rect, 0);
                    painter->setPen(m_colorTextNormal);
                }
                //
                painter->drawText(vopt->rect,Qt::AlignCenter, vopt->text);
            }
            break;
        }
    }

};

QStyle* WizGetImageButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
                               const QString& downBackgroundFileName, const QString& disabledBackgroundFileName,
                               const QColor& normalTextColor, const QColor& activeTextColor, const QColor& disableTextColor)
{
    CWizImageButtonStyle* style = new CWizImageButtonStyle(normalBackgroundFileName,
                                                           hotBackgroundFileName, downBackgroundFileName,
                                                           disabledBackgroundFileName, normalTextColor,
                                                           activeTextColor, disableTextColor);
    return style;
}

