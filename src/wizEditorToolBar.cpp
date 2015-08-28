#include "wizEditorToolBar.h"

#include <QStylePainter>
#include <QToolButton>
#include <QFontComboBox>
#include <QHBoxLayout>
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

#include "share/wizmisc.h"
#include "wizdef.h"
#include "share/wizsettings.h"
#include "wizDocumentWebEngine.h"
#include "wizDocumentWebView.h"
#include "wizactions.h"
#include "utils/logger.h"
#include "share/wizObjectDataDownloader.h"
#include "share/wizAnalyzer.h"
#include "wizdef.h"
#include "utils/stylehelper.h"

const int WizCheckStateRole = (int)Qt::UserRole + 5;
const int WizFontFamilyHelperRole = WizCheckStateRole + 1;

#define WIZSEPARATOR    "separator"
#define WIZFONTPANEL    "fontpanel"
#define WIZRECENTFONT   "recentfont"

#define WIZRECENTFONTLIST  "RecentlyUsedFont"

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
        {"9px", "9px", "", 9, false},
        {"10px", "10px", "", 10, false},
        {"11px", "11px", "", 11, false},
        {"12px", "12px", "", 12, false},
        {"13px", "13px", "", 13, false},
        {"14px", "14px", "", 14, false},
        {"15px", "15px", "", 15, false},
        {"16px", "16px", "", 16, false},
        {"17px", "17px", "", 17, false},
        {"18px", "18px", "", 18, false},
        {"24px", "24px", "", 24, false},
        {"36px", "36px", "", 36, false},
        {"48px", "48px", "", 48, false},
        {"64px", "64px", "", 64, false},
        {"72px", "72px", "", 72, false}
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
            painter->fillRect(option.rect, QBrush(QColor("#448aff")));
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

        const int nIconSize = 16;
        if (index.model()->data(index, WizCheckStateRole).toInt() == Qt::Checked)
        {
            static QIcon icon = Utils::StyleHelper::loadIcon("listViewItemSelected");
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
        QStyleOptionViewItemV4 opt = option;
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

using namespace Core::Internal;

void drawComboPrimitive(QStylePainter* p, QStyle::PrimitiveElement pe, const QStyleOption &opt);

void drawCombo(QComboBox* cm, QStyleOptionComboBox& opt)
{
    QStylePainter painter(cm);

    opt.palette.setColor(QPalette::Text, "#646464");
    painter.setPen(cm->palette().color(QPalette::Text));

    // draw arrow
    if (opt.subControls & QStyle::SC_ComboBoxArrow) {
        QStyleOption subOpt = opt;

        QRect rectSub = cm->style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow);
        //painter.drawRect(rectSub);
        rectSub.adjust(6, 0, -12, 0);

//        subOpt.rect = rectSub.adjusted(0, 1, 0, -rectSub.height()/2);
//        drawComboPrimitive(&painter, QStyle::PE_IndicatorArrowUp, subOpt);

        subOpt.rect = rectSub.adjusted(0, rectSub.height()/2 - 3, 0, -rectSub.height()/2 + 3);
//        drawComboPrimitive(&painter, QStyle::PE_IndicatorArrowDown, subOpt);
        QRect rcArrow = opt.rect;
        rcArrow.setX(opt.rect.right() - TOOLBUTTON_ARRWO_WIDTH - 8);
        rcArrow.setY((opt.rect.height() - TOOLBUTTON_ARRWO_WIDTH) / 2);
        rcArrow.setSize(QSize(TOOLBUTTON_ARRWO_WIDTH, TOOLBUTTON_ARRWO_WIDTH));
        static QPixmap arrow = QPixmap(Utils::StyleHelper::skinResourceFileName("editorToolbarComboboxArrow", true));
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

class CWizToolButton : public QToolButton
{
public:
    CWizToolButton(QWidget* parent = 0)
        : QToolButton(parent)
        , m_colorHoverBorder("#c8dae8")
        , m_colorHoverFill("#e8f0f3")
        , m_colorSunkenBorder("#0072c4")
    {
        setFocusPolicy(Qt::NoFocus);
        setCheckable(true);
        setIconSize(QSize(16, 16));
    }

protected:
    void drawCommonPrimitive(QPainter* p, QStyleOptionToolButton* opt)
    {
        QRect rectInner = opt->rect.adjusted(1, 1, -1, -1);
        if ((opt->state & QStyle::State_MouseOver) && !(opt->state & QStyle::State_Sunken)) {
            p->setPen(m_colorHoverBorder);
            p->fillRect(rectInner, m_colorHoverFill);
            p->drawRect(rectInner);
        }

        if (opt->state & QStyle::State_Sunken) {
            p->setPen(m_colorSunkenBorder);
            p->drawRect(rectInner);
        }
    }

    virtual void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);

        QStyleOptionToolButton opt;
        initStyleOption(&opt);

        QPainter p(this);
        p.setClipRect(opt.rect);

        drawCommonPrimitive(&p, &opt);

        QIcon::Mode mode = opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
        if (mode == QIcon::Normal && (opt.state & QStyle::State_Sunken))
            mode = QIcon::Active;
        QIcon::State state = QIcon::Off;
        if (opt.state & QStyle::State_On)
            state = QIcon::On;

        QSize size = iconSize();
        QRect rcIcon((opt.rect.width() - size.width()) / 2, (opt.rect.height() - size.height()) / 2, size.width(), size.height());
        if (opt.arrowType == Qt::RightArrow)
            rcIcon.setX((opt.rect.width() - size.width()) / 2 - TOOLBUTTON_MARGIN_WIDTH);
        opt.icon.paint(&p, rcIcon, Qt::AlignCenter, mode, state);

        if (opt.arrowType == Qt::RightArrow)
        {
            QRect rcArrow = opt.rect;
            rcArrow.setX(opt.rect.right() - TOOLBUTTON_ARRWO_WIDTH);
            rcArrow.setY((opt.rect.height() - TOOLBUTTON_ARRWO_WIDTH) / 2);
            rcArrow.setSize(QSize(TOOLBUTTON_ARRWO_WIDTH, TOOLBUTTON_ARRWO_WIDTH));
            static QPixmap arrow = QPixmap(Utils::StyleHelper::skinResourceFileName("editorToolbarDownArrow", true));
            p.drawPixmap(rcArrow, arrow);

//            QMatrix matrix;
//            matrix.translate(opt.rect.right() - TOOLBUTTON_ARRWO_WIDTH / 2 -1, opt.rect.center().y() + 2);
//            QPainterPath path;
//            path.moveTo(0, 2.3);
//            path.lineTo(-2.3, -2.3);
//            path.lineTo(2.3, -2.3);
//            p.setMatrix(matrix);
//            p.setPen(Qt::NoPen);
//            p.setBrush(QColor(0, 0, 0, 255));
//            p.setRenderHint(QPainter::Antialiasing);
//            p.drawPath(path);
        }
    }

    virtual void leaveEvent(QEvent* event) {
        QToolButton::leaveEvent(event);

        update();
    }

    virtual void enterEvent(QEvent* event) {
        QToolButton::enterEvent(event);

        update();
    }

    virtual QSize sizeHint() const
    {
        if (arrowType() == Qt::RightArrow)
            return QSize(20 + TOOLBUTTON_MARGIN_WIDTH, 20);
        return QSize(20, 20);
    }

private:
    QColor m_colorHoverBorder;
    QColor m_colorHoverFill;
    QColor m_colorSunkenBorder;
};

class CWizToolButtonColor : public CWizToolButton
{
public:
    CWizToolButtonColor(QWidget* parent = 0) : CWizToolButton(parent)
    {
        setCheckable(false);
        setIconSize(QSize(16, 16));
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

protected:
    virtual void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);

        QStyleOptionToolButton opt;
        initStyleOption(&opt);

        QStylePainter p(this);
        p.setClipRect(opt.rect);

        drawCommonPrimitive(&p, &opt);

        QIcon::Mode mode = opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
        if (mode == QIcon::Normal && (opt.state & QStyle::State_HasFocus || opt.state & QStyle::State_Sunken))
            mode = QIcon::Active;
        QIcon::State state = QIcon::Off;
        if (opt.state & QStyle::State_On)
            state = QIcon::On;

        QSize size = iconSize();
        QRect rcIcon((opt.rect.width() - size.width()) / 2, (opt.rect.height() - size.height()) / 2, size.width(), size.height());
        opt.icon.paint(&p, rcIcon, Qt::AlignCenter, mode, state);

        QRect rectColor(opt.rect.x() + 4, opt.iconSize.height() + 1, opt.iconSize.width() - 4, 4);
        p.fillRect(QRect(rectColor), m_color);
    }

private:
    QColor m_color;
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
        return QSize(65, fontMetrics().height());
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
        return QSize(100, fontMetrics().height());
    }

private:
    QString m_strText;
    bool m_isPopup;
};


EditorToolBar::EditorToolBar(CWizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
    , m_resetLocked(false)
{
    QString skin = "default";   

    m_comboParagraph = new CWizToolComboBox(this);
    if (m_app.userSettings().locale() == ::WizGetDefaultTranslatedLocal())
    {
        m_comboParagraph->setMinimumWidth(90);
    }
    else
    {
        m_comboParagraph->setMinimumWidth(70);
    }

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

    m_comboFontSize = new CWizToolComboBox(this);
    WizComboboxStyledItem* fontItems = FontSizes();
#ifdef Q_OS_MAC
    m_comboParagraph->setStyleSheet("QComboBox QListView{min-width:95px;background:#ffffff;}"
                                    "QComboBox QAbstractItemView::item {min-height:20px;background:transparent;}");
    WizToolComboboxItemDelegate* paragraphDelegate = new WizToolComboboxItemDelegate(m_comboParagraph, m_comboParagraph, paraItems, nParagraphItemCount);
    m_comboParagraph->setItemDelegate(paragraphDelegate);
    //
    m_comboFontFamily->setStyleSheet("QComboBox QListView{min-width:95px;background:#ffffff;}"
                                     "QComboBox QAbstractItemView::item {min-height:30px;background:transparent;}");
    WizToolComboboxItemDelegate* fontFamilyDelegate = new WizToolComboboxItemDelegate(m_comboFontFamily, m_comboFontFamily, paraItems, nParagraphItemCount);
    m_comboFontFamily->setItemDelegate(fontFamilyDelegate);
    //
    m_comboFontSize->setStyleSheet("QComboBox QListView{min-width:210px;background:#ffffff;}"
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

    m_btnFormatMatch = new CWizToolButton(this);
    m_btnFormatMatch->setIcon(::WizLoadSkinIcon(skin, "actionFormatMatch"));
    m_btnFormatMatch->setToolTip(tr("FormatMatch"));
    connect(m_btnFormatMatch, SIGNAL(clicked()), SLOT(on_btnFormatMatch_clicked()));

    m_btnForeColor = new CWizToolButtonColor(this);
    m_btnForeColor->setIcon(::WizLoadSkinIcon(skin, "actionFormatForeColor"));
    m_btnForeColor->setToolTip(tr("ForeColor"));
    m_btnForeColor->setCheckable(false);
    QMenu* foreColorMenu = createColorMenu(SLOT(on_foreColor_changed()),
                                           SLOT(on_showForeColorBoard()));
    m_btnForeColor->setMenu(foreColorMenu);

    m_btnBackColor = new CWizToolButtonColor(this);
    m_btnBackColor->setIcon(::WizLoadSkinIcon(skin, "actionFormatBackColor"));
    m_btnBackColor->setToolTip(tr("BackColor"));
    m_btnBackColor->setCheckable(false);
    QMenu* backColorMenu = createColorMenu(SLOT(on_backColor_changed()),
                                           SLOT(on_showBackColorBoard()));
    m_btnBackColor->setMenu(backColorMenu);

    m_btnBold = new CWizToolButton(this);
    m_btnBold->setIcon(::WizLoadSkinIcon(skin, "actionFormatBold"));
    m_btnBold->setToolTip(tr("Bold"));
    connect(m_btnBold, SIGNAL(clicked()), SLOT(on_btnBold_clicked()));

    m_btnItalic = new CWizToolButton(this);
    m_btnItalic->setIcon(::WizLoadSkinIcon(skin, "actionFormatItalic"));
    m_btnItalic->setToolTip(tr("Italic"));
    connect(m_btnItalic, SIGNAL(clicked()), SLOT(on_btnItalic_clicked()));

    m_btnUnderLine = new CWizToolButton(this);
    m_btnUnderLine->setIcon(::WizLoadSkinIcon(skin, "actionFormatUnderLine"));
    m_btnUnderLine->setToolTip(tr("UnderLine"));
    connect(m_btnUnderLine, SIGNAL(clicked()), SLOT(on_btnUnderLine_clicked()));

    m_btnStrikeThrough = new CWizToolButton(this);
    m_btnStrikeThrough->setIcon(::WizLoadSkinIcon(skin, "actionFormatStrikeThrough"));
    m_btnStrikeThrough->setToolTip(tr("StrikeThrough"));
    connect(m_btnStrikeThrough, SIGNAL(clicked()), SLOT(on_btnStrikeThrough_clicked()));

    m_btnJustify = new CWizToolButton(this);
    m_btnJustify->setIcon(::WizLoadSkinIcon(skin, "actionFormatJustifyLeft"));
    m_btnJustify->setCheckable(false);
    m_btnJustify->setArrowType(Qt::RightArrow);
    m_btnJustify->setPopupMode(QToolButton::MenuButtonPopup);
    m_btnJustify->setToolTip(tr("Justify"));
    connect(m_btnJustify, SIGNAL(clicked()), SLOT(on_btnJustify_clicked()));
    m_menuJustify = new QMenu(m_btnJustify);
    m_actionJustifyLeft = m_menuJustify->addAction(::WizLoadSkinIcon(skin, "actionFormatJustifyLeft"),
                             tr("JustifyLeft"), this, SLOT(on_btnJustifyLeft_clicked()));
    m_actionJustifyLeft->setCheckable(true);
    m_actionJustifyCenter = m_menuJustify->addAction(::WizLoadSkinIcon(skin, "actionFormatJustifyCenter"),
                             tr("JustifyCenter"), this, SLOT(on_btnJustifyCenter_clicked()));
    m_actionJustifyCenter->setCheckable(true);
    m_actionJustifyRight = m_menuJustify->addAction(::WizLoadSkinIcon(skin, "actionFormatJustifyRight"),
                             tr("JustifyRight"), this, SLOT(on_btnJustifyRight_clicked()));
    m_actionJustifyRight->setCheckable(true);
    m_btnJustify->setMenu(m_menuJustify);


    m_btnUnorderedList = new CWizToolButton(this);
    m_btnUnorderedList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertUnorderedList"));
    m_btnUnorderedList->setToolTip(tr("UnorderedList"));
    connect(m_btnUnorderedList, SIGNAL(clicked()), SLOT(on_btnUnorderedList_clicked()));

    m_btnOrderedList = new CWizToolButton(this);
    m_btnOrderedList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertOrderedList"));
    m_btnOrderedList->setToolTip(tr("OrderedList"));
    connect(m_btnOrderedList, SIGNAL(clicked()), SLOT(on_btnOrderedList_clicked()));

    m_btnTable = new CWizToolButton(this);
    m_btnTable->setCheckable(false);
    m_btnTable->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertTable"));
    m_btnTable->setToolTip(tr("InsertTable"));
    connect(m_btnTable, SIGNAL(clicked()), SLOT(on_btnTable_clicked()));

    m_btnHorizontal = new CWizToolButton(this);
    m_btnHorizontal->setCheckable(false);
    m_btnHorizontal->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertHorizontal"));
    m_btnHorizontal->setToolTip(tr("InsertHorizontal"));
    connect(m_btnHorizontal, SIGNAL(clicked()), SLOT(on_btnHorizontal_clicked()));

    m_btnCheckList = new CWizToolButton(this);
    m_btnCheckList->setCheckable(false);
    m_btnCheckList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertCheckList"));
    m_btnCheckList->setToolTip(tr("InsertCheckList"));
    connect(m_btnCheckList, SIGNAL(clicked()), SLOT(on_btnCheckList_clicked()));

    m_btnInsertImage = new CWizToolButton(this);
    m_btnInsertImage->setCheckable(false);
    m_btnInsertImage->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertImage"));
    m_btnInsertImage->setToolTip(tr("InsertImage"));
    connect(m_btnInsertImage, SIGNAL(clicked()), SLOT(on_btnImage_clicked()));

    m_btnMobileImage = new CWizToolButton(this);
    m_btnMobileImage->setIcon(::WizLoadSkinIcon(skin, "actionMobileImage"));
    m_btnMobileImage->setToolTip(tr("Receive mobile image"));
    connect(m_btnMobileImage, SIGNAL(clicked()), SLOT(on_btnMobileImage_clicked()));

    m_btnSearchReplace = new CWizToolButton(this);
    m_btnSearchReplace->setCheckable(false);
    m_btnSearchReplace->setIcon(::WizLoadSkinIcon(skin, "actionFormatSearchReplace"));
    m_btnSearchReplace->setToolTip(tr("Find & Replace"));
    connect(m_btnSearchReplace, SIGNAL(clicked()), SLOT(on_btnSearchReplace_clicked()));

#ifndef Q_OS_MAC
    m_btnScreenShot = new CWizToolButton(this);
    m_btnScreenShot->setCheckable(false);
    m_btnScreenShot->setIcon(::WizLoadSkinIcon(skin, "actionFormatScreenShot"));
    m_btnScreenShot->setToolTip(tr("Screen shot"));
    connect(m_btnScreenShot, SIGNAL(clicked()), SLOT(on_btnScreenShot_clicked()));
#else
    m_btnScreenShot = 0;
#endif

    m_btnViewSource = new CWizToolButton(this);
    m_btnViewSource->setCheckable(true);
    m_btnViewSource->setIcon(::WizLoadSkinIcon(skin, "actionFormatViewSource"));
    m_btnViewSource->setToolTip(tr("View source"));
    connect(m_btnViewSource, SIGNAL(clicked()), SLOT(on_btnViewSource_clicked()));

    m_btnInsertCode = new CWizToolButton(this);
    m_btnInsertCode->setCheckable(false);
    m_btnInsertCode->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertCode"));
    m_btnInsertCode->setToolTip(tr("Insert code"));
    connect(m_btnInsertCode, SIGNAL(clicked()), SLOT(on_btnInsertCode_clicked()));

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(3, 0, 3, 0);
    layout->setAlignment(Qt::AlignVCenter);
    layout->setSpacing(2);
    setLayout(layout);

    layout->addSpacing(6);
    layout->addWidget(m_btnViewSource);
    layout->addWidget(m_btnMobileImage);
    layout->addWidget(m_btnCheckList);
    layout->addWidget(m_btnInsertCode);
    layout->addWidget(m_comboParagraph);
    layout->addSpacing(4);
    layout->addWidget(m_comboFontFamily);
    layout->addSpacing(4);
    layout->addWidget(m_comboFontSize);
    layout->addSpacing(8);
    layout->addWidget(m_btnFormatMatch);
    layout->addWidget(m_btnForeColor);
    layout->addWidget(m_btnBackColor);
    layout->addWidget(m_btnBold);
    layout->addWidget(m_btnItalic);
    layout->addWidget(m_btnUnderLine);
    layout->addWidget(m_btnStrikeThrough);
    layout->addSpacing(12);
    layout->addWidget(m_btnJustify);
    layout->addSpacing(12);
    layout->addWidget(m_btnUnorderedList);
    layout->addWidget(m_btnOrderedList);
    layout->addSpacing(12);
    layout->addWidget(m_btnTable);
    layout->addWidget(m_btnHorizontal);
    layout->addWidget(m_btnInsertImage);
#ifndef Q_OS_MAC
    layout->addWidget(m_btnScreenShot);
#endif
    layout->addSpacing(12);
    layout->addWidget(m_btnSearchReplace);
    layout->addStretch();

    connect(&m_resetLockTimer, SIGNAL(timeout()), SLOT(on_resetLockTimer_timeOut()));
}

QSize EditorToolBar::sizeHint() const
{
    return QSize(1, 32);
}

void EditorToolBar::resetToolbar()
{
    Q_ASSERT(m_editor);

#ifdef USEWEBENGINE
    m_editor->editorCommandQueryCommandValue("fontFamily", [this](const QVariant& returnValue) {
        m_comboFontFamily->setText(returnValue.toString());
    });

    m_editor->editorCommandQueryCommandValue("fontSize", [this](const QVariant& returnValue) {
        m_comboFontSize->setText(returnValue.toString());
    });

    m_editor->editorCommandQueryCommandState("formatMatch", [this](const QVariant& returnValue) {
        int state = returnValue.toInt();
        if (state == -1) {
            m_btnFormatMatch->setEnabled(false);
        } else if (state == 0) {
            m_btnFormatMatch->setEnabled(true);
            m_btnFormatMatch->setChecked(false);
        } else if (state == 1) {
            m_btnFormatMatch->setEnabled(true);
            m_btnFormatMatch->setChecked(true);
        } else {
            Q_ASSERT(0);
        }
    });

    m_editor->editorCommandQueryCommandValue("foreColor", [this](const QVariant& returnValue) {
        m_btnForeColor->setColor(QColor(returnValue.toString()));
    });

    m_editor->editorCommandQueryCommandValue("backColor", [this](const QVariant& returnValue) {
        m_btnBackColor->setColor(QColor(returnValue.toString()));
    });

    m_editor->editorCommandQueryCommandState("bold", [this](const QVariant& returnValue) {
        int state = returnValue.toInt();
        if (state == -1) {
            m_btnBold->setEnabled(false);
        } else if (state == 0) {
            m_btnBold->setEnabled(true);
            m_btnBold->setChecked(false);
        } else if (state == 1) {
            m_btnBold->setEnabled(true);
            m_btnBold->setChecked(true);
        } else {
            Q_ASSERT(0);
        }
    });

    m_editor->editorCommandQueryCommandState("italic", [this](const QVariant& returnValue) {
        int state = returnValue.toInt();
        if (state == -1) {
            m_btnItalic->setEnabled(false);
        } else if (state == 0) {
            m_btnItalic->setEnabled(true);
            m_btnItalic->setChecked(false);
        } else if (state == 1) {
            m_btnItalic->setEnabled(true);
            m_btnItalic->setChecked(true);
        } else {
            Q_ASSERT(0);
        }
    });

    m_editor->editorCommandQueryCommandState("underline", [this](const QVariant& returnValue) {
        int state = returnValue.toInt();
        if (state == -1) {
            m_btnUnderLine->setEnabled(false);
        } else if (state == 0) {
            m_btnUnderLine->setEnabled(true);
            m_btnUnderLine->setChecked(false);
        } else if (state == 1) {
            m_btnUnderLine->setEnabled(true);
            m_btnUnderLine->setChecked(true);
        } else {
            Q_ASSERT(0);
        }
    });

    m_editor->editorCommandQueryCommandState("strikethrough", [this](const QVariant& returnValue) {
        int state = returnValue.toInt();
        if (state == -1) {
            m_btnStrikeThrough->setEnabled(false);
        } else if (state == 0) {
            m_btnStrikeThrough->setEnabled(true);
            m_btnStrikeThrough->setChecked(false);
        } else if (state == 1) {
            m_btnStrikeThrough->setEnabled(true);
            m_btnStrikeThrough->setChecked(true);
        } else {
            Q_ASSERT(0);
        }
    });

    m_editor->editorCommandQueryCommandState("justify", [this](const QVariant& returnValue) {
        int state = returnValue.toInt();
        if (state == -1) {
            m_btnJustifyLeft->setEnabled(false);
            m_btnJustifyCenter->setEnabled(false);
            m_btnJustifyRight->setEnabled(false);
        } else {
            m_btnJustifyLeft->setEnabled(true);
            m_btnJustifyCenter->setEnabled(true);
            m_btnJustifyRight->setEnabled(true);
        }
    });

    m_editor->editorCommandQueryCommandValue("justify", [this](const QVariant& returnValue) {
        QString value = returnValue.toString();
        if (value == "left") {
            m_btnJustifyLeft->setChecked(true);
            m_btnJustifyCenter->setChecked(false);
            m_btnJustifyRight->setChecked(false);
        } else if (value == "center") {
            m_btnJustifyLeft->setChecked(false);
            m_btnJustifyCenter->setChecked(true);
            m_btnJustifyRight->setChecked(false);
        } else if (value == "right") {
            m_btnJustifyLeft->setChecked(false);
            m_btnJustifyCenter->setChecked(false);
            m_btnJustifyRight->setChecked(true);
        }
    });

    m_editor->editorCommandQueryCommandState("insertOrderedList", [this](const QVariant& returnValue) {
        int state = returnValue.toInt();
        if (state == -1) {
            m_btnOrderedList->setEnabled(false);
        } else if (state == 0) {
            m_btnOrderedList->setEnabled(true);
            m_btnOrderedList->setChecked(false);
        } else if (state == 1) {
            m_btnOrderedList->setEnabled(true);
            m_btnOrderedList->setChecked(true);
        } else {
            Q_ASSERT(0);
        }
    });

    m_editor->editorCommandQueryCommandState("insertUnorderedList", [this](const QVariant& returnValue) {
        int state = returnValue.toInt();
        if (state == -1) {
            m_btnUnorderedList->setEnabled(false);
        } else if (state == 0) {
            m_btnUnorderedList->setEnabled(true);
            m_btnUnorderedList->setChecked(false);
        } else if (state == 1) {
            m_btnUnorderedList->setEnabled(true);
            m_btnUnorderedList->setChecked(true);
        } else {
            Q_ASSERT(0);
        }
    });

    m_editor->editorCommandQueryCommandState("insertTable", [this](const QVariant& returnValue) {
        int state = returnValue.toInt();
        if (state == -1) {
            m_btnTable->setEnabled(false);
        } else if (state == 0) {
            m_btnTable->setEnabled(true);
            m_btnTable->setChecked(false);
        } else if (state == 1) {
            m_btnTable->setEnabled(true);
            m_btnTable->setChecked(true);
        } else {
            Q_ASSERT(0);
        }
    });

    m_editor->editorCommandQueryCommandState("horizontal", [this](const QVariant& returnValue) {
        int state = returnValue.toInt();
        if (state == -1) {
            m_btnHorizontal->setEnabled(false);
        } else if (state == 0) {
            m_btnHorizontal->setEnabled(true);
            m_btnHorizontal->setChecked(false);
        } else if (state == 1) {
            m_btnHorizontal->setEnabled(true);
            m_btnHorizontal->setChecked(true);
        } else {
            Q_ASSERT(0);
        }
    });

    m_editor->editorCommandQueryCommandState("source", [this](const QVariant& returnValue) {
        int state = returnValue.toInt();
        if (state == -1) {
            m_btnViewSource->setEnabled(false);
        } else if (state == 0) {
            m_btnViewSource->setEnabled(true);
            m_btnViewSource->setChecked(false);
        } else if (state == 1) {
            m_btnViewSource->setEnabled(true);
            m_btnViewSource->setChecked(true);
        } else {
            Q_ASSERT(0);
        }
    });

    bool bReceiveImage = m_editor->editorCommandQueryMobileFileReceiverState();
    m_btnMobileImage->setChecked(bReceiveImage);
    m_btnMobileImage->setEnabled(true);

#else
    int state;
    QString value;

    state = m_editor->editorCommandQueryCommandState("source");
    if (state == -1) {
        m_btnViewSource->setEnabled(false);
    } else if (state == 0) {
        m_btnViewSource->setEnabled(true);
        m_btnViewSource->setChecked(false);
    } else if (state == 1) {
        m_btnViewSource->setEnabled(true);
        m_btnViewSource->setChecked(true);
    } else {
        Q_ASSERT(0);
    }
    bool isSourceMode = (1 == state);

    m_btnCheckList->setEnabled(!isSourceMode);
    m_btnSearchReplace->setEnabled(!isSourceMode);
    m_btnInsertImage->setEnabled(!isSourceMode);
    m_btnInsertCode->setEnabled(!isSourceMode);

    value = m_editor->editorCommandQueryCommandValue("Paragraph");
    WizComboboxStyledItem* paraItems = ParagraphItems();
    WizComboboxStyledItem styledItem = itemFromArrayByKey(value, paraItems, nParagraphItemCount);
    m_comboParagraph->setText(styledItem.strText);
    m_comboParagraph->setEnabled(!isSourceMode);

    CString fontName = m_editor->editorCommandQueryCommandValue("fontFamily");
    QStringList fontList = fontName.split(',', QString::SkipEmptyParts);
    fontName = fontList.isEmpty() ? "" : fontList.first();
    fontName.Trim('\'');
    m_comboFontFamily->setEnabled(!isSourceMode);
    if (!fontName.isEmpty())
    {
        selectCurrentFontFamily(fontName);
    }

    value = m_editor->editorCommandQueryCommandValue("fontSize");
    m_comboFontSize->setText(value);
    m_comboFontSize->setEnabled(!isSourceMode);

    //
    if (!isSourceMode)
    {
        clearWizCheckState(m_comboParagraph);
        QModelIndex modelIndex = m_comboParagraph->model()->index(m_comboParagraph->currentIndex(), 0);
        m_comboParagraph->model()->setData(modelIndex, Qt::Checked, WizCheckStateRole);        
        //
        clearWizCheckState(m_comboFontFamily);
        modelIndex = m_comboFontFamily->model()->index(m_comboFontFamily->currentIndex(), 0);
        m_comboFontFamily->model()->setData(modelIndex, Qt::Checked, WizCheckStateRole);
        //
        clearWizCheckState(m_comboFontSize);
        modelIndex = m_comboFontSize->model()->index(m_comboFontSize->currentIndex(), 0);
        m_comboFontSize->model()->setData(modelIndex, Qt::Checked, WizCheckStateRole);
    }

    state = m_editor->editorCommandQueryCommandState("formatMatch");
    if (state == -1) {
        m_btnFormatMatch->setEnabled(false);
    } else if (state == 0) {
        m_btnFormatMatch->setEnabled(true);
        m_btnFormatMatch->setChecked(false);
    } else if (state == 1) {
        m_btnFormatMatch->setEnabled(true);
        m_btnFormatMatch->setChecked(true);
    } else {
        Q_ASSERT(0);
    }

    value = m_editor->editorCommandQueryCommandValue("foreColor");
    m_btnForeColor->setColor(value.isEmpty() ? QColor(Qt::transparent) : QColor(value));
    m_btnForeColor->setEnabled(!isSourceMode);

    value = m_editor->editorCommandQueryCommandValue("backColor");
    m_btnBackColor->setColor(value.isEmpty() ? QColor(Qt::transparent) : QColor(value));
    m_btnBackColor->setEnabled(!isSourceMode);

    state = m_editor->editorCommandQueryCommandState("bold");
    if (state == -1) {
        m_btnBold->setEnabled(false);
    } else if (state == 0) {
        m_btnBold->setEnabled(true);
        m_btnBold->setChecked(false);
    } else if (state == 1) {
        m_btnBold->setEnabled(true);
        m_btnBold->setChecked(true);
    } else {
        Q_ASSERT(0);
    }

    state = m_editor->editorCommandQueryCommandState("italic");
    if (state == -1) {
        m_btnItalic->setEnabled(false);
    } else if (state == 0) {
        m_btnItalic->setEnabled(true);
        m_btnItalic->setChecked(false);
    } else if (state == 1) {
        m_btnItalic->setEnabled(true);
        m_btnItalic->setChecked(true);
    } else {
        Q_ASSERT(0);
    }

    state = m_editor->editorCommandQueryCommandState("underline");
    if (state == -1) {
        m_btnUnderLine->setEnabled(false);
    } else if (state == 0) {
        m_btnUnderLine->setEnabled(true);
        m_btnUnderLine->setChecked(false);
    } else if (state == 1) {
        m_btnUnderLine->setEnabled(true);
        m_btnUnderLine->setChecked(true);
    } else {
        Q_ASSERT(0);
    }

    state = m_editor->editorCommandQueryCommandState("strikethrough");
    if (state == -1) {
        m_btnStrikeThrough->setEnabled(false);
    } else if (state == 0) {
        m_btnStrikeThrough->setEnabled(true);
        m_btnStrikeThrough->setChecked(false);
    } else if (state == 1) {
        m_btnStrikeThrough->setEnabled(true);
        m_btnStrikeThrough->setChecked(true);
    } else {
        Q_ASSERT(0);
    }

    state = m_editor->editorCommandQueryCommandState("justify");
    value = m_editor->editorCommandQueryCommandValue("justify");
    if (state == -1) {
        m_btnJustify->setEnabled(false);
    } else {
        m_btnJustify->setEnabled(true);

        if (value == "left") {
            m_actionJustifyLeft->setChecked(true);
            m_actionJustifyCenter->setChecked(false);
            m_actionJustifyRight->setChecked(false);
            m_btnJustify->setIcon(m_actionJustifyLeft->icon());
        } else if (value == "center") {
            m_actionJustifyLeft->setChecked(false);
            m_actionJustifyCenter->setChecked(true);
            m_actionJustifyRight->setChecked(false);
            m_btnJustify->setIcon(m_actionJustifyCenter->icon());
        } else if (value == "right") {
            m_actionJustifyLeft->setChecked(false);
            m_actionJustifyCenter->setChecked(false);
            m_actionJustifyRight->setChecked(true);
            m_btnJustify->setIcon(m_actionJustifyRight->icon());
        }
    }

    state = m_editor->editorCommandQueryCommandState("insertOrderedList");
    if (state == -1) {
        m_btnOrderedList->setEnabled(false);
    } else if (state == 0) {
        m_btnOrderedList->setEnabled(true);
        m_btnOrderedList->setChecked(false);
    } else if (state == 1) {
        m_btnOrderedList->setEnabled(true);
        m_btnOrderedList->setChecked(true);
    } else {
        Q_ASSERT(0);
    }

    state = m_editor->editorCommandQueryCommandState("insertUnorderedList");
    if (state == -1) {
        m_btnUnorderedList->setEnabled(false);
    } else if (state == 0) {
        m_btnUnorderedList->setEnabled(true);
        m_btnUnorderedList->setChecked(false);
    } else if (state == 1) {
        m_btnUnorderedList->setEnabled(true);
        m_btnUnorderedList->setChecked(true);
    } else {
        Q_ASSERT(0);
    }

    state = m_editor->editorCommandQueryCommandState("insertTable");
    if (state == -1) {
        m_btnTable->setEnabled(false);
    } else if (state == 0) {
        m_btnTable->setEnabled(true);
        m_btnTable->setChecked(false);
    } else if (state == 1) {
        m_btnTable->setEnabled(true);
        m_btnTable->setChecked(true);
    } else {
        Q_ASSERT(0);
    }

    state = m_editor->editorCommandQueryCommandState("horizontal");
    if (state == -1) {
        m_btnHorizontal->setEnabled(false);
    } else if (state == 0) {
        m_btnHorizontal->setEnabled(true);
        m_btnHorizontal->setChecked(false);
    } else if (state == 1) {
        m_btnHorizontal->setEnabled(true);
        m_btnHorizontal->setChecked(true);
    } else {
        Q_ASSERT(0);
    }

    bool bReceiveImage = m_editor->editorCommandQueryMobileFileReceiverState();
    m_btnMobileImage->setChecked(bReceiveImage);
    m_btnMobileImage->setEnabled(!isSourceMode);
#endif
}

struct WizEditorContextMenuItem
{
    QString label;
    QString command;
    QString execute;
    bool localSlot;
};

#define WIZEDITOR_ACTION_GOOGLE         QObject::tr("Use \"Google\" search")
#define WIZEDITOR_ACTION_BAIDU           QObject::tr("Use \"Baidu\" search")

#define WIZEDITOR_ACTION_CUT            QObject::tr("Cut")
#define WIZEDITOR_ACTION_COPY           QObject::tr("Copy")
#define WIZEDITOR_ACTION_PASTE          QObject::tr("Paste")

#define WIZEDITOR_ACTION_SAVEIMGAS          QObject::tr("Save Image as...")
#define WIZEDITOR_ACTION_COPYIMG          QObject::tr("Copy Image")
#define WIZEDITOR_ACTION_COPYIMGLINK          QObject::tr("Copy Image Link")

#define WIZEDITOR_ACTION_LINK_INSERT    QObject::tr("Insert Link")
#define WIZEDITOR_ACTION_LINK_EDIT      QObject::tr("Edit Link")
#define WIZEDITOR_ACTION_LINK_REMOVE    QObject::tr("Remove Link")

#define WIZEDITOR_ACTION_FONT_BOLD          QObject::tr("Bold")
#define WIZEDITOR_ACTION_FONT_ITALIC        QObject::tr("Italic")
#define WIZEDITOR_ACTION_FONT_UNDERLINE     QObject::tr("Underline")
#define WIZEDITOR_ACTION_FONT_STRIKETHROUGH QObject::tr("Strike through")
#define WIZEDITOR_ACTION_FONT_FORECOLOR     QObject::tr("font color")
#define WIZEDITOR_ACTION_FONT_BACKCOLOR     QObject::tr("background color")

#define WIZEDITOR_ACTION_JUSTIFY_LEFT       QObject::tr("Justify left")
#define WIZEDITOR_ACTION_JUSTIFY_CENTER     QObject::tr("Justify center")
#define WIZEDITOR_ACTION_JUSTIFY_RIGHT      QObject::tr("Justify right")

#define WIZEDITOR_ACTION_TABLE_INSERT       QObject::tr("Insert table")
#define WIZEDITOR_ACTION_TABLE_DELETE       QObject::tr("Delete table")

#define WIZEDITOR_ACTION_TABLE_DELETE_ROW   QObject::tr("Delete row")
#define WIZEDITOR_ACTION_TABLE_DELETE_COLUM QObject::tr("Delete colum")

#define WIZEDITOR_ACTION_TABLE_INSERT_ROW           QObject::tr("Insert row")
#define WIZEDITOR_ACTION_TABLE_INSERT_ROW_NEXT      QObject::tr("Insert row next")
#define WIZEDITOR_ACTION_TABLE_INSERT_COLUM         QObject::tr("Insert colum")
#define WIZEDITOR_ACTION_TABLE_INSERT_COLUM_NEXT    QObject::tr("Insert colum next")

#define WIZEDITOR_ACTION_TABLE_INSERT_CAPTION   QObject::tr("Insert caption")
#define WIZEDITOR_ACTION_TABLE_DELETE_CAPTION   QObject::tr("Delete caption")
#define WIZEDITOR_ACTION_TABLE_INSERT_TITLE     QObject::tr("Insert title")
#define WIZEDITOR_ACTION_TABLE_DELETE_TITLE     QObject::tr("Delete title")

#define WIZEDITOR_ACTION_TABLE_MERGE_CELLS  QObject::tr("Merge cells")
#define WIZEDITOR_ACTION_TABLE_MERGE_RIGHT  QObject::tr("Merge right")
#define WIZEDITOR_ACTION_TABLE_MERGE_DOWN   QObject::tr("Merge down")

#define WIZEDITOR_ACTION_TABLE_SPLIT_CELLS  QObject::tr("Split cells")
#define WIZEDITOR_ACTION_TABLE_SPLIT_ROWS   QObject::tr("Split rows")
#define WIZEDITOR_ACTION_TABLE_SPLIT_COLUMS QObject::tr("Split colums")

#define WIZEDITOR_ACTION_TABLE_AVERAGE_ROWS     QObject::tr("Averaged distribute rows")
#define WIZEDITOR_ACTION_TABLE_AVERAGE_COLUMS   QObject::tr("Averaged distribute colums")


WizEditorContextMenuItem* EditorToolBar::contextMenuData()
{
    static WizEditorContextMenuItem arrayData[] =
    {
        {WIZEDITOR_ACTION_GOOGLE,                   "",                 "on_editor_google_triggered", true},
        {WIZEDITOR_ACTION_BAIDU,                   "",                 "on_editor_baidu_triggered", true},
        {"-", "-", "-"},

        {WIZEDITOR_ACTION_CUT,                      "",                 "on_editor_cut_triggered", true},
        {WIZEDITOR_ACTION_COPY,                     "",                 "on_editor_copy_triggered", true},
        {WIZEDITOR_ACTION_PASTE,                    "",                 "on_editor_paste_triggered", true},
        {"-", "-", "-"},

        {WIZEDITOR_ACTION_SAVEIMGAS,                      "",                 "on_editor_saveImageAs_triggered", true},
        {WIZEDITOR_ACTION_COPYIMG,                     "",                 "on_editor_copyImage_triggered", true},
        {WIZEDITOR_ACTION_COPYIMGLINK,                    "",                 "on_editor_copyImageLink_triggered", true},
        {"-", "-", "-"},

        {QObject::tr("Link"),                       "+",                "+"},
        {WIZEDITOR_ACTION_LINK_INSERT,              "link",             "on_editor_insertLink_triggered", true},
        {WIZEDITOR_ACTION_LINK_EDIT,                "link",             "on_editor_editLink_triggered", true},
        {WIZEDITOR_ACTION_LINK_REMOVE,              "unlink",           "on_editor_removeLink_triggered", true},
        {"+", "+", "+"},

        {QObject::tr("Font"),                       "+",                "+"},
        {WIZEDITOR_ACTION_FONT_BOLD,                "bold",             "on_editor_bold_triggered", true},
        {WIZEDITOR_ACTION_FONT_ITALIC,              "italic",           "on_editor_italic_triggered", true},
        {WIZEDITOR_ACTION_FONT_UNDERLINE,           "underline",        "on_editor_underline_triggered", true},
        {WIZEDITOR_ACTION_FONT_STRIKETHROUGH,       "strikethrough",    "on_editor_strikethrough_triggered", true},
//        {"-", "-", "-"},
//        {WIZEDITOR_ACTION_FONT_FORECOLOR,           "foreColor",        "editorCommandExecuteForeColor"},
//        {WIZEDITOR_ACTION_FONT_BACKCOLOR,           "backColor",        "editorCommandExecuteBackColor"},
        {"+", "+", "+"},

        {QObject::tr("Justify"),                    "+",          "+"},
        {WIZEDITOR_ACTION_JUSTIFY_LEFT,             "justify",          "on_editor_justifyLeft_triggered", true},
        {WIZEDITOR_ACTION_JUSTIFY_CENTER,           "justify",          "on_editor_justifyCenter_triggered", true},
        {WIZEDITOR_ACTION_JUSTIFY_RIGHT,            "justify",          "on_editor_justifyRight_triggered", true},
        {"+", "+", "+"},

        {QObject::tr("Table"),                      "+",                "+"},
        {WIZEDITOR_ACTION_TABLE_INSERT,             "inserttable",      "on_editor_insertTable_triggered", true},
        {WIZEDITOR_ACTION_TABLE_DELETE,             "deletetable",     "on_editor_deleteTable_triggered", true},
        {"-", "-", "-"},
        {QObject::tr("Cell Alignment"),                      "+",                "+"},
        {QObject::tr("Align leftTop"),         "cellalignment",        "editorCommandExecuteTableCellAlignLeftTop", false},
        {QObject::tr("Align top"),         "cellalignment",        "editorCommandExecuteTableCellAlignTop", false},
        {QObject::tr("Align rightTop"),         "cellalignment",        "editorCommandExecuteTableCellAlignRightTop", false},
        {QObject::tr("Align left"),         "cellalignment",        "editorCommandExecuteTableCellAlignLeft", false},
        {QObject::tr("Align center"),         "cellalignment",        "editorCommandExecuteTableCellAlignCenter", false},
        {QObject::tr("Align right"),         "cellalignment",        "editorCommandExecuteTableCellAlignRight", false},
        {QObject::tr("Align leftBottom"),         "cellalignment",        "editorCommandExecuteTableCellAlignLeftBottom", false},
        {QObject::tr("Align bottom"),         "cellalignment",        "editorCommandExecuteTableCellAlignBottom", false},
        {QObject::tr("Align rightBottom"),         "cellalignment",        "editorCommandExecuteTableCellAlignRightBottom", false},
        {"+", "+", "+"},
        {"-", "-", "-"},
        {WIZEDITOR_ACTION_TABLE_DELETE_ROW,         "deleterow",        "editorCommandExecuteTableDeleteRow", false},
        {WIZEDITOR_ACTION_TABLE_DELETE_COLUM,       "deletecol",        "editorCommandExecuteTableDeleteCol", false},
        {WIZEDITOR_ACTION_TABLE_INSERT_ROW,         "insertrow",        "editorCommandExecuteTableInsertRow", false},
        {WIZEDITOR_ACTION_TABLE_INSERT_ROW_NEXT,    "insertrownext",    "editorCommandExecuteTableInsertRowNext", false},
        {WIZEDITOR_ACTION_TABLE_INSERT_COLUM,       "insertcol",        "editorCommandExecuteTableInsertCol", false},
        {WIZEDITOR_ACTION_TABLE_INSERT_COLUM_NEXT,  "insertcolnext",    "editorCommandExecuteTableInsertColNext", false},
        {"-", "-", "-"},
        {WIZEDITOR_ACTION_TABLE_INSERT_CAPTION,     "insertcaption",    "editorCommandExecuteTableInsertCaption", false},
        {WIZEDITOR_ACTION_TABLE_DELETE_CAPTION,     "deletecaption",    "editorCommandExecuteTableDeleteCaption", false},
        {WIZEDITOR_ACTION_TABLE_INSERT_TITLE,       "inserttitle",      "editorCommandExecuteTableInsertTitle", false},
        {WIZEDITOR_ACTION_TABLE_DELETE_TITLE,       "deletetitle",      "editorCommandExecuteTableDeleteTitle", false},
        {"-", "-", "-"},
        {WIZEDITOR_ACTION_TABLE_MERGE_CELLS,        "mergecells",       "editorCommandExecuteTableMergeCells", false},
        {WIZEDITOR_ACTION_TABLE_MERGE_RIGHT,        "mergeright",       "editorCommandExecuteTalbeMergeRight", false},
        {WIZEDITOR_ACTION_TABLE_MERGE_DOWN,         "mergedown",        "editorCommandExecuteTableMergeDown", false},
        {"-", "-", "-"},
        {WIZEDITOR_ACTION_TABLE_SPLIT_CELLS,        "splittocells",     "editorCommandExecuteTableSplitCells", false},
        {WIZEDITOR_ACTION_TABLE_SPLIT_ROWS,         "splittorows",      "editorCommandExecuteTableSplitRows", false},
        {WIZEDITOR_ACTION_TABLE_SPLIT_COLUMS,       "splittocols",      "editorCommandExecuteTableSplitCols", false},
        {"-", "-", "-"},
        {WIZEDITOR_ACTION_TABLE_AVERAGE_ROWS,       "averagedistributerow", "editorCommandExecuteTableAverageRows", false},
        {WIZEDITOR_ACTION_TABLE_AVERAGE_COLUMS,     "averagedistributecol", "editorCommandExecuteTableAverageCols", false},
        {"+", "+", "+"},

        {"", "", ""}
    };

    return arrayData;
}

#ifdef USEWEBENGINE
void EditorToolBar::setDelegate(CWizDocumentWebEngine* editor)
{
    Q_ASSERT(editor);

    m_editor = editor;

    connect(m_editor, SIGNAL(showContextMenuRequest(QPoint)),
            SLOT(on_delegate_showContextMenuRequest(QPoint)), Qt::QueuedConnection);

    connect(m_editor, SIGNAL(selectionChanged()),
            SLOT(on_delegate_selectionChanged()));
}
#else
void EditorToolBar::setDelegate(CWizDocumentWebView* editor)
{
    Q_ASSERT(editor);

    m_editor = editor;

    connect(m_editor, SIGNAL(showContextMenuRequest(QPoint)),
            SLOT(on_delegate_showContextMenuRequest(QPoint)));
    connect(m_editor, SIGNAL(selectionChanged()),
            SLOT(on_delegate_selectionChanged()));
    connect(m_editor, SIGNAL(updateEditorToolBarRequest()),
            SLOT(on_updateToolBarStatus_request()));
}
#endif

void EditorToolBar::on_delegate_showContextMenuRequest(const QPoint& pos)
{
    if (!m_editor)
        return;

    buildMenu();

    m_strImageSrc.clear();
    if (m_editor->findIMGElementAt(pos, m_strImageSrc))
    {
        actionFromName(WIZEDITOR_ACTION_SAVEIMGAS)->setVisible(true);
        actionFromName(WIZEDITOR_ACTION_COPYIMG)->setVisible(true);
        actionFromName(WIZEDITOR_ACTION_COPYIMGLINK)->setVisible(true);
    }
    else
    {
        actionFromName(WIZEDITOR_ACTION_SAVEIMGAS)->setVisible(false);
        actionFromName(WIZEDITOR_ACTION_COPYIMG)->setVisible(false);
        actionFromName(WIZEDITOR_ACTION_COPYIMGLINK)->setVisible(false);
    }

    if (m_editor->selectedText().isEmpty()) {
        actionFromName(WIZEDITOR_ACTION_GOOGLE)->setEnabled(false);
    } else {
        actionFromName(WIZEDITOR_ACTION_GOOGLE)->setEnabled(true);
    }

    if (m_editor->selectedText().isEmpty()) {
        actionFromName(WIZEDITOR_ACTION_BAIDU)->setEnabled(false);
    } else {
        actionFromName(WIZEDITOR_ACTION_BAIDU)->setEnabled(true);
    }

    if (m_editor->isEditing()){
        actionFromName(WIZEDITOR_ACTION_CUT)->setEnabled(true);
        actionFromName(WIZEDITOR_ACTION_PASTE)->setEnabled(true);
    } else {
        actionFromName(WIZEDITOR_ACTION_CUT)->setEnabled(false);
        actionFromName(WIZEDITOR_ACTION_PASTE)->setEnabled(false);
    }    

    if (m_editor->page()->settings()->globalSettings()->testAttribute(QWebSettings::DeveloperExtrasEnabled)) {
        m_menuContext->addAction(m_editor->pageAction(QWebPage::InspectElement));
    }

    m_menuContext->popup(pos);
    m_menuContext->update();

    WizGetAnalyzer().LogAction("editorContextMenu");
}

/**     此处对slectionChanged引起的刷新做延迟和屏蔽处理。在输入中文的时候频繁的刷新会引起输入卡顿的问题
 * @brief EditorToolBar::on_delegate_selectionChanged
 */
void EditorToolBar::on_delegate_selectionChanged()
{
    static int counter = 0;
    if (m_resetLocked)
    {
        if (counter == 0)
        {
            counter ++;
            QTimer::singleShot(1600, this,SLOT(on_delegate_selectionChanged()));
        }
        return;
    }

    resetToolbar();

    // 锁定更新
    m_resetLocked = true;
    m_resetLockTimer.start(1500);
    counter = 0;
}

void EditorToolBar::on_updateToolBarStatus_request()
{
    resetToolbar();
}

void EditorToolBar::on_resetLockTimer_timeOut()
{
    m_resetLockTimer.stop();
    m_resetLocked = false;
}


QMenu* EditorToolBar::createColorMenu(const char *slot, const char *slotColorBoard)
{
    // 设置透明色
    QAction *pActionTransparent = new QAction(this);
    pActionTransparent->setData(QColor(0, 0, 0, 0));
    pActionTransparent->setText(tr("transparent"));
    connect(pActionTransparent, SIGNAL(triggered()), this, slot);
    QToolButton *pBtnTransparent = new QToolButton(this);
    pBtnTransparent->setFixedSize(QSize(110, 20));
    pBtnTransparent->setText(tr("transparent"));
    pBtnTransparent->setDefaultAction(pActionTransparent);

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

    QWidget *widget = new QWidget;
    widget->setLayout(pGridLayout);

    QVBoxLayout *pVLayout = new QVBoxLayout;
    pVLayout->setContentsMargins(5, 5, 5, 5);
    pVLayout->addWidget(pBtnTransparent);
    pVLayout->addWidget(widget);
    pVLayout->addWidget(pBtnOtherColor);

    QMenu *colorMenu = new QMenu(this);
    colorMenu->setLayout(pVLayout);

    return colorMenu;
}

void EditorToolBar::on_foreColor_changed()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarForeColor");
    QAction *pColorAction = qobject_cast<QAction *>(sender());
    if (!pColorAction)
        return;

    m_btnForeColor->menu()->close();
    QColor color = qvariant_cast<QColor>(pColorAction->data());
    if (!color.isValid())
        return;

    if (m_editor) {
        m_editor->editorCommandExecuteForeColor(color);
    }
}

void EditorToolBar::on_showForeColorBoard()
{
    m_btnForeColor->menu()->close();
    QColorDialog dlg(m_btnForeColor->color(), this);
    connect(&dlg, SIGNAL(currentColorChanged(QColor)), m_editor,
            SLOT(editorCommandExecuteForeColor(QColor)));
    connect(&dlg, SIGNAL(colorSelected(QColor)), m_editor,
            SLOT(editorCommandExecuteForeColor(QColor)));
    dlg.exec();
}

void EditorToolBar::on_backColor_changed()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarBackColor");
    QAction *pColorAction = qobject_cast<QAction *>(sender());
    if (!pColorAction)
        return;

    m_btnBackColor->menu()->close();
    QColor color = qvariant_cast<QColor>(pColorAction->data());
    if (!color.isValid())
        return;

    if (m_editor) {
        m_editor->editorCommandExecuteBackColor(color);
    }
}

void EditorToolBar::on_showBackColorBoard()
{
    m_btnBackColor->menu()->close();
    QColorDialog dlg(m_btnBackColor->color(), this);
    connect(&dlg, SIGNAL(currentColorChanged(QColor)), m_editor,
            SLOT(editorCommandExecuteBackColor(QColor)));
    connect(&dlg, SIGNAL(colorSelected(QColor)), m_editor,
            SLOT(editorCommandExecuteBackColor(QColor)));
    dlg.exec();
}

void EditorToolBar::on_fontDailogFontChanged(const QFont& font)
{
    if (m_editor)
    {
        setCurrentFont(font);
    }
}

void EditorToolBar::queryCurrentFont(QFont& font)
{
    QString familyName = m_editor->editorCommandQueryCommandValue("fontFamily");
    familyName.isEmpty() ? void() : font.setFamily(familyName);

    int value = m_editor->editorCommandQueryCommandState("bold");
//    qDebug() << "query current font bold : " << value;
    font.setBold(value == 1);

    value = m_editor->editorCommandQueryCommandState("italic");
//    qDebug() << "query current font italic : " << value;
    font.setItalic(value == 1);

    QString fontSize = m_editor->editorCommandQueryCommandValue("fontSize");
    fontSize.remove("px");
    value = fontSize.toInt();
//    qDebug() << "query current font size : " << value;
    font.setPointSize(value == 0 ? m_app.userSettings().defaultFontSize() : value);

    value = m_editor->editorCommandQueryCommandState("underline");
//    qDebug() << "query current font underline : " << value;
    font.setUnderline(value == 1);

    value = m_editor->editorCommandQueryCommandState("strikethrough");
//    qDebug() << "query current font strikethrough : " << value;
    font.setStrikeOut(value == 1);
}

void EditorToolBar::setCurrentFont(const QFont& font)
{
    QFont currentFont;
    queryCurrentFont(currentFont);

    if (font.family() != currentFont.family())
    {
        QString strFontFamily = font.family();
        m_editor->editorCommandExecuteFontFamily(strFontFamily);
        selectCurrentFontFamily(strFontFamily);
    }

    if (font.pointSize() != currentFont.pointSize())
    {
        setFontPointSize(QString("%1px").arg(font.pointSize()));
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
}

void EditorToolBar::selectCurrentFontFamily(const QString& strFontFamily)
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

void EditorToolBar::selectCurrentFontFamilyItem(const QString& strFontFamily)
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

void EditorToolBar::setFontPointSize(const QString& strSize)
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

void EditorToolBar::saveImage(QString strFileName)
{
    QFileInfo info(strFileName);
    QPixmap pix(info.filePath());
    if (pix.isNull())
    {
        TOLOG(_T("[Save] : image is null"));
        return;
    }

    savePixmap(pix, info.suffix(), false);
}

void EditorToolBar::copyImage(QString strFileName)
{
    QFileInfo info(strFileName);
    QPixmap pix(info.filePath());
    if (pix.isNull())
    {
        TOLOG(_T("[Copy] : image is null"));
        return;
    }

    QClipboard* clip = QApplication::clipboard();
    clip->setPixmap(pix);
}

QAction* EditorToolBar::actionFromName(const QString& strName)
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

bool EditorToolBar::processImageSrc(bool bUseForCopy, bool& bNeedSubsequent)
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
        CWizFileDownloader* downloader = new CWizFileDownloader(m_strImageSrc, fileName);
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

bool EditorToolBar::processBase64Image(bool bUseForCopy)
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

void EditorToolBar::savePixmap(QPixmap& pix, const QString& strType, bool bUseForCopy)
{
    if (!bUseForCopy)
    {
        QString strFilePath = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                                           QDir::homePath() + "/untitled." + strType, tr("Image Files (*.%1)").arg(strType));
        if (strFilePath.isEmpty())
            return;

        bool ret = pix.save(strFilePath, strType.toUtf8());
        TOLOG2(_T("[Save] : save image to %1, result : %2"), strFilePath,
               ret ? "OK" : "Failed");    //pix formart should use ascii or capital letter.
    }
    else
    {
        QClipboard* clip = QApplication::clipboard();
        clip->setPixmap(pix);
    }
}

void EditorToolBar::saveGif(const QByteArray& ba)
{

}

bool EditorToolBar::hasFocus()
{
    return QWidget::hasFocus() || m_comboFontFamily->isPopuping() || m_comboFontSize->isPopuping() || m_comboParagraph->isPopuping();
}

void EditorToolBar::buildMenu()
{
    if (!m_menuContext) {
        m_menuContext = new QMenu(this);
    }

    m_menuContext->clear();
    m_actions.clear();

    int index = 0;
    WizEditorContextMenuItem* arrayData = contextMenuData();
    while (1) {
        WizEditorContextMenuItem& item = arrayData[index];
        if (item.label.isEmpty() && item.command.isEmpty() && item.execute.isEmpty()) {
            break;

        } else if (item.label == "-") {
            m_menuContext->addSeparator();

        } else if (item.execute == "+") {
            index = buildMenu(m_menuContext, index);

        } else if (item.command != "+") {
            if (!item.command.isEmpty()) {
                int value = m_editor->editorCommandQueryCommandState(item.command);
                if (value == -1) {
                    index++;
                    continue;
                }
            }

            QString strSlot = "1" + item.execute + "()";
            if (item.localSlot) {
                m_actions[item.label] = m_menuContext->addAction(item.label, this, strSlot.toUtf8());
            } else {
                m_actions[item.label] = m_menuContext->addAction(item.label, m_editor, strSlot.toUtf8());
            }
        } else {
            Q_ASSERT(0);
        }

        index++;
    }
}

int EditorToolBar::buildMenu(QMenu* pMenu, int indx)
{
    int index = indx;
    bool bSkip = true;
    WizEditorContextMenuItem* arrayData = contextMenuData();
    WizEditorContextMenuItem& curItem = arrayData[index];
    QMenu* pSubMenu = new QMenu(curItem.label, pMenu);

    while(1) {
        index++;

        WizEditorContextMenuItem& item = arrayData[index];
        if (item.label == "+") {
            break;
        } else if (item.execute == "+") {
            index = buildMenu(pSubMenu, index);
        } else if (item.label == "-") {
            pSubMenu->addSeparator();

        } else if (item.command != "+" && !item.execute.isEmpty()) {

//            // special case
//            if (m_editor->editorCommandQueryLink()
//                    && item.label == WIZEDITOR_ACTION_LINK_INSERT) {
//                continue;
//            } else if (!m_editor->editorCommandQueryLink()
//                       && item.label == WIZEDITOR_ACTION_LINK_EDIT) {
//                continue;
//            }

            if (!item.command.isEmpty()) {
                int value = m_editor->editorCommandQueryCommandState(item.command);
                if (value == -1) {
                    continue;
                }
            }

            bSkip = false;
            QString strSlot = "1" + item.execute + "()";
            if (item.localSlot) {
                m_actions[item.label] = pSubMenu->addAction(item.label, this, strSlot.toUtf8());
            } else {
                m_actions[item.label] = pSubMenu->addAction(item.label, m_editor, strSlot.toUtf8());
            }

        } else if (item.command.isEmpty() && item.execute.isEmpty()) {
            continue;
        } else {
            Q_ASSERT(0);
        }
    }

    if (!bSkip) {
        pMenu->addMenu(pSubMenu);
    } else {
        pSubMenu->deleteLater();
    }

    return index;
}

void EditorToolBar::on_editor_google_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuSearchByGoogle");
    QUrl url("http://google.com/search?q=" + m_editor->page()->selectedText());
    QDesktopServices::openUrl(url);
}

void EditorToolBar::on_editor_baidu_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuSearchByBaidu");
    QUrl url("http://www.baidu.com/s?wd=" + m_editor->page()->selectedText());
    QDesktopServices::openUrl(url);
}

#ifdef USEWEBENGINE
void EditorToolBar::on_editor_cut_triggered()
{
    m_editor->triggerPageAction(QWebEnginePage::Cut);
}

void EditorToolBar::on_editor_copy_triggered()
{
    m_editor->triggerPageAction(QWebEnginePage::Copy);
}

void EditorToolBar::on_editor_paste_triggered()
{
    m_editor->triggerPageAction(QWebEnginePage::Paste);
}
#else
void EditorToolBar::on_editor_cut_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuCut");
    m_editor->triggerPageAction(QWebPage::Cut);
}

void EditorToolBar::on_editor_copy_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuCopy");
    m_editor->triggerPageAction(QWebPage::Copy);
}

void EditorToolBar::on_editor_paste_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuPaste");
    m_editor->triggerPageAction(QWebPage::Paste);
}

void EditorToolBar::on_editor_bold_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuBold");
    m_editor->editorCommandExecuteBold();
}

void EditorToolBar::on_editor_italic_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuItalic");
    m_editor->editorCommandExecuteItalic();
}

void EditorToolBar::on_editor_underline_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuUnderline");
    m_editor->editorCommandExecuteUnderLine();
}

void EditorToolBar::on_editor_strikethrough_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuStrikeThrough");
    m_editor->editorCommandExecuteStrikeThrough();
}

void EditorToolBar::on_editor_insertLink_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuInsertLink");
    m_editor->editorCommandExecuteLinkInsert();
}

void EditorToolBar::on_editor_editLink_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuEditLink");
    m_editor->editorCommandExecuteLinkInsert();
}

void EditorToolBar::on_editor_removeLink_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuRemoveLink");
    m_editor->editorCommandExecuteLinkRemove();
}

void EditorToolBar::on_editor_insertTable_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuInsertTable");
    m_editor->editorCommandExecuteTableInsert();
}

void EditorToolBar::on_editor_deleteTable_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuDeleteTable");
    m_editor->editorCommandExecuteTableDelete();
}

void EditorToolBar::on_editor_justifyLeft_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuJustifyLeft");
    m_editor->editorCommandExecuteJustifyLeft();
}

void EditorToolBar::on_editor_justifyCenter_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuJustifyCenter");
    m_editor->editorCommandExecuteJustifyCenter();
}

void EditorToolBar::on_editor_justifyRight_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuJustifyRight");
    m_editor->editorCommandExecuteJustifyRight();
}
#endif

void EditorToolBar::on_comboParagraph_indexChanged(int index)
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarParagraph");
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

void EditorToolBar::on_comboFontFamily_indexChanged(int index)
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarFontFamily");

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
        QFont font;
        queryCurrentFont(font);
        //NOTE : 在QT5.4.2版本中存在问题，打开QFontDialog后，在编辑器中选择文本，再次回到QFontDialog时
        // currentFontChanged 将不会再次发出。 所以使用模态对话框强制用户关闭
        QFontDialog fontDialog;
        fontDialog.setCurrentFont(font);
        connect(&fontDialog, SIGNAL(currentFontChanged(QFont)), SLOT(on_fontDailogFontChanged(QFont)));
        fontDialog.exec();
    }
    else if (helperData == WIZSEPARATOR)
    {
        QString value = m_editor->editorCommandQueryCommandValue("fontFamily");
        m_comboFontFamily->setText(value);
    }
}

void EditorToolBar::on_comboFontSize_indexChanged(const QString& strSize)
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarFontSize");
    setFontPointSize(strSize);
}

void EditorToolBar::on_btnFormatMatch_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarFormatMatch");
    if (m_editor) {
        m_editor->editorCommandExecuteFormatMatch();
    }
}

void EditorToolBar::on_btnBold_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarBold");
    if (m_editor) {
        m_editor->editorCommandExecuteBold();
    }
}

void EditorToolBar::on_btnItalic_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarItalic");
    if (m_editor) {
        m_editor->editorCommandExecuteItalic();
    }
}

void EditorToolBar::on_btnUnderLine_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarUnderLine");
    if (m_editor) {
        m_editor->editorCommandExecuteUnderLine();
    }
}

void EditorToolBar::on_btnStrikeThrough_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarStrikeThrough");
    if (m_editor) {
        m_editor->editorCommandExecuteStrikeThrough();
    }
}

void EditorToolBar::on_btnJustify_clicked()
{    
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarJustify");
    //
    QPoint pos = m_btnJustify->mapToGlobal(QPoint(0, m_btnJustify->height()));
    m_menuJustify->move(pos);
    QAction*action = m_menuJustify->exec();
    if (action)
    {
        m_btnJustify->setIcon(action->icon());
    }
}

void EditorToolBar::on_btnJustifyLeft_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarJustifyLeft");
    if (m_editor) {
        m_editor->editorCommandExecuteJustifyLeft();
    }
}

void EditorToolBar::on_btnJustifyCenter_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarJustifyCenter");
    if (m_editor) {
        m_editor->editorCommandExecuteJustifyCenter();
    }
}

void EditorToolBar::on_btnJustifyRight_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarJustifyLeft");
    if (m_editor) {
        m_editor->editorCommandExecuteJustifyRight();
    }
}

void EditorToolBar::on_btnSearchReplace_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarSearchReplace");
    if (m_editor) {
        m_editor->editorCommandExecuteFindReplace();
    }
}

void EditorToolBar::on_btnUnorderedList_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarUnorderedList");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertUnorderedList();
    }
}

void EditorToolBar::on_btnOrderedList_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarOrderedList");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertOrderedList();
    }
}

void EditorToolBar::on_btnTable_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarTable");
    if (m_editor) {
        m_editor->editorCommandExecuteTableInsert();
    }
}

void EditorToolBar::on_btnHorizontal_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarHorizontal");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertHorizontal();
    }
}

void EditorToolBar::on_btnCheckList_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarCheckList");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertCheckList();
    }
}

void EditorToolBar::on_btnImage_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarImage");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertImage();
    }
}

void EditorToolBar::on_btnMobileImage_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarMobileImage");
    bool bReceiveImage = m_btnMobileImage->isChecked();
    if (m_editor)
    {
        m_editor->editorCommandExecuteMobileImage(bReceiveImage);
        //need update button status after show dialog
        m_btnMobileImage->setChecked(bReceiveImage);
        update();
    }
}

void EditorToolBar::on_btnScreenShot_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarScreenShot");
    if (m_editor) {
        m_editor->editorCommandExecuteScreenShot();
    }
}

void EditorToolBar::on_btnViewSource_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarViewSource");
    if (m_editor) {
        m_editor->editorCommandExecuteViewSource();
    }
}

void EditorToolBar::on_btnInsertCode_clicked()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorToolBarInsertCode");
    if (m_editor) {
        m_editor->editorCommandExecuteInsertCode();
    }
}

void EditorToolBar::on_editor_saveImageAs_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuSaveImageAs");
    bool bNeedSubsequent = false;
    if (processImageSrc(false, bNeedSubsequent) && bNeedSubsequent)
    {
        saveImage(m_strImageSrc);
    }
}

void EditorToolBar::on_editor_copyImage_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuCopyImage");
    bool bNeedSubsequent = false;
    if (processImageSrc(true, bNeedSubsequent) && bNeedSubsequent)
    {
        copyImage(m_strImageSrc);
    }
}

void EditorToolBar::on_editor_copyImageLink_triggered()
{
    CWizAnalyzer::GetAnalyzer().LogAction("editorMenuCopyImageLink");
    if (m_strImageSrc.isEmpty())
        return;

    QClipboard* clip = QApplication::clipboard();
    clip->setText(m_strImageSrc);
}
