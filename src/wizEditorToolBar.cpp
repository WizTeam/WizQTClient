#include "wizEditorToolBar.h"

#include <QStylePainter>
#include <QToolButton>
#include <QFontComboBox>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QAction>
#include <QMenu>

#include "share/wizmisc.h"
#include "wizdef.h"
#include "share/wizsettings.h"
#include "wizDocumentWebView.h"
#include "wizactions.h"

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

        subOpt.rect = rectSub.adjusted(0, 1, 0, -rectSub.height()/2);
        drawComboPrimitive(&painter, QStyle::PE_IndicatorArrowUp, subOpt);

        subOpt.rect = rectSub.adjusted(0, rectSub.height()/2 + 5, 0, -rectSub.height()/2);
        drawComboPrimitive(&painter, QStyle::PE_IndicatorArrowDown, subOpt);
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

        opt.icon.paint(&p, opt.rect, Qt::AlignCenter, mode, state);
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
    }

    void setColor(const QColor& color)
    {
        m_color = color;
        repaint();
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

        opt.icon.paint(&p, opt.rect, Qt::AlignCenter, mode, state);

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
};

class CWizToolComboBoxFont : public QFontComboBox
{
public:
    CWizToolComboBoxFont(QWidget* parent = 0) : QFontComboBox(parent)
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
};


EditorToolBar::EditorToolBar(QWidget *parent)
    : QWidget(parent)
{
    QString skin = "default";

    m_comboFontFamily = new CWizToolComboBoxFont(this);
    connect(m_comboFontFamily, SIGNAL(activated(const QString&)),
            SLOT(on_comboFontFamily_indexChanged(const QString&)));

    QStringList listSize;
    listSize << "9px" << "10px" << "11px" << "12px" << "13px"<< "14px"
             << "18px" << "24px" << "36px" << "48px" << "64px" << "72px";
    m_comboFontSize = new CWizToolComboBox(this);
    m_comboFontSize->addItems(listSize);
    connect(m_comboFontSize, SIGNAL(activated(const QString&)),
            SLOT(on_comboFontSize_indexChanged(const QString&)));

    m_btnFormatMatch = new CWizToolButton(this);
    m_btnFormatMatch->setIcon(::WizLoadSkinIcon(skin, "actionFormatMatch"));
    connect(m_btnFormatMatch, SIGNAL(clicked()), SLOT(on_btnFormatMatch_clicked()));

    m_btnForeColor = new CWizToolButtonColor(this);
    m_btnForeColor->setIcon(::WizLoadSkinIcon(skin, "actionFormatForeColor"));
    connect(m_btnForeColor, SIGNAL(clicked()), SLOT(on_BtnForeColor_clicked()));

    m_btnBackColor = new CWizToolButtonColor(this);
    m_btnBackColor->setIcon(::WizLoadSkinIcon(skin, "actionFormatBackColor"));
    connect(m_btnBackColor, SIGNAL(clicked()), SLOT(on_BtnBackColor_clicked()));

    m_btnBold = new CWizToolButton(this);
    m_btnBold->setIcon(::WizLoadSkinIcon(skin, "actionFormatBold"));
    connect(m_btnBold, SIGNAL(clicked()), SLOT(on_btnBold_clicked()));

    m_btnItalic = new CWizToolButton(this);
    m_btnItalic->setIcon(::WizLoadSkinIcon(skin, "actionFormatItalic"));
    connect(m_btnItalic, SIGNAL(clicked()), SLOT(on_btnItalic_clicked()));

    m_btnUnderLine = new CWizToolButton(this);
    m_btnUnderLine->setIcon(::WizLoadSkinIcon(skin, "actionFormatUnderLine"));
    connect(m_btnUnderLine, SIGNAL(clicked()), SLOT(on_btnUnderLine_clicked()));

    m_btnJustifyLeft = new CWizToolButton(this);
    m_btnJustifyLeft->setIcon(::WizLoadSkinIcon(skin, "actionFormatJustifyLeft"));
    connect(m_btnJustifyLeft, SIGNAL(clicked()), SLOT(on_btnJustifyLeft_clicked()));

    m_btnJustifyCenter = new CWizToolButton(this);
    m_btnJustifyCenter->setIcon(::WizLoadSkinIcon(skin, "actionFormatJustifyCenter"));
    connect(m_btnJustifyCenter, SIGNAL(clicked()), SLOT(on_btnJustifyCenter_clicked()));

    m_btnJustifyRight = new CWizToolButton(this);
    m_btnJustifyRight->setIcon(::WizLoadSkinIcon(skin, "actionFormatJustifyRight"));
    connect(m_btnJustifyRight, SIGNAL(clicked()), SLOT(on_btnJustifyRight_clicked()));

    m_btnUnorderedList = new CWizToolButton(this);
    m_btnUnorderedList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertUnorderedList"));
    connect(m_btnUnorderedList, SIGNAL(clicked()), SLOT(on_btnUnorderedList_clicked()));

    m_btnOrderedList = new CWizToolButton(this);
    m_btnOrderedList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertOrderedList"));
    connect(m_btnOrderedList, SIGNAL(clicked()), SLOT(on_btnOrderedList_clicked()));

    m_btnTable = new CWizToolButton(this);
    m_btnTable->setCheckable(false);
    m_btnTable->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertTable"));
    connect(m_btnTable, SIGNAL(clicked()), SLOT(on_btnTable_clicked()));

    m_btnHorizontal = new CWizToolButton(this);
    m_btnHorizontal->setCheckable(false);
    m_btnHorizontal->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertHorizontal"));
    connect(m_btnHorizontal, SIGNAL(clicked()), SLOT(on_btnHorizontal_clicked()));

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(3, 0, 3, 0);
    layout->setAlignment(Qt::AlignBottom);
    layout->setSpacing(2);
    setLayout(layout);

    layout->addWidget(m_comboFontFamily);
    layout->addSpacing(6);
    layout->addWidget(m_comboFontSize);
    layout->addSpacing(12);
    layout->addWidget(m_btnFormatMatch);
    layout->addWidget(m_btnForeColor);
    layout->addWidget(m_btnBackColor);
    layout->addWidget(m_btnBold);
    layout->addWidget(m_btnItalic);
    layout->addWidget(m_btnUnderLine);
    layout->addSpacing(12);
    layout->addWidget(m_btnJustifyLeft);
    layout->addWidget(m_btnJustifyCenter);
    layout->addWidget(m_btnJustifyRight);
    layout->addSpacing(12);
    layout->addWidget(m_btnUnorderedList);
    layout->addWidget(m_btnOrderedList);
    layout->addSpacing(12);
    layout->addWidget(m_btnTable);
    layout->addWidget(m_btnHorizontal);
    layout->addStretch();
}

QSize EditorToolBar::sizeHint() const
{
    return QSize(1, 32);
}

void EditorToolBar::resetToolbar()
{
    if (!m_editor)
        return;

    int state;
    QString value;

    value = m_editor->editorCommandQueryCommandValue("fontFamily");
    m_comboFontFamily->setText(value);

    value = m_editor->editorCommandQueryCommandValue("fontSize");
    m_comboFontSize->setText(value);

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
    m_btnForeColor->setColor(QColor(value));

    value = m_editor->editorCommandQueryCommandValue("backColor");
    m_btnBackColor->setColor(QColor(value));

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

    state = m_editor->editorCommandQueryCommandState("justify");
    value = m_editor->editorCommandQueryCommandValue("justify");
    if (state == -1) {
        m_btnJustifyLeft->setEnabled(false);
        m_btnJustifyCenter->setEnabled(false);
        m_btnJustifyRight->setEnabled(false);
    } else {
        m_btnJustifyLeft->setEnabled(true);
        m_btnJustifyCenter->setEnabled(true);
        m_btnJustifyRight->setEnabled(true);

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
}

struct WizEditorContextMenuItem
{
    QString label;
    QString command;
    QString execute;
};

#define WIZEDITOR_ACTION_GOOGLE         QObject::tr("Use \"Google\" search")

#define WIZEDITOR_ACTION_CUT            QObject::tr("Cut")
#define WIZEDITOR_ACTION_COPY           QObject::tr("Copy")
#define WIZEDITOR_ACTION_PASTE          QObject::tr("Paste")

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
        {WIZEDITOR_ACTION_GOOGLE,                   "",                 "on_editor_google_triggered"},
        {"-", "-", "-"},

        {WIZEDITOR_ACTION_CUT,                      "",                 "on_editor_cut_triggered"},
        {WIZEDITOR_ACTION_COPY,                     "",                 "on_editor_copy_triggered"},
        {WIZEDITOR_ACTION_PASTE,                    "",                 "on_editor_paste_triggered"},
        {"-", "-", "-"},

        {QObject::tr("Link"),                       "+",                "+"},
        {WIZEDITOR_ACTION_LINK_INSERT,              "link",             "editorCommandExecuteLinkInsert"},
        {WIZEDITOR_ACTION_LINK_EDIT,                "link",             "editorCommandExecuteLinkInsert"},
        {WIZEDITOR_ACTION_LINK_REMOVE,              "unlink",           "editorCommandExecuteLinkRemove"},
        {"+", "+", "+"},

        {QObject::tr("Font"),                       "+",                "+"},
        {WIZEDITOR_ACTION_FONT_BOLD,                "bold",             "editorCommandExecuteBold"},
        {WIZEDITOR_ACTION_FONT_ITALIC,              "italic",           "editorCommandExecuteItalic"},
        {WIZEDITOR_ACTION_FONT_UNDERLINE,           "underline",        "editorCommandExecuteUnderLine"},
        {WIZEDITOR_ACTION_FONT_STRIKETHROUGH,       "strikethrough",    "editorCommandExecuteStrikeThrough"},
        {"-", "-", "-"},
        {WIZEDITOR_ACTION_FONT_FORECOLOR,           "foreColor",        "editorCommandExecuteForeColor"},
        {WIZEDITOR_ACTION_FONT_BACKCOLOR,           "backColor",        "editorCommandExecuteBackColor"},
        {"+", "+", "+"},

        {QObject::tr("Justify"),                    "+",          "+"},
        {WIZEDITOR_ACTION_JUSTIFY_LEFT,             "justify",          "editorCommandExecuteJustifyLeft"},
        {WIZEDITOR_ACTION_JUSTIFY_CENTER,           "justify",          "editorCommandExecuteJustifyCenter"},
        {WIZEDITOR_ACTION_JUSTIFY_RIGHT,            "justify",          "editorCommandExecuteJustifyRight"},
        {"+", "+", "+"},

        {QObject::tr("Table"),                      "+",                "+"},
        {WIZEDITOR_ACTION_TABLE_INSERT,             "inserttable",      "editorCommandExecuteTableInsert"},
        {WIZEDITOR_ACTION_TABLE_DELETE,             "deletetable",      "editorCommandExecuteTableDelete"},
        {"-", "-", "-"},
        {WIZEDITOR_ACTION_TABLE_DELETE_ROW,         "deleterow",        "editorCommandExecuteTableDeleteRow"},
        {WIZEDITOR_ACTION_TABLE_DELETE_COLUM,       "deletecol",        "editorCommandExecuteTableDeleteCol"},
        {WIZEDITOR_ACTION_TABLE_INSERT_ROW,         "insertrow",        "editorCommandExecuteTableInsertRow"},
        {WIZEDITOR_ACTION_TABLE_INSERT_ROW_NEXT,    "insertrownext",    "editorCommandExecuteTableInsertRowNext"},
        {WIZEDITOR_ACTION_TABLE_INSERT_COLUM,       "insertcol",        "editorCommandExecuteTableInsertCol"},
        {WIZEDITOR_ACTION_TABLE_INSERT_COLUM_NEXT,  "insertcolnext",    "editorCommandExecuteTableInsertColNext"},
        {"-", "-", "-"},
        {WIZEDITOR_ACTION_TABLE_INSERT_CAPTION,     "insertcaption",    "editorCommandExecuteTableInsertCaption"},
        {WIZEDITOR_ACTION_TABLE_DELETE_CAPTION,     "deletecaption",    "editorCommandExecuteTableDeleteCaption"},
        {WIZEDITOR_ACTION_TABLE_INSERT_TITLE,       "inserttitle",      "editorCommandExecuteTableInsertTitle"},
        {WIZEDITOR_ACTION_TABLE_DELETE_TITLE,       "deletetitle",      "editorCommandExecuteTableDeleteTitle"},
        {"-", "-", "-"},
        {WIZEDITOR_ACTION_TABLE_MERGE_CELLS,        "mergecells",       "editorCommandExecuteTableMergeCells"},
        {WIZEDITOR_ACTION_TABLE_MERGE_RIGHT,        "mergeright",       "editorCommandExecuteTalbeMergeRight"},
        {WIZEDITOR_ACTION_TABLE_MERGE_DOWN,         "mergedown",        "editorCommandExecuteTableMergeDown"},
        {"-", "-", "-"},
        {WIZEDITOR_ACTION_TABLE_SPLIT_CELLS,        "splittocells",     "editorCommandExecuteTableSplitCells"},
        {WIZEDITOR_ACTION_TABLE_SPLIT_ROWS,         "splittorows",      "editorCommandExecuteTableSplitRows"},
        {WIZEDITOR_ACTION_TABLE_SPLIT_COLUMS,       "splittocols",      "editorCommandExecuteTableSplitCols"},
        {"-", "-", "-"},
        {WIZEDITOR_ACTION_TABLE_AVERAGE_ROWS,       "averagedistributerow", "editorCommandExecuteTableAverageRows"},
        {WIZEDITOR_ACTION_TABLE_AVERAGE_COLUMS,     "averagedistributecol", "editorCommandExecuteTableAverageCols"},
        {"+", "+", "+"},

        {"", "", ""}
    };

    return arrayData;
}

void EditorToolBar::setDelegate(CWizDocumentWebView* editor)
{
    Q_ASSERT(editor);

    m_editor = editor;

    connect(m_editor, SIGNAL(requestShowContextMenu(QPoint)),
            SLOT(on_delegate_requestShowContextMenu(QPoint)));

    connect(m_editor, SIGNAL(selectionChanged()),
            SLOT(on_delegate_selectionChanged()));
}

void EditorToolBar::on_delegate_requestShowContextMenu(const QPoint& pos)
{
    if (!m_editor)
        return;

    buildMenu();

    if (m_editor->selectedText().isEmpty()) {
        actionFromName(WIZEDITOR_ACTION_GOOGLE)->setEnabled(false);
    } else {
        actionFromName(WIZEDITOR_ACTION_GOOGLE)->setEnabled(true);
    }

    if (m_editor->isEditing()){
        actionFromName(WIZEDITOR_ACTION_CUT)->setEnabled(true);
        actionFromName(WIZEDITOR_ACTION_PASTE)->setEnabled(true);
    } else {
        actionFromName(WIZEDITOR_ACTION_CUT)->setEnabled(false);
        actionFromName(WIZEDITOR_ACTION_PASTE)->setEnabled(false);
    }

#ifdef QT_DEBUG
    m_menuContext->addAction(m_editor->pageAction(QWebPage::InspectElement));
#endif

    m_menuContext->popup(pos);
    m_menuContext->update();
}

void EditorToolBar::on_delegate_selectionChanged()
{
   resetToolbar();
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
            if (!item.command.isEmpty()) {
                m_actions[item.label] = m_menuContext->addAction(item.label, m_editor, strSlot.toUtf8());
            } else {
                m_actions[item.label] = m_menuContext->addAction(item.label, this, strSlot.toUtf8());
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

        } else if (item.label == "-") {
            pSubMenu->addSeparator();

        } else if (item.command != "+" && !item.execute.isEmpty()) {

            // special case
            if (m_editor->editorCommandQueryLink()
                    && item.label == WIZEDITOR_ACTION_LINK_INSERT) {
                continue;
            } else if (!m_editor->editorCommandQueryLink()
                       && item.label == WIZEDITOR_ACTION_LINK_EDIT) {
                continue;
            }

            if (!item.command.isEmpty()) {
                int value = m_editor->editorCommandQueryCommandState(item.command);
                if (value == -1) {
                    continue;
                }
            }

            bSkip = false;
            QString strSlot = "1" + item.execute + "()";
            m_actions[item.label] = pSubMenu->addAction(item.label, m_editor, strSlot.toUtf8());

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
    QUrl url("http://google.com/search?q=" + m_editor->page()->selectedText());
    QDesktopServices::openUrl(url);
}

void EditorToolBar::on_editor_cut_triggered()
{
    m_editor->triggerPageAction(QWebPage::Cut);
}

void EditorToolBar::on_editor_copy_triggered()
{
    m_editor->triggerPageAction(QWebPage::Copy);
}

void EditorToolBar::on_editor_paste_triggered()
{
    m_editor->triggerPageAction(QWebPage::Paste);
}

void EditorToolBar::on_comboFontFamily_indexChanged(const QString& strFamily)
{
    if (strFamily == m_comboFontFamily->text())
        return;

    if (m_editor) {
        m_editor->editorCommandExecuteFontFamily(strFamily);
        m_comboFontFamily->setText(strFamily);
    }
}

void EditorToolBar::on_comboFontSize_indexChanged(const QString& strSize)
{
    if (strSize == m_comboFontSize->text())
        return;

    if (m_editor) {
        m_editor->editorCommandExecuteFontSize(strSize);
        m_comboFontSize->setText(strSize);
    }
}

void EditorToolBar::on_btnFormatMatch_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteFormatMatch();
    }
}

void EditorToolBar::on_BtnForeColor_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteForeColor();
    }
}

void EditorToolBar::on_BtnBackColor_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteBackColor();
    }
}

void EditorToolBar::on_btnBold_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteBold();
    }
}

void EditorToolBar::on_btnItalic_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteItalic();
    }
}

void EditorToolBar::on_btnUnderLine_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteUnderLine();
    }
}

void EditorToolBar::on_btnJustifyLeft_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteJustifyLeft();
    }
}

void EditorToolBar::on_btnJustifyCenter_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteJustifyCenter();
    }
}

void EditorToolBar::on_btnJustifyRight_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteJustifyRight();
    }
}

void EditorToolBar::on_btnUnorderedList_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteInsertUnorderedList();
    }
}

void EditorToolBar::on_btnOrderedList_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteInsertOrderedList();
    }
}

void EditorToolBar::on_btnTable_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteTableInsert();
    }
}

void EditorToolBar::on_btnHorizontal_clicked()
{
    if (m_editor) {
        m_editor->editorCommandExecuteInsertHorizontal();
    }
}
