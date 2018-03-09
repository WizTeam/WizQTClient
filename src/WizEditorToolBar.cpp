#include "WizEditorToolBar.h"

#include <QStylePainter>
#include <QToolButton>
#include <QFontComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QAction>
#include <QMenu>
#include <QFileDialog>
#include <QImageWriter>
#include <QClipboard>
#include <QApplication>
#include <QMimeData>
#include <QStyledItemDelegate>
#include <QListWidget>
#include <QFontDialog>
#include <QDebug>
#include <QWidgetAction>
#include <QActionGroup>

#include "share/WizMisc.h"
#include "WizDef.h"
#include "share/WizSettings.h"
#include "WizDocumentWebEngine.h"
#include "WizDocumentWebView.h"
#include "WizActions.h"
#include "utils/WizLogger.h"
#include "share/WizObjectDataDownloader.h"
#include "share/WizAnalyzer.h"
#include "WizDef.h"
#include "utils/WizStyleHelper.h"
#include "WizDocumentView.h"
#include "widgets/WizTipsWidget.h"
#include "WizMainWindow.h"
#include "widgets/WizTableSelector.h"
#include "share/jsoncpp/json/json.h"

const int WizCheckStateRole = (int)Qt::UserRole + 5;
const int WizFontFamilyHelperRole = WizCheckStateRole + 1;

const int RecommendedWidthForTwoLine = 685;
const int RecommendedWidthForOneLine = 1060;

#define WIZSEPARATOR    "separator"
#define WIZFONTPANEL    "fontpanel"
#define WIZRECENTFONT   "recentfont"

#define WIZRECENTFONTLIST  "RecentlyUsedFont"

#define WIZSHOWEXTRABUTTONITEMS "ShowExtraButtonItems"

struct WizComboboxStyledItem
{    
    QString strText;
    QString strUserData;
    QString strFontFamily;
    int nFontSize;
    bool bBold;
};

const int nCommonlyUsedFontCount = 9;
const QString CommonlyUsedFont[] =
{
    "Arial",
    "Helvetica Neue",
    "Times",
    "Times New Roman",
    "Tahoma",
    "Verdana",
    "Songti SC",
    "Heiti SC",
    "Kaiti SC"
};

const int nParagraphItemCount = 8;
WizComboboxStyledItem* ParagraphItems()
{
    static WizComboboxStyledItem paragraphItems[] =
    {
        {QObject::tr("Paragraph"), "p", "", 14, false},
        {QObject::tr("Text"), "div", "", 14, false},
        {QObject::tr("H6"), "h6", "", 14, true},
        {QObject::tr("H5"), "h5", "", 15, true},
        {QObject::tr("H4"), "h4", "", 17, true},
        {QObject::tr("H3"), "h3", "", 18, true},
        {QObject::tr("H2"), "h2", "", 20, true},
        {QObject::tr("H1"), "h1", "", 21, true }
    };

    return paragraphItems;
}

const int nFontSizeCount = 15;
WizComboboxStyledItem* FontSizes()
{
    static WizComboboxStyledItem fontItems[] =
    {
        {"9", "9pt", "", 14, false},
        {"10", "10pt", "", 14, false},
        {"11", "11pt", "", 14, false},
        {"12", "12pt", "", 14, false},
        {"13", "13pt", "", 14, false},
        {"14", "14pt", "", 14, false},
        {"15", "15pt", "", 14, false},
        {"16", "16pt", "", 14, false},
        {"17", "17pt", "", 14, false},
        {"18", "18pt", "", 14, false},
        {"24", "24pt", "", 14, false},
        {"36", "36pt", "", 14, false},
        {"48", "48pt", "", 14, false},
        {"64", "64pt", "", 14, false},
        {"72", "72pt", "", 14, false}
    };
    return fontItems;
}

const int nFontFamilyCount =  30;

WizComboboxStyledItem* FontFamilies()
{
    static WizComboboxStyledItem fontItems[] =
    {
        {QObject::tr("Adobe Fangsong Std"), "Adobe Fangsong Std", "Adobe Fangsong Std", 14, false},
        {QObject::tr("Adobe Heiti Std"), "Adobe Heiti Std", "Adobe Heiti Std", 14, false},
        {QObject::tr("Adobe Kaiti Std"), "Adobe Kaiti Std", "Adobe Kaiti Std", 14, false},
        {QObject::tr("Adobe Song Std"), "Adobe Song Std", "Adobe Song Std", 14, false},
        {QObject::tr("Baoli SC"), "Baoli SC", "Baoli SC", 14, false},
        {QObject::tr("Hannotate SC"), "Hannotate SC", "Hannotate SC", 14, false},
        {QObject::tr("Hannotate TC"), "Hannotate TC", "Hannotate TC", 14, false},
        {QObject::tr("HanziPen SC"), "HanziPen SC", "HanziPen SC", 14, false},
        {QObject::tr("HanziPen TC"), "HanziPen TC", "HanziPen TC", 14, false},
        {QObject::tr("Heiti SC"), "Heiti SC", "Heiti SC", 14, false},
        {QObject::tr("Heiti TC"), "Heiti TC", "Heiti TC", 14, false},
        {QObject::tr("Kaiti SC"), "Kaiti SC", "Kaiti SC", 14, false},
        {QObject::tr("Kaiti TC"), "Kaiti TC", "Kaiti TC", 14, false},
        {QObject::tr("Lantinghei SC"), "Lantinghei SC", "Lantinghei SC", 14, false},
        {QObject::tr("Lantinghei TC"), "Lantinghei TC", "Lantinghei TC", 14, false},
        {QObject::tr("Libian SC"), "Libian SC", "Libian SC", 14, false},
        {QObject::tr("Microsoft YaHei"), "Microsoft YaHei", "Microsoft YaHei", 14, false},
        {QObject::tr("Songti SC"), "Songti SC", "Songti SC", 14, false},
        {QObject::tr("Songti TC"), "Songti TC", "Songti TC", 14, false},
        {QObject::tr("STFangsong"), "STFangsong", "STFangsong", 14, false},
        {QObject::tr("STHeiti"), "STHeiti", "STHeiti", 14, false},
        {QObject::tr("STKaiti"), "STKaiti", "STKaiti", 14, false},
        {QObject::tr("STSong"), "STSong", "STSong", 14, false},
        {QObject::tr("Wawati SC"), "Wawati SC", "Wawati SC", 14, false},
        {QObject::tr("Wawati TC"), "Wawati TC", "Wawati TC", 14, false},
        {QObject::tr("Weibei SC"), "Weibei SC", "Weibei SC", 14, false},
        {QObject::tr("Weibei TC"), "Weibei TC", "Weibei TC", 14, false},
        {QObject::tr("Xingkai SC"), "Xingkai SC", "Xingkai SC", 14, false},
        {QObject::tr("Yuanti SC"), "Yuanti SC", "Yuanti SC", 14, false},
        {QObject::tr("Yuppy TC"), "Yuppy TC", "Yuppy TC", 14, false},
        {QObject::tr("Yuppy SC"), "Yuppy SC", "Yuppy SC", 14, false}
    };
    return fontItems;
};

WizComboboxStyledItem itemFromArrayByKey(const QString& key, const WizComboboxStyledItem array[], const int count)
{
    for (int i = 0; i < count; i++)
    {
        if (array[i].strUserData == key)
        {
            return array[i];
        }
    }
    WizComboboxStyledItem defaultItem;
    return defaultItem;
}

WizComboboxStyledItem itemFromArrayByText(const QString& text, const WizComboboxStyledItem array[], const int count)
{
    for (int i = 0; i < count; i++)
    {
        if (array[i].strText == text)
        {
            return array[i];
        }
    }
    WizComboboxStyledItem defaultItem;
    return defaultItem;
}

void clearWizCheckState(QComboBox* combobox)
{
    for (int i = 0; i < combobox->count(); i++)
    {
        QModelIndex index = combobox->model()->index(i, 0);
        combobox->model()->setData(index, Qt::Unchecked, WizCheckStateRole);
    }
}

class WizToolComboboxItemDelegate : public QStyledItemDelegate
{
public:
    WizToolComboboxItemDelegate(QObject *parent, QComboBox* widget, const WizComboboxStyledItem* items, int count)
        : QStyledItemDelegate(parent), m_itemArray(items), m_arrayCount(count), m_widget(widget)
    {}

    void drawTextItem(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index, bool useDefaultFont = false) const
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        //
        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, QBrush(QColor("#5990EF")));
            opt.palette.setColor(QPalette::Text, QColor(Qt::white));
            opt.palette.setColor(QPalette::WindowText, QColor(Qt::white));
            opt.palette.setColor(QPalette::HighlightedText, QColor(Qt::white));
        }
        else if (option.state & QStyle::State_MouseOver)
        {
            opt.palette.setColor(QPalette::Text, QColor(Qt::black));
            opt.palette.setColor(QPalette::WindowText, QColor(Qt::black));
            opt.palette.setColor(QPalette::HighlightedText, QColor(Qt::black));
        }
        //
        if (!useDefaultFont)
        {
            QString text = index.model()->data(index, Qt::DisplayRole).toString();
            WizComboboxStyledItem styledItem = itemFromArrayByText(text, m_itemArray, m_arrayCount);
            if (styledItem.strText.isEmpty())
            {
                //set default data
                styledItem.bBold = false;
                styledItem.nFontSize = 14;
                styledItem.strFontFamily = text;
            }
            opt.font.setPointSize(styledItem.nFontSize);
            opt.font.setBold(styledItem.bBold);
            if (!styledItem.strFontFamily.isEmpty())
            {
                opt.font.setFamily(styledItem.strFontFamily);
            }
        }

        const int nIconSize = WizSmartScaleUI(16);
        if (index.model()->data(index, WizCheckStateRole).toInt() == Qt::Checked)
        {
            static QIcon icon = Utils::WizStyleHelper::loadIcon("listViewItemSelected");
            QPixmap pix = icon.pixmap(QSize(nIconSize, nIconSize), QIcon::Normal, (opt.state & QStyle::State_MouseOver) ? QIcon::On : QIcon::Off);
            if (!pix.isNull())
            {
                painter->drawPixmap(opt.rect.x() + 4, opt.rect.y() + (opt.rect.height() - nIconSize) / 2, nIconSize, nIconSize , pix);
            }
        }
        opt.rect.setX(opt.rect.x() + nIconSize + 4);

        QStyledItemDelegate::paint(painter, opt, index);
    }

    void drawSeparatorItem(QPainter *painter,
                           const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        //
        painter->save();
        painter->setPen(QColor("#cccccc"));
        painter->setBrush(Qt::NoBrush);
        int lineY = opt.rect.y() + opt.rect.height() / 2;
        painter->drawLine(QLine(opt.rect.x(), lineY, opt.rect.right(), lineY));
        painter->restore();
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QString helperData = index.data(WizFontFamilyHelperRole).toString();
        if (helperData.isEmpty() || helperData == WIZRECENTFONT)
        {
            drawTextItem(painter, option, index);
        }
        else if (helperData == WIZSEPARATOR)
        {
            drawSeparatorItem(painter, option, index);
        }
        else
        {
            drawTextItem(painter, option, index, true);
        }
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        if (index.data(WizFontFamilyHelperRole).toString() == WIZSEPARATOR)
        {
            return QSize(opt.rect.width(), 5);
        }

        //
        WizComboboxStyledItem styledItem = itemFromArrayByText(opt.text, m_itemArray, m_arrayCount);
        if (styledItem.strText.isEmpty())
        {
            styledItem.nFontSize = 14;
        }
        QFont font = opt.font;
        font.setPointSize(styledItem.nFontSize);
        QFontMetrics fm(font);
        QRect rc = fm.boundingRect(opt.text);
        //
        QSize size(rc.width() + 4, rc.height() + 4);
        return size;
    }

private:
    const WizComboboxStyledItem* m_itemArray;
    int m_arrayCount;
    QComboBox* m_widget;
};


const QColor colors[6][8] =
{
    {QColor(0, 0, 0, 255), QColor(170, 0, 0, 255), QColor(0, 85, 0, 255), QColor(170, 85, 0, 255),
    QColor(0, 170, 0, 255), QColor(170, 170, 0, 255), QColor(0, 255, 0, 255), QColor(170, 250, 0, 255)},

    {QColor(0, 0, 127, 255), QColor(170, 0, 127, 255), QColor(0, 85, 127, 255), QColor(170, 85, 127, 255),
    QColor(0, 170, 127, 255), QColor(170, 170, 127, 255), QColor(0, 255, 127, 255), QColor(170, 255, 127, 255)},

    {QColor(0, 0, 255, 255), QColor(170, 0, 255, 255), QColor(0, 85, 255, 255), QColor(170, 85, 255, 255),
    QColor(0, 170, 255, 255), QColor(170, 170, 255, 255), QColor(0, 255, 255, 255), QColor(170, 255, 255, 255)},

    {QColor(85, 0, 0, 255), QColor(255, 0, 0, 255), QColor(85, 85, 0, 255), QColor(255, 85, 0, 255),
    QColor(85, 170, 0, 255), QColor(255, 170, 0, 255), QColor(85, 255, 0, 255), QColor(255, 255, 0, 255)},

    {QColor(85, 0, 127, 255), QColor(255, 0, 127, 255), QColor(85, 85, 127, 255), QColor(255, 85, 127, 255),
    QColor(85, 170, 127, 255), QColor(255, 170, 127, 255), QColor(85, 255, 127, 255), QColor(255, 255, 127, 255)},

    {QColor(85, 0, 255, 255), QColor(255, 0, 255, 255), QColor(85, 85, 255, 255), QColor(255, 85, 255, 255),
    QColor(85, 170, 255, 255), QColor(255, 170, 255, 255), QColor(85, 255, 255, 255), QColor(255, 255, 255, 255)}
};

const int nMacColorRow = 6;
const int nMacColorColum = 6;
const QColor macColors[nMacColorRow][nMacColorColum] =
{
    {QColor("#00b0f0"), QColor("#92d050"), QColor("#ffff00"),
    QColor("#fcc00f"), QColor("#ff0000"), QColor("#7030a0")},

    {QColor(100, 179, 223, 255), QColor(157, 225, 89, 255), QColor(255, 224, 97, 255),
    QColor(255, 192, 114, 255), QColor(255, 95, 94, 255), QColor(157, 69, 184, 255)},

    {QColor(73, 155, 201, 255), QColor(111, 192, 55, 255), QColor(241, 209, 48, 255),
    QColor(255, 169, 58, 255), QColor(255, 44, 33, 255), QColor(107, 33, 133, 255)},

    {QColor(54, 125, 162, 255), QColor(122, 174, 61, 255), QColor(226, 184, 0, 255),
    QColor(236, 159, 46, 255), QColor(206, 34, 43, 255), QColor(84, 20, 108, 255)},

    {QColor(23, 87, 121, 255), QColor(88, 135, 37, 255), QColor(198, 147, 0, 255),
    QColor(209, 127, 20, 255), QColor(174, 25, 22, 255), QColor(60, 10, 74, 255)},

    {QColor(255, 255, 255, 255), QColor(204, 204, 204, 255), QColor(153, 153, 153, 255),
    QColor(102, 102, 102, 255), QColor(51, 51, 51, 255), QColor(0, 0, 0, 255)},
};

QIcon createColorIcon(QColor color)
{
    QPixmap pixmap(16, 16);
    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(0, 0, 16, 16), color);

    return QIcon(pixmap);
}

const int TOOLBUTTON_MARGIN_WIDTH = 12;
const int TOOLBUTTON_ARRWO_WIDTH = 16;


void drawComboPrimitive(QStylePainter* p, QStyle::PrimitiveElement pe, const QStyleOption &opt);

void drawButtonBackground(QPainter* painter, const QRect& rect, bool bDrawLeft, bool bDrawRight, bool hasFocus)
{
    int nScale = WizIsHighPixel() ? 2 : 1;

    // load file
    static QPixmap normalPixBackground = QPixmap(Utils::WizStyleHelper::skinResourceFileName("editorToolButtonBackground", true));
    static QPixmap focusPixBackground = QPixmap(Utils::WizStyleHelper::skinResourceFileName("editorToolButtonBackground_on", true));
    static QPixmap normalPixBackgroundMid = normalPixBackground.copy(6, 0, 2, normalPixBackground.height());
    static QPixmap focusPixBackgroundMid = focusPixBackground.copy(6, 0, 2, focusPixBackground.height());
    QRect rcLeft(rect.x(), rect.y(), 6, rect.size().height());
    static QPixmap normalPixBackgroundLeft = normalPixBackground.copy(0, 0, 6 * nScale, normalPixBackground.height());
    static QPixmap focusPixBackgroundLeft = focusPixBackground.copy(0, 0, 6 * nScale, focusPixBackground.height());
    int rightWidth = 8;
    static QPixmap normalPixBackgroundRight = normalPixBackground.copy(normalPixBackground.width() - rightWidth * nScale, 0, rightWidth * nScale, normalPixBackground.height());
    static QPixmap focusPixBackgroundRight = focusPixBackground.copy(focusPixBackground.width() - rightWidth * nScale, 0, rightWidth * nScale, focusPixBackground.height());
    QRect rcRight(rect.size().width() - rightWidth, rect.y(), rightWidth, rect.size().height());

    const QPixmap& pixLeft = hasFocus ? focusPixBackgroundLeft : normalPixBackgroundLeft;
    const QPixmap& pixMid = hasFocus ? focusPixBackgroundMid : normalPixBackgroundMid;
    const QPixmap& pixRight = hasFocus ? focusPixBackgroundRight : normalPixBackgroundRight;
    //
    painter->drawPixmap(rcLeft, bDrawLeft ? pixLeft : pixMid);
    painter->drawPixmap(rect.x() + 5, rect.y(), rect.size().width() - rightWidth, rect.size().height(), pixMid);
    painter->drawPixmap(rcRight, bDrawRight ? pixRight : pixMid);
}

void drawCombo(QComboBox* cm, QStyleOptionComboBox& opt)
{
    QStylePainter painter(cm);

    opt.palette.setColor(QPalette::Text, "#646464");
    painter.setPen(cm->palette().color(QPalette::Text));

    drawButtonBackground(&painter, opt.rect, true, true, opt.state & QStyle::State_MouseOver);

    // draw arrow
    if (opt.subControls & QStyle::SC_ComboBoxArrow) {
        QStyleOption subOpt = opt;

        QRect rectSub = cm->style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow);
        rectSub.adjust(6, 0, -12, 0);


        subOpt.rect = rectSub.adjusted(0, rectSub.height()/2 - 3, 0, -rectSub.height()/2 + 3);
        QRect rcArrow = opt.rect;
        rcArrow.setX(opt.rect.right() - TOOLBUTTON_ARRWO_WIDTH - 8);
        rcArrow.setY((opt.rect.height() - TOOLBUTTON_ARRWO_WIDTH) / 2);
        rcArrow.setSize(QSize(TOOLBUTTON_ARRWO_WIDTH, TOOLBUTTON_ARRWO_WIDTH));
        static QPixmap arrow = QPixmap(Utils::WizStyleHelper::skinResourceFileName("editorToolbarComboboxArrow", true));
        painter.drawPixmap(rcArrow, arrow);
    }

    // draw text
    QFont f = painter.font();
    f.setPixelSize(11);
    painter.setFont(f);

    QRect editRect = cm->style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField);
    if (!opt.currentText.isEmpty()) {
        painter.drawItemText(editRect.adjusted(1, 0, -1, 0),
                     cm->style()->visualAlignment(opt.direction, Qt::AlignLeft | Qt::AlignVCenter),
                     opt.palette, opt.state & QStyle::State_Enabled, opt.currentText, QPalette::Text);
    }
}

void drawComboPrimitive(QStylePainter* p, QStyle::PrimitiveElement pe, const QStyleOption &opt)
{
    p->save();

    int xOffset = opt.direction == Qt::LeftToRight ? 2 : -1;
    QMatrix matrix;
    matrix.translate(opt.rect.center().x() + xOffset, opt.rect.center().y() + 2);
    QPainterPath path;
    switch(pe) {
    default:
    case QStyle::PE_IndicatorArrowDown:
        break;
    case QStyle::PE_IndicatorArrowUp:
        matrix.rotate(180);
        break;
    case QStyle::PE_IndicatorArrowLeft:
        matrix.rotate(90);
        break;
    case QStyle::PE_IndicatorArrowRight:
        matrix.rotate(-90);
        break;
    }
    path.moveTo(0, 2.3);
    path.lineTo(-2.3, -2.3);
    path.lineTo(2.3, -2.3);
    p->setMatrix(matrix);
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(0, 0, 0, 255));
    p->setRenderHint(QPainter::Antialiasing);
    p->drawPath(path);
    p->restore();
}


WizDblclickableToolButton::WizDblclickableToolButton(QWidget *parent)
    : QToolButton(parent)
{
    m_tDblClicked = QDateTime::currentDateTime();
}

void WizDblclickableToolButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    QToolButton::mouseDoubleClickEvent(event);
    m_tDblClicked = QDateTime::currentDateTime();
    //
    emit dblClicked();
}

void WizDblclickableToolButton::mouseReleaseEvent(QMouseEvent* event)
{
    QDateTime now = QDateTime::currentDateTime();
    int seconds = now.toTime_t() - m_tDblClicked.toTime_t();
    if (seconds < 1) {
        event->ignore();
    } else {
        QToolButton::mouseReleaseEvent(event);
    }
}


class CWizToolButton : public WizDblclickableToolButton
{
public:
    enum Position {
        NoPosition,
        left,
        Center,
        right
    };

    CWizToolButton(QWidget* parent = 0)
        : WizDblclickableToolButton(parent)
        , m_colorHoverBorder("#c8dae8")
        , m_colorHoverFill("#e8f0f3")
        , m_colorSunkenBorder("#0072c4")
        , m_horizontalPadding_left(10)
        , m_horizontalPadding_rgiht(10)
    {
        setFocusPolicy(Qt::NoFocus);
        setCheckable(true);
        setIconSize(QSize(WizSmartScaleUI(12), WizSmartScaleUI(12)));
        setFixedHeight(Utils::WizStyleHelper::editorButtonHeight());
    }

    void setPosition(Position position)
    {
        m_position = position;
    }

    void setHorizontalPadding(int padding)
    {
        m_horizontalPadding_left = padding;
        m_horizontalPadding_rgiht = padding;
    }

    void setHorizontalPadding(int leftPadding, int rightPadding)
    {
        m_horizontalPadding_left = leftPadding;
        m_horizontalPadding_rgiht = rightPadding;
    }

protected:
    virtual void leaveEvent(QEvent* event)
    {
        WizDblclickableToolButton::leaveEvent(event);

        update();
    }

    virtual void enterEvent(QEvent* event)
    {
        WizDblclickableToolButton::enterEvent(event);

        update();
    }
    void mouseReleaseEvent(QMouseEvent* ev)
    {
#ifdef Q_OS_OSX
        QMenu* m = menu();
        if (m)
        {
            QPoint pt = mapToGlobal(rect().bottomLeft());
            m->popup(pt);
            return;
        }
#endif
        //
        WizDblclickableToolButton::mouseReleaseEvent(ev);
    }

    virtual void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);

        QStyleOptionToolButton opt;
        initStyleOption(&opt);

        QPainter p(this);        
        p.setClipRect(opt.rect);

        QIcon::Mode mode = opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
        if (mode == QIcon::Normal && (opt.state & QStyle::State_Sunken))
            mode = QIcon::Active;
        QIcon::State state = QIcon::Off;
        if (opt.state & QStyle::State_On)
            state = QIcon::On;

        bool bDrawLeft = (m_position == left) || (m_position == NoPosition);
        bool bDrawRight = (m_position == right) || (m_position == NoPosition);
        drawButtonBackground(&p, opt.rect, bDrawLeft, bDrawRight, opt.state & QStyle::State_MouseOver);

        QSize size = iconSize();
        QRect rcIcon((opt.rect.width() - size.width()) / 2, (opt.rect.height() - size.height()) / 2, size.width(), size.height());
        if (opt.arrowType == Qt::RightArrow)
            rcIcon.setX((opt.rect.width() - size.width()) / 2 - TOOLBUTTON_MARGIN_WIDTH);
        opt.icon.paint(&p, rcIcon, Qt::AlignCenter, mode, state);



        if (opt.arrowType == Qt::RightArrow)
        {
            QRect rcArrow = opt.rect;
            rcArrow.setX(opt.rect.right() - TOOLBUTTON_ARRWO_WIDTH - 3);
            rcArrow.setY((opt.rect.height() - TOOLBUTTON_ARRWO_WIDTH) / 2);
            rcArrow.setSize(QSize(TOOLBUTTON_ARRWO_WIDTH, TOOLBUTTON_ARRWO_WIDTH));
            static QPixmap arrow = QPixmap(Utils::WizStyleHelper::skinResourceFileName("editorToolbarDownArrow", true));
            p.drawPixmap(rcArrow, arrow);

        }
    }

    QSize sizeHint() const
    {
        int width = m_horizontalPadding_left + m_horizontalPadding_rgiht + iconSize().width();
        if (arrowType() == Qt::RightArrow)
            return QSize(width + TOOLBUTTON_MARGIN_WIDTH, Utils::WizStyleHelper::editorButtonHeight());
        return QSize(width, Utils::WizStyleHelper::editorButtonHeight());
    }


    Position m_position;
    int m_horizontalPadding_left;
    int m_horizontalPadding_rgiht;
private:
    QColor m_colorHoverBorder;
    QColor m_colorHoverFill;
    QColor m_colorSunkenBorder;
};

const int ColorButtonRightArrowWidth = 18;
class CWizToolButtonColor : public CWizToolButton
{
public:
    CWizToolButtonColor(QWidget* parent = 0) : CWizToolButton(parent)
      , m_menu(NULL)
      , m_color(Qt::transparent)
    {
        setCheckable(false);
        setIconSize(QSize(WizSmartScaleUI(12), WizSmartScaleUI(12)));
        setPopupMode(QToolButton::MenuButtonPopup);
    }

    void setColor(const QColor& color)
    {
        m_color = color;
        repaint();
    }

    QColor color() const
    {
        return m_color;
    }

    //修复按钮点击不弹出菜单的问题
    void setMenu(QMenu* menu)
    {
        m_menu = menu;
        QToolButton::setMenu(nullptr);
    }

    QMenu* menu() const
    {
        return m_menu;
    }

protected:
    virtual void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);

        QStyleOptionToolButton opt;
        initStyleOption(&opt);

        QStylePainter p(this);
        p.setClipRect(opt.rect);

        bool bDrawLeft = (m_position == left) || (m_position == NoPosition);
        bool bDrawRight = (m_position == right) || (m_position == NoPosition);
        drawButtonBackground(&p, opt.rect, bDrawLeft, bDrawRight, opt.state & QStyle::State_MouseOver);

        //
        QIcon::Mode mode = opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
        if (mode == QIcon::Normal && (opt.state & QStyle::State_HasFocus || opt.state & QStyle::State_Sunken))
            mode = QIcon::Active;
        QIcon::State state = QIcon::Off;
        if (opt.state & QStyle::State_On)
            state = QIcon::On;

        QSize size = iconSize();
        QRect rcIcon((opt.rect.width() - size.width() - ColorButtonRightArrowWidth) / 2, (opt.rect.height() - size.height()) / 2, size.width(), size.height());
        opt.icon.paint(&p, rcIcon, Qt::AlignCenter, mode, state);

        QRect rectColor(rcIcon.x() + 1, opt.iconSize.height() + 6, opt.iconSize.width() - 2, 2);
        p.fillRect(QRect(rectColor), m_color);

        if (opt.state & QStyle::State_MouseOver)
        {
            QPoint top(rcIcon.right() + 7, opt.rect.x() + (opt.rect.height() - 13) / 2);
            p.setPen(QPen(QColor("#C4C4C4")));
            p.drawLine(top, QPoint(top.x(), top.y() + 13));
        }

        //arrow
        static QPixmap arrow = QPixmap(Utils::WizStyleHelper::skinResourceFileName("editorToolbarDownArrow", true));
        int arrowHeight = arrow.height() * (WizIsHighPixel() ? 0.5f : 1);
        QRect rcArrow(rcIcon.right() + 7, (opt.rect.height() - arrowHeight) / 2, TOOLBUTTON_ARRWO_WIDTH, TOOLBUTTON_ARRWO_WIDTH);
        p.drawPixmap(rcArrow, arrow);
    }

    void mousePressEvent(QMouseEvent* ev)
    {
        QToolButton::mousePressEvent(ev);
    }

    void mouseReleaseEvent(QMouseEvent* ev)
    {
        QPoint pos = ev->pos();
        QRect rcIcon = rect();
        rcIcon.setWidth(rcIcon.width() - ColorButtonRightArrowWidth);
        if (rcIcon.contains(pos))
        {
            QToolButton::mouseReleaseEvent(ev);
            return;
        }

        ev->ignore();
        QRect rcArrow = rect();
        rcArrow.setLeft(rcIcon.right());
        if (m_menu && rcArrow.contains(pos))
        {
            QPoint pt = mapToGlobal(rect().bottomLeft());
            m_menu->popup(pt);
        }
    }

    QSize sizeHint() const
    {
        QSize size = CWizToolButton::sizeHint();
        size.setWidth(size.width() + ColorButtonRightArrowWidth);
        return size;
    }

private:
    QColor m_color;
    QMenu* m_menu;
};

class CWizToolComboBox : public QComboBox
{
public:
    CWizToolComboBox(QWidget* parent = 0)
        : QComboBox(parent)
        , m_isPopup(false)
    {
        setFocusPolicy(Qt::NoFocus);
        setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    }

    void setText(const QString& strText)
    {
        int index = findText(strText);
        if (index != -1) {
            setCurrentIndex(index);
        }

        m_strText = strText;
        repaint();
    }
    //
    void setFontName(const QString& strFontName)
    {
        CWizStdStringArray arrayFontName;
        WizSplitTextToArray(strFontName, _T(','), arrayFontName);
        //
        QString firstName;
        //
        for (auto name : arrayFontName)
        {
            QString fontName = name;
            fontName = fontName.trimmed();
            fontName.remove("\"");
            fontName.remove("\'");
            //
            if (firstName.isEmpty())
                firstName = fontName;
            //
            int index = findText(fontName);
            if (index != -1) {
                setCurrentIndex(index);
                m_strText = fontName;
                return;
            }
        }
        //
        m_strText = firstName;
        setCurrentIndex(-1);
    }

    QString text() const { return m_strText; }

    bool isPopuping() const { return m_isPopup; }

    void	showPopup()
    {
        m_isPopup = true;
        QComboBox::showPopup();        
    }

    void hidePopup()
    {
        m_isPopup = false;
        QComboBox::hidePopup();
    }

    bool event(QEvent* event)
    {
        if (event->type() == QEvent::Paint)
        {
            //FIXME: QT5.4.1 通过点击combobox外区域来隐藏列表不会触发  hidePopup() 事件
            if (m_isPopup)
            {
                m_isPopup = false;
            }
        }
        return QComboBox::event(event);
    }

protected:
    virtual void leaveEvent(QEvent* event) {
        QComboBox::leaveEvent(event);

        update();
    }

    virtual void enterEvent(QEvent* event) {
        QComboBox::enterEvent(event);

        update();
    }

    virtual QSize sizeHint() const
    {
        return QSize(65, Utils::WizStyleHelper::editorButtonHeight());
    }

    virtual void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);

        QStyleOptionComboBox opt;
        initStyleOption(&opt);

        opt.currentText = m_strText;
        drawCombo(this, opt);
    }    

private:
    QString m_strText;
    bool m_isPopup;
};

class CWizToolComboBoxFont : public QFontComboBox
{
public:
    CWizToolComboBoxFont(QWidget* parent = 0)
        : QFontComboBox(parent)
        , m_isPopup(false)
    {
        setFocusPolicy(Qt::NoFocus);
        setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
        setEditable(false);
    }

    void setText(const QString& strText)
    {
        int index = findText(strText);
        if (index != -1) {
            setCurrentIndex(index);
        }

        m_strText = strText;
        repaint();
    }

    QString text() const { return m_strText; }

    bool isPopuping() const { return m_isPopup; }

    void	showPopup()
    {
        m_isPopup = true;
        QFontComboBox::showPopup();
    }

    void hidePopup()
    {
        m_isPopup = false;
        QFontComboBox::hidePopup();
    }

    bool event(QEvent* event)
    {
        if (event->type() == QEvent::Paint)
        {
            //FIXME: QT5.4.1 通过点击combobox外区域来隐藏列表不会触发  hidePopup() 事件
            if (m_isPopup)
            {
                m_isPopup = false;
            }
        }
        return QComboBox::event(event);
    }

protected:    
    virtual void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);

        QStyleOptionComboBox opt;
        initStyleOption(&opt);

        opt.currentText = m_strText;
        drawCombo(this, opt);
    }

    virtual QSize sizeHint() const
    {
        return QSize(100, Utils::WizStyleHelper::editorButtonHeight());
    }

private:
    QString m_strText;
    bool m_isPopup;
};


class CWizEditorButtonSpliter : public QWidget
{
public:
    CWizEditorButtonSpliter(QWidget* parent = 0)
        : QWidget(parent)
    {
        setFixedWidth(1);
        setFixedHeight(Utils::WizStyleHelper::editorButtonHeight());
        setStyleSheet("background-color:#E7E7E7;");
        setAutoFillBackground(true);
    }
};


QWidget* createMoveAbleWidget(QWidget* parent)
{
    QWidget*  moveableButtonContainer = new QWidget(parent);
    QHBoxLayout* moveableLayout = new QHBoxLayout(moveableButtonContainer);
    moveableLayout->setContentsMargins(0, 0, 0, 0);
    moveableLayout->setSpacing(0);
    moveableLayout->setAlignment(Qt::AlignVCenter);
    moveableButtonContainer->setLayout(moveableLayout);

    return moveableButtonContainer;
}

QString commandKey()
{
#ifdef Q_OS_MAC
    return "⌘";
#else
    return "Ctrl+";
#endif
}

QString optionKey()
{
#ifdef Q_OS_MAC
    return "⌥";
#else
    return "Alt+";
#endif
}
QString shiftKey()
{
#ifdef Q_OS_MAC
    return "⇧";
#else
    return "Shift+";
#endif
}

WizEditorToolBar::WizEditorToolBar(WizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
    , m_lastUpdateUIRequest(0)
{
    setContentsMargins(0, 4, 0, 0);

    QString skin = Utils::WizStyleHelper::themeName();

    m_comboParagraph = new CWizToolComboBox(this);    
    m_comboParagraph->setFixedWidth(90);


    WizComboboxStyledItem* paraItems = ParagraphItems();    
    for (int i = 0; i < nParagraphItemCount; i ++)
    {
        m_comboParagraph->addItem(paraItems[i].strText, paraItems[i].strUserData);
    }
    //
    connect(m_comboParagraph, SIGNAL(activated(int)),
            SLOT(on_comboParagraph_indexChanged(int)));

    //
    m_comboFontFamily = new CWizToolComboBox(this);
    m_comboFontFamily->setFixedWidth(122);

    m_comboFontSize = new CWizToolComboBox(this);
    m_comboFontSize->setFixedWidth(48);
    WizComboboxStyledItem* fontItems = FontSizes();
#ifdef Q_OS_MAC
    m_comboParagraph->setStyleSheet("QComboBox QListView{min-width:95px;background:#F6F6F6;}"
                                    "QComboBox QAbstractItemView::item {min-height:20px;background:transparent;}");
    WizToolComboboxItemDelegate* paragraphDelegate = new WizToolComboboxItemDelegate(m_comboParagraph, m_comboParagraph, paraItems, nParagraphItemCount);
    m_comboParagraph->setItemDelegate(paragraphDelegate);
    //
    m_comboFontFamily->setStyleSheet("QComboBox QListView{min-width:150px;background:#F6F6F6;}"
                                     "QComboBox QAbstractItemView::item {min-height:30px;background:transparent;}");
    WizToolComboboxItemDelegate* fontFamilyDelegate = new WizToolComboboxItemDelegate(m_comboFontFamily, m_comboFontFamily, paraItems, nParagraphItemCount);
    m_comboFontFamily->setItemDelegate(fontFamilyDelegate);
    //
    m_comboFontSize->setStyleSheet("QComboBox QListView{min-width:50px;background:#F6F6F6;}"
                                   "QComboBox QAbstractItemView::item {min-height:20px;background:transparent;}");
    WizToolComboboxItemDelegate* fontDelegate = new WizToolComboboxItemDelegate(m_comboParagraph, m_comboParagraph, fontItems, nFontSizeCount);
    m_comboFontSize->setItemDelegate(fontDelegate);
#endif

    QStringList fontList = m_app.userSettings().get(WIZRECENTFONTLIST).split('/', QString::SkipEmptyParts);
    WizComboboxStyledItem* fontFamilyItems = FontFamilies();
    for (QString recent : fontList)
    {
        WizComboboxStyledItem familyItem = itemFromArrayByKey(recent, fontFamilyItems, nFontFamilyCount);
        m_comboFontFamily->addItem(familyItem.strText.isEmpty() ? recent : familyItem.strText, recent);
        m_comboFontFamily->setItemData(m_comboFontFamily->count() - 1, WIZRECENTFONT, WizFontFamilyHelperRole);
    }

    if (!fontList.isEmpty())
    {
        m_comboFontFamily->addItem("separator");
        m_comboFontFamily->setItemData(m_comboFontFamily->count() - 1, WIZSEPARATOR, WizFontFamilyHelperRole);
    }

    for (int i = 0; i < nCommonlyUsedFontCount; i++)
    {
        QString font = CommonlyUsedFont[i];
        WizComboboxStyledItem familyItem = itemFromArrayByKey(font, fontFamilyItems, nFontFamilyCount);
         m_comboFontFamily->addItem(familyItem.strText.isEmpty() ? font : familyItem.strText, font);
    }

    m_comboFontFamily->addItem("separator");
    m_comboFontFamily->setItemData(m_comboFontFamily->count() - 1, WIZSEPARATOR, WizFontFamilyHelperRole);

    m_comboFontFamily->addItem(tr("Font Panel"), WIZFONTPANEL);
    m_comboFontFamily->setItemData(m_comboFontFamily->count() - 1, WIZFONTPANEL, WizFontFamilyHelperRole);

    connect(m_comboFontFamily, SIGNAL(activated(int)),
            SLOT(on_comboFontFamily_indexChanged(int)));

    for (int i = 0; i < nFontSizeCount; i++)
    {
        m_comboFontSize->addItem(fontItems[i].strText, fontItems[i].strUserData);
    }
    connect(m_comboFontSize, SIGNAL(activated(const QString&)),
            SLOT(on_comboFontSize_indexChanged(const QString&)));

    m_btnFormatPainter = new CWizToolButton(this);
    m_btnFormatPainter->setIcon(::WizLoadSkinIcon(skin, "formatter"));
    //m_btnFormatPainter->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatRemoveFormat")).size());
    m_btnFormatPainter->setToolTip(tr("Format Painter"));
    m_btnFormatPainter->setCheckable(true);
    m_btnFormatPainter->setChecked(false);
    m_btnFormatPainter->setPosition(CWizToolButton::left);
    connect(m_btnFormatPainter, SIGNAL(clicked(bool)), SLOT(on_btnFormatPainter_clicked(bool)));
    connect(m_btnFormatPainter, SIGNAL(dblClicked()), SLOT(on_btnFormatPainter_dblClicked()));
    //
    m_btnRemoveFormat = new CWizToolButton(this);
    m_btnRemoveFormat->setIcon(::WizLoadSkinIcon(skin, "actionFormatRemoveFormat"));
    //m_btnRemoveFormat->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatRemoveFormat")).size());
    m_btnRemoveFormat->setToolTip(tr("Remove Format"));
    m_btnRemoveFormat->setCheckable(false);
    m_btnRemoveFormat->setPosition(CWizToolButton::right);
    connect(m_btnRemoveFormat, SIGNAL(clicked()), SLOT(on_btnRemoveFormat_clicked()));

    m_btnForeColor = new CWizToolButtonColor(this);
    m_btnForeColor->setIcon(::WizLoadSkinIcon(skin, "actionFormatForeColor"));
    m_btnForeColor->setColor(QColor("#ff0000"));
    //m_btnForeColor->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatForeColor")).size());
    m_btnForeColor->setToolTip(tr("ForeColor"));
    m_btnForeColor->setCheckable(false);
    m_btnForeColor->setPosition(CWizToolButton::left);
    connect(m_btnForeColor, SIGNAL(released()), SLOT(on_btnForeColor_clicked()));
    QMenu* foreColorMenu = createColorMenu(SLOT(on_foreColor_changed()),
                                           SLOT(on_showForeColorBoard()));
    m_btnForeColor->setMenu(foreColorMenu);

    m_btnBackColor = new CWizToolButtonColor(this);
    m_btnBackColor->setIcon(::WizLoadSkinIcon(skin, "actionFormatBackColor"));
    m_btnBackColor->setColor(QColor("#ffff00"));
    //m_btnBackColor->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatBackColor")).size());
    m_btnBackColor->setToolTip(tr("BackColor"));
    m_btnBackColor->setCheckable(false);
    m_btnBackColor->setPosition(CWizToolButton::right);
    connect(m_btnBackColor, SIGNAL(released()), SLOT(on_btnBackColor_clicked()));
    QMenu* backColorMenu = createColorMenu(SLOT(on_backColor_changed()),
                                           SLOT(on_showBackColorBoard()));
    m_btnBackColor->setMenu(backColorMenu);
    m_btnBold = new CWizToolButton(this);
    m_btnBold->setIcon(::WizLoadSkinIcon(skin, "actionFormatBold"));
    //m_btnBold->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatBold")).size());
    m_btnBold->setToolTip(tr("Bold %1B").arg(commandKey()));
    m_btnBold->setPosition(CWizToolButton::left);
    connect(m_btnBold, SIGNAL(clicked()), SLOT(on_btnBold_clicked()));

    m_btnItalic = new CWizToolButton(this);
    m_btnItalic->setIcon(::WizLoadSkinIcon(skin, "actionFormatItalic"));
    m_btnItalic->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatItalic")).size());
    m_btnItalic->setToolTip(tr("Italic %1I").arg(commandKey()));
    m_btnItalic->setPosition(CWizToolButton::Center);
    connect(m_btnItalic, SIGNAL(clicked()), SLOT(on_btnItalic_clicked()));

    m_btnShowExtra = new CWizToolButton(this);
    m_btnShowExtra->setIcon(::WizLoadSkinIcon(skin, "actionFormatExtra"));
    //m_btnShowExtra->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatExtra")).size());
    m_btnShowExtra->setToolTip(tr("Extra"));
    m_btnShowExtra->setPosition(CWizToolButton::NoPosition);
    connect(m_btnShowExtra, SIGNAL(clicked()), SLOT(on_btnShowExtra_clicked()));

    m_btnUnderLine = new CWizToolButton(this);
    m_btnUnderLine->setIcon(::WizLoadSkinIcon(skin, "actionFormatUnderLine"));
    //m_btnUnderLine->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatUnderLine")).size());
    m_btnUnderLine->setToolTip(tr("Underline %1U").arg(commandKey()));
    m_btnUnderLine->setPosition(CWizToolButton::Center);
    connect(m_btnUnderLine, SIGNAL(clicked()), SLOT(on_btnUnderLine_clicked()));

    m_btnStrikeThrough = new CWizToolButton(this);
    m_btnStrikeThrough->setIcon(::WizLoadSkinIcon(skin, "actionFormatStrikeThrough"));
    //m_btnStrikeThrough->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatStrikeThrough")).size());
    m_btnStrikeThrough->setToolTip(tr("Strike Through %1%2K").arg(optionKey()).arg(commandKey()));
    m_btnStrikeThrough->setPosition(CWizToolButton::right);
    connect(m_btnStrikeThrough, SIGNAL(clicked()), SLOT(on_btnStrikeThrough_clicked()));

    m_btnJustify = new CWizToolButton(this);
    m_btnJustify->setIcon(::WizLoadSkinIcon(skin, "actionFormatJustifyLeft"));
    //m_btnJustify->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatJustifyLeft")).size());
    m_btnJustify->setCheckable(false);
    m_btnJustify->setArrowType(Qt::RightArrow);
    m_btnJustify->setPopupMode(QToolButton::MenuButtonPopup);
    m_btnJustify->setToolTip(tr("Justify"));
    m_btnJustify->setPosition(CWizToolButton::NoPosition);
    connect(m_btnJustify, SIGNAL(clicked()), SLOT(on_btnJustify_clicked()));
    m_menuJustify = new QMenu(m_btnJustify);
    m_actionJustifyLeft = m_menuJustify->addAction(::WizLoadSkinIcon(skin, "actionFormatJustifyLeft"),
                             tr("Justify Left"), this, SLOT(on_btnJustifyLeft_clicked()));
    m_actionJustifyLeft->setCheckable(true);
    m_actionJustifyCenter = m_menuJustify->addAction(::WizLoadSkinIcon(skin, "actionFormatJustifyCenter"),
                             tr("Justify Center"), this, SLOT(on_btnJustifyCenter_clicked()));
    m_actionJustifyCenter->setCheckable(true);
    m_actionJustifyRight = m_menuJustify->addAction(::WizLoadSkinIcon(skin, "actionFormatJustifyRight"),
                             tr("Justify Right"), this, SLOT(on_btnJustifyRight_clicked()));
    m_actionJustifyRight->setCheckable(true);
    m_btnJustify->setMenu(m_menuJustify);
    //
    QActionGroup* alignGroup = new QActionGroup(this);
    alignGroup->setExclusive(true);
    alignGroup->addAction(m_actionJustifyLeft);
    alignGroup->addAction(m_actionJustifyCenter);
    alignGroup->addAction(m_actionJustifyRight);

    m_btnUnorderedList = new CWizToolButton(this);
    m_btnUnorderedList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertUnorderedList"));
    //m_btnUnorderedList->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatInsertUnorderedList")).size());
    m_btnUnorderedList->setToolTip(tr("UnorderedList %1%2U").arg(optionKey()).arg(commandKey()));
    m_btnUnorderedList->setPosition(CWizToolButton::left);
    connect(m_btnUnorderedList, SIGNAL(clicked()), SLOT(on_btnUnorderedList_clicked()));

    m_btnOrderedList = new CWizToolButton(this);
    m_btnOrderedList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertOrderedList"));
    //m_btnOrderedList->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatInsertOrderedList")).size());
    m_btnOrderedList->setToolTip(tr("OrderedList %1%2O").arg(optionKey()).arg(commandKey()));
    m_btnOrderedList->setPosition(CWizToolButton::right);
    connect(m_btnOrderedList, SIGNAL(clicked()), SLOT(on_btnOrderedList_clicked()));

    QWidgetAction* tableAction = new QWidgetAction(this);
    tableAction->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertTable"));
    tableAction->setText(tr("Insert Table"));
    WizTableSelectorWidget* tableWidget = new WizTableSelectorWidget(this);
    tableAction->setDefaultWidget(tableWidget);
    //
    connect(tableWidget, SIGNAL(itemSelected(int,int)), SLOT(on_btnTable_clicked(int,int)));

    //
    QMenu* menuTable = new QMenu(this);
    menuTable->addAction(tableAction);
    //
    m_btnTable = new CWizToolButton(this);
    m_btnTable->setCheckable(false);
//    m_btnTable->setHorizontalPadding(8);
    m_btnTable->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertTable"));
    //m_btnTable->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatInsertTable")).size());
    m_btnTable->setToolTip(tr("Insert Table"));
    m_btnTable->setPosition(CWizToolButton::Center);
    m_btnTable->setMenu(menuTable);
    m_btnTable->setPopupMode(QToolButton::MenuButtonPopup);
    //

    m_btnHorizontal = new CWizToolButton(this);
    m_btnHorizontal->setCheckable(false);
    m_btnHorizontal->setHorizontalPadding(9);
    m_btnHorizontal->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertHorizontal"));
    //m_btnHorizontal->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatInsertHorizontal")).size());
    m_btnHorizontal->setToolTip(tr("Insert Horizontal %1%2H").arg(shiftKey()).arg(commandKey()));
    m_btnHorizontal->setPosition(CWizToolButton::Center);
    connect(m_btnHorizontal, SIGNAL(clicked()), SLOT(on_btnHorizontal_clicked()));

    m_btnCheckList = new CWizToolButton(this);
    m_btnCheckList->setCheckable(false);
    m_btnCheckList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertCheckList"));
    //m_btnCheckList->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatInsertCheckList")).size());
    m_btnCheckList->setToolTip(tr("Insert Checklist %1O").arg(commandKey()));
    m_btnCheckList->setPosition(CWizToolButton::left);
    connect(m_btnCheckList, SIGNAL(clicked()), SLOT(on_btnCheckList_clicked()));

    m_btnInsertLink = new CWizToolButton(this);
    m_btnInsertLink->setCheckable(false);
    m_btnInsertLink->setHorizontalPadding(6, 10);
    m_btnInsertLink->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertLink"));
    //m_btnInsertLink->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatInsertLink")).size());
    m_btnInsertLink->setToolTip(tr("Insert Link %1K").arg(commandKey()));
    m_btnInsertLink->setPosition(CWizToolButton::Center);
    connect(m_btnInsertLink, SIGNAL(clicked()), SLOT(on_btnInsertLink_clicked()));

    m_btnInsertImage = new CWizToolButton(this);
    m_btnInsertImage->setCheckable(false);
    m_btnInsertImage->setHorizontalPadding(8);
    m_btnInsertImage->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertImage"));
    //m_btnInsertImage->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatInsertImage")).size());
    m_btnInsertImage->setToolTip(tr("Insert Image %1%2I").arg(shiftKey()).arg(commandKey()));
    m_btnInsertImage->setPosition(CWizToolButton::Center);
    connect(m_btnInsertImage, SIGNAL(clicked()), SLOT(on_btnInsertImage_clicked()));

    m_btnInsertDate = new CWizToolButton(this);
    m_btnInsertDate->setCheckable(false);
    m_btnInsertDate->setHorizontalPadding(8, 10);
    m_btnInsertDate->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertDate"));
    //m_btnInsertDate->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatInsertDate")).size());
    m_btnInsertDate->setToolTip(tr("Insert Date %1%2D").arg(shiftKey()).arg(commandKey()));
    m_btnInsertDate->setPosition(CWizToolButton::right);
    connect(m_btnInsertDate, SIGNAL(clicked()), SLOT(on_btnInsertDate_clicked()));

    m_btnMobileImage = new CWizToolButton(this);
//    m_btnMobileImage->setHorizontalPadding(6);
    m_btnMobileImage->setIcon(::WizLoadSkinIcon(skin, "actionMobileImage"));
    //m_btnMobileImage->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionMobileImage")).size());
    m_btnMobileImage->setToolTip(tr("Receive mobile image"));
    m_btnMobileImage->setPosition(CWizToolButton::Center);
    connect(m_btnMobileImage, SIGNAL(clicked()), SLOT(on_btnMobileImage_clicked()));

    m_btnSearchReplace = new CWizToolButton(this);
    m_btnSearchReplace->setCheckable(false);
    m_btnSearchReplace->setIcon(::WizLoadSkinIcon(skin, "actionFormatSearchReplace"));
    //m_btnSearchReplace->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatSearchReplace")).size());
    m_btnSearchReplace->setToolTip(tr("Find & Replace %1F").arg(commandKey()));
    m_btnSearchReplace->setPosition(CWizToolButton::right);
    connect(m_btnSearchReplace, SIGNAL(clicked()), SLOT(on_btnSearchReplace_clicked()));

#ifndef Q_OS_MAC
    m_btnScreenShot = new CWizToolButton(this);
    m_btnScreenShot->setCheckable(false);
    m_btnScreenShot->setIcon(::WizLoadSkinIcon(skin, "actionFormatScreenShot"));
    m_btnScreenShot->setToolTip(tr("Screen shot"));
    m_btnScreenShot->setPosition(CWizToolButton::Center);
    connect(m_btnScreenShot, SIGNAL(clicked()), SLOT(on_btnScreenShot_clicked()));
#else
    m_btnScreenShot = 0;
#endif

    m_btnInsertCode = new CWizToolButton(this);
    m_btnInsertCode->setCheckable(false);
    m_btnInsertCode->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertCode"));
    //m_btnInsertCode->setIconSize(QPixmap(WizGetSkinResourceFileName(skin, "actionFormatInsertCode")).size());
    m_btnInsertCode->setToolTip(tr("Insert code %1%2C").arg(shiftKey()).arg(commandKey()));
    m_btnInsertCode->setPosition(CWizToolButton::left);
    connect(m_btnInsertCode, SIGNAL(clicked()), SLOT(on_btnInsertCode_clicked()));


    m_firstLineButtonContainer = new QWidget(this);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignVCenter);
    m_firstLineButtonContainer->setLayout(layout);

    QWidget*  buttonContainer0 = createMoveAbleWidget(this);
    QHBoxLayout* containerLayout = qobject_cast<QHBoxLayout*>(buttonContainer0->layout());
    containerLayout->addWidget(m_comboParagraph);
    containerLayout->addSpacing(18);
    containerLayout->addWidget(m_comboFontFamily);
    containerLayout->addSpacing(18);
    containerLayout->addWidget(m_comboFontSize);
    containerLayout->addSpacing(16);

    layout->addWidget(buttonContainer0);

    QWidget*  buttonContainer1 = createMoveAbleWidget(this);
    QHBoxLayout* containerLayout1 = qobject_cast<QHBoxLayout*>(buttonContainer1->layout());
    containerLayout1->addWidget(m_btnBold);
    containerLayout1->addWidget(new CWizEditorButtonSpliter(this));
    containerLayout1->addWidget(m_btnItalic);
    containerLayout1->addWidget(new CWizEditorButtonSpliter(this));
    containerLayout1->addWidget(m_btnUnderLine);
    containerLayout1->addWidget(new CWizEditorButtonSpliter(this));
    containerLayout1->addWidget(m_btnStrikeThrough);
    containerLayout1->addSpacing(12);
    containerLayout1->addWidget(m_btnForeColor);
    containerLayout1->addWidget(new CWizEditorButtonSpliter(this));
    containerLayout1->addWidget(m_btnBackColor);
    containerLayout1->addSpacing(12);

    layout->addWidget(buttonContainer1);


    QWidget*  moveableButtonContainer1 = createMoveAbleWidget(this);
    moveableButtonContainer1->layout()->addWidget(m_btnJustify);
    qobject_cast<QHBoxLayout*>(moveableButtonContainer1->layout())->addSpacing(12);

    layout->addWidget(moveableButtonContainer1);

    QWidget*  moveableButtonContainer2 = createMoveAbleWidget(this);
    QHBoxLayout* moveableLayout2 = qobject_cast<QHBoxLayout*>(moveableButtonContainer2->layout());
    moveableLayout2->addWidget(m_btnUnorderedList);
    moveableLayout2->addWidget(new CWizEditorButtonSpliter(this));
    moveableLayout2->addWidget(m_btnOrderedList);
    moveableLayout2->addSpacing(12);

    layout->addWidget(moveableButtonContainer2);
    layout->addWidget(m_btnShowExtra);

    QWidget* firstLineWidget = createMoveAbleWidget(this);
    firstLineWidget->layout()->addWidget(m_firstLineButtonContainer);
    qobject_cast<QHBoxLayout*>(firstLineWidget->layout())->addStretch();

    m_buttonContainersInFirstLine.append(buttonContainer0);
    m_buttonContainersInFirstLine.append(buttonContainer1);
    m_buttonContainersInFirstLine.append(moveableButtonContainer1);
    m_buttonContainersInFirstLine.append(moveableButtonContainer2);

    m_secondLineButtonContainer = new QWidget(this);
    QHBoxLayout* hLayout = new QHBoxLayout(m_secondLineButtonContainer);
    hLayout->setContentsMargins(0, 1, 0, 1);
    hLayout->setSpacing(0);

    QWidget*  moveableButtonContainer3 = createMoveAbleWidget(this);
    QHBoxLayout* moveableLayout3 = qobject_cast<QHBoxLayout*>(moveableButtonContainer3->layout());
    moveableLayout3->addWidget(m_btnCheckList);
    moveableLayout3->addWidget(m_btnInsertLink);
    moveableLayout3->addWidget(new CWizEditorButtonSpliter(this));
    moveableLayout3->addWidget(m_btnHorizontal);
    moveableLayout3->addWidget(m_btnInsertImage);
    moveableLayout3->addWidget(m_btnMobileImage);
    moveableLayout3->addWidget(new CWizEditorButtonSpliter(this));
    moveableLayout3->addWidget(m_btnTable);
    moveableLayout3->addWidget(m_btnInsertDate);
    moveableLayout3->addSpacing(12);

    hLayout->addWidget(moveableButtonContainer3);


    QWidget*  moveableButtonContainer4 = createMoveAbleWidget(this);
    QHBoxLayout* moveableLayout4 = qobject_cast<QHBoxLayout*>(moveableButtonContainer4->layout());
    //
    moveableLayout4->addWidget(m_btnFormatPainter);
    moveableLayout4->addWidget(new CWizEditorButtonSpliter(this));
    moveableLayout4->addWidget(m_btnRemoveFormat);
    moveableLayout4->addSpacing(12);

    hLayout->addWidget(moveableButtonContainer4);


    QWidget*  moveableButtonContainer5 = createMoveAbleWidget(this);
    QHBoxLayout* moveableLayout5 = qobject_cast<QHBoxLayout*>(moveableButtonContainer5->layout());
    moveableLayout5->addWidget(m_btnInsertCode);
#ifndef Q_OS_MAC
    hLayout->addWidget(m_btnScreenShot);
#endif
    moveableLayout5->addWidget(new CWizEditorButtonSpliter(this));
    moveableLayout5->addWidget(m_btnSearchReplace);

    hLayout->addWidget(moveableButtonContainer5);
    hLayout->addStretch();


    m_buttonContainersInSecondLine.append(moveableButtonContainer3);
    m_buttonContainersInSecondLine.append(moveableButtonContainer4);
    m_buttonContainersInSecondLine.append(moveableButtonContainer5);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(8);
    vLayout->addWidget(firstLineWidget);
    vLayout->addWidget(m_secondLineButtonContainer);

    int nHeight = Utils::WizStyleHelper::editToolBarHeight();
    m_firstLineButtonContainer->setFixedHeight(nHeight);
    m_secondLineButtonContainer->setFixedHeight(nHeight);

    bool showExtraButtons = m_app.userSettings().get(WIZSHOWEXTRABUTTONITEMS).toInt();
    m_secondLineButtonContainer->setVisible(showExtraButtons);
    m_btnShowExtra->setChecked(showExtraButtons);

    m_delayUpdateUITimer.setInterval(100);
    connect(&m_delayUpdateUITimer, SIGNAL(timeout()), SLOT(on_delay_updateToolbar()));
}

void WizEditorToolBar::resetToolbar(const QString& currentStyle)
{
    Q_ASSERT(m_editor);
    //
    Json::Value d;
    Json::Reader reader;
    if (!reader.parse(currentStyle.toUtf8().constData(), d))
        return;
    //
    CString strBlockFormat = QString::fromStdString(d["blockFormat"].asString());
    CString strForeColor = QString::fromStdString(d["foreColor"].asString());
    CString strBackColor = QString::fromStdString(d["backColor"].asString());
    //
    CString strFontName = QString::fromStdString(d["fontName"].asString());
    CString strFontSize = QString::fromStdString(d["fontSize"].asString());
    //
    bool subscript = QString::fromStdString(d["subscript"].asString()) == "1";
    bool superscript = QString::fromStdString(d["superscript"].asString()) == "1";
    //
    bool bold = QString::fromStdString(d["bold"].asString()) == "1";
    bool italic = QString::fromStdString(d["italic"].asString()) == "1";
    bool underline = QString::fromStdString(d["underline"].asString()) == "1";
    bool strikeThrough = QString::fromStdString(d["strikeThrough"].asString()) == "1";
    //
    bool justifyleft = QString::fromStdString(d["justifyleft"].asString()) == "1";
    bool justifycenter = QString::fromStdString(d["justifycenter"].asString()) == "1";
    bool justifyright = QString::fromStdString(d["justifyright"].asString()) == "1";
    bool justifyfull = QString::fromStdString(d["justifyfull"].asString()) == "1";
    //
    bool InsertOrderedList = QString::fromStdString(d["InsertOrderedList"].asString()) == "1";
    bool InsertUnorderedList = QString::fromStdString(d["InsertUnorderedList"].asString()) == "1";
    //
    bool canInsertTable = QString::fromStdString(d["canCreateTable"].asString()) == "1";
    bool canCreateCode = QString::fromStdString(d["canCreateCode"].asString()) == "1";
    bool canCreateTodo = QString::fromStdString(d["canCreateTodo"].asString()) == "1";
    int formatPainterStatus = QString::fromStdString(d["formatPainterStatus"].asString()).toInt();

    //
    bool blockFormatSetted = false;
    WizComboboxStyledItem* paraItems = ParagraphItems();
    for (int i = 0; i < nParagraphItemCount; i ++)
    {
        if (0 == paraItems[i].strUserData.compare(strBlockFormat, Qt::CaseInsensitive))
        {
            m_comboParagraph->setText(paraItems[i].strText);
            blockFormatSetted = true;
            break;
        }
    }
    if (!blockFormatSetted)
    {
        m_comboParagraph->setText("");
    }

    //
    m_comboFontFamily->setFontName(strFontName);
    //
    strFontSize.remove("pt");
    int fontSizeInPt = wiz_ttoi(strFontSize);
    if (0 != fontSizeInPt)
    {
        CString strsFontsizeInPt = WizIntToStr(fontSizeInPt);
        m_comboFontSize->setText(strsFontsizeInPt);
    }
    else
    {
        m_comboFontSize->setText("");
    }

    //m_btnForeColor->setColor(QColor(strForeColor));
    //m_btnBackColor->setColor(QColor(strBackColor));

    m_btnBold->setChecked(bold);
    m_btnItalic->setChecked(italic);
    m_btnUnderLine->setChecked(underline);
    m_btnStrikeThrough->setChecked(strikeThrough);

    m_actionJustifyLeft->setChecked(justifyleft);
    m_actionJustifyCenter->setChecked(justifycenter);
    m_actionJustifyRight->setChecked(justifyright);

    m_btnOrderedList->setChecked(InsertOrderedList);
    m_btnUnorderedList->setChecked(InsertUnorderedList);

    m_btnTable->setEnabled(canInsertTable);
    m_btnInsertCode->setEnabled(canCreateCode);
    m_btnCheckList->setEnabled(canCreateTodo);
    m_btnFormatPainter->setEnabled(formatPainterStatus != 0);
    m_btnFormatPainter->setChecked(formatPainterStatus == 2);

    bool bReceiveImage = m_editor->editorCommandQueryMobileFileReceiverState();
    m_btnMobileImage->setChecked(bReceiveImage);
    m_btnMobileImage->setEnabled(true);
}



#define WIZEDITOR_ACTION_GOOGLE         QObject::tr("Use \"Google\" search")
#define WIZEDITOR_ACTION_BAIDU           QObject::tr("Use \"Baidu\" search")

#define WIZEDITOR_ACTION_CUT            QObject::tr("Cut")
#define WIZEDITOR_ACTION_COPY           QObject::tr("Copy")
#define WIZEDITOR_ACTION_PASTE          QObject::tr("Paste")
#define WIZEDITOR_ACTION_PASTE_PLAIN    QObject::tr("Paste plain text")
#define WIZEDITOR_ACTION_REMOVE_LINK    QObject::tr("Remove link")


void WizEditorToolBar::setDelegate(WizDocumentWebView* editor)
{
    Q_ASSERT(editor);

    m_editor = editor;

    connect(m_editor, SIGNAL(showContextMenuRequest(QPoint)),
            SLOT(on_delegate_showContextMenuRequest(QPoint)), Qt::QueuedConnection);

    connect(m_editor, SIGNAL(statusChanged(const QString&)),
            SLOT(on_delegate_selectionChanged(const QString&)));
}

static std::map<QString, QWebEnginePage::WebAction> g_webActions;
//
void initWebActions(QWebEnginePage* page)
{
    if (!g_webActions.empty())
        return;
    //
    for (int action = QWebEnginePage::NoWebAction + 1;
         action < QWebEnginePage::WebActionCount;
         action++)
    {
        QWebEnginePage::WebAction a = (QWebEnginePage::WebAction)action;
        QAction* actionObj = page->action(a);
        //
        QString text = actionObj->text();
        g_webActions[text] = a;
        //
        text = text.replace("&", "");
        g_webActions[text] = a;
    }
}
//
QWebEnginePage::WebAction menuText2WebAction(QWebEnginePage* page, QString text)
{
    initWebActions(page);
    //
    text = text.replace("&", "");
    auto it = g_webActions.find(text);
    if (it == g_webActions.end())
        return QWebEnginePage::NoWebAction;
    //
    return it->second;
}


void WizEditorToolBar::on_delegate_showContextMenuRequest(const QPoint& pos)
{
    if (!m_editor)
        return;
    //
    QWebEnginePage* page = m_editor->page();
    if (!page)
        return;
    //
    QMenu *menu = page->createStandardContextMenu();
    if (!menu)
        return;
    //
    bool editing = m_editor->isEditing();
    //
    bool hasPasteMenu = false;
    bool hasLinkMenu = false;
    //
    QList<QAction*> actions = menu->actions();
    for (QAction* action : actions)
    {
        QWebEnginePage::WebAction a = menuText2WebAction(page, action->iconText());
        switch (a)
        {
        case QWebEnginePage::Copy:
            action->setText(QObject::tr("Copy"));
            break;
        case QWebEnginePage::Unselect:
            action->setText(QObject::tr("Unselect"));
            break;
        case QWebEnginePage::Back:
        case QWebEnginePage::Forward:
        case QWebEnginePage::Stop:
        case QWebEnginePage::Reload:
#if QT_VERSION >= 0x050600
        case QWebEnginePage::DownloadImageToDisk:
#endif
#if QT_VERSION >= 0x050800
        case QWebEnginePage::ViewSource:
#endif
            menu->removeAction(action);
            break;
            //
        case QWebEnginePage::Paste:
            hasPasteMenu = true;
            break;
        case QWebEnginePage::OpenLinkInThisWindow:
        case QWebEnginePage::OpenLinkInNewWindow:
        case QWebEnginePage::OpenLinkInNewTab:
        case QWebEnginePage::DownloadLinkToDisk:
            menu->removeAction(action);
            hasLinkMenu = true;
            break;
        case QWebEnginePage::CopyLinkToClipboard:
            hasLinkMenu = true;
            break;
        default:
            break;
        }
    }
    //
    if (!m_editor->selectedText().isEmpty())
    {
        if (!menu->actions().isEmpty())
        {
            menu->addSeparator();
        }
        menu->addAction(WIZEDITOR_ACTION_GOOGLE, this, SLOT(on_editor_google_triggered()));
        menu->addAction(WIZEDITOR_ACTION_BAIDU, this, SLOT(on_editor_baidu_triggered()));
    }
    //
    if (editing)
    {
        if (!hasPasteMenu)
        {
            if (!menu->actions().isEmpty())
            {
                menu->addSeparator();
            }

            menu->addAction(WIZEDITOR_ACTION_PASTE, this, SLOT(on_editor_paste_triggered()));
            menu->addAction(WIZEDITOR_ACTION_PASTE_PLAIN, this, SLOT(on_editor_pastePlain_triggered()));
        }
        //
        if (hasLinkMenu)
        {
            menu->addAction(WIZEDITOR_ACTION_REMOVE_LINK, this, SLOT(on_editor_removeLink_triggered()));
        }
    }
    //
    if (menu->actions().isEmpty())
        return;
    //
    menu->popup(pos);

    WizGetAnalyzer().logAction("editorContextMenu");
}

/*
 * 此处对slectionChanged引起的刷新做延迟和屏蔽处理。在输入中文的时候频繁的刷新会引起输入卡顿的问题
 */
void WizEditorToolBar::on_delegate_selectionChanged(const QString& currentStyle)
{
    if (m_currentStyle == currentStyle)
        return;
    //
    m_currentStyle = currentStyle;
    m_lastUpdateUIRequest = WizGetTickCount();
    //
    if (!m_delayUpdateUITimer.isActive())
    {
        m_delayUpdateUITimer.start();
    }
}

void WizEditorToolBar::on_delay_updateToolbar()
{
    if (m_currentStyle.isEmpty())
        return;
    if (0 == m_lastUpdateUIRequest)
        return;
    //
    int delay = WizGetTickCount() - m_lastUpdateUIRequest;
    if (delay < 250)
        return;
    //
    resetToolbar(m_currentStyle);
    m_lastUpdateUIRequest = 0;
    //
#ifdef QT_DEBUG
    qDebug() << "update editor toolbar ui";
#endif
}



QMenu* WizEditorToolBar::createColorMenu(const char *slot, const char *slotColorBoard)
{
    /*
    // 设置透明色
    QAction *pActionTransparent = new QAction(this);
    pActionTransparent->setData(QColor(0, 0, 0, 0));
    pActionTransparent->setText(tr("transparent"));
    connect(pActionTransparent, SIGNAL(triggered()), this, slot);
    QToolButton *pBtnTransparent = new QToolButton(this);
    pBtnTransparent->setFixedSize(QSize(110, 20));
    pBtnTransparent->setText(tr("transparent"));
    pBtnTransparent->setDefaultAction(pActionTransparent);
    */

    // 选择其他颜色
    QToolButton *pBtnOtherColor = new QToolButton(this);
    pBtnOtherColor->setText(tr("show more colors..."));
    pBtnOtherColor->setFixedSize(QSize(110, 20));
    pBtnOtherColor->setAutoRaise(true);
    pBtnOtherColor->setToolTip(tr("show more colors..."));
    connect(pBtnOtherColor, SIGNAL(clicked()), this, slotColorBoard);

    // 基本色
    QGridLayout *pGridLayout = new QGridLayout;
    pGridLayout->setAlignment(Qt::AlignCenter);
    pGridLayout->setContentsMargins(0, 0, 0, 0);
    pGridLayout->setSpacing(2);

    for (int iRow = 0; iRow < nMacColorRow; iRow++)
    {
        for (int iCol = 0; iCol < nMacColorColum; iCol++)
        {
            QAction *action = new QAction(this);
            action->setData(macColors[iRow][iCol]);
            action->setIcon(createColorIcon(macColors[iRow][iCol]));
            connect(action, SIGNAL(triggered()), this, slot);

            QToolButton *pBtnColor = new QToolButton(this);
            pBtnColor->setFixedSize(QSize(16, 16));
            pBtnColor->setAutoRaise(true);
            pBtnColor->setDefaultAction(action);

            pGridLayout->addWidget(pBtnColor, iRow, iCol);
        }
    }

    QMenu *colorMenu = new QMenu(this);
    //
    QWidget *widget = new QWidget(colorMenu);
    widget->setLayout(pGridLayout);

    QVBoxLayout *pVLayout = new QVBoxLayout;
    pVLayout->setContentsMargins(5, 5, 5, 5);
    //pVLayout->addWidget(pBtnTransparent);
    pVLayout->addWidget(widget);
    pVLayout->addWidget(pBtnOtherColor);
    //
    QWidget* itemsWidget = new QWidget(colorMenu);
    itemsWidget->setLayout(pVLayout);

    QWidgetAction* widgetAction = new QWidgetAction(colorMenu);
    widgetAction->setDefaultWidget(itemsWidget);

    colorMenu->addAction(widgetAction);

    return colorMenu;
}

void WizEditorToolBar::on_foreColor_changed()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarForeColor");
    QAction *pColorAction = qobject_cast<QAction *>(sender());
    if (!pColorAction)
        return;

    m_btnForeColor->menu()->close();
    QColor color = qvariant_cast<QColor>(pColorAction->data());
    applyForeColor(color);
}

void WizEditorToolBar::on_showForeColorBoard()
{
    m_btnForeColor->menu()->close();
    QColorDialog dlg(m_btnForeColor->color(), this);
    connect(&dlg, SIGNAL(currentColorChanged(QColor)), SLOT(applyForeColor(QColor)));
    connect(&dlg, SIGNAL(colorSelected(QColor)), SLOT(applyForeColor(QColor)));
    dlg.exec();
}

void WizEditorToolBar::on_backColor_changed()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarBackColor");
    QAction *pColorAction = qobject_cast<QAction *>(sender());
    if (!pColorAction)
        return;

    m_btnBackColor->menu()->close();
    QColor color = qvariant_cast<QColor>(pColorAction->data());    

    applyBackColor(color);
}

void WizEditorToolBar::on_showBackColorBoard()
{
    m_btnBackColor->menu()->close();
    QColorDialog dlg(m_btnBackColor->color(), this);
    connect(&dlg, SIGNAL(currentColorChanged(QColor)), SLOT(applyBackColor(QColor)));
    connect(&dlg, SIGNAL(colorSelected(QColor)), SLOT(applyBackColor(QColor)));
    dlg.exec();
}

void WizEditorToolBar::on_fontDailogFontChanged(const QFont& font)
{
    if (m_editor)
    {
        setCurrentFont(font);
    }
}

void WizEditorToolBar::queryCurrentFont(std::function<void(const QFont& font)> callback)
{
    m_editor->editorCommandQueryCommandValue("fontFamily", [=](QString familyName){
        //
        m_editor->editorCommandQueryCommandState("bold", [=](int bold){

            m_editor->editorCommandQueryCommandState("italic", [=](int italic){

                m_editor->editorCommandQueryCommandState("underline", [=](int underline){

                    m_editor->editorCommandQueryCommandValue("fontSize", [=](QString fontSize){

                        m_editor->editorCommandQueryCommandState("strikethrough", [=](int strikethrough){
                            //
                            QFont font;

                            familyName.isEmpty() ? void() : font.setFamily(familyName);
                            font.setBold(bold == 1);
                            font.setItalic(italic == 1);
                            font.setStrikeOut(strikethrough == 1);
                            font.setUnderline(underline == 1);
                            //
                            QString size = fontSize;
                            size.remove("pt");
                            int sizeValue = size.toInt();
                            font.setPointSize(sizeValue == 0 ? m_app.userSettings().defaultFontSize() : sizeValue);
                            //
                            callback(font);
                            //
                        });
                    });
                });
            });
        });
    });
}

void WizEditorToolBar::setCurrentFont(const QFont& fontTemp)
{
    QFont font = fontTemp;
    //
    queryCurrentFont([=](const QFont& currentFont){

        if (font.family() != currentFont.family())
        {
            QString strFontFamily = font.family();
            m_editor->editorCommandExecuteFontFamily(strFontFamily);
            selectCurrentFontFamily(strFontFamily);
        }

        if (font.pointSize() != currentFont.pointSize())
        {
            setFontPointSize(QString::number(font.pointSize()));
        }

        if (font.bold() != currentFont.bold())
        {
            m_editor->editorCommandExecuteBold();
        }

        if (font.italic() !=  currentFont.italic())
        {
            m_editor->editorCommandExecuteItalic();
        }

        if (font.strikeOut() != currentFont.strikeOut())
        {
            m_editor->editorCommandExecuteStrikeThrough();
        }

        if (font.underline() != currentFont.underline())
        {
            m_editor->editorCommandExecuteUnderLine();
        }
    });

}

void WizEditorToolBar::selectCurrentFontFamily(const QString& strFontFamily)
{
    //
    for (int i = 0; i < nCommonlyUsedFontCount; i++)
    {
        if (CommonlyUsedFont[i] == strFontFamily)
        {
            selectCurrentFontFamilyItem(strFontFamily);
            return;
        }
    }

    QStringList fontList = m_app.userSettings().get(WIZRECENTFONTLIST).split('/', QString::SkipEmptyParts);
    for (QString recent : fontList)
    {
        if (recent == strFontFamily)
        {
            selectCurrentFontFamilyItem(strFontFamily);
            return;
        }
    }

    if (fontList.isEmpty())
    {
        m_comboFontFamily->insertItem(0, "separator");
        m_comboFontFamily->setItemData(0, WIZSEPARATOR, WizFontFamilyHelperRole);
    }

    if (m_comboFontFamily->itemData(2, WizFontFamilyHelperRole).toString() == WIZRECENTFONT)
    {
        m_comboFontFamily->removeItem(2);
    }

    WizComboboxStyledItem* fontFamilyItems = FontFamilies();
    WizComboboxStyledItem familyItem = itemFromArrayByKey(strFontFamily, fontFamilyItems, nFontFamilyCount);
    m_comboFontFamily->insertItem(0, familyItem.strText.isEmpty() ? strFontFamily : familyItem.strText, strFontFamily);
    m_comboFontFamily->setItemData(0, WIZRECENTFONT, WizFontFamilyHelperRole);
    selectCurrentFontFamilyItem(strFontFamily);

    fontList.insert(0, strFontFamily);
    while (fontList.count() > 3)
        fontList.pop_back();
    QString fonts = fontList.join("/");
    m_app.userSettings().set(WIZRECENTFONTLIST, fonts);
}

void WizEditorToolBar::selectCurrentFontFamilyItem(const QString& strFontFamily)
{
    WizComboboxStyledItem* fontFamilyItems = FontFamilies();
    WizComboboxStyledItem familyItem = itemFromArrayByKey(strFontFamily, fontFamilyItems, nFontFamilyCount);
//    m_comboFontFamily->setCurrentText(familyItem.strText.isEmpty() ? strFontFamily : familyItem.strText);
    m_comboFontFamily->setText(familyItem.strText.isEmpty() ? strFontFamily : familyItem.strText);
    //
    clearWizCheckState(m_comboFontFamily);
    QModelIndex modelIndex = m_comboFontFamily->model()->index(m_comboFontFamily->currentIndex(), 0);
    m_comboFontFamily->model()->setData(modelIndex, Qt::Checked, WizCheckStateRole);
}

void WizEditorToolBar::setFontPointSize(const QString& strSize)
{
    if (strSize == m_comboFontSize->text())
        return;

    if (m_editor) {
        m_editor->editorCommandExecuteFontSize(strSize);
        m_comboFontSize->setText(strSize);
        //
        clearWizCheckState(m_comboFontSize);
        QModelIndex modelIndex = m_comboFontSize->model()->index(m_comboFontSize->currentIndex(), 0);
        m_comboFontSize->model()->setData(modelIndex, Qt::Checked, WizCheckStateRole);
    }
}

void WizEditorToolBar::saveImage(QString strFileName)
{
    QFileInfo info(strFileName);
    QPixmap pix(info.filePath());
    if (pix.isNull())
    {
        TOLOG("[Save] : image is null");
        return;
    }

    savePixmap(pix, info.suffix(), false);
}

void WizEditorToolBar::copyImage(QString strFileName)
{
    QFileInfo info(strFileName);
    QPixmap pix(info.filePath());
    if (pix.isNull())
    {
        TOLOG("[Copy] : image is null");
        return;
    }

    QClipboard* clip = QApplication::clipboard();
    clip->setPixmap(pix);
}

void WizEditorToolBar::moveWidgetFromSecondLineToFirstLine(QWidget* widget)
{
    m_secondLineButtonContainer->layout()->removeWidget(widget);
    QHBoxLayout* firstLayout = qobject_cast<QHBoxLayout*>(m_firstLineButtonContainer->layout());
    int index = firstLayout->indexOf(m_btnShowExtra);
    firstLayout->insertWidget(index, widget);
    m_buttonContainersInSecondLine.removeFirst();
    m_buttonContainersInFirstLine.append(widget);

    if (m_buttonContainersInSecondLine.size() == 0)
    {
        m_secondLineButtonContainer->setVisible(false);
    }
}

void WizEditorToolBar::moveWidgetFromFristLineToSecondLine(QWidget* widget)
{
    m_firstLineButtonContainer->layout()->removeWidget(widget);
    QHBoxLayout* secondLayout = qobject_cast<QHBoxLayout*>(m_secondLineButtonContainer->layout());
    secondLayout->insertWidget(0, widget);
    m_buttonContainersInFirstLine.removeLast();
    m_buttonContainersInSecondLine.insert(0, widget);

    if (m_buttonContainersInSecondLine.size() > 0)
    {
        m_secondLineButtonContainer->setVisible(true);
    }
}

void WizEditorToolBar::applyForeColor(const QColor& color)
{
    if (!color.isValid())
        return;

    m_btnForeColor->setColor(color);

    if (m_editor) {
        m_editor->editorCommandExecuteForeColor(color);
    }
}

void WizEditorToolBar::applyBackColor(const QColor& color)
{
    if (!color.isValid())
        return;

    m_btnBackColor->setColor(color);

    if (m_editor) {
        m_editor->editorCommandExecuteBackColor(color);
    }
}

void WizEditorToolBar::adjustButtonPosition()
{
    //
    int parentWidgetWidth = m_editor->width() - 28;
    int firstLineWidth = m_btnShowExtra->width();
    for (QWidget* widget : m_buttonContainersInFirstLine)
    {
        firstLineWidth += widget->sizeHint().width();
    }
    //
    if (parentWidgetWidth < RecommendedWidthForTwoLine)
    {
        //  move moveable buttons to second line
        if (m_buttonContainersInFirstLine.size() <= 2)
            return;

        // except first button container
        for (int i = m_buttonContainersInFirstLine.count() - 1; i > 1; i--)
        {
            QWidget* widget = m_buttonContainersInFirstLine.at(i);
            moveWidgetFromFristLineToSecondLine(widget);
        }
    }
    else if ((parentWidgetWidth < RecommendedWidthForOneLine) || (parentWidgetWidth == m_editor->view()->maximumWidth()))
    {
        // move movealbe buttons to first line or to second line
        if (firstLineWidth < parentWidgetWidth &&
                m_buttonContainersInSecondLine.count() > 0)
        {
            QWidget * widget = m_buttonContainersInSecondLine.first();
            while (widget && widget->sizeHint().width() + firstLineWidth < parentWidgetWidth)
            {
                moveWidgetFromSecondLineToFirstLine(widget);
                firstLineWidth += widget->sizeHint().width();

                widget = m_buttonContainersInSecondLine.isEmpty() ? nullptr :
                                                                            m_buttonContainersInSecondLine.first();
            }
            if (m_buttonContainersInSecondLine.isEmpty())
            {
                m_btnShowExtra->hide();
            }
        }
        else if (firstLineWidth >= parentWidgetWidth &&
                 m_buttonContainersInFirstLine.count() > 1)
        {
            QWidget* widget = m_buttonContainersInFirstLine.last();
            while(widget && firstLineWidth >= parentWidgetWidth)
            {
                moveWidgetFromFristLineToSecondLine(widget);
                firstLineWidth -= widget->sizeHint().width();

                widget = m_buttonContainersInFirstLine.isEmpty() ? nullptr :
                                                                           m_buttonContainersInFirstLine.last();
            }
            if (!m_buttonContainersInSecondLine.isEmpty())
            {
                m_btnShowExtra->show();                
            }
        }
    }
    else
    {
        // move moveable buttons to first line
        for (int i = 0; i < m_buttonContainersInSecondLine.count(); i++)
        {
            QWidget* widget = m_buttonContainersInSecondLine.first();
            moveWidgetFromSecondLineToFirstLine(widget);
        }
        m_btnShowExtra->hide();
    }

    m_firstLineButtonContainer->updateGeometry();
    m_secondLineButtonContainer->updateGeometry();

    m_btnShowExtra->setChecked(false);
    if (!m_buttonContainersInSecondLine.isEmpty())
    {
        bool showExtra = m_app.userSettings().get(WIZSHOWEXTRABUTTONITEMS).toInt();
        m_secondLineButtonContainer->setVisible(showExtra);
        m_btnShowExtra->setChecked(showExtra);
    }
    m_btnShowExtra->setVisible(!m_buttonContainersInSecondLine.isEmpty());
}

#define EDITORTOOLBARTIPSCHECKED   "EditorToolBarTipsChecked"

WizTipsWidget* WizEditorToolBar::showCoachingTips()
{
    if (m_buttonContainersInSecondLine.isEmpty())
        return nullptr;

    bool showTips = false;
    if (WizMainWindow* mainWindow = WizMainWindow::instance())
    {
        showTips = mainWindow->userSettings().get(EDITORTOOLBARTIPSCHECKED).toInt() == 0;
    }

    if (showTips && !WizTipsWidget::isTipsExists(EDITORTOOLBARTIPSCHECKED))
    {
        WizTipsWidget* tipWidget = new WizTipsWidget(EDITORTOOLBARTIPSCHECKED, this);
        connect(m_btnShowExtra, SIGNAL(clicked(bool)), tipWidget, SLOT(on_targetWidgetClicked()));
        tipWidget->setAttribute(Qt::WA_DeleteOnClose, true);
        tipWidget->setText(tr("More tool items"), tr("Use to show or hide extra tool items."));
        tipWidget->setSizeHint(QSize(280, 60));
        tipWidget->setButtonVisible(false);
        tipWidget->setAutoAdjustPosition(true);

        //NOTE：绑定方法来控制webview的焦点问题
        tipWidget->bindShowFunction([](){
            if (WizMainWindow* mainWindow = WizMainWindow::instance())
            {
                if (WizDocumentView* docView = mainWindow->docView())
                {
                    docView->web()->setIgnoreActiveWindowEvent(true);
                }
            }
        });
        tipWidget->bindHideFunction([](){
            if (WizMainWindow* mainWindow = WizMainWindow::instance())
            {
                if (WizDocumentView* docView = mainWindow->docView())
                {
                    docView->web()->setIgnoreActiveWindowEvent(false);
                }
            }
        });
        tipWidget->bindCloseFunction([](){
            if (WizMainWindow* mainWindow = WizMainWindow::instance())
            {
                mainWindow->userSettings().set(EDITORTOOLBARTIPSCHECKED, "1");
                if (WizDocumentView* docView = mainWindow->docView())
                {
                    docView->web()->setIgnoreActiveWindowEvent(false);
                }
            }
        });
        //
        tipWidget->bindTargetWidget(m_btnShowExtra, 0, -4);
        QTimer::singleShot(1000, tipWidget, SLOT(on_showRequest()));
        return tipWidget;
    }
    return nullptr;
}

QAction* WizEditorToolBar::actionFromName(const QString& strName)
{
    for (QMap<QString, QAction*>::const_iterator it = m_actions.begin();
         it != m_actions.end(); it++) {
        if (it.key() == strName) {
            return it.value();
        }
    }

    Q_ASSERT(0);
    return NULL;
}

bool WizEditorToolBar::processImageSrc(bool bUseForCopy, bool& bNeedSubsequent)
{
    if (m_strImageSrc.isEmpty())
    {
        bNeedSubsequent = true;
        return true;
    }

    //
    if (m_strImageSrc.left(7) == "file://")
    {
        m_strImageSrc.remove(0, 7);
        bNeedSubsequent = true;
        return true;
    }
    else if (m_strImageSrc.left(7) == "http://")
    {
        QFileInfo info(m_strImageSrc);
        QString fileName = WizGenGUIDLowerCaseLetterOnly() + "." + info.suffix();
        WizFileDownloader* downloader = new WizFileDownloader(m_strImageSrc, fileName, "", true);
        if (bUseForCopy)
        {
            connect(downloader, SIGNAL(downloadDone(QString,bool)), SLOT(copyImage(QString)));
        }
        else
        {
            connect(downloader, SIGNAL(downloadDone(QString,bool)), SLOT(saveImage(QString)));
        }
        downloader->startDownload();
        bNeedSubsequent = false;
    }
    else if (m_strImageSrc.left(12) == "data:image/.")
    {
        if (!processBase64Image(bUseForCopy))
            return false;
        bNeedSubsequent = false;
    }

    return true;
}

bool WizEditorToolBar::processBase64Image(bool bUseForCopy)
{
    m_strImageSrc.remove(0, 12);
    QString strType = m_strImageSrc.left(m_strImageSrc.indexOf(';'));
    m_strImageSrc.remove(0, m_strImageSrc.indexOf(',') + 1);
    //
    QByteArray baData;
    if (!WizBase64Decode(m_strImageSrc, baData))
        return false;

    // QT only support to read gif
    //  GIF	Graphic Interchange Format (optional)	Read
//    if (strType.toLower() == "gif" && !bUseForCopy)
//    {
//        saveGif(baData);
//    }

    //
    QPixmap pix;
    pix.loadFromData(baData, strType.toUtf8());

    savePixmap(pix, strType, bUseForCopy);

    return true;
}

void WizEditorToolBar::savePixmap(QPixmap& pix, const QString& strType, bool bUseForCopy)
{
    if (!bUseForCopy)
    {
        QString strFilePath = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                                           QDir::homePath() + "/untitled." + strType, tr("Image Files (*.%1)").arg(strType));
        if (strFilePath.isEmpty())
            return;

        bool ret = pix.save(strFilePath, strType.toUtf8());
        TOLOG2("[Save] : save image to %1, result : %2", strFilePath,
               ret ? "OK" : "Failed");    //pix formart should use ascii or capital letter.
    }
    else
    {
        QClipboard* clip = QApplication::clipboard();
        clip->setPixmap(pix);
    }
}

void WizEditorToolBar::saveGif(const QByteArray& ba)
{

}

bool WizEditorToolBar::hasFocus()
{
    return QWidget::hasFocus() || m_comboFontFamily->isPopuping() || m_comboFontSize->isPopuping() || m_comboParagraph->isPopuping();
}


void WizEditorToolBar::on_editor_google_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuSearchByGoogle");
    QUrl url("http://google.com/search?q=" + m_editor->page()->selectedText());
    QDesktopServices::openUrl(url);
}

void WizEditorToolBar::on_editor_baidu_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuSearchByBaidu");
    QUrl url("http://www.baidu.com/s?wd=" + m_editor->page()->selectedText());
    QDesktopServices::openUrl(url);
}


void WizEditorToolBar::on_editor_cut_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuCut");
    m_editor->triggerPageAction(QWebEnginePage::Cut);
}

void WizEditorToolBar::on_editor_copy_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuCopy");
    m_editor->triggerPageAction(QWebEnginePage::Copy);
}

void WizEditorToolBar::on_editor_paste_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuPaste");
    m_editor->triggerPageAction(QWebEnginePage::Paste);
}

void WizEditorToolBar::on_editor_pastePlain_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuPaste");
    m_editor->editorCommandExecutePastePlainText();
}

void WizEditorToolBar::on_editor_bold_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuBold");
    m_editor->editorCommandExecuteBold();
}

void WizEditorToolBar::on_editor_italic_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuItalic");
    m_editor->editorCommandExecuteItalic();
}

void WizEditorToolBar::on_editor_underline_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuUnderline");
    m_editor->editorCommandExecuteUnderLine();
}

void WizEditorToolBar::on_editor_strikethrough_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuStrikeThrough");
    m_editor->editorCommandExecuteStrikeThrough();
}

void WizEditorToolBar::on_editor_insertLink_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuInsertLink");
    m_editor->editorCommandExecuteLinkInsert();
}

void WizEditorToolBar::on_editor_editLink_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuEditLink");
    m_editor->editorCommandExecuteLinkInsert();
}

void WizEditorToolBar::on_editor_removeLink_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuRemoveLink");
    m_editor->editorCommandExecuteLinkRemove();
}


void WizEditorToolBar::on_editor_justifyLeft_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuJustifyLeft");
    m_editor->editorCommandExecuteJustifyLeft();
}

void WizEditorToolBar::on_editor_justifyCenter_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuJustifyCenter");
    m_editor->editorCommandExecuteJustifyCenter();
}

void WizEditorToolBar::on_editor_justifyRight_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuJustifyRight");
    m_editor->editorCommandExecuteJustifyRight();
}


void WizEditorToolBar::on_comboParagraph_indexChanged(int index)
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarParagraph");
    QString type = m_comboParagraph->itemData(index).toString();
    Q_ASSERT (index >=0 && index < nParagraphItemCount);
    WizComboboxStyledItem* paraItems = ParagraphItems();
    WizComboboxStyledItem item = itemFromArrayByKey(type, paraItems, nParagraphItemCount);
    QString text = item.strText;
    if (text == m_comboParagraph->text())
        return;

    if (m_editor) {

        m_editor->editorCommandExecuteParagraph(type);
        m_comboParagraph->setText(text);
        clearWizCheckState(m_comboParagraph);
        QModelIndex modelIndex = m_comboParagraph->model()->index(index, 0);
        m_comboParagraph->model()->setData(modelIndex, Qt::Checked, WizCheckStateRole);
    }
}

void WizEditorToolBar::on_comboFontFamily_indexChanged(int index)
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarFontFamily");

    QString helperData = m_comboFontFamily->itemData(index, WizFontFamilyHelperRole).toString();
    if (helperData.isEmpty() || helperData == WIZRECENTFONT)
    {
        if (m_editor) {
            QString text = m_comboFontFamily->itemData(index, Qt::UserRole).toString();
            m_editor->editorCommandExecuteFontFamily(text);
            QString displayText = m_comboFontFamily->itemData(index, Qt::DisplayRole).toString();
            m_comboFontFamily->setText(displayText);
            //
            clearWizCheckState(m_comboFontFamily);
            QModelIndex modelIndex = m_comboFontFamily->model()->index(m_comboFontFamily->currentIndex(), 0);
            m_comboFontFamily->model()->setData(modelIndex, Qt::Checked, WizCheckStateRole);
        }
    }
    else if (helperData == WIZFONTPANEL)
    {
        queryCurrentFont([=](const QFont& font){
            //NOTE : 在QT5.4.2版本中存在问题，打开QFontDialog后，在编辑器中选择文本，再次回到QFontDialog时
            // currentFontChanged 将不会再次发出。 所以使用模态对话框强制用户关闭
            QFontDialog fontDialog;
            fontDialog.setCurrentFont(font);
            connect(&fontDialog, SIGNAL(currentFontChanged(QFont)), SLOT(on_fontDailogFontChanged(QFont)));
            fontDialog.exec();
        });
    }
    else if (helperData == WIZSEPARATOR)
    {
        m_editor->editorCommandQueryCommandValue("fontFamily", [=](const QString& fontFamily){
            //
            m_comboFontFamily->setText(fontFamily);

        });
    }
}

void WizEditorToolBar::on_comboFontSize_indexChanged(const QString& strSize)
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarFontSize");
    setFontPointSize(strSize);
}

void WizEditorToolBar::on_btnFormatPainter_clicked(bool checked)
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarFormatPainter");
    if (m_editor) {
        if (checked) {
            m_btnFormatPainter->setChecked(true);
            m_editor->editorCommandExecuteFormatPainterOn(false);
        } else {
            m_btnFormatPainter->setChecked(false);
            m_editor->editorCommandExecuteFormatPainterOff();
        }
    }
}
void WizEditorToolBar::on_btnFormatPainter_dblClicked()
{
    qDebug() << "formatPainter dblClicked";
    WizAnalyzer::getAnalyzer().logAction("editorToolBarFormatPainter");
    if (m_editor) {
        m_btnFormatPainter->setChecked(true);
        m_editor->editorCommandExecuteFormatPainterOn(true);
    }
}


void WizEditorToolBar::on_btnRemoveFormat_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarRemoveFormat");
    if (m_editor) {
        m_editor->editorCommandExecuteRemoveFormat();
    }
}

void WizEditorToolBar::on_btnForeColor_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarForeColorClicked");
    if (m_editor) {
        m_editor->editorCommandExecuteForeColor(m_btnForeColor->color());
    }
}

void WizEditorToolBar::on_btnBackColor_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarBackColorClicked");
    if (m_editor) {
        m_editor->editorCommandExecuteBackColor(m_btnBackColor->color());
    }
}

void WizEditorToolBar::on_btnBold_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarBold");
    if (m_editor) {
        m_editor->editorCommandExecuteBold();
    }
}

void WizEditorToolBar::on_btnItalic_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarItalic");
    if (m_editor) {
        m_editor->editorCommandExecuteItalic();
    }
}

void WizEditorToolBar::on_btnUnderLine_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarUnderLine");
    if (m_editor) {
        m_editor->editorCommandExecuteUnderLine();
    }
}

void WizEditorToolBar::on_btnStrikeThrough_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarStrikeThrough");
    if (m_editor) {
        m_editor->editorCommandExecuteStrikeThrough();
    }
}

void WizEditorToolBar::on_btnJustify_clicked()
{    
    WizAnalyzer::getAnalyzer().logAction("editorToolBarJustify");
    //
    QPoint pos = m_btnJustify->mapToGlobal(QPoint(0, m_btnJustify->height()));
    m_menuJustify->move(pos);
    QAction*action = m_menuJustify->exec();
    if (action)
    {
        m_btnJustify->setIcon(action->icon());
    }
}

void WizEditorToolBar::on_btnJustifyLeft_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarJustifyLeft");
    if (m_editor) {
        m_editor->editorCommandExecuteJustifyLeft();
    }
}

void WizEditorToolBar::on_btnJustifyCenter_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarJustifyCenter");
    if (m_editor) {
        m_editor->editorCommandExecuteJustifyCenter();
    }
}

void WizEditorToolBar::on_btnJustifyRight_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarJustifyRight");
    if (m_editor) {
        m_editor->editorCommandExecuteJustifyRight();
    }
}

void WizEditorToolBar::on_btnSearchReplace_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarSearchReplace");
    if (m_editor) {
        m_editor->editorCommandExecuteFindReplace();
    }
}

void WizEditorToolBar::on_btnUnorderedList_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarUnorderedList");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertUnorderedList();
    }
}

void WizEditorToolBar::on_btnOrderedList_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarOrderedList");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertOrderedList();
    }
}

void WizEditorToolBar::on_btnTable_clicked(int row, int col)
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarTable");
    if (m_editor) {
        m_editor->editorCommandExecuteTableInsert(row, col);
    }
}

void WizEditorToolBar::on_btnHorizontal_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarHorizontal");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertHorizontal();
    }
}

void WizEditorToolBar::on_btnCheckList_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarCheckList");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertCheckList();
    }
}

void WizEditorToolBar::on_btnInsertImage_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarImage");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertImage();
    }
}

void WizEditorToolBar::on_btnInsertLink_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarInserLink");
    if (m_editor) {
        m_editor->editorCommandExecuteLinkInsert();
    }
}

void WizEditorToolBar::on_btnInsertDate_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarInserDate");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertDate();
    }
}

void WizEditorToolBar::on_btnMobileImage_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarMobileImage");
    bool bReceiveImage = m_btnMobileImage->isChecked();
    if (m_editor)
    {
        m_editor->editorCommandExecuteMobileImage(bReceiveImage);
        //need update button status after show dialog
        m_btnMobileImage->setChecked(bReceiveImage);
        update();
    }
}

void WizEditorToolBar::on_btnScreenShot_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarScreenShot");
    if (m_editor) {
        m_editor->editorCommandExecuteScreenShot();
    }
}

void WizEditorToolBar::on_btnInsertCode_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarInsertCode");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertCode();
    }
}

void WizEditorToolBar::on_btnShowExtra_clicked()
{
    m_secondLineButtonContainer->setVisible(!m_secondLineButtonContainer->isVisible());
    m_app.userSettings().set(WIZSHOWEXTRABUTTONITEMS, QString::number(m_secondLineButtonContainer->isVisible()));
}

void WizEditorToolBar::on_editor_saveImageAs_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuSaveImageAs");
    bool bNeedSubsequent = false;
    if (processImageSrc(false, bNeedSubsequent) && bNeedSubsequent)
    {
        saveImage(m_strImageSrc);
    }
}

void WizEditorToolBar::on_editor_copyImage_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuCopyImage");
    bool bNeedSubsequent = false;
    if (processImageSrc(true, bNeedSubsequent) && bNeedSubsequent)
    {
        copyImage(m_strImageSrc);
    }
}

void WizEditorToolBar::on_editor_copyImageLink_triggered()
{
    WizAnalyzer::getAnalyzer().logAction("editorMenuCopyImageLink");
    if (m_strImageSrc.isEmpty())
        return;

    QClipboard* clip = QApplication::clipboard();
    clip->setText(m_strImageSrc);
}

