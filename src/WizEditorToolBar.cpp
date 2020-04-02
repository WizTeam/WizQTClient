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

#include <QPixmap>

#include "share/WizMisc.h"
#include "WizDef.h"
#include "share/WizSettings.h"
#include "WizDocumentWebEngine.h"
#include "WizDocumentWebView.h"
#include "WizActions.h"
#include "utils/WizLogger.h"
#include "utils/WizPathResolve.h"
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

enum ButtonPosition {
    NoPosition,
    Left,
    Center,
    Right
};

#define RecommendedWidthForTwoLine WizSmartScaleUI(685)
#define RecommendedWidthForOneLine WizSmartScaleUI(1060)

#define WIZSEPARATOR    "separator"
#define WIZFONTPANEL    "fontpanel"
#define WIZRECENTFONT   "recentfont"

#define WIZRECENTFONTLIST  "RecentlyUsedFont"

#define WIZSHOWEXTRABUTTONITEMS "ShowExtraButtonItems"

static const WizIconOptions ICON_OPTIONS(WIZ_TINT_COLOR, WizColorButtonIcon, WIZ_TINT_COLOR);

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

const int PENCIL_SIZE_COUNT = 5;
WizComboboxStyledItem* PencilSizes()
{
    static WizComboboxStyledItem sizes[] =
    {
        {"2", "14pt", "", 14, false},
        {"3", "14pt", "", 14, false},
        {"4", "14pt", "", 14, false},
        {"5", "14pt", "", 14, false},
        {"6", "14pt", "", 14, false},
    };
    return sizes;
}

const int HIGHLIGHTER_SIZE_COUNT = 5;
WizComboboxStyledItem* HighlighterSizes()
{
    static WizComboboxStyledItem sizes[] =
    {
        {"10", "14pt", "", 14, false},
        {"20", "14pt", "", 20, false},
        {"25", "14pt", "", 25, false},
        {"30", "14pt", "", 30, false},
        {"35", "14pt", "", 35, false},
    };
    return sizes;
}

const int ERASER_SIZE_COUNT = 3;
WizComboboxStyledItem* EraserSizes()
{
    static WizComboboxStyledItem sizes[] =
    {
        {"10", "14pt", "", 14, false},
        {"20", "14pt", "", 20, false},
        {"30", "14pt", "", 30, false},
    };
    return sizes;
}


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

    virtual void drawTextItem(QPainter *painter, const QStyleOptionViewItem &option,
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
                styledItem.nFontSize = WizSmartScaleUI(14);
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
                painter->drawPixmap(opt.rect.x() + WizSmartScaleUI(4), opt.rect.y() + (opt.rect.height() - nIconSize) / 2, nIconSize, nIconSize , pix);
            }
        }
        opt.rect.setX(opt.rect.x() + nIconSize + WizSmartScaleUI(4));

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
            return QSize(opt.rect.width(), WizSmartScaleUI(5));
        }

        //
        WizComboboxStyledItem styledItem = itemFromArrayByText(opt.text, m_itemArray, m_arrayCount);
        if (styledItem.strText.isEmpty()) {
            styledItem.nFontSize = WizSmartScaleUI(14);
        } else {
            styledItem.nFontSize = WizSmartScaleUI(styledItem.nFontSize);
        }
        QFont font = opt.font;
        font.setPointSize(styledItem.nFontSize);
        QFontMetrics fm(font);
        QRect rc = fm.boundingRect(opt.text);
        //
        QSize size(rc.width() + WizSmartScaleUI(4), rc.height() + WizSmartScaleUI(4));
        return size;
    }

protected:
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
    QPixmap pixmap(WizSmartScaleUI(16), WizSmartScaleUI(16));
    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(0, 0, WizSmartScaleUI(16), WizSmartScaleUI(16)), color);

    return QIcon(pixmap);
}

#define TOOLBUTTON_MARGIN_WIDTH WizSmartScaleUI(12)
#define TOOLBUTTON_ARRWO_WIDTH  WizSmartScaleUI(16)


extern QPixmap qpixmapWithTintColor(const QPixmap& image, QColor tintColor);

void drawButtonBackground(QPainter* painter, const QRect& rect, bool bDrawLeft, bool bDrawRight, bool hasFocus)
{
    // load file
    static QPixmap normalPixBackground = QPixmap(Utils::WizStyleHelper::loadPixmap("editorToolButtonBackground"));
    static QPixmap focusPixBackground = QPixmap(Utils::WizStyleHelper::loadPixmap("editorToolButtonBackground_on"));
    //
    static bool first = true;
    if (first) {
        first = false;
        if (isDarkMode()) {
            normalPixBackground = qpixmapWithTintColor(normalPixBackground, QColor("#333333"));
            focusPixBackground = qpixmapWithTintColor(normalPixBackground, QColor("#888888"));
        }
    }
    //
    int leftWidth = WizSmartScaleUI(8);
    int rightWidth = WizSmartScaleUI(8);
    int width = rect.size().width();
    int height = rect.size().height();
    double ratio = normalPixBackground.devicePixelRatio();
    //
    static QPixmap normalPixBackgroundMid = normalPixBackground.copy(leftWidth, 0, WizSmartScaleUI(2), normalPixBackground.height());
    static QPixmap focusPixBackgroundMid = focusPixBackground.copy(leftWidth, 0, WizSmartScaleUI(2), focusPixBackground.height());
    static QPixmap normalPixBackgroundLeft = normalPixBackground.copy(0, 0, int(leftWidth * ratio), normalPixBackground.height());
    static QPixmap focusPixBackgroundLeft = focusPixBackground.copy(0, 0, int(leftWidth * ratio), focusPixBackground.height());
    static QPixmap normalPixBackgroundRight = normalPixBackground.copy(normalPixBackground.width() - int(rightWidth * ratio), 0, int(rightWidth * ratio), normalPixBackground.height());
    static QPixmap focusPixBackgroundRight = focusPixBackground.copy(focusPixBackground.width() - int(rightWidth * ratio), 0, int(rightWidth * ratio), focusPixBackground.height());

    QRect rcLeft(rect.x(), rect.y(), leftWidth, height);
    QRect rcRight(rect.size().width() - rightWidth, rect.y(), rightWidth, height);

    const QPixmap& pixLeft = hasFocus ? focusPixBackgroundLeft : normalPixBackgroundLeft;
    const QPixmap& pixMid = hasFocus ? focusPixBackgroundMid : normalPixBackgroundMid;
    const QPixmap& pixRight = hasFocus ? focusPixBackgroundRight : normalPixBackgroundRight;
    //
    if (bDrawLeft && bDrawRight) {
        painter->drawPixmap(rcLeft, pixLeft);
        painter->drawPixmap(rect.x() + leftWidth, rect.y(), width - leftWidth - rightWidth, height, pixMid);
        painter->drawPixmap(rcRight, pixRight);
    } else if (bDrawLeft) {
        painter->drawPixmap(rcLeft, pixLeft);
        painter->drawPixmap(rect.x() + leftWidth, rect.y(), width - leftWidth, height, pixMid);
    } else if (bDrawRight) {
        painter->drawPixmap(rect.x(), rect.y(), width - rightWidth, height, pixMid);
        painter->drawPixmap(rcRight, pixRight);
    } else {
        painter->drawPixmap(rect, pixMid);
    }
}

void drawCombo(QComboBox* cm, QStyleOptionComboBox& opt, ButtonPosition position)
{
    QStylePainter painter(cm);

    if (isDarkMode()) {
        opt.palette.setColor(QPalette::Text, "#AAAAAA");
    } else {
        opt.palette.setColor(QPalette::Text, "#646464");
    }
    painter.setPen(cm->palette().color(QPalette::Text));

    bool bDrawLeft = (position == Left) || (position == NoPosition);
    bool bDrawRight = (position == Right) || (position == NoPosition);
    drawButtonBackground(&painter, opt.rect, bDrawLeft, bDrawRight, opt.state & QStyle::State_MouseOver);

    // draw arrow
    if (opt.subControls & QStyle::SC_ComboBoxArrow) {
        QStyleOption subOpt = opt;

        QRect rectSub = cm->style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow);
        rectSub.adjust(WizSmartScaleUI(6), 0, WizSmartScaleUI(-12), 0);


        subOpt.rect = rectSub.adjusted(0, rectSub.height()/2 - WizSmartScaleUI(3), 0, -rectSub.height()/2 + WizSmartScaleUI(3));
        QRect rcArrow = opt.rect;
        rcArrow.setX(opt.rect.right() - TOOLBUTTON_ARRWO_WIDTH - WizSmartScaleUI(8));
        rcArrow.setY((opt.rect.height() - TOOLBUTTON_ARRWO_WIDTH) / 2);
        rcArrow.setSize(QSize(TOOLBUTTON_ARRWO_WIDTH, TOOLBUTTON_ARRWO_WIDTH));
        static QPixmap arrow = QPixmap(Utils::WizStyleHelper::loadPixmap("editorToolbarComboboxArrow"));
        static bool first = true;
        if (first) {
            first = false;
            if (isDarkMode()) {
                arrow = qpixmapWithTintColor(arrow, WizColorButtonIcon);
            }
        }
        painter.drawPixmap(rcArrow, arrow);
    }

    // draw text
    QFont f = painter.font();
    f.setPixelSize(Utils::WizStyleHelper::editComboFontSize());
    painter.setFont(f);

    QRect editRect = cm->style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField);
    if (!opt.currentText.isEmpty()) {
        QPoint center = editRect.center();
        center.setY(opt.rect.center().y());
        editRect.moveCenter(center);
        painter.drawItemText(editRect,//.adjusted(1, 0, -1, 0),
                     cm->style()->visualAlignment(opt.direction, Qt::AlignLeft | Qt::AlignVCenter),
                     opt.palette, opt.state & QStyle::State_Enabled, opt.currentText, QPalette::Text);
    }
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
    CWizToolButton(QWidget* parent = 0, QString text = "")
        : WizDblclickableToolButton(parent)
        , m_colorHoverBorder("#c8dae8")
        , m_colorHoverFill("#e8f0f3")
        , m_colorSunkenBorder("#0072c4")
        , m_horizontalPadding_left(WizSmartScaleUI(10))
        , m_horizontalPadding_rgiht(WizSmartScaleUI(10))
    {
        if (!text.isEmpty()) {
            setText(text);
        }
        setFocusPolicy(Qt::NoFocus);
        setCheckable(true);
        setIconSize(QSize(Utils::WizStyleHelper::editIconHeight(), Utils::WizStyleHelper::editIconHeight()));
        setFixedHeight(Utils::WizStyleHelper::editorButtonHeight());
    }

    void setPosition(ButtonPosition position)
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

//    void setIcon(const QIcon &icon)
//    {
//        if (isDarkMode()) {
//            //
//            QList<QSize> sizes = icon.availableSizes();
//            QIcon iconNew;
//            for (auto size: sizes) {
//                QPixmap pixmap = icon.pixmap(size);
//                QPixmap pixmapNew = qpixmapWithTintColor(pixmap, "#cccccc");
//                QString strTempFileName = Utils::WizPathResolve::tempPath() + WizIntToStr(WizGetTickCount()) + ".png";
//                if (pixmap.size().width() / size.width() == 2) {
//                    strTempFileName = Utils::WizPathResolve::tempPath() + WizIntToStr(WizGetTickCount()) + "@2x.png";
//                }
//                pixmapNew.save(strTempFileName);
//                iconNew.addFile(strTempFileName);
//                WizDeleteFile(strTempFileName);
//            }
//            //
//            WizDblclickableToolButton::setIcon(iconNew);
//        } else {
//            WizDblclickableToolButton::setIcon(icon);
//        }
//    }


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
        if (opt.state & QStyle::State_On)
            mode = QIcon::Active;

        bool bDrawLeft = (m_position == Left) || (m_position == NoPosition);
        bool bDrawRight = (m_position == Right) || (m_position == NoPosition);
        drawButtonBackground(&p, opt.rect, bDrawLeft, bDrawRight, opt.state & QStyle::State_MouseOver);
        //
        QString text = opt.text;
        QIcon icon = opt.icon;
        int textWidth = opt.fontMetrics.boundingRect(text).width();
        //
        if (textWidth > 0 && !icon.isNull()) {
            //not implement
        } else if (!icon.isNull()) {
            //
            QSize iconSize = this->iconSize();
            QRect rcIcon((opt.rect.width() - iconSize.width()) / 2, (opt.rect.height() - iconSize.height()) / 2, iconSize.width(), iconSize.height());
            if (opt.arrowType == Qt::RightArrow)
                rcIcon.setX((opt.rect.width() - iconSize.width()) / 2 - TOOLBUTTON_MARGIN_WIDTH);
            opt.icon.paint(&p, rcIcon, Qt::AlignCenter, mode);
            //
        } else if (textWidth > 0) {
            //
            p.drawText(opt.rect, Qt::AlignCenter, text);

            //
        }


        if (opt.arrowType == Qt::RightArrow)
        {
            QRect rcArrow = opt.rect;
            rcArrow.setX(opt.rect.right() - TOOLBUTTON_ARRWO_WIDTH - 3);
            rcArrow.setY((opt.rect.height() - TOOLBUTTON_ARRWO_WIDTH) / 2);
            rcArrow.setSize(QSize(TOOLBUTTON_ARRWO_WIDTH, TOOLBUTTON_ARRWO_WIDTH));
            static QPixmap arrow = QPixmap(Utils::WizStyleHelper::loadPixmap("editorToolbarDownArrow"));
            static bool first = true;
            if (first) {
                first = false;
                if (isDarkMode()) {
                    arrow = qpixmapWithTintColor(arrow, WizColorButtonIcon);
                }
            }
            p.drawPixmap(rcArrow, arrow);

        }
    }

    QSize sizeHint() const
    {        
        int width = m_horizontalPadding_left + m_horizontalPadding_rgiht + iconSize().width();
        //
        if (!text().isEmpty()) {
            QFontMetrics metrics(font());
            int textWidth = metrics.boundingRect(text()).width();
            if (icon().isNull()) {
                width += textWidth;
                width -= iconSize().width();
            } else {
                width += textWidth + WizSmartScaleUI(4);
            }
        }
        //
        if (arrowType() == Qt::RightArrow)
            return QSize(width + TOOLBUTTON_MARGIN_WIDTH, Utils::WizStyleHelper::editorButtonHeight());
        return QSize(width, Utils::WizStyleHelper::editorButtonHeight());
    }


    ButtonPosition m_position;
    int m_horizontalPadding_left;
    int m_horizontalPadding_rgiht;
private:
    QColor m_colorHoverBorder;
    QColor m_colorHoverFill;
    QColor m_colorSunkenBorder;
};

#define ColorButtonRightArrowWidth  WizSmartScaleUI(18)

class CWizToolButtonColor : public CWizToolButton
{
public:
    CWizToolButtonColor(QWidget* parent = 0) : CWizToolButton(parent)
      , m_menu(NULL)
      , m_color(Qt::transparent)
    {
        setCheckable(false);
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

        bool bDrawLeft = (m_position == Left) || (m_position == NoPosition);
        bool bDrawRight = (m_position == Right) || (m_position == NoPosition);
        drawButtonBackground(&p, opt.rect, bDrawLeft, bDrawRight, opt.state & QStyle::State_MouseOver);

        //
        QIcon::Mode mode = opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
        if (opt.state & QStyle::State_On)
            mode = QIcon::Active;

        QSize size = iconSize();
        QRect rcIcon((opt.rect.width() - size.width() - ColorButtonRightArrowWidth) / 2, (opt.rect.height() - size.height()) / 2, size.width(), size.height());
        opt.icon.paint(&p, rcIcon, Qt::AlignCenter, mode);

        QRect rectColor(rcIcon.x() + 1, opt.iconSize.height() + 6, opt.iconSize.width() - 2, 2);
        p.fillRect(QRect(rectColor), m_color);

        if (opt.state & QStyle::State_MouseOver)
        {
            QPoint top(rcIcon.right() + WizSmartScaleUI(7), opt.rect.x() + (opt.rect.height() - 13) / 2);
            p.setPen(QPen(QColor("#C4C4C4")));
            p.drawLine(top, QPoint(top.x(), top.y() + 13));
        }

        //arrow
        static QPixmap arrow = QPixmap(Utils::WizStyleHelper::loadPixmap("editorToolbarDownArrow"));
#ifdef Q_OS_MAC
        int arrowHeight = arrow.height() * (WizIsHighPixel() ? 0.5f : 1);
        //
        static bool first = true;
        if (first) {
            first = false;
            if (isDarkMode()) {
                arrow = qpixmapWithTintColor(arrow, WizColorButtonIcon);
            }
        }
#else
        int arrowHeight = TOOLBUTTON_ARRWO_WIDTH;
#endif

        QRect rcArrow(rcIcon.right() + WizSmartScaleUI(7), (opt.rect.height() - arrowHeight) / 2, TOOLBUTTON_ARRWO_WIDTH, TOOLBUTTON_ARRWO_WIDTH);
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

protected:
    QColor m_color;
    QMenu* m_menu;
};

class CWizToolComboBox : public QComboBox
{
public:
    CWizToolComboBox(QWidget* parent = 0)
        : QComboBox(parent)
        , m_isPopup(false)
        , m_position(NoPosition)
    {
        setFocusPolicy(Qt::NoFocus);
        setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
        QFont f = font();
        f.setPixelSize(Utils::WizStyleHelper::editComboFontSize());
        setFont(f);
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

    //
    void setPosition(ButtonPosition pos) {
        //
        m_position = pos;
    }

    void showPopup()
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
        drawCombo(this, opt, m_position);
    }    

protected:
    QString m_strText;
    bool m_isPopup;
    ButtonPosition m_position;
};

class CWizToolLineWidthComboBox : public CWizToolComboBox
{
public:
    CWizToolLineWidthComboBox(QWidget* parent = 0)
        : CWizToolComboBox(parent)
    {
    }

    //
    void setColor(QColor color) { m_color = color; }
    QColor color() const { return m_color; }

protected:

    virtual void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);

        QStyleOptionComboBox opt;
        initStyleOption(&opt);

        opt.currentText = m_strText;
        drawCombo(this, opt, m_position);
        //
        int lineWidth = currentText().toInt();
        if (lineWidth != 0) {
            //
            QPainter painter(this);
            //
            QPoint center = opt.rect.center();
            center.setX(center.x() - WizSmartScaleUI(8));
            QRect ellipse(center.x() - lineWidth / 2, center.y() - lineWidth / 2 + 1, lineWidth, lineWidth);
            //
            painter.setBrush(QBrush(m_color));
            painter.setPen(QPen(m_color));
            painter.setRenderHint(QPainter::Antialiasing);
            painter.drawEllipse(ellipse);
        }
    }
protected:
    QColor m_color;
};


class WizToolLineWidthComboboxItemDelegate : public WizToolComboboxItemDelegate
{
public:
    WizToolLineWidthComboboxItemDelegate(QObject *parent, QComboBox* widget, const WizComboboxStyledItem* items, int count)
        : WizToolComboboxItemDelegate(parent, widget, items, count)
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
        QString text = index.model()->data(index, Qt::DisplayRole).toString();
        WizComboboxStyledItem styledItem = itemFromArrayByText(text, m_itemArray, m_arrayCount);
        int lineHeight = styledItem.strText.toInt();
        int lineWidth = opt.rect.width() - 16;
        //
        QPoint center = opt.rect.center();
        QRect rect = QRect(center.x() - lineWidth / 2, center.y() - lineHeight / 2, lineWidth, lineHeight);
        //
        QColor color("red");
        if (CWizToolLineWidthComboBox* combo = dynamic_cast<CWizToolLineWidthComboBox *>(m_widget)) {
            //
            color = combo->color();
        }
        //
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillRect(rect, QBrush(color));

        //QStyledItemDelegate::paint(painter, opt, index);
    }

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
        drawCombo(this, opt, ButtonPosition::NoPosition);
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
        if (isDarkMode()) {
            setStyleSheet("background-color:#333333;");
        } else {
            setStyleSheet("background-color:#E7E7E7;");
        }
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


#define TOOLBARTYPE_HIGHLIGHTER "highlighter"
#define TOOLBARTYPE_OUTLINE     "outline"
#define TOOLBARTYPE_NORMAL      "normal"

QColor getColorFromSettings(WizExplorerApp& app, QString name, QColor def)
{
    QString colorString = app.userSettings().get(name);
    if (colorString.isEmpty()) {
        return def;
    }
    //
    return QColor(colorString);
}

//
void setColorToSettings(WizExplorerApp& app, QString name, QColor color)
{
    QString colorString = color.name();
    app.userSettings().set(name, colorString);
}


WizEditorToolBar::WizEditorToolBar(WizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
    , m_lastUpdateUIRequest(0)
    , m_strToolbarType("normal")
{
    setContentsMargins(0, 4, 0, 0);

    QString skin = Utils::WizStyleHelper::themeName();

    m_comboParagraph = new CWizToolComboBox(this);    
    m_comboParagraph->setFixedWidth(90);
    m_comboParagraph->setFixedHeight(Utils::WizStyleHelper::editorButtonHeight());


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
    m_comboFontFamily->setFixedHeight(Utils::WizStyleHelper::editorButtonHeight());

    m_comboFontSize = new CWizToolComboBox(this);
    m_comboFontSize->setFixedWidth(WizSmartScaleUI(52));
    m_comboFontSize->setFixedHeight(Utils::WizStyleHelper::editorButtonHeight());
    //
    WizComboboxStyledItem* fontItems = FontSizes();
#ifdef Q_OS_MAC
    if (isDarkMode()) {
        m_comboParagraph->setStyleSheet("QComboBox QListView{min-width:95px;background:#323232;}"
                                        "QComboBox QAbstractItemView::item {min-height:20px;background:transparent;}");
    } else {
        m_comboParagraph->setStyleSheet("QComboBox QListView{min-width:95px;background:#F6F6F6;}"
                                        "QComboBox QAbstractItemView::item {min-height:20px;background:transparent;}");
    }

    WizToolComboboxItemDelegate* paragraphDelegate = new WizToolComboboxItemDelegate(m_comboParagraph, m_comboParagraph, paraItems, nParagraphItemCount);
    m_comboParagraph->setItemDelegate(paragraphDelegate);
    //
    if (isDarkMode()) {
        m_comboFontFamily->setStyleSheet("QComboBox QListView{min-width:150px;background:#323232;}"
                                         "QComboBox QAbstractItemView::item {min-height:30px;background:transparent;}");
    } else {
        m_comboFontFamily->setStyleSheet("QComboBox QListView{min-width:150px;background:#F6F6F6;}"
                                         "QComboBox QAbstractItemView::item {min-height:30px;background:transparent;}");
    }
    WizToolComboboxItemDelegate* fontFamilyDelegate = new WizToolComboboxItemDelegate(m_comboFontFamily, m_comboFontFamily, paraItems, nParagraphItemCount);
    m_comboFontFamily->setItemDelegate(fontFamilyDelegate);
    //
    if (isDarkMode()) {
        m_comboFontSize->setStyleSheet("QComboBox QListView{min-width:50px;background:#323232;}"
                                       "QComboBox QAbstractItemView::item {min-height:20px;background:transparent;}");
    } else {
        m_comboFontSize->setStyleSheet("QComboBox QListView{min-width:50px;background:#F6F6F6;}"
                                       "QComboBox QAbstractItemView::item {min-height:20px;background:transparent;}");
    }
    WizToolComboboxItemDelegate* fontDelegate = new WizToolComboboxItemDelegate(m_comboParagraph, m_comboParagraph, fontItems, nFontSizeCount);
    m_comboFontSize->setItemDelegate(fontDelegate);
#else
    if (isDarkMode()) {
        m_comboParagraph->setStyleSheet("color:#ffffff");
        m_comboFontFamily->setStyleSheet("color:#ffffff");
        m_comboFontSize->setStyleSheet("color:#ffffff");
    }
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

    QSize editIconSize = QSize(Utils::WizStyleHelper::editIconHeight(), Utils::WizStyleHelper::editIconHeight());

    m_btnFormatPainter = new CWizToolButton(this);
    m_btnFormatPainter->setIcon(::WizLoadSkinIcon(skin, "formatter", editIconSize, ICON_OPTIONS));
    m_btnFormatPainter->setToolTip(tr("Format Painter"));
    m_btnFormatPainter->setCheckable(true);
    m_btnFormatPainter->setChecked(false);
    m_btnFormatPainter->setPosition(ButtonPosition::Left);
    connect(m_btnFormatPainter, SIGNAL(clicked(bool)), SLOT(on_btnFormatPainter_clicked(bool)));
    connect(m_btnFormatPainter, SIGNAL(dblClicked()), SLOT(on_btnFormatPainter_dblClicked()));
    //
    m_btnRemoveFormat = new CWizToolButton(this);
    m_btnRemoveFormat->setIcon(::WizLoadSkinIcon(skin, "actionFormatRemoveFormat", editIconSize, ICON_OPTIONS));
    m_btnRemoveFormat->setToolTip(tr("Remove Format"));
    m_btnRemoveFormat->setCheckable(false);
    m_btnRemoveFormat->setPosition(ButtonPosition::Right);
    connect(m_btnRemoveFormat, SIGNAL(clicked()), SLOT(on_btnRemoveFormat_clicked()));

    m_btnForeColor = new CWizToolButtonColor(this);
    m_btnForeColor->setIcon(::WizLoadSkinIcon(skin, "actionFormatForeColor", editIconSize, ICON_OPTIONS));
    m_btnForeColor->setColor(QColor("#ff0000"));
    m_btnForeColor->setToolTip(tr("ForeColor"));
    m_btnForeColor->setCheckable(false);
    m_btnForeColor->setPosition(ButtonPosition::Left);
    connect(m_btnForeColor, SIGNAL(released()), SLOT(on_btnForeColor_clicked()));
    QMenu* foreColorMenu = createColorMenu(SLOT(on_foreColor_changed()),
                                           SLOT(on_showForeColorBoard()));
    m_btnForeColor->setMenu(foreColorMenu);

    m_btnBackColor = new CWizToolButtonColor(this);
    m_btnBackColor->setIcon(::WizLoadSkinIcon(skin, "actionFormatBackColor", editIconSize, ICON_OPTIONS));
    m_btnBackColor->setColor(QColor("#ffff00"));
    m_btnBackColor->setToolTip(tr("BackColor"));
    m_btnBackColor->setCheckable(false);
    m_btnBackColor->setPosition(ButtonPosition::Right);
    connect(m_btnBackColor, SIGNAL(released()), SLOT(on_btnBackColor_clicked()));
    QMenu* backColorMenu = createColorMenu(SLOT(on_backColor_changed()),
                                           SLOT(on_showBackColorBoard()));
    m_btnBackColor->setMenu(backColorMenu);

    m_btnBold = new CWizToolButton(this);
    m_btnBold->setIcon(::WizLoadSkinIcon(skin, "actionFormatBold", editIconSize, ICON_OPTIONS));
    m_btnBold->setToolTip(tr("Bold %1B").arg(commandKey()));
    m_btnBold->setPosition(ButtonPosition::Left);
    connect(m_btnBold, SIGNAL(clicked()), SLOT(on_btnBold_clicked()));

    m_btnItalic = new CWizToolButton(this);
    m_btnItalic->setIcon(::WizLoadSkinIcon(skin, "actionFormatItalic", editIconSize, ICON_OPTIONS));
    m_btnItalic->setToolTip(tr("Italic %1I").arg(commandKey()));
    m_btnItalic->setPosition(ButtonPosition::Center);
    connect(m_btnItalic, SIGNAL(clicked()), SLOT(on_btnItalic_clicked()));

    m_btnShowExtra = new CWizToolButton(this);
    m_btnShowExtra->setIcon(::WizLoadSkinIcon(skin, "actionFormatExtra", editIconSize, ICON_OPTIONS));
    m_btnShowExtra->setToolTip(tr("Extra"));
    m_btnShowExtra->setPosition(ButtonPosition::NoPosition);
    connect(m_btnShowExtra, SIGNAL(clicked()), SLOT(on_btnShowExtra_clicked()));

    m_btnUnderLine = new CWizToolButton(this);
    m_btnUnderLine->setIcon(::WizLoadSkinIcon(skin, "actionFormatUnderLine", editIconSize, ICON_OPTIONS));
    m_btnUnderLine->setToolTip(tr("Underline %1U").arg(commandKey()));
    m_btnUnderLine->setPosition(ButtonPosition::Center);
    connect(m_btnUnderLine, SIGNAL(clicked()), SLOT(on_btnUnderLine_clicked()));

    m_btnStrikeThrough = new CWizToolButton(this);
    m_btnStrikeThrough->setIcon(::WizLoadSkinIcon(skin, "actionFormatStrikeThrough", editIconSize, ICON_OPTIONS));
    m_btnStrikeThrough->setToolTip(tr("Strike Through %1%2K").arg(optionKey()).arg(commandKey()));
    m_btnStrikeThrough->setPosition(ButtonPosition::Right);
    connect(m_btnStrikeThrough, SIGNAL(clicked()), SLOT(on_btnStrikeThrough_clicked()));

    m_btnJustify = new CWizToolButton(this);
    m_btnJustify->setIcon(::WizLoadSkinIcon(skin, "actionFormatJustifyLeft", editIconSize, ICON_OPTIONS));
    m_btnJustify->setCheckable(false);
    m_btnJustify->setArrowType(Qt::RightArrow);
    m_btnJustify->setPopupMode(QToolButton::MenuButtonPopup);
    m_btnJustify->setToolTip(tr("Justify"));
    m_btnJustify->setPosition(ButtonPosition::NoPosition);
    m_menuJustify = new QMenu(m_btnJustify);
    m_actionJustifyLeft = m_menuJustify->addAction(::WizLoadSkinIcon(skin, "actionFormatJustifyLeft", QSize(), ICON_OPTIONS),
                             tr("Justify Left"), this, SLOT(on_btnJustifyLeft_clicked()));
    m_actionJustifyLeft->setCheckable(true);
    m_actionJustifyCenter = m_menuJustify->addAction(::WizLoadSkinIcon(skin, "actionFormatJustifyCenter", QSize(), ICON_OPTIONS),
                             tr("Justify Center"), this, SLOT(on_btnJustifyCenter_clicked()));
    m_actionJustifyCenter->setCheckable(true);
    m_actionJustifyRight = m_menuJustify->addAction(::WizLoadSkinIcon(skin, "actionFormatJustifyRight", QSize(), ICON_OPTIONS),
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
    m_btnUnorderedList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertUnorderedList", editIconSize, ICON_OPTIONS));
    m_btnUnorderedList->setToolTip(tr("UnorderedList %1%2U").arg(optionKey()).arg(commandKey()));
    m_btnUnorderedList->setPosition(ButtonPosition::Left);
    connect(m_btnUnorderedList, SIGNAL(clicked()), SLOT(on_btnUnorderedList_clicked()));

    m_btnOrderedList = new CWizToolButton(this);
    m_btnOrderedList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertOrderedList", editIconSize, ICON_OPTIONS));
    m_btnOrderedList->setToolTip(tr("OrderedList %1%2O").arg(optionKey()).arg(commandKey()));
    m_btnOrderedList->setPosition(ButtonPosition::Right);
    connect(m_btnOrderedList, SIGNAL(clicked()), SLOT(on_btnOrderedList_clicked()));

    QWidgetAction* tableAction = new QWidgetAction(this);
    tableAction->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertTable", editIconSize, ICON_OPTIONS));
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
    m_btnTable->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertTable", editIconSize, ICON_OPTIONS));
    m_btnTable->setToolTip(tr("Insert Table"));
    m_btnTable->setPosition(ButtonPosition::Center);
    m_btnTable->setMenu(menuTable);
    m_btnTable->setPopupMode(QToolButton::MenuButtonPopup);
    //

    m_btnHorizontal = new CWizToolButton(this);
    m_btnHorizontal->setCheckable(false);
    m_btnHorizontal->setHorizontalPadding(9);
    m_btnHorizontal->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertHorizontal", editIconSize, ICON_OPTIONS));
    m_btnHorizontal->setToolTip(tr("Insert Horizontal %1%2H").arg(shiftKey()).arg(commandKey()));
    m_btnHorizontal->setPosition(ButtonPosition::Center);
    connect(m_btnHorizontal, SIGNAL(clicked()), SLOT(on_btnHorizontal_clicked()));

    m_btnStartMarkup = new CWizToolButton(this);
    m_btnStartMarkup->setCheckable(false);
    m_btnStartMarkup->setHorizontalPadding(8);
    m_btnStartMarkup->setIcon(::WizLoadSkinIcon(skin, "actionFormatStartMarkup", editIconSize, ICON_OPTIONS));
    m_btnStartMarkup->setToolTip(tr("Start Markup"));
    m_btnStartMarkup->setPosition(ButtonPosition::Left);
    connect(m_btnStartMarkup, SIGNAL(clicked()), SLOT(on_btnStartMarkup_clicked()));

    m_btnInsertPainter = new CWizToolButton(this);
    m_btnInsertPainter->setCheckable(false);
    m_btnInsertPainter->setHorizontalPadding(8);
    m_btnInsertPainter->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertPainter", editIconSize, ICON_OPTIONS));
    m_btnInsertPainter->setToolTip(tr("Insert Handwriting"));
    m_btnInsertPainter->setPosition(ButtonPosition::Center);
    connect(m_btnInsertPainter, SIGNAL(clicked()), SLOT(on_btnInsertPainter_clicked()));

    m_btnCheckList = new CWizToolButton(this);
    m_btnCheckList->setCheckable(false);
    m_btnCheckList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertCheckList", editIconSize, ICON_OPTIONS));
    m_btnCheckList->setToolTip(tr("Insert Checklist %1O").arg(commandKey()));
    m_btnCheckList->setPosition(ButtonPosition::Center);
    connect(m_btnCheckList, SIGNAL(clicked()), SLOT(on_btnCheckList_clicked()));

    m_btnInsertLink = new CWizToolButton(this);
    m_btnInsertLink->setCheckable(false);
    m_btnInsertLink->setHorizontalPadding(6, 10);
    m_btnInsertLink->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertLink", editIconSize, ICON_OPTIONS));
    m_btnInsertLink->setToolTip(tr("Insert Link %1K").arg(commandKey()));
    m_btnInsertLink->setPosition(ButtonPosition::Center);
    connect(m_btnInsertLink, SIGNAL(clicked()), SLOT(on_btnInsertLink_clicked()));

    m_btnInsertImage = new CWizToolButton(this);
    m_btnInsertImage->setCheckable(false);
    m_btnInsertImage->setHorizontalPadding(8);
    m_btnInsertImage->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertImage", editIconSize, ICON_OPTIONS));
    m_btnInsertImage->setToolTip(tr("Insert Image %1%2I").arg(shiftKey()).arg(commandKey()));
    m_btnInsertImage->setPosition(ButtonPosition::Center);
    connect(m_btnInsertImage, SIGNAL(clicked()), SLOT(on_btnInsertImage_clicked()));

    m_btnInsertDate = new CWizToolButton(this);
    m_btnInsertDate->setCheckable(false);
    m_btnInsertDate->setHorizontalPadding(8, 10);
    m_btnInsertDate->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertDate", editIconSize, ICON_OPTIONS));
    m_btnInsertDate->setToolTip(tr("Insert Date %1%2D").arg(shiftKey()).arg(commandKey()));
    m_btnInsertDate->setPosition(ButtonPosition::Right);
    connect(m_btnInsertDate, SIGNAL(clicked()), SLOT(on_btnInsertDate_clicked()));

    m_btnMobileImage = new CWizToolButton(this);
    m_btnMobileImage->setIcon(::WizLoadSkinIcon(skin, "actionMobileImage", editIconSize, ICON_OPTIONS));
    m_btnMobileImage->setToolTip(tr("Receive mobile image"));
    m_btnMobileImage->setPosition(ButtonPosition::Center);
    connect(m_btnMobileImage, SIGNAL(clicked()), SLOT(on_btnMobileImage_clicked()));

    m_btnSearchReplace = new CWizToolButton(this);
    m_btnSearchReplace->setCheckable(false);
    m_btnSearchReplace->setIcon(::WizLoadSkinIcon(skin, "actionFormatSearchReplace", editIconSize, ICON_OPTIONS));
    m_btnSearchReplace->setToolTip(tr("Find & Replace %1F").arg(commandKey()));
    m_btnSearchReplace->setPosition(ButtonPosition::Right);
    connect(m_btnSearchReplace, SIGNAL(clicked()), SLOT(on_btnSearchReplace_clicked()));

#ifndef Q_OS_MAC
    m_btnScreenShot = new CWizToolButton(this);
    m_btnScreenShot->setCheckable(false);
    m_btnScreenShot->setIcon(::WizLoadSkinIcon(skin, "actionFormatScreenShot"));
    m_btnScreenShot->setToolTip(tr("Screen shot"));
    m_btnScreenShot->setPosition(ButtonPosition::Center);
    connect(m_btnScreenShot, SIGNAL(clicked()), SLOT(on_btnScreenShot_clicked()));
#else
    m_btnScreenShot = 0;
#endif

    m_btnInsertCode = new CWizToolButton(this);
    m_btnInsertCode->setCheckable(false);
    m_btnInsertCode->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertCode", editIconSize, ICON_OPTIONS));
    m_btnInsertCode->setToolTip(tr("Insert code %1%2C").arg(shiftKey()).arg(commandKey()));
    m_btnInsertCode->setPosition(ButtonPosition::Left);
    connect(m_btnInsertCode, SIGNAL(clicked()), SLOT(on_btnInsertCode_clicked()));

    //
    //
    m_btnPencil = new CWizToolButtonColor(this);
    m_btnPencil->setCheckable(true);
    m_btnPencil->setIcon(::WizLoadSkinIcon(skin, "actionMarkupPencil", editIconSize, ICON_OPTIONS));
    m_btnPencil->setPosition(ButtonPosition::Left);
    QMenu* colorPencilMenu = createColorMenu(SLOT(on_btnPencilColor_changed()));
    m_btnPencil->setMenu(colorPencilMenu);
    connect(m_btnPencil, SIGNAL(clicked()), SLOT(on_btnPencil_clicked()));
    CWizToolLineWidthComboBox* pencilSize = new CWizToolLineWidthComboBox(this);
    pencilSize->setPosition(ButtonPosition::Right);
    pencilSize->setFixedWidth(60);
    pencilSize->setFixedHeight(Utils::WizStyleHelper::editorButtonHeight());
    pencilSize->addItem("2");
    pencilSize->addItem("3");
    pencilSize->addItem("4");
    pencilSize->addItem("5");
    pencilSize->addItem("6");
    WizToolComboboxItemDelegate* pencilSizeDelegate = new WizToolLineWidthComboboxItemDelegate(pencilSize, pencilSize, PencilSizes(), PENCIL_SIZE_COUNT);
    pencilSize->setItemDelegate(pencilSizeDelegate);
    connect(pencilSize, SIGNAL(activated(int)), SLOT(on_pencilSize_activated(int)));
    m_comboPencil = pencilSize;


    m_btnHighlighter = new CWizToolButtonColor(this);
    m_btnHighlighter->setCheckable(true);
    m_btnHighlighter->setIcon(::WizLoadSkinIcon(skin, "actionMarkupHighlighter", editIconSize, ICON_OPTIONS));
    m_btnHighlighter->setPosition(ButtonPosition::Left);
    QMenu* colorHighlighterMenu = createColorMenu(SLOT(on_btnHighlighterColor_changed()));
    m_btnHighlighter->setMenu(colorHighlighterMenu);
    connect(m_btnHighlighter, SIGNAL(clicked()), SLOT(on_btnHighlighter_clicked()));
    CWizToolLineWidthComboBox* highlighterSize = new CWizToolLineWidthComboBox(this);
    highlighterSize->setPosition(ButtonPosition::Right);
    highlighterSize->setFixedWidth(60);
    highlighterSize->setFixedHeight(Utils::WizStyleHelper::editorButtonHeight());
    highlighterSize->addItem("10");
    highlighterSize->addItem("20");
    highlighterSize->addItem("25");
    highlighterSize->addItem("30");
    highlighterSize->addItem("35");
    WizToolComboboxItemDelegate* highlighterSizeDelegate = new WizToolLineWidthComboboxItemDelegate(highlighterSize, highlighterSize, HighlighterSizes(), HIGHLIGHTER_SIZE_COUNT);
    highlighterSize->setItemDelegate(highlighterSizeDelegate);
    connect(highlighterSize, SIGNAL(activated(int)), SLOT(on_highlighterSize_activated(int)));
    m_comboHighlighter = highlighterSize;
    highlighterSize->setColor(Qt::white);
    //
    m_btnEraser = new CWizToolButton(this);
    m_btnEraser->setCheckable(true);
    m_btnEraser->setIcon(::WizLoadSkinIcon(skin, "actionMarkupEraser", editIconSize, ICON_OPTIONS));
    m_btnEraser->setPosition(ButtonPosition::Left);
    connect(m_btnEraser, SIGNAL(clicked()), SLOT(on_btnEraser_clicked()));
    CWizToolLineWidthComboBox* eraserSize = new CWizToolLineWidthComboBox(this);
    eraserSize->setPosition(ButtonPosition::Right);
    eraserSize->setFixedWidth(48);
    eraserSize->setFixedHeight(Utils::WizStyleHelper::editorButtonHeight());
    eraserSize->addItem("10");
    eraserSize->addItem("20");
    eraserSize->addItem("30");
    WizToolComboboxItemDelegate* eraserSizeDelegate = new WizToolLineWidthComboboxItemDelegate(eraserSize, eraserSize, EraserSizes(), ERASER_SIZE_COUNT);
    eraserSize->setItemDelegate(eraserSizeDelegate);
    connect(eraserSize, SIGNAL(activated(int)), SLOT(on_eraserSize_activated(int)));
    m_comboEraser = eraserSize;


    m_btnShape = new CWizToolButtonColor(this);
    m_btnShape->setCheckable(true);
    m_btnShape->setIcon(::WizLoadSkinIcon(skin, "actionMarkupShape", editIconSize, ICON_OPTIONS));
    m_btnShape->setPosition(ButtonPosition::Left);
    QMenu* colorShapeMenu = createColorMenu(SLOT(on_btnShapeColor_changed()));
    m_btnShape->setMenu(colorShapeMenu);
    connect(m_btnShape, SIGNAL(clicked()), SLOT(on_btnShape_clicked()));
    CWizToolLineWidthComboBox* shapeSize = new CWizToolLineWidthComboBox(this);
    shapeSize->setPosition(ButtonPosition::Right);
    shapeSize->setFixedWidth(56);
    shapeSize->setFixedHeight(Utils::WizStyleHelper::editorButtonHeight());
    shapeSize->addItem("2");
    shapeSize->addItem("3");
    shapeSize->addItem("4");
    shapeSize->addItem("5");
    shapeSize->addItem("6");
    WizToolComboboxItemDelegate* shapeSizeDelegate = new WizToolLineWidthComboboxItemDelegate(shapeSize, shapeSize, PencilSizes(), PENCIL_SIZE_COUNT);
    shapeSize->setItemDelegate(shapeSizeDelegate);
    connect(shapeSize, SIGNAL(activated(int)), SLOT(on_shapeSize_activated(int)));
    m_comboShape = shapeSize;
    //
    QString itemStyle;
#ifdef Q_OS_MAC
    if (isDarkMode()) {
        itemStyle = "QComboBox QListView{min-width:95px;background:#323232;}"
                              "QComboBox QAbstractItemView::item {min-height:20px;background:transparent;}";
    } else {
        itemStyle = "QComboBox QListView{min-width:95px;background:#F6F6F6;}"
                              "QComboBox QAbstractItemView::item {min-height:20px;background:transparent;}";
    }
#else
    if (isDarkMode()) {
        itemStyle = "color:#ffffff";
    }
#endif
    pencilSize->setStyleSheet(itemStyle);
    highlighterSize->setStyleSheet(itemStyle);
    eraserSize->setStyleSheet(itemStyle);
    shapeSize->setStyleSheet(itemStyle);

    //
    m_btnUndo = new CWizToolButton(this);
    m_btnUndo->setCheckable(false);
    m_btnUndo->setIcon(::WizLoadSkinIcon(skin, "actionMarkupUndo", editIconSize, ICON_OPTIONS));
    m_btnUndo->setPosition(ButtonPosition::Left);
    connect(m_btnUndo, SIGNAL(clicked()), SLOT(on_btnUndo_clicked()));

    m_btnRedo = new CWizToolButton(this);
    m_btnRedo->setCheckable(false);
    m_btnRedo->setIcon(::WizLoadSkinIcon(skin, "actionMarkupRedo", editIconSize, ICON_OPTIONS));
    m_btnRedo->setPosition(ButtonPosition::Right);
    connect(m_btnRedo, SIGNAL(clicked()), SLOT(on_btnRedo_clicked()));

    m_btnGoback = new CWizToolButton(this, tr("Back to Text Editor"));
    m_btnGoback->setCheckable(false);
    m_btnGoback->setPosition(ButtonPosition::NoPosition);
    connect(m_btnGoback, SIGNAL(clicked()), SLOT(on_btnGoback_clicked()));


    m_firstLineButtonContainer = new QWidget(this);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignVCenter);
    m_firstLineButtonContainer->setLayout(layout);
    //
    //
    QWidget*  buttonContainerMarkup1 = createMoveAbleWidget(this);
    QHBoxLayout* markupLayout1 = qobject_cast<QHBoxLayout*>(buttonContainerMarkup1->layout());
    markupLayout1->addWidget(m_btnPencil);
    markupLayout1->addWidget(pencilSize);
    markupLayout1->addSpacing(16);
    //
    QWidget*  buttonContainerMarkup2 = createMoveAbleWidget(this);
    QHBoxLayout* markupLayout2 = qobject_cast<QHBoxLayout*>(buttonContainerMarkup2->layout());
    markupLayout2->addWidget(m_btnHighlighter);
    markupLayout2->addWidget(highlighterSize);
    markupLayout2->addSpacing(16);
    //
    QWidget*  buttonContainerMarkup3 = createMoveAbleWidget(this);
    QHBoxLayout* markupLayout3 = qobject_cast<QHBoxLayout*>(buttonContainerMarkup3->layout());
    markupLayout3->addWidget(m_btnEraser);
    markupLayout3->addWidget(eraserSize);
    markupLayout3->addSpacing(16);
    //
    QWidget*  buttonContainerMarkup4 = createMoveAbleWidget(this);
    QHBoxLayout* markupLayout4 = qobject_cast<QHBoxLayout*>(buttonContainerMarkup4->layout());
    markupLayout4->addWidget(m_btnShape);
    markupLayout4->addWidget(shapeSize);
    markupLayout4->addSpacing(16);
    //
    QWidget*  buttonContainerMarkup5 = createMoveAbleWidget(this);
    QHBoxLayout* markupLayout5 = qobject_cast<QHBoxLayout*>(buttonContainerMarkup5->layout());
    markupLayout5->addWidget(m_btnUndo);
    markupLayout5->addWidget(m_btnRedo);
    markupLayout5->addSpacing(16);
    markupLayout5->addWidget(m_btnGoback);
    //
    layout->addWidget(buttonContainerMarkup1);
    setAvaliableInToolbar(buttonContainerMarkup1, TOOLBARTYPE_HIGHLIGHTER);
    layout->addWidget(buttonContainerMarkup2);
    setAvaliableInToolbar(buttonContainerMarkup2, TOOLBARTYPE_HIGHLIGHTER);
    layout->addWidget(buttonContainerMarkup3);
    setAvaliableInToolbar(buttonContainerMarkup3, TOOLBARTYPE_HIGHLIGHTER);
    layout->addWidget(buttonContainerMarkup4);
    setAvaliableInToolbar(buttonContainerMarkup4, TOOLBARTYPE_HIGHLIGHTER);
    layout->addWidget(buttonContainerMarkup5);
    setAvaliableInToolbar(buttonContainerMarkup5, TOOLBARTYPE_HIGHLIGHTER);
    //
    //
    //outline tool bar
    m_btnUndoOutline = new CWizToolButton(this);
    m_btnUndoOutline->setCheckable(false);
    m_btnUndoOutline->setIcon(::WizLoadSkinIcon(skin, "actionMarkupUndo", editIconSize, ICON_OPTIONS));
    m_btnUndoOutline->setPosition(ButtonPosition::Left);
    connect(m_btnUndoOutline, SIGNAL(clicked()), SLOT(on_btnUndoOutline_clicked()));

    m_btnRedoOutline = new CWizToolButton(this);
    m_btnRedoOutline->setCheckable(false);
    m_btnRedoOutline->setIcon(::WizLoadSkinIcon(skin, "actionMarkupRedo", editIconSize, ICON_OPTIONS));
    m_btnRedoOutline->setPosition(ButtonPosition::Right);
    connect(m_btnRedoOutline, SIGNAL(clicked()), SLOT(on_btnRedoOutline_clicked()));

    m_btnOutdentOutline = new CWizToolButton(this);
    m_btnOutdentOutline->setCheckable(false);
    m_btnOutdentOutline->setIcon(::WizLoadSkinIcon(skin, "outline_outdent", editIconSize, ICON_OPTIONS));
    m_btnOutdentOutline->setPosition(ButtonPosition::Left);
    connect(m_btnOutdentOutline, SIGNAL(clicked()), SLOT(on_btnOutdentOutline_clicked()));

    m_btnIndentOutline = new CWizToolButton(this);
    m_btnIndentOutline->setCheckable(false);
    m_btnIndentOutline->setIcon(::WizLoadSkinIcon(skin, "outline_indent", editIconSize, ICON_OPTIONS));
    m_btnIndentOutline->setPosition(ButtonPosition::Right);
    connect(m_btnIndentOutline, SIGNAL(clicked()), SLOT(on_btnIndentOutline_clicked()));

    m_btnNotesOutline = new CWizToolButton(this);
    m_btnNotesOutline->setCheckable(false);
    m_btnNotesOutline->setIcon(::WizLoadSkinIcon(skin, "outline_notes", editIconSize, ICON_OPTIONS));
    m_btnNotesOutline->setPosition(ButtonPosition::NoPosition);
    connect(m_btnNotesOutline, SIGNAL(clicked()), SLOT(on_btnNotesOutline_clicked()));

    m_btnCompleteOutline = new CWizToolButton(this);
    m_btnCompleteOutline->setCheckable(false);
    m_btnCompleteOutline->setIcon(::WizLoadSkinIcon(skin, "outline_complete", editIconSize, ICON_OPTIONS));
    m_btnCompleteOutline->setPosition(ButtonPosition::NoPosition);
    connect(m_btnCompleteOutline, SIGNAL(clicked()), SLOT(on_btnCompleteOutline_clicked()));


    QWidget*  buttonContainerOutline = createMoveAbleWidget(this);
    QHBoxLayout* outlineLayout = qobject_cast<QHBoxLayout*>(buttonContainerOutline->layout());
    outlineLayout->addWidget(m_btnUndoOutline);
    outlineLayout->addWidget(m_btnRedoOutline);
    outlineLayout->addSpacing(16);
    outlineLayout->addWidget(m_btnOutdentOutline);
    outlineLayout->addWidget(m_btnIndentOutline);
    outlineLayout->addSpacing(16);
    outlineLayout->addWidget(m_btnNotesOutline);
    outlineLayout->addSpacing(16);
    outlineLayout->addWidget(m_btnCompleteOutline);

    layout->addWidget(buttonContainerOutline);
    setAvaliableInToolbar(buttonContainerOutline, TOOLBARTYPE_OUTLINE);
    //markupLayout5->addSpacing(16);
    //markupLayout5->addWidget(m_btnGoback);

    //
    //
    //normal tools
    QWidget*  buttonContainer0 = createMoveAbleWidget(this);
    QHBoxLayout* containerLayout = qobject_cast<QHBoxLayout*>(buttonContainer0->layout());
    containerLayout->addWidget(m_comboParagraph);
    containerLayout->addSpacing(16);
    containerLayout->addWidget(m_comboFontFamily);
    containerLayout->addSpacing(16);
    containerLayout->addWidget(m_comboFontSize);
    containerLayout->addSpacing(16);

    layout->addWidget(buttonContainer0);
    setAvaliableInToolbar(buttonContainer0, TOOLBARTYPE_NORMAL);


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
    setAvaliableInToolbar(buttonContainer1, TOOLBARTYPE_NORMAL);
    //
    setMinimumWidth(firstLineWidth() - 20);

    QWidget*  moveableButtonContainer1 = createMoveAbleWidget(this);
    moveableButtonContainer1->layout()->addWidget(m_btnJustify);
    qobject_cast<QHBoxLayout*>(moveableButtonContainer1->layout())->addSpacing(12);

    layout->addWidget(moveableButtonContainer1);
    setAvaliableInToolbar(moveableButtonContainer1, TOOLBARTYPE_NORMAL);


    QWidget*  moveableButtonContainer2 = createMoveAbleWidget(this);
    QHBoxLayout* moveableLayout2 = qobject_cast<QHBoxLayout*>(moveableButtonContainer2->layout());
    moveableLayout2->addWidget(m_btnUnorderedList);
    moveableLayout2->addWidget(new CWizEditorButtonSpliter(this));
    moveableLayout2->addWidget(m_btnOrderedList);
    moveableLayout2->addSpacing(12);

    layout->addWidget(moveableButtonContainer2);
    setAvaliableInToolbar(moveableButtonContainer2, TOOLBARTYPE_NORMAL);

    layout->addWidget(m_btnShowExtra);
    setAvaliableInToolbar(m_btnShowExtra, TOOLBARTYPE_NORMAL);

    QWidget* firstLineWidget = createMoveAbleWidget(this);
    firstLineWidget->layout()->addWidget(m_firstLineButtonContainer);
    qobject_cast<QHBoxLayout*>(firstLineWidget->layout())->addStretch();

    //m_buttonContainersInFirstLine.append(buttonContainer0);
   // m_buttonContainersInFirstLine.append(buttonContainer1);
    //m_buttonContainersInFirstLine.append(moveableButtonContainer1);
    //m_buttonContainersInFirstLine.append(moveableButtonContainer2);

    m_secondLineButtonContainer = new QWidget(this);
    QHBoxLayout* hLayout = new QHBoxLayout(m_secondLineButtonContainer);
    hLayout->setContentsMargins(0, 1, 0, 1);
    hLayout->setSpacing(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget*  moveableButtonContainer3 = createMoveAbleWidget(this);
    QHBoxLayout* moveableLayout3 = qobject_cast<QHBoxLayout*>(moveableButtonContainer3->layout());
    moveableLayout3->addWidget(m_btnStartMarkup);
    moveableLayout3->addWidget(m_btnInsertPainter);
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
    setAvaliableInToolbar(moveableButtonContainer3, TOOLBARTYPE_NORMAL);

    QWidget*  moveableButtonContainer4 = createMoveAbleWidget(this);
    QHBoxLayout* moveableLayout4 = qobject_cast<QHBoxLayout*>(moveableButtonContainer4->layout());
    //
    moveableLayout4->addWidget(m_btnFormatPainter);
    moveableLayout4->addWidget(new CWizEditorButtonSpliter(this));
    moveableLayout4->addWidget(m_btnRemoveFormat);
    moveableLayout4->addSpacing(12);

    hLayout->addWidget(moveableButtonContainer4);
    setAvaliableInToolbar(moveableButtonContainer4, TOOLBARTYPE_NORMAL);


    QWidget*  moveableButtonContainer5 = createMoveAbleWidget(this);
    QHBoxLayout* moveableLayout5 = qobject_cast<QHBoxLayout*>(moveableButtonContainer5->layout());
    moveableLayout5->addWidget(m_btnInsertCode);
#ifndef Q_OS_MAC
    hLayout->addWidget(m_btnScreenShot);
#endif
    moveableLayout5->addWidget(new CWizEditorButtonSpliter(this));
    moveableLayout5->addWidget(m_btnSearchReplace);

    hLayout->addWidget(moveableButtonContainer5);
    setAvaliableInToolbar(moveableButtonContainer5, TOOLBARTYPE_NORMAL);

    hLayout->addStretch();


    //m_buttonContainersInSecondLine.append(moveableButtonContainer3);
    //m_buttonContainersInSecondLine.append(moveableButtonContainer4);
    //m_buttonContainersInSecondLine.append(moveableButtonContainer5);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(WizSmartScaleUI(8));
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
    //
    setToolbarType(TOOLBARTYPE_NORMAL);
    //setToolbarType(TOOLBARTYPE_HIGHLIGHTER);
    //setToolbarType(TOOLBARTYPE_OUTLINE);
}

void WizEditorToolBar::switchToNormalMode() {
    if (m_editor->isOutline()) {
        setToolbarType(TOOLBARTYPE_OUTLINE);
    } else {
        setToolbarType(TOOLBARTYPE_NORMAL);
    }
}

void WizEditorToolBar::setToolbarType(QString type) {
    //
    m_strToolbarType = type;
    //
    QHBoxLayout* firstLayout = qobject_cast<QHBoxLayout*>(m_firstLineButtonContainer->layout());
    for (int i = 0; i < firstLayout->count(); i++)
    {
        QLayoutItem* item = firstLayout->itemAt(i);
        QWidget* widget = item->widget();
        if (widget) {
            widget->setVisible(avaliableInToolbar(widget));
        }
    }
    //
    QHBoxLayout* secondLayout = qobject_cast<QHBoxLayout*>(m_secondLineButtonContainer->layout());
    for (int i = 0; i < secondLayout->count(); i++)
    {
        QLayoutItem* item = secondLayout->itemAt(i);
        QWidget* widget = item->widget();
        if (widget){
            widget->setVisible(avaliableInToolbar(widget));
        }
    }
}

bool WizEditorToolBar::avaliableInToolbar(QWidget* widget) {
    return widget->property("toolbarType").toString() == m_strToolbarType;
}

void WizEditorToolBar::setAvaliableInToolbar(QWidget* widget, QString type) {
    widget->setProperty("toolbarType", type);
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
    bool canFormatPainter = QString::fromStdString(d["canFormatPainter"].asString()) == "1";
    bool canInsertHorizontalRule = QString::fromStdString(d["canInsertHorizontalRule"].asString()) == "1";
    bool canInsertImage = QString::fromStdString(d["canInsertImage"].asString()) == "1";
    bool canSetLink = QString::fromStdString(d["canSetLink"].asString()) == "1";

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
    //
    m_btnFormatPainter->setEnabled(canFormatPainter);
    m_btnHorizontal->setEnabled(canInsertHorizontalRule);
    m_btnInsertImage->setEnabled(canInsertImage);
    m_btnInsertLink->setEnabled(canSetLink);
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
    //
    connect(m_editor, SIGNAL(markerUndoStatusChanged(const QString&)),
            SLOT(on_delegate_markerUndoStatusChanged(const QString&)));
    //
    connect(m_editor, SIGNAL(markerInitiated(const QString&)),
            SLOT(on_delegate_markerInitiated(const QString&)));
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
        qDebug() << action->iconText();
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

void WizEditorToolBar::on_delegate_markerUndoStatusChanged(const QString& data)
{
    try {
        Json::Value d;
        Json::Reader reader;
        if (!reader.parse(data.toUtf8().constData(), d))
            return;
        //
        bool canUndo = d["canUndo"].asBool();
        bool canRedo = d["canRedo"].asBool();
        //
        m_btnUndo->setEnabled(canUndo);
        m_btnRedo->setEnabled(canRedo);
    } catch (...) {

    }
}

void WizEditorToolBar::on_delegate_markerInitiated(const QString& data)
{
    qDebug() << data;
    //
    try {

        Json::Value d;
        Json::Reader reader;
        if (!reader.parse(data.toUtf8().constData(), d))
            return;
        //
        QString type = QString::fromStdString(d["curType"].asString());
        Json::Value tools = d["tools"];
        for (int i = 0; i < tools.size(); i++) {
            //
            Json::Value tool = tools[i];
            QString toolType = QString::fromStdString(tool["type"].asString());
            int lineWidth = tool["lineWidth"].asInt();
            QColor color = QColor(QString::fromStdString(tool["color"].asString()));
            //
            if (toolType == "PENCIL") {
                m_btnPencil->setColor(color);
                m_comboPencil->setColor(color);
                m_comboPencil->setCurrentText(WizIntToStr(lineWidth));
            } else if (toolType == "HIGHLIGHTER") {
                m_btnHighlighter->setColor(color);
                m_comboHighlighter->setColor(color);
                m_comboHighlighter->setCurrentText(WizIntToStr(lineWidth));
            } else if (toolType == "ERASER") {
                m_comboEraser->setColor(QColor(-1));
                m_comboEraser->setCurrentText(WizIntToStr(lineWidth));
            } else if (toolType == "SHAPE") {
                m_btnShape->setColor(color);
                m_comboShape->setColor(color);
                m_comboShape->setCurrentText(WizIntToStr(lineWidth));
            }
        }
        //
        setPainterToolType(type);
        //
    } catch (...) {

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
            pBtnColor->setFixedSize(QSize(WizSmartScaleUI(16), WizSmartScaleUI(16)));
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

    if (slotColorBoard) {

        // 选择其他颜色
        QToolButton *pBtnOtherColor = new QToolButton(this);
        pBtnOtherColor->setText(tr("show more colors..."));
        pBtnOtherColor->setFixedSize(QSize(WizSmartScaleUI(110), WizSmartScaleUI(20)));
        pBtnOtherColor->setAutoRaise(true);
        pBtnOtherColor->setToolTip(tr("show more colors..."));
        if (isDarkMode()) {
            pBtnOtherColor->setStyleSheet("color:#AAAAAA");
        }
        connect(pBtnOtherColor, SIGNAL(clicked()), this, slotColorBoard);
        pVLayout->addWidget(pBtnOtherColor);
    }
    //
    QWidget* itemsWidget = new QWidget(colorMenu);
    itemsWidget->setLayout(pVLayout);

    QWidgetAction* widgetAction = new QWidgetAction(colorMenu);
    widgetAction->setDefaultWidget(itemsWidget);

    colorMenu->addAction(widgetAction);

    return colorMenu;
}

QMenu* WizEditorToolBar::createLineWidthMenu(const char *slot)
{
    QMenu *menu = new QMenu(this);
    //
    QWidget *widget = new QWidget(menu);
    QVBoxLayout* widgetLayout = new QVBoxLayout(widget);
    widget->setLayout(widgetLayout);

    const int lineWidthArr[] = {1, 2, 3, 5, 10};
    for (auto lineWidth : lineWidthArr) {
        //
        //lineWidget->setBackgroundRole()
        QWidget* itemWidget = new QWidget(this);
        itemWidget->setFixedSize(::WizSmartScaleUI(100), ::WizSmartScaleUI(24));
        //
        QVBoxLayout* itemLayout = new QVBoxLayout(itemWidget);
        itemWidget->setLayout(itemLayout);

        QWidget* lineWidget = new QWidget(itemWidget);
        lineWidget->setFixedSize(::WizSmartScaleUI(100), ::WizSmartScaleUI(lineWidth));
        lineWidget->setStyleSheet("background-color: black");
        //
        itemLayout->addWidget(lineWidget);
        //
        widgetLayout->addWidget(itemWidget);
        //
        connect(itemWidget, SIGNAL(clicked()), slot);
    }
    //
    QWidgetAction* widgetAction = new QWidgetAction(menu);
    widgetAction->setDefaultWidget(widget);
    //
    //
    menu->addAction(widgetAction);
    //
    return menu;
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



QList<QWidget*> WizEditorToolBar::buttonContainersInFirstLine()
{
    QList<QWidget*> ret;
    QHBoxLayout* firstLayout = qobject_cast<QHBoxLayout*>(m_firstLineButtonContainer->layout());
    for (int i = 0; i < firstLayout->count(); i++)
    {
        QLayoutItem* item = firstLayout->itemAt(i);
        QWidget* widget = item->widget();
        if (widget && widget != m_btnShowExtra)
        {
            if (avaliableInToolbar(widget)) {
                ret.push_back(widget);
            }
        }
    }
    return ret;

}

QList<QWidget*> WizEditorToolBar::buttonContainersInSecondLine()
{
    QList<QWidget*> ret;
    QHBoxLayout* secondLayout = qobject_cast<QHBoxLayout*>(m_secondLineButtonContainer->layout());
    for (int i = 0; i < secondLayout->count(); i++)
    {
        QLayoutItem* item = secondLayout->itemAt(i);
        QWidget* widget = item->widget();
        if (widget && widget != m_btnShowExtra)
        {
            if (avaliableInToolbar(widget)) {
                ret.push_back(widget);
            }
        }
    }
    return ret;
}

void WizEditorToolBar::moveWidgetFromSecondLineToFirstLine(QWidget* widget)
{
    m_secondLineButtonContainer->layout()->removeWidget(widget);
    QHBoxLayout* firstLayout = qobject_cast<QHBoxLayout*>(m_firstLineButtonContainer->layout());
    int index = firstLayout->indexOf(m_btnShowExtra);
    firstLayout->insertWidget(index, widget);
    //m_buttonContainersInSecondLine.removeFirst();
    //m_buttonContainersInFirstLine.append(widget);

    if (buttonContainersInSecondLine().size() == 0)
    {
        m_secondLineButtonContainer->setVisible(false);
    }
}

void WizEditorToolBar::moveWidgetFromFristLineToSecondLine(QWidget* widget)
{
    m_firstLineButtonContainer->layout()->removeWidget(widget);
    QHBoxLayout* secondLayout = qobject_cast<QHBoxLayout*>(m_secondLineButtonContainer->layout());
    secondLayout->insertWidget(0, widget);
    //m_buttonContainersInFirstLine.removeLast();
    //m_buttonContainersInSecondLine.insert(0, widget);

    if (buttonContainersInSecondLine().size() > 0)
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


int WizEditorToolBar::firstLineWidth()
{
    int total = 0;
    auto items = buttonContainersInFirstLine();
    for (QWidget* widget : items)
    {
        if (!widget->property("hideInToolbar").toBool()) {
            total += widget->sizeHint().width();// + WizSmartScaleUI(8);
        }
    }
    return total + m_btnShowExtra->size().width();
}

int WizEditorToolBar::secondLineWidth()
{
    int total = 0;
    auto items = buttonContainersInSecondLine();
    for (QWidget* widget : items)
    {
        if (!widget->property("hideInToolbar").toBool()) {
            total += widget->sizeHint().width();// + WizSmartScaleUI(8);
        }
    }
    return total;// - WizSmartScaleUI(8);
}

void WizEditorToolBar::adjustButtonPosition()
{
    int parentWidgetWidth = m_editor->width() - WizSmartScaleUI(28) - 1; //28 ???
    //
    int firstWidth = firstLineWidth();
    //
    bool changed = false;
    //
    if (firstWidth > parentWidgetWidth)
    {
        while (firstWidth > parentWidgetWidth)
        {
            auto items = buttonContainersInFirstLine();
            if (items.isEmpty())
                break;
            //
            QWidget* last = items.last();
            moveWidgetFromFristLineToSecondLine(last);
            changed = true;
            //
            firstWidth = firstLineWidth();
        }
    }
    else if (firstWidth < parentWidgetWidth)
    {
        while (firstWidth < parentWidgetWidth)
        {
            auto items = buttonContainersInSecondLine();
            if (items.isEmpty())
                break;
            //
            QWidget * widget = items.first();
            int width = widget->sizeHint().width();
            if (width + firstWidth > parentWidgetWidth)
                break;
            //
            moveWidgetFromSecondLineToFirstLine(widget);
            changed = true;
            //
            firstWidth = firstLineWidth();
        }
    }
    //
    if (!changed)
        return;
    //
    for (auto widget : buttonContainersInFirstLine())
    {
        widget->setVisible(true);
    }
    //
    auto items = buttonContainersInSecondLine();
    //
    int total = 0;
    //
    for (auto widget : items)
    {
        total += widget->sizeHint().width();
        if (total < parentWidgetWidth)
        {
            widget->setVisible(true);
        }
        else
        {
            widget->setVisible(false);
        }
    }
    //
    //
    m_firstLineButtonContainer->updateGeometry();
    m_secondLineButtonContainer->updateGeometry();
    //
#ifdef QT_DEBUG
    qDebug() << "firstLineWidth: " << firstLineWidth();
    qDebug() << "secondLineWidth: " << secondLineWidth();
    qDebug() << "parentWidgetWidth: " << parentWidgetWidth;
    qDebug() << "m_firstLineButtonContainer->sizeHint().width(): " << m_firstLineButtonContainer->sizeHint().width();
    qDebug() << "m_secondLineButtonContainer->sizeHint().width(): " << m_secondLineButtonContainer->sizeHint().width();
    qDebug() << "m_firstLineButtonContainer->size().width(): " << m_firstLineButtonContainer->size().width();
#endif
    //
    if (buttonContainersInSecondLine().isEmpty())
    {
        m_btnShowExtra->hide();
    }
    else
    {
        m_btnShowExtra->show();
    }

    m_btnShowExtra->setChecked(false);
    if (!buttonContainersInSecondLine().isEmpty())
    {
        bool showExtra = m_app.userSettings().get(WIZSHOWEXTRABUTTONITEMS).toInt();
        m_secondLineButtonContainer->setVisible(showExtra);
        m_btnShowExtra->setChecked(showExtra);
    }
    m_btnShowExtra->setVisible(!buttonContainersInSecondLine().isEmpty());
}

#define EDITORTOOLBARTIPSCHECKED   "EditorToolBarTipsChecked"

WizTipsWidget* WizEditorToolBar::showCoachingTips()
{
    if (buttonContainersInSecondLine().isEmpty())
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

void WizEditorToolBar::on_btnStartMarkup_clicked()
{
    setToolbarType(TOOLBARTYPE_HIGHLIGHTER);
    WizAnalyzer::getAnalyzer().logAction("editorStartMarkup");
    if (m_editor) {
        m_editor->editorCommandExecuteStartMarkup();
    }
}

void WizEditorToolBar::on_btnInsertPainter_clicked()
{
    WizAnalyzer::getAnalyzer().logAction("editorToolBarPainter");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertPainter();
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

void WizEditorToolBar::on_btnPencil_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs("WizEditor.marker.set(WizEditor.marker.ToolType.PENCIL);");
    }
    setPainterToolType("PENCIL");
}

void WizEditorToolBar::on_btnHighlighter_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs("WizEditor.marker.set(WizEditor.marker.ToolType.HIGHLIGHTER);");
    }
    setPainterToolType("HIGHLIGHTER");
}

void WizEditorToolBar::on_btnEraser_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs("WizEditor.marker.set(WizEditor.marker.ToolType.ERASER);");
    }
    setPainterToolType("ERASER");
}

void WizEditorToolBar::on_btnShape_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs("WizEditor.marker.set(WizEditor.marker.ToolType.SHAPE);");
    }
    setPainterToolType("SHAPE");
}

void WizEditorToolBar::on_btnUndo_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs("WizEditor.marker.undo();");
    }
}
void WizEditorToolBar::on_btnRedo_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs("WizEditor.marker.redo();");
    }
}

void WizEditorToolBar::on_btnGoback_clicked()
{
    setToolbarType(TOOLBARTYPE_NORMAL);
    WizAnalyzer::getAnalyzer().logAction("editorStopMarkup");
    if (m_editor) {
        m_editor->editorCommandExecuteStopMarkup();
    }
}

void WizEditorToolBar::on_btnPencilColor_changed()
{
    QAction *pColorAction = qobject_cast<QAction *>(sender());
    if (!pColorAction)
        return;

    m_btnPencil->menu()->close();
    QColor color = qvariant_cast<QColor>(pColorAction->data());
    //
    if (m_editor) {
        on_btnPencil_clicked();
        m_editor->editorExecJs(QString("WizEditor.marker.setColor('%1');").arg(color.name()));
        m_comboPencil->setColor(color);
    }
}
void WizEditorToolBar::on_btnHighlighterColor_changed()
{
    QAction *pColorAction = qobject_cast<QAction *>(sender());
    if (!pColorAction)
        return;

    m_btnHighlighter->menu()->close();
    QColor color = qvariant_cast<QColor>(pColorAction->data());
    //
    if (m_editor) {
        on_btnHighlighter_clicked();
        m_editor->editorExecJs(QString("WizEditor.marker.setColor('%1');").arg(color.name()));
        m_comboHighlighter->setColor(color);
    }
}
void WizEditorToolBar::on_btnShapeColor_changed()
{
    QAction *pColorAction = qobject_cast<QAction *>(sender());
    if (!pColorAction)
        return;

    m_btnShape->menu()->close();
    QColor color = qvariant_cast<QColor>(pColorAction->data());
    //
    if (m_editor) {
        on_btnShape_clicked();
        m_editor->editorExecJs(QString("WizEditor.marker.setColor('%1');").arg(color.name()));
        m_comboShape->setColor(color);
    }
}

void WizEditorToolBar::setPainterToolType(QString type)
{
    m_btnPencil->setChecked(type == "PENCIL");
    m_btnHighlighter->setChecked(type == "HIGHLIGHTER");
    m_btnEraser->setChecked(type == "ERASER");
    m_btnShape->setChecked(type == "SHAPE");
}



void WizEditorToolBar::on_pencilSize_activated(int index)
{
    QObject* s = sender();
    if (CWizToolComboBox* combo = dynamic_cast<CWizToolComboBox*>(s)) {
        QString text = combo->itemText(index);
        if (m_editor) {
            on_btnPencil_clicked();
            m_editor->editorExecJs(QString("WizEditor.marker.setLineWidth(%1);").arg(text));
        }
    }
}
void WizEditorToolBar::on_highlighterSize_activated(int index)
{
    QObject* s = sender();
    if (CWizToolComboBox* combo = dynamic_cast<CWizToolComboBox*>(s)) {
        QString text = combo->itemText(index);
        if (m_editor) {
            on_btnHighlighter_clicked();
            m_editor->editorExecJs(QString("WizEditor.marker.setLineWidth(%1);").arg(text));
        }
    }
}
void WizEditorToolBar::on_eraserSize_activated(int index)
{
    QObject* s = sender();
    if (CWizToolComboBox* combo = dynamic_cast<CWizToolComboBox*>(s)) {
        QString text = combo->itemText(index);
        if (m_editor) {
            on_btnEraser_clicked();
            m_editor->editorExecJs(QString("WizEditor.marker.setLineWidth(%1);").arg(text));
        }
    }
}
void WizEditorToolBar::on_shapeSize_activated(int index)
{
    QObject* s = sender();
    if (CWizToolComboBox* combo = dynamic_cast<CWizToolComboBox*>(s)) {
        QString text = combo->itemText(index);
        if (m_editor) {
            on_btnShape_clicked();
            m_editor->editorExecJs(QString("WizEditor.marker.setLineWidth(%1);").arg(text));
        }
    }
}
//
//
//outline
void WizEditorToolBar::on_btnUndoOutline_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs(QString("WizEditor.undo();"));
    }
}
void WizEditorToolBar::on_btnRedoOutline_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs(QString("WizEditor.redo();"));
    }
}
void WizEditorToolBar::on_btnIndentOutline_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs(QString("WizEditor.outline.indent();"));
    }
}
void WizEditorToolBar::on_btnOutdentOutline_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs(QString("WizEditor.outline.outdent();"));
    }
}
void WizEditorToolBar::on_btnNotesOutline_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs(QString("WizEditor.outline.addNotes();"));
    }
}
void WizEditorToolBar::on_btnCompleteOutline_clicked()
{
    if (m_editor) {
        m_editor->editorExecJs(QString("WizEditor.outline.setCompleted();"));
    }
}

