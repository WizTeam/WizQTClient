#include "wiznotestyle.h"
#include "wizdocumentlistview.h"
#include "wizcategoryview.h"
#include "share/wizdrawtexthelper.h"
#include "share/wizqthelper.h"
#include "share/wizsettings.h"
#include "share/wizuihelper.h"
#include "share/wizui.h"
#include "share/wizmultilinelistwidget.h"
#include "share/wizimagepushbutton.h"

#include <QtGui>

#ifdef BUILD_WITH_QT5
#include <QtWidgets>
#endif

class CWizCategoryBaseView;
class CWizDocumentListView;
//class QStyleOptionViewItemV4;

//#if defined(Q_OS_WIN32)
//typedef QWindowsVistaStyle CWizNoteBaseStyle;
//
//#elif defined(Q_OS_LINUX)
//
//#if defined(QT_NO_STYLE_GTK)
//#include <QCleanlooksStyle>
//typedef QCleanlooksStyle CWizLinuxStyle;
//#else
//#include <QGtkStyle>
//typedef QGtkStyle CWizLinuxStyle;
//#endif
//
//typedef CWizLinuxStyle CWizNoteBaseStyle;
//
//#elif defined(Q_OS_MAC)
//#include <QMacStyle>
//typedef QMacStyle CWizNoteBaseStyle;
//#endif

typedef QProxyStyle CWizNoteBaseStyle;

class CWizNoteStyle : public CWizNoteBaseStyle
{
public:
    CWizNoteStyle(const QString& skinName);

private:
    QImage m_expandedImage;
    QImage m_collapsedImage;
    CWizSkin9GridImage m_toolBarImage;
    CWizSkin9GridImage m_splitterShadowImage;
    CWizSkin9GridImage m_categorySelectedItemBackground;
    CWizSkin9GridImage m_categorySelectedItemBackground_unfocus;
    CWizSkin9GridImage m_documentsSelectedItemBackground;
    CWizSkin9GridImage m_documentsSelectedItemBackgroundHot;
    CWizSkin9GridImage m_multiLineListSelectedItemBackground;
    CWizSkin9GridImage m_multiLineListSelectedItemBackgroundHot;
    CWizSkin9GridImage m_imagePushButton;
    CWizSkin9GridImage m_imagePushButtonHot;
    CWizSkin9GridImage m_imagePushButtonPressed;
    CWizSkin9GridImage m_imagePushButtonDisabled;
    CWizSkin9GridImage m_imagePushButtonLabel;
    CWizSkin9GridImage m_imagePushButtonLabelRed;

    QColor m_colorCategoryBackground;
    QColor m_colorCategoryText;
    QColor m_colorCategoryTextSelected;

    QColor m_colorDocumentsBackground;
    QColor m_colorDocumentsTitle;
    QColor m_colorDocumentsDate;
    QColor m_colorDocumentsSummary;
    QColor m_colorDocumentsTitleSelected;
    QColor m_colorDocumentsDateSelected;
    QColor m_colorDocumentsSummarySelected;
    QColor m_colorDocumentsLine;

    QColor m_colorMultiLineListFirstLine;
    QColor m_colorMultiLineListFirstLineSelected;
    QColor m_colorMultiLineListOtherLine;
    QColor m_colorMultiLineListOtherLineSelected;

    QFont m_fontImagePushButtonLabel;
protected:
    virtual void drawCategoryViewItem(const QStyleOptionViewItemV4 *option, QPainter *painter, const CWizCategoryBaseView *widget) const;
    virtual void drawDocumentListViewItem(const QStyleOptionViewItemV4 *option, QPainter *painter, const CWizDocumentListView *widget) const;
    virtual void drawMultiLineListWidgetItem(const QStyleOptionViewItemV4 *option, QPainter *painter, const CWizMultiLineListWidget *widget) const;
    virtual void drawImagePushButton(const QStyleOptionButton *option, QPainter *painter, const CWizImagePushButton *button) const;
    virtual void drawSplitter(const QStyleOption *option, QPainter *painter, const QSplitter *splitter) const;

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

    m_expandedImage.load(strSkinPath + "button_expanded.png");
    //m_expandedImage = m_expandedImage.scaled(24, 24, Qt::KeepAspectRatio);
    m_collapsedImage.load(strSkinPath + "button_collapsed.png");
    //m_collapsedImage = m_collapsedImage.scaled(24, 24, Qt::KeepAspectRatio);

    m_toolBarImage.SetImage(strSkinPath + "toolbar_background.png", QPoint(8, 8));
    m_splitterShadowImage.SetImage(strSkinPath + "leftview_line_shadow.png", QPoint(4, 4));
    m_categorySelectedItemBackground.SetImage(strSkinPath + "left_row_view_bg.tiff", QPoint(4, 4));
    m_categorySelectedItemBackground_unfocus.SetImage(strSkinPath + "left_row_view_bg_unfocus.tiff", QPoint(4, 4));
    m_documentsSelectedItemBackground.SetImage(strSkinPath + "documents_selected_background.png", QPoint(4, 4));
    m_documentsSelectedItemBackgroundHot.SetImage(strSkinPath + "documents_selected_background_hot.png", QPoint(4, 4));
    m_multiLineListSelectedItemBackground.SetImage(strSkinPath + "multilinelist_selected_background.png", QPoint(4, 4));
    m_multiLineListSelectedItemBackgroundHot.SetImage(strSkinPath + "multilinelist_selected_background_hot.png", QPoint(4, 4));
    m_imagePushButton.SetImage(strSkinPath + "imagepushbutton.png", QPoint(4, 4));
    m_imagePushButtonHot.SetImage(strSkinPath + "imagepushbutton_hot.png", QPoint(4, 4));
    m_imagePushButtonPressed.SetImage(strSkinPath + "imagepushbutton_pressed.png", QPoint(4, 4));
    m_imagePushButtonDisabled.SetImage(strSkinPath + "imagepushbutton_disabled.png", QPoint(4, 4));
    m_imagePushButtonLabel.SetImage(strSkinPath + "imagepushbutton_label.png", QPoint(8, 8));
    m_imagePushButtonLabelRed.SetImage(strSkinPath + "imagepushbutton_label_red.png", QPoint(8, 8));
    //
    CWizSettings settings(strSkinPath + "skin.ini");
    m_colorCategoryBackground = settings.GetColor("Category", "Background", "#808080");
    m_colorCategoryText = settings.GetColor("Category", "Text", "#000000");
    m_colorCategoryTextSelected = settings.GetColor("Category", "TextSelected", "#ffffff");
    //
    m_colorDocumentsBackground = settings.GetColor("Documents", "Background", "#ffffff");
    m_colorDocumentsTitle = settings.GetColor("Documents", "Title", "#000000");
    m_colorDocumentsDate = settings.GetColor("Documents", "Date", "#0000ff");
    m_colorDocumentsSummary = settings.GetColor("Documents", "Summary", "#666666");
    m_colorDocumentsTitleSelected = settings.GetColor("Documents", "TitleSelected", "#000000");
    m_colorDocumentsDateSelected = settings.GetColor("Documents", "DateSelected", "#0000ff");
    m_colorDocumentsSummarySelected = settings.GetColor("Documents", "SummarySelected", "#666666");
    m_colorDocumentsLine = settings.GetColor("Documents", "Line", "#d9dcdd");
    //
    m_colorMultiLineListFirstLine = settings.GetColor("MultiLineList", "First", "#000000");
    m_colorMultiLineListFirstLineSelected = settings.GetColor("MultiLineList", "FirstSelected", "#000000");
    m_colorMultiLineListOtherLine = settings.GetColor("MultiLineList", "Other", "#666666");
    m_colorMultiLineListOtherLineSelected = settings.GetColor("MultiLineList", "OtherSelected", "#666666");
    //
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
    if (NULL != dynamic_cast<const CWizCategoryViewSeparatorItem*>(pItem)) {
        return;
    } else if (NULL != dynamic_cast<const CWizCategoryViewSpacerItem*>(pItem)) {
        return;
    }

//    if (view->isSeparatorItemByIndex(vopt->index))
//        return;

    QPalette palette = vopt->palette;
    QStyleOptionViewItemV4 adjustedOption = *vopt;
    adjustedOption.palette = palette;
    // We hide the focus rect in single selection as it is not required
    if ((view->selectionMode() == QAbstractItemView::SingleSelection)
        && !(vopt->state & State_KeyboardFocusChange)) {
        adjustedOption.state &= ~State_HasFocus;
    }

    QStyleOptionViewItemV4* opt = &adjustedOption;

    QPainter* p = painter;
    p->save();
    p->setClipRect(opt->rect);

    // draw the icon
    if (!vopt->icon.isNull())
    {
        QRect iconRect = subElementRect(SE_ItemViewItemDecoration, vopt, view);

        // draw icon little bigger than qt default rect
        iconRect.adjust(0, -4, 4, 4);

        if (vopt->state.testFlag(State_Selected)) {
            vopt->icon.paint(p, iconRect, vopt->decorationAlignment, QIcon::Selected, QIcon::Off);
        } else {
            vopt->icon.paint(p, iconRect, vopt->decorationAlignment, QIcon::Normal, QIcon::Off);
        }
    }

    // text should not empty
    if (vopt->text.isEmpty()) {
        Q_ASSERT(0);
        return;
    }

    QRect textRect = subElementRect(SE_ItemViewItemText, vopt, view);

    // draw text little far from icon than qt default
    textRect.adjust(10, 0, 0, 0);

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
    //CWizCategoryViewItemBase* pItem = view->categoryItemFromIndex(vopt->index);
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

    QColor colorText = vopt->state.testFlag(State_Selected) ? m_colorCategoryTextSelected : m_colorCategoryText;
    CString strText = vopt->text;
    int nWidth = ::WizDrawTextSingleLine(p, textRect, strText,
                                         Qt::TextSingleLine | Qt::AlignVCenter, colorText, true);

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


void CWizNoteStyle::drawDocumentListViewItem(const QStyleOptionViewItemV4 *vopt, QPainter *p, const CWizDocumentListView *view) const
{
    WIZDOCUMENTDATA document = view->documentFromIndex(vopt->index);
    WIZABSTRACT abstract = view->documentAbstractFromIndex(vopt->index);
    CString tagsText = view->documentTagsFromIndex(vopt->index);

    //palette.setColor(QPalette::All, QPalette::HighlightedText, palette.color(QPalette::Active, QPalette::Text));
    // Note that setting a saturated color here results in ugly XOR colors in the focus rect
    //palette.setColor(QPalette::All, QPalette::Highlight, palette.base().color().darker(108));

    // TODO: hard-coded background brush color
    QPalette palette = vopt->palette;
    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(58, 81, 134));
    palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(101, 122, 148));
    palette.setColor(QPalette::Active, QPalette::BrightText, Qt::lightGray);

    QStyleOptionViewItemV4 adjustedOption = *vopt;
    adjustedOption.palette = palette;

    // We hide the  focusrect in singleselection as it is not required
    //if ((view->selectionMode() == QAbstractItemView::SingleSelection)
    //    && !(vopt->state & State_KeyboardFocusChange)) {
    //    adjustedOption.state &= ~State_HasFocus;
    //}

    QStyleOptionViewItemV4* opt = &adjustedOption;

    p->save();
    p->setClipRect(opt->rect);

    QRect textLine = opt->rect;
    textLine.adjust(4, 0, -4, 0);
    p->setPen(m_colorDocumentsLine);
    p->drawLine(textLine.bottomLeft(), textLine.bottomRight());

    QRect textRect = subElementRect(SE_ItemViewItemText, vopt, view);

    // draw background behaviour
    if (vopt->state.testFlag(QStyle::State_Selected)) {
        if (view->hasFocus()) {
            p->fillRect(vopt->rect, adjustedOption.palette.brush(QPalette::Active, QPalette::Highlight));
        } else {
            p->fillRect(vopt->rect, adjustedOption.palette.brush(QPalette::Inactive, QPalette::Highlight));
        }
    }


//#ifndef Q_OS_MAC
//    else if (vopt->state.testFlag(State_MouseOver))
//    {
//        m_documentsSelectedItemBackgroundHot.Draw(p, vopt->rect, 0);
//    }
//#endif
    //proxy()->drawPrimitive(PE_PanelItemViewItem, opt, p, view);
    //
    const QImage& img = abstract.image;
    if (img.width() > 0 && img.height() > 0)
    {
        QRect imageRect = textRect;
        imageRect.setLeft(imageRect.right() - IMAGE_WIDTH);
        imageRect.adjust(4, 4, -4, -4);

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
        p->drawImage(imageRect, img);
        //
        textRect.setRight(imageRect.left());
    }
    //
    // draw the text
    if (!vopt->text.isEmpty()) {
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
        QColor selectedLightColor = palette.color(QPalette::HighlightedText);
        QColor selectedBrightColor = palette.color(QPalette::BrightText);

        //QColor colorTitle = selected ? m_colorDocumentsTitleSelected : m_colorDocumentsTitle;

        p->save();
        QFont fontTitle;
        fontTitle.setPointSize(13);
        p->setFont(fontTitle);
        QRect rcTitle = textRect;
        rcTitle.setBottom(rcTitle.top() + p->fontMetrics().height());
        CString strTitle = document.strTitle;
        QColor colorTitle = selected ? selectedLightColor : m_colorDocumentsTitle;
        ::WizDrawTextSingleLine(p, rcTitle, strTitle,  Qt::TextSingleLine | Qt::AlignVCenter, colorTitle, true);
        p->restore();

        //QColor colorDate = selected ? m_colorDocumentsDateSelected : m_colorDocumentsDate;
        QColor colorDate = selected ? selectedLightColor : m_colorDocumentsDate;
        QRect rcInfo = rcTitle;
        rcInfo.moveTo(rcInfo.left(), rcInfo.bottom() + 4);
        CString strInfo = document.tCreated.date().toString(Qt::DefaultLocaleShortDate) + tagsText;
        int infoWidth = ::WizDrawTextSingleLine(p, rcInfo, strInfo,  Qt::TextSingleLine | Qt::AlignVCenter, colorDate, true);

        //QColor colorSummary = selected ? m_colorDocumentsSummarySelected : m_colorDocumentsSummary;
        QColor colorSummary = selected ? selectedBrightColor : m_colorDocumentsSummary;
        QRect rcAbstract1 = rcInfo;
        rcAbstract1.setLeft(rcInfo.left() + infoWidth + 16);
        rcAbstract1.setRight(rcTitle.right());
        CString strAbstract = abstract.text;
        ::WizDrawTextSingleLine(p, rcAbstract1, strAbstract, Qt::TextSingleLine | Qt::AlignVCenter, colorSummary, false);

        QRect rcAbstract2 = textRect;
        rcAbstract2.setTop(rcAbstract1.bottom() + 2);
        rcAbstract2.setBottom(rcAbstract2.top() + rcTitle.height());
        ::WizDrawTextSingleLine(p, rcAbstract2, strAbstract, Qt::TextSingleLine | Qt::AlignVCenter, colorSummary, false);

        QRect rcAbstract3 = textRect;
        rcAbstract3.setTop(rcAbstract2.bottom() + 2);
        rcAbstract3.setBottom(rcAbstract3.top() + rcTitle.height());
        ::WizDrawTextSingleLine(p, rcAbstract3, strAbstract, Qt::TextSingleLine | Qt::AlignVCenter, colorSummary, true);
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
        drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, view);
    }
    //
    p->restore();
}

void CWizNoteStyle::drawMultiLineListWidgetItem(const QStyleOptionViewItemV4 *vopt, QPainter *p, const CWizMultiLineListWidget *view) const
{
    bool imageAlignLeft = view->imageAlignLeft();
    int imageWidth = view->imageWidth();
    int lineCount = view->lineCount();
    int wrapTextLineText = view->wrapTextLineIndex();
    const QPixmap img = view->itemImage(vopt->index);
    //
    QPalette palette = vopt->palette;
    palette.setColor(QPalette::All, QPalette::HighlightedText, palette.color(QPalette::Active, QPalette::Text));
    // Note that setting a saturated color here results in ugly XOR colors in the focus rect
    palette.setColor(QPalette::All, QPalette::Highlight, palette.base().color().darker(108));
    QStyleOptionViewItemV4 adjustedOption = *vopt;
    adjustedOption.palette = palette;
    // We hide the  focusrect in singleselection as it is not required
    if ((view->selectionMode() == QAbstractItemView::SingleSelection)
        && !(vopt->state & State_KeyboardFocusChange))
    {
        adjustedOption.state &= ~State_HasFocus;
    }

    QStyleOptionViewItemV4* opt = &adjustedOption;
    //
    p->save();
    p->setClipRect(opt->rect);

    QRect textLine = opt->rect;
    textLine.adjust(4, 0, -4, 0);
    p->setPen(m_colorDocumentsLine);
    p->drawLine(textLine.bottomLeft(), textLine.bottomRight());

    QRect textRect = subElementRect(SE_ItemViewItemText, vopt, view);

    // draw the background
    if (vopt->state.testFlag(State_Selected))
    {
        m_multiLineListSelectedItemBackground.Draw(p, vopt->rect, 0);
    }
#ifndef Q_OS_MAC
    else if (vopt->state.testFlag(State_MouseOver))
    {
        m_multiLineListSelectedItemBackgroundHot.Draw(p, vopt->rect, 0);
    }
#endif
    //proxy()->drawPrimitive(PE_PanelItemViewItem, opt, p, view);
    //
    if (!img.isNull()
        && img.width() > 0
        && img.height() > 0)
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
        //
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
        //
    }
    //
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
    //
    textRect.adjust(8, 8, -8, -8);
    //
    bool selected = vopt->state.testFlag(State_Selected);
    //
    int lineHeight = vopt->fontMetrics.height() + 2;
    //
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
    //
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
    //
    p->restore();
}


void CWizNoteStyle::drawImagePushButton(const QStyleOptionButton *option, QPainter *painter, const CWizImagePushButton *button) const
{
    bool disabled = false;
    bool pressed = false;
    bool hot = false;
    //
    State flags = option->state;
    if (!(flags & State_Enabled))
        disabled = true;
    else if (flags & (State_Sunken | State_On))
        pressed = true;
    else if (flags & State_MouseOver)
        hot = true;
    //
    QRect rect(option->rect);
    QRect rectImage = rect;
    //
    CString label = button->text();
    if (!label.isEmpty())
    {
        rectImage.setWidth(std::min<int>(16 + 16, rect.width()));
    }
    //
    if (disabled)
    {
        m_imagePushButtonDisabled.Draw(painter, rectImage, 0);
    }
    else if (pressed)
    {
        m_imagePushButtonPressed.Draw(painter, rectImage, 0);
    }
    else if (hot)
    {
        m_imagePushButtonHot.Draw(painter, rectImage, 0);
    }
    else
    {
        m_imagePushButton.Draw(painter, rectImage, 0);
    }
    //
    QIcon::Mode mode = disabled ? QIcon::Disabled : QIcon::Normal;
    QPixmap pixmap = button->icon().pixmap(16, 16, mode);
    if (!pixmap.isNull())
    {
        QPoint pt = rectImage.center();
        pt.setX(pt.x() - pixmap.width() / 2 + 1);
        pt.setY(pt.y() - pixmap.height() / 2 + 1);
        //
        painter->drawPixmap(pt, pixmap);
    }
    //
    if (!label.isEmpty())
    {
        QRect rectLabel = rect;
        rectLabel.setLeft(rectImage.right() - 8);
        rectLabel.setHeight(m_imagePushButtonLabel.actualSize().height());
        //
        if (button->redFlag())
        {
            m_imagePushButtonLabelRed.Draw(painter, rectLabel, 0);
        }
        else
        {
            m_imagePushButtonLabel.Draw(painter, rectLabel, 0);
        }
        //
        QRect rectLabelText = rectLabel;
        rectLabelText.setRight(rectLabelText.right() - 2);    //shadow
        rectLabelText.setBottom(rectLabelText.bottom() - 4);    //shadow
        painter->save();
        painter->setFont(m_fontImagePushButtonLabel);
        ::WizDrawTextSingleLine(painter, rectLabelText, label,  Qt::TextSingleLine | Qt::AlignVCenter | Qt::AlignCenter, QColor(0xff, 0xff, 0xff), false);
        painter->restore();
    }
}

void CWizNoteStyle::drawSplitter(const QStyleOption *option, QPainter *painter, const QSplitter *splitter) const
{
    QRect rectSplitter = option->rect;
    qDebug() << rectSplitter.x() << ":" << rectSplitter.y() << ":" << rectSplitter.width() << ":" << rectSplitter.height();
    //m_splitterShadowImage.Draw(painter, , 255);
}

void CWizNoteStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element)
    {
#ifndef Q_OS_MAC
    case CE_ToolBar:
        if (const QStyleOptionToolBar *toolbar = qstyleoption_cast<const QStyleOptionToolBar *>(option))
        {
            Q_UNUSED(toolbar);
            //
            painter->fillRect(option->rect, QColor(241, 241, 241));
            m_toolBarImage.Draw(painter, option->rect, 0);
        }
        break;
#endif
    case CE_ItemViewItem:
        {
            const QStyleOptionViewItemV4 *vopt = qstyleoption_cast<const QStyleOptionViewItemV4 *>(option);
            Q_ASSERT(vopt);

            if (const CWizDocumentListView *view = dynamic_cast<const CWizDocumentListView *>(widget))
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
    case CE_PushButton:
        {
            const QStyleOptionButton* vopt = qstyleoption_cast<const QStyleOptionButton *>(option);
            ATLASSERT(vopt);
            //
            if (const CWizImagePushButton *button = dynamic_cast<const CWizImagePushButton *>(widget))
            {
                drawImagePushButton(vopt, painter, button);
            }
            else
            {
                CWizNoteBaseStyle::drawControl(element, option, painter, widget);
            }
        }
        break;
    case CE_Splitter:
    {
        if (const QSplitter* splitter = dynamic_cast<const QSplitter *>(widget))
        {
            drawSplitter(option, painter, splitter);
        } else {
            CWizNoteBaseStyle::drawControl(element, option, painter, widget);
        }
    }

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
                if (opt->state & QStyle::State_Children)
                {
                    bool bExpanded = (opt->state & QStyle::State_Open) ? true : false;
                    drawcenterImage(p, bExpanded ? m_expandedImage : m_collapsedImage, opt->rect);
                }
                //CWizNoteBaseStyle::drawPrimitive(pe, opt, p, w);
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

                if (view->isSeparatorItemByPosition(vopt->rect.center()))
                    return;

                if (opt->state & QStyle::State_Selected)
                {
                    if (!m_categorySelectedItemBackground.Valid()
                            || !m_categorySelectedItemBackground_unfocus.Valid())
                        return;

                        QRect rect = opt->rect;
                        rect.setWidth(p->window().width());
                        if (opt->state & QStyle::State_HasFocus) {
                            m_categorySelectedItemBackground.Draw(p, rect, 0);
                        } else {
                            m_categorySelectedItemBackground_unfocus.Draw(p, rect, 0);
                        }

                        // draw the focus rect
                        //if (opt->state & QStyle::State_HasFocus)
                        //{
                        //    QStyleOptionFocusRect o;
                        //    o.QStyleOption::operator=(*opt);
                        //    o.rect = rect;
                        //    o.state |= QStyle::State_KeyboardFocusChange;
                        //    o.state |= QStyle::State_Item;
                        //    QPalette::ColorGroup cg = (opt->state & QStyle::State_Enabled)
                        //                              ? QPalette::Normal : QPalette::Disabled;
                        //    o.backgroundColor = opt->palette.color(cg, (opt->state & QStyle::State_Selected)
                        //                                           ? QPalette::Highlight : QPalette::Window);
                        //    proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, w);
                        //}
                        return;
                }
            }
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

