#include "wiznotestyle.h"
#include "wizdocumentlistview.h"
#include "wizcategoryview.h"
#include "share/wizdrawtexthelper.h"
#include "share/wizqthelper.h"
#include "share/wizsettings.h"

#include <QPainter>
#include <QPixmapCache>

#include <QDebug>


#if defined(Q_OS_WIN32)
#include <QWindowsVistaStyle>
typedef QWindowsVistaStyle CWizNoteBaseStyle;
#elif defined(Q_OS_LINUX) || defined(Q_OS_LINUX)
#include <QGtkStyle>
typedef QGtkStyle CWizNoteBaseStyle;
#elif defined(Q_OS_MAC)
#include <QMacStyle>
typedef QMacStyle CWizNoteBaseStyle;
#endif

class CWizCategoryView;
class CWizDocumentListView;
class QStyleOptionViewItemV4;



class CWizSkin9GridImage
{
protected:
    QImage m_img;
    QRect m_arrayImageGrid[9];
    //
    BOOL Clear();
public:
    static BOOL SplitRect(const QRect& rcSrc, QPoint ptTopLeft, QRect* parrayRect, int nArrayCount);
    BOOL SetImage(const CString& strImageFileName, QPoint ptTopLeft);
    //
    void Draw(QPainter* p, QRect rc, int nAlpha) const;
    BOOL Valid() const;
};


class CWizNoteStyle : public CWizNoteBaseStyle
{
public:
    CWizNoteStyle();
private:
    QImage m_expandedImage;
    QImage m_collapsedImage;
    CWizSkin9GridImage m_toolBarImage;
    CWizSkin9GridImage m_categorySelectedItemBackground;
    CWizSkin9GridImage m_documentsSelectedItemBackground;
    CWizSkin9GridImage m_documentsSelectedItemBackgroundHot;
    //
    QColor m_colorCategoryBackground;
    QColor m_colorCategoryText;
    QColor m_colorCategoryTextSelected;
    //
    QColor m_colorDocumentsBackground;
    QColor m_colorDocumentsTitle;
    QColor m_colorDocumentsDate;
    QColor m_colorDocumentsSummary;
    QColor m_colorDocumentsTitleSelected;
    QColor m_colorDocumentsDateSelected;
    QColor m_colorDocumentsSummarySelected;
    QColor m_colorDocumentsLine;
    //
protected:
    virtual void drawCategoryViewItem(const QStyleOptionViewItemV4 *option, QPainter *painter, const CWizCategoryView *widget) const;
    virtual void drawDocumentListViewItem(const QStyleOptionViewItemV4 *option, QPainter *painter, const CWizDocumentListView *widget) const;
    //
    void drawcenterImage(QPainter* p, const QImage& image, const QRect& rc) const;
public:
    virtual void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    virtual void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const;

public:
    QColor categoryBackground() { return m_colorCategoryBackground; }
    QColor documentsBackground() { return m_colorDocumentsBackground; }
public:
    static CWizNoteStyle* noteStyle();
};


const int IMAGE_WIDTH = 80;

CWizNoteStyle::CWizNoteStyle()
{
    m_expandedImage.load(WizGetSkinPath() + "button_expanded.png");
    m_collapsedImage.load(WizGetSkinPath() + "button_collapsed.png");
    m_toolBarImage.SetImage(WizGetSkinPath() + "toolbar_background.png", QPoint(8, 8));
    m_categorySelectedItemBackground.SetImage(WizGetSkinPath() + "category_selected_background.png", QPoint(4, 4));
    m_documentsSelectedItemBackground.SetImage(WizGetSkinPath() + "documents_selected_background.png", QPoint(4, 4));
    m_documentsSelectedItemBackgroundHot.SetImage(WizGetSkinPath() + "documents_selected_background_hot.png", QPoint(4, 4));
    //
    CWizSettings settings(WizGetSkinPath() + "skin.ini");
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
    m_colorDocumentsLine = settings.GetColor("Documents", "Line", "#666666");
}


void CWizNoteStyle::drawCategoryViewItem(const QStyleOptionViewItemV4 *vopt,
                                         QPainter *painter, const CWizCategoryView *view) const
{
    if (view->isSeparatorItemByIndex(vopt->index))
        return;
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
    QPainter* p = painter;
    p->save();
    p->setClipRect(opt->rect);

    QRect textRect = subElementRect(SE_ItemViewItemText, vopt, view);
    QRect iconRect = subElementRect(SE_ItemViewItemDecoration, vopt, view);

    // draw the background
    //proxy()->drawPrimitive(PE_PanelItemViewItem, opt, p, widget);
    //
    if (!vopt->icon.isNull())
    {
        // draw the icon
        QIcon::Mode mode = QIcon::Normal;
        QIcon::State state = QIcon::On;
        vopt->icon.paint(p, iconRect, vopt->decorationAlignment, mode, state);
    }

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
        //
        QColor colorText = vopt->state.testFlag(State_Selected) ? m_colorCategoryTextSelected : m_colorCategoryText;
        QRect rcTitle = textRect;
        CString strTitle = vopt->text;
        ::WizDrawTextSingleLine(p, rcTitle, strTitle,  Qt::TextSingleLine | Qt::AlignVCenter, colorText, true);
    }

    p->restore();
}


void CWizNoteStyle::drawDocumentListViewItem(const QStyleOptionViewItemV4 *vopt, QPainter *p, const CWizDocumentListView *view) const
{
    WIZDOCUMENTDATA document = view->documentFromIndex(vopt->index);
    WIZABSTRACT abstract = view->documentAbstractFromIndex(vopt->index);
    CString tagsText = view->documentTagsFromIndex(vopt->index);
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
        m_documentsSelectedItemBackground.Draw(p, vopt->rect, 0);
    }
    else if (vopt->state.testFlag(State_MouseOver))
    {
        m_documentsSelectedItemBackgroundHot.Draw(p, vopt->rect, 0);
    }
    //proxy()->drawPrimitive(PE_PanelItemViewItem, opt, p, view);
    //
    const QImage& img = abstract.image;
    if (img.width() > 0
        && img.height() > 0)
    {
        QRect imageRect = textRect;
        imageRect.setLeft(imageRect.right() - IMAGE_WIDTH);
        imageRect.adjust(4, 4, -4, -4);
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
        //
        textRect.adjust(8, 8, -8, -8);
        //
        bool selected = vopt->state.testFlag(State_Selected);
        //
        QColor colorTitle = selected ? m_colorDocumentsTitleSelected : m_colorDocumentsTitle;
        QRect rcTitle = textRect;
        rcTitle.setBottom(rcTitle.top() + vopt->fontMetrics.height());
        CString strTitle = document.strTitle;
        ::WizDrawTextSingleLine(p, rcTitle, strTitle,  Qt::TextSingleLine | Qt::AlignVCenter, colorTitle, true);
        //
        QColor colorDate = selected ? m_colorDocumentsDateSelected : m_colorDocumentsDate;
        QRect rcInfo = rcTitle;
        rcInfo.moveTo(rcInfo.left(), rcInfo.bottom() + 4);
        CString strInfo = document.tCreated.date().toString(Qt::DefaultLocaleShortDate) + tagsText;
        int infoWidth = ::WizDrawTextSingleLine(p, rcInfo, strInfo,  Qt::TextSingleLine | Qt::AlignVCenter, colorDate, true);
        //
        QColor colorSummary = selected ? m_colorDocumentsSummarySelected : m_colorDocumentsSummary;
        QRect rcAbstract1 = rcInfo;
        rcAbstract1.setLeft(rcInfo.left() + infoWidth + 16);
        rcAbstract1.setRight(rcTitle.right());
        CString strAbstract = abstract.text;
        ::WizDrawTextSingleLine(p, rcAbstract1, strAbstract, Qt::TextSingleLine | Qt::AlignVCenter, colorSummary, false);
        //
        QRect rcAbstract2 = textRect;
        rcAbstract2.setTop(rcAbstract1.bottom() + 2);
        rcAbstract2.setBottom(rcAbstract2.top() + rcTitle.height());
        ::WizDrawTextSingleLine(p, rcAbstract2, strAbstract, Qt::TextSingleLine | Qt::AlignVCenter, colorSummary, false);
        //
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
        proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, view);
    }
    //
    p->restore();
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
            ATLASSERT(vopt);
            if (const CWizDocumentListView *view = dynamic_cast<const CWizDocumentListView *>(widget))
            {
                drawDocumentListViewItem(vopt, painter, view);
            }
            else if (const CWizCategoryView *view = dynamic_cast<const CWizCategoryView *>(widget))
            {
                drawCategoryViewItem(vopt, painter, view);
            }
            else
            {
                CWizNoteBaseStyle::drawControl(element, option, painter, widget);
            }
            break;
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
            if (const CWizCategoryView *view = dynamic_cast<const CWizCategoryView *>(w))
            {
                Q_UNUSED(view);
                if (opt->state & State_Children)
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
            if (const CWizCategoryView *view = dynamic_cast<const CWizCategoryView *>(w))
            {
                const QStyleOptionViewItemV4 *vopt = qstyleoption_cast<const QStyleOptionViewItemV4 *>(opt);
                ATLASSERT(vopt);
                //
                if (view->isSeparatorItemByPosition(vopt->rect.center()))
                    return;
                //
                if (opt->state & QStyle::State_Selected)
                {
                    if (m_categorySelectedItemBackground.Valid())
                    {
                        QRect rect = opt->rect;
                        rect.setWidth(p->window().width());
                        //
                        m_categorySelectedItemBackground.Draw(p, rect, 0);
                        //
                        // draw the focus rect
                        if (opt->state & QStyle::State_HasFocus)
                        {
                            QStyleOptionFocusRect o;
                            o.QStyleOption::operator=(*opt);
                            o.rect = rect;
                            o.state |= QStyle::State_KeyboardFocusChange;
                            o.state |= QStyle::State_Item;
                            QPalette::ColorGroup cg = (opt->state & QStyle::State_Enabled)
                                                      ? QPalette::Normal : QPalette::Disabled;
                            o.backgroundColor = opt->palette.color(cg, (opt->state & QStyle::State_Selected)
                                                                   ? QPalette::Highlight : QPalette::Window);
                            proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, w);
                        }
                        return;
                    }
                }
            }
        }
        break;
    default:
        break;
    }
    CWizNoteBaseStyle::drawPrimitive(pe, opt, p, w);
}

void CWizNoteStyle::drawcenterImage(QPainter* p, const QImage& image, const QRect& rc) const
{
    int width = image.width();
    int height = image.height();
    //
    int x = rc.left() + (rc.width() - width) / 2;
    int y = rc.top() + (rc.height() - height) / 2;
    //
    p->drawImage(x, y, image);
}



CWizNoteStyle* CWizNoteStyle::noteStyle()
{
    static CWizNoteStyle style;
    return &style;
}

QStyle* WizGetStyle()
{
    return CWizNoteStyle::noteStyle();
}

QColor WizGetCategoryBackroundColor()
{
    return CWizNoteStyle::noteStyle()->categoryBackground();
}

QColor WizGetDocumentsBackroundColor()
{
    return CWizNoteStyle::noteStyle()->documentsBackground();
}

/////////////////////////////////////////////////////////////////////////////



BOOL CWizSkin9GridImage::Clear()
{
    if (!m_img.isNull())
    {
        m_img = QImage();
    }
    return TRUE;
}

BOOL CWizSkin9GridImage::SplitRect(const QRect& rcSrc, QPoint ptTopLeft, QRect* parrayRect, int nArrayCount)
{
    ATLASSERT(nArrayCount == 9);
    //
    QRect* arrayRect = parrayRect;
    //
    int nWidth = rcSrc.width();
    int nHeight = rcSrc.height();
    //
    if (ptTopLeft.x() <= 0)
        return FALSE;
    if (ptTopLeft.y() <= 0)
        return FALSE;
    if (ptTopLeft.x() >= nWidth / 2)
        return FALSE;
    if (ptTopLeft.y() >= nHeight / 2)
        return FALSE;
    //
    int x1 = rcSrc.left() + 0;
    int x2 = rcSrc.left() + ptTopLeft.x();
    int x3 = rcSrc.left() + nWidth - ptTopLeft.x();
    int x4 = rcSrc.left() + nWidth;
    //
    int y1 = rcSrc.top() + 0;
    int y2 = rcSrc.top() + ptTopLeft.y();
    int y3 = rcSrc.top() + nHeight - ptTopLeft.y();
    int y4 = rcSrc.top() + nHeight;
    //
    arrayRect[0] = QRect(QPoint(x1, y1), QPoint(x2, y2));
    arrayRect[1] = QRect(QPoint(x2, y1), QPoint(x3, y2));
    arrayRect[2] = QRect(QPoint(x3, y1), QPoint(x4, y2));

    arrayRect[3] = QRect(QPoint(x1, y2), QPoint(x2, y3));
    arrayRect[4] = QRect(QPoint(x2, y2), QPoint(x3, y3));
    arrayRect[5] = QRect(QPoint(x3, y2), QPoint(x4, y3));

    arrayRect[6] = QRect(QPoint(x1, y3), QPoint(x2, y4));
    arrayRect[7] = QRect(QPoint(x2, y3), QPoint(x3, y4));
    arrayRect[8] = QRect(QPoint(x3, y3), QPoint(x4, y4));
    //
    return TRUE;
}

BOOL CWizSkin9GridImage::SetImage(const CString& strImageFileName, QPoint ptTopLeft)
{
    Clear();
    //
    if (FAILED(m_img.load(strImageFileName)))
        return FALSE;
    //
    int nImageWidth = m_img.width();
    int nImageHeight = m_img.height();
    //
    return SplitRect(QRect(0, 0, nImageWidth, nImageHeight), ptTopLeft, m_arrayImageGrid, 9);
}

BOOL CWizSkin9GridImage::Valid() const
{
    return m_img.width() > 0 && m_img.height() > 0;
}

void CWizSkin9GridImage::Draw(QPainter* p, QRect rc, int nAlpha) const
{
    QRect arrayDest[9];
    //
    SplitRect(rc, m_arrayImageGrid[0].bottomRight(), arrayDest, 9);
    //
    for (int i = 0; i < 9; i++)
    {
        const QRect& rcSrc = m_arrayImageGrid[i];
        const QRect& rcDest = arrayDest[i];
        //
        if (rcDest.width() > 255)
        {
            int i = 0;
            i = 1;
        }
        //
        if (nAlpha > 0 && nAlpha <= 255)
        {
            p->drawImage(rcDest, m_img, rcSrc);
        }
        else
        {
            p->drawImage(rcDest, m_img, rcSrc);
        }
    }
}


