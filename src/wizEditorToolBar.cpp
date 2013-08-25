#include "wizEditorToolBar.h"

#include <QtGui>

#include "share/wizmisc.h"
#include "wizdef.h"
#include "wizmainwindow.h"
#include "wizDocumentWebView.h"
#include "wizactions.h"

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
    p->setRenderHint(QPainter::Antialiasing);
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
    path.moveTo(0, 4);
    path.lineTo(-3, -2);
    path.lineTo(3, -2);
    p->setMatrix(matrix);
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(0, 0, 0, 160));
    p->drawPath(path);
    p->restore();
}

class CWizToolButton : public QToolButton
{
public:
    CWizToolButton(QWidget* parent = 0) : QToolButton(parent)
    {
        setFocusPolicy(Qt::NoFocus);
        setCheckable(true);
        setIconSize(QSize(16, 16));
    }

protected:
    virtual void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);

        QStylePainter p(this);
        QStyleOptionToolButton option;
        initStyleOption(&option);

        QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
        if (mode == QIcon::Normal && (option.state & QStyle::State_HasFocus || option.state & QStyle::State_Sunken))
            mode = QIcon::Active;
        QIcon::State state = QIcon::Off;
        if (option.state & QStyle::State_On)
            state = QIcon::On;

        option.icon.paint(&p, option.rect, Qt::AlignCenter, mode, state);
    }

    virtual QSize sizeHint() const
    {
        return iconSize() + QSize(4, 4);
    }
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

        QStylePainter p(this);
        QStyleOptionToolButton opt;
        initStyleOption(&opt);

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
    CWizToolComboBox(QWidget* parent = 0) : QComboBox(parent)
    {
        setFocusPolicy(Qt::NoFocus);
        setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
        //setSizeAdjustPolicy(QComboBox::QComboBox::AdjustToMinimumContentsLength);
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


CWizEditorToolBar::CWizEditorToolBar(CWizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
{
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    QString skin = m_app.userSettings().skin();

    m_comboFontFamily = new CWizToolComboBoxFont(this);
    connect(m_comboFontFamily, SIGNAL(activated(const QString&)),
            SLOT(on_actionFormatFontFamily_activated(const QString&)));

    QStringList listSize;
    listSize << "9px" << "10px" << "11px" << "12px" << "13px"<< "14px"
             << "18px" << "24px" << "36px" << "48px" << "64px" << "72px";
    m_comboFontSize = new CWizToolComboBox(this);
    m_comboFontSize->addItems(listSize);
    connect(m_comboFontSize, SIGNAL(activated(const QString&)),
            SLOT(on_actionFormatFontSize_activated(const QString&)));

    m_btnForeColor = new CWizToolButtonColor(this);
    m_btnForeColor->setIcon(::WizLoadSkinIcon(skin, "actionFormatColor"));
    connect(m_btnForeColor, SIGNAL(clicked()), mainWindow,
            SLOT(on_actionFormatForeColor_triggered()));

    m_btnBold = new CWizToolButton(this);
    m_btnBold->setIcon(::WizLoadSkinIcon(skin, "actionFormatBold"));
    connect(m_btnBold, SIGNAL(clicked()), mainWindow,
            SLOT(on_actionFormatBold_triggered()));

    m_btnItalic = new CWizToolButton(this);
    m_btnItalic->setIcon(::WizLoadSkinIcon(skin, "actionFormatItalic"));
    connect(m_btnItalic, SIGNAL(clicked()), mainWindow,
            SLOT(on_actionFormatItalic_triggered()));

    m_btnUnderLine = new CWizToolButton(this);
    m_btnUnderLine->setIcon(::WizLoadSkinIcon(skin, "actionFormatUnderLine"));
    connect(m_btnUnderLine, SIGNAL(clicked()), mainWindow,
            SLOT(on_actionFormatUnderLine_triggered()));

    m_btnJustifyLeft = new CWizToolButton(this);
    m_btnJustifyLeft->setIcon(::WizLoadSkinIcon(skin, "actionFormatJustifyLeft"));
    connect(m_btnJustifyLeft, SIGNAL(clicked()), mainWindow,
            SLOT(on_actionFormatJustifyLeft_triggered()));

    m_btnJustifyCenter = new CWizToolButton(this);
    m_btnJustifyCenter->setIcon(::WizLoadSkinIcon(skin, "actionFormatJustifyCenter"));
    connect(m_btnJustifyCenter, SIGNAL(clicked()), mainWindow,
            SLOT(on_actionFormatJustifyCenter_triggered()));

    m_btnJustifyRight = new CWizToolButton(this);
    m_btnJustifyRight->setIcon(::WizLoadSkinIcon(skin, "actionFormatJustifyRight"));
    connect(m_btnJustifyRight, SIGNAL(clicked()), mainWindow,
            SLOT(on_actionFormatJustifyRight_triggered()));

    m_btnUnorderedList = new CWizToolButton(this);
    m_btnUnorderedList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertUnorderedList"));
    connect(m_btnUnorderedList, SIGNAL(clicked()), mainWindow,
            SLOT(on_actionFormatInsertUnorderedList_triggered()));

    m_btnOrderedList = new CWizToolButton(this);
    m_btnOrderedList->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertOrderedList"));
    connect(m_btnOrderedList, SIGNAL(clicked()), mainWindow,
            SLOT(on_actionFormatInsertOrderedList_triggered()));

    m_btnTable = new CWizToolButton(this);
    m_btnTable->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertTable"));
    connect(m_btnTable, SIGNAL(clicked()), mainWindow,
            SLOT(on_actionFormatInsertTable_triggered()));

    m_btnHorizontal = new CWizToolButton(this);
    m_btnHorizontal->setIcon(::WizLoadSkinIcon(skin, "actionFormatInsertHorizontal"));
    connect(m_btnHorizontal, SIGNAL(clicked()), mainWindow,
            SLOT(on_actionFormatInsertHorizontal_triggered()));

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(3, 0, 3, 0);
    layout->setAlignment(Qt::AlignBottom);
    layout->setSpacing(2);
    setLayout(layout);

    layout->addWidget(m_comboFontFamily);
    layout->addSpacing(6);
    layout->addWidget(m_comboFontSize);
    layout->addSpacing(12);
    layout->addWidget(m_btnForeColor);
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

QSize CWizEditorToolBar::sizeHint() const
{
    return QSize(1, 32);
}

void CWizEditorToolBar::on_actionFormatFontFamily_activated(const QString& strFamily)
{
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    mainWindow->web()->editorCommandExecuteFontFamily(strFamily);

    m_comboFontFamily->setText(strFamily);
}

void CWizEditorToolBar::on_actionFormatFontSize_activated(const QString& strSize)
{
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    mainWindow->web()->editorCommandExecuteFontSize(strSize);

    m_comboFontSize->setText(strSize);
}

void CWizEditorToolBar::resetToolbar()
{
    int state;
    QString value;
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());

    value = mainWindow->web()->editorCommandQueryCommandValue("fontFamily");
    m_comboFontFamily->setText(value);

    value = mainWindow->web()->editorCommandQueryCommandValue("fontSize");
    m_comboFontSize->setText(value);

    value = mainWindow->web()->editorCommandQueryCommandValue("foreColor");
    m_btnForeColor->setColor(QColor(value));

    state = mainWindow->web()->editorCommandQueryCommandState("bold");
    if (state == 1) {
        m_btnBold->setChecked(true);
    } else {
        m_btnBold->setChecked(false);
    }

    state = mainWindow->web()->editorCommandQueryCommandState("italic");
    if (state == 1) {
        m_btnItalic->setChecked(true);
    } else {
        m_btnItalic->setChecked(false);
    }

    state = mainWindow->web()->editorCommandQueryCommandState("underline");
    if (state == 1) {
        m_btnUnderLine->setChecked(true);
    } else {
        m_btnUnderLine->setChecked(false);
    }

    value = mainWindow->web()->editorCommandQueryCommandValue("justify");
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

    state = mainWindow->web()->editorCommandQueryCommandState("insertOrderedList");
    if (state == 1) {
        m_btnOrderedList->setChecked(true);
    } else {
        m_btnOrderedList->setChecked(false);
    }

    state = mainWindow->web()->editorCommandQueryCommandState("insertUnorderedList");
    if (state == 1) {
        m_btnUnorderedList->setChecked(true);
    } else {
        m_btnUnorderedList->setChecked(false);
    }
}

struct WizEditorContextMenuItem
{
    QString label;
    QString command;
    QString execute;
};

#define WIZEDITOR_ACTION_CUT            QObject::tr("Cut")
#define WIZEDITOR_ACTION_COPY           QObject::tr("Copy")
#define WIZEDITOR_ACTION_PASTE          QObject::tr("Paste")

#define WIZEDITOR_ACTION_LINK_INSERT    QObject::tr("Insert Link")
#define WIZEDITOR_ACTION_LINK_EDIT       QObject::tr("Edit Link")
#define WIZEDITOR_ACTION_LINK_REMOVE    QObject::tr("Remove Link")

#define WIZEDITOR_ACTION_FONT_BOLD      QObject::tr("Bold")
#define WIZEDITOR_ACTION_FONT_ITALIC    QObject::tr("Italic")
#define WIZEDITOR_ACTION_FONT_UNDERLINE QObject::tr("Underline")
#define WIZEDITOR_ACTION_FONT_STRIKETHROUGH QObject::tr("Strike through")
#define WIZEDITOR_ACTION_FONT_FORECOLOR QObject::tr("font color")

#define WIZEDITOR_ACTION_JUSTIFY_LEFT QObject::tr("Justify left")
#define WIZEDITOR_ACTION_JUSTIFY_CENTER QObject::tr("Justify center")
#define WIZEDITOR_ACTION_JUSTIFY_RIGHT QObject::tr("Justify right")

#define WIZEDITOR_ACTION_TABLE_INSERT QObject::tr("Insert table")
#define WIZEDITOR_ACTION_TABLE_DELETE QObject::tr("Delete table")

#define WIZEDITOR_ACTION_TABLE_DELETE_ROW QObject::tr("Delete row")
#define WIZEDITOR_ACTION_TABLE_DELETE_COLUM QObject::tr("Delete colum")

#define WIZEDITOR_ACTION_TABLE_INSERT_ROW QObject::tr("Insert row")
#define WIZEDITOR_ACTION_TABLE_INSERT_ROW_NEXT QObject::tr("Insert row next")
#define WIZEDITOR_ACTION_TABLE_INSERT_COLUM QObject::tr("Insert colum")
#define WIZEDITOR_ACTION_TABLE_INSERT_COLUM_NEXT QObject::tr("Insert colum next")

#define WIZEDITOR_ACTION_TABLE_INSERT_CAPTION QObject::tr("Insert caption")
#define WIZEDITOR_ACTION_TABLE_DELETE_CAPTION QObject::tr("Delete caption")
#define WIZEDITOR_ACTION_TABLE_INSERT_TITLE QObject::tr("Insert title")
#define WIZEDITOR_ACTION_TABLE_DELETE_TITLE QObject::tr("Delete title")

#define WIZEDITOR_ACTION_TABLE_MERGE_CELLS QObject::tr("Merge cells")
#define WIZEDITOR_ACTION_TABLE_MERGE_RIGHT QObject::tr("Merge right")
#define WIZEDITOR_ACTION_TABLE_MERGE_DOWN QObject::tr("Merge down")

#define WIZEDITOR_ACTION_TABLE_SPLIT_CELLS QObject::tr("Split cells")
#define WIZEDITOR_ACTION_TABLE_SPLIT_ROWS QObject::tr("Split rows")
#define WIZEDITOR_ACTION_TABLE_SPLIT_COLUMS QObject::tr("Split colums")

#define WIZEDITOR_ACTION_TABLE_AVERAGE_ROWS QObject::tr("Averaged distribute rows")
#define WIZEDITOR_ACTION_TABLE_AVERAGE_COLUMS QObject::tr("Averaged distribute colums")


WizEditorContextMenuItem* CWizEditorToolBar::contextMenuData()
{
    static WizEditorContextMenuItem arrayData[] =
    {
        {WIZEDITOR_ACTION_CUT,                      "",                 "editorCommandExecuteCut"},
        {WIZEDITOR_ACTION_COPY,                     "",                 "editorCommandExecuteCopy"},
        {WIZEDITOR_ACTION_PASTE,                    "",                 "editorCommandExecutePaste"},
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
        {WIZEDITOR_ACTION_FONT_FORECOLOR,           "",                 "editorCommandExecuteForeColor"},
        {"+", "+", "+"},

        {QObject::tr("Justify"),                    "justify",          "+"},
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

void CWizEditorToolBar::resetContextMenuAndPop(const QPoint& pos)
{
    buildMenu();
    m_menuContext->popup(pos);
}

void CWizEditorToolBar::buildMenu()
{
    if (!m_menuContext) {
        m_menuContext = new QMenu(this);
    }

    m_menuContext->clear();

    int index = 0;
    WizEditorContextMenuItem* arrayData = contextMenuData();
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    while (1) {
        WizEditorContextMenuItem& item = arrayData[index];
        if (item.label.isEmpty() && item.command.isEmpty() && item.execute.isEmpty()) {
            break;

        } else if (item.label == "-") {
            m_menuContext->addSeparator();

        } else if (item.execute == "+") {
            index = buildMenu(m_menuContext, index);

        } else if (item.command != "+" && !item.execute.isEmpty()) {
            if (!item.command.isEmpty()) {
                int value = mainWindow->web()->editorCommandQueryCommandState(item.command);
                if (value == -1) {
                    index++;
                    continue;
                }
            }

            QString strSlot = "1" + item.execute + "()";
            m_menuContext->addAction(item.label, mainWindow->web(), strSlot.toUtf8());
        } else if (item.command.isEmpty() && item.execute.isEmpty()) {
            index++;
            continue;
        } else {
            Q_ASSERT(0);
        }

        index++;
    }
}

int CWizEditorToolBar::buildMenu(QMenu* pMenu, int indx)
{
    int index = indx;
    bool bSkip = false;
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    WizEditorContextMenuItem* arrayData = contextMenuData();

    WizEditorContextMenuItem& curItem = arrayData[index];
    if (curItem.command != "+") {
        int value = mainWindow->web()->editorCommandQueryCommandState(curItem.command);
        if (value == -1) {
            bSkip = true;
        }
    }

    QMenu* pSubMenu = new QMenu(curItem.label, pMenu);

    if (!bSkip) {
        pMenu->addMenu(pSubMenu);
    }

    while(1) {
        index++;

        WizEditorContextMenuItem& item = arrayData[index];
        if (item.label == "+") {
            break;

        } else if (item.label == "-") {
            pSubMenu->addSeparator();

        } else if (item.command != "+" && !item.execute.isEmpty()) {

            // special case
            if (mainWindow->web()->editorCommandQueryLink()
                    && item.label == WIZEDITOR_ACTION_LINK_INSERT) {
                continue;
            } else if (!mainWindow->web()->editorCommandQueryLink()
                       && item.label == WIZEDITOR_ACTION_LINK_EDIT) {
                continue;
            }

            if (!item.command.isEmpty()) {
                int value = mainWindow->web()->editorCommandQueryCommandState(item.command);
                if (value == -1) {
                    continue;
                }
            }

            QString strSlot = "1" + item.execute + "()";
            pSubMenu->addAction(item.label, mainWindow->web(), strSlot.toUtf8());

        } else if (item.command.isEmpty() && item.execute.isEmpty()) {
            continue;
        } else {
            Q_ASSERT(0);
        }
    }

    return index;
}
