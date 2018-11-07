#include "WizSearchWidget.h"

#ifdef USECOCOATOOLBAR
#include "mac/WizSearchWidget_mm.h"
#else
#include "share/WizSettings.h"
#include "share/WizAnalyzer.h"
#include "share/WizUIBase.h"
#include "WizDef.h"
#include "utils/WizStyleHelper.h"

#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPixmap>
#include <QMenu>
#include <QMouseEvent>
#include <QDebug>

WizSearchView::WizSearchView(QWidget* parent /* = 0 */)
    : QWidget(parent)
    , m_widthHint(360)
{
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
    setContentsMargins(1, 0, 1, 1);

    m_editSearch = new WizSearchEdit(this);
    m_editSearch->setTextMargins(WizSmartScaleUI(25), 1, 0, 1);
    //
    QString styleSheet;
    if (isDarkMode()) {
        styleSheet = QString("QLineEdit{color:#ffffff;background-color:#686868;border:1px solid #333333; border-radius:%1px;}"
                                     "QLineEdit::focus{background-color:#686868;border:1px solid #6699cb; border-radius:%1px;}")
                .arg(WizSmartScaleUI(10));
    } else {
        styleSheet = QString("QLineEdit{background-color:#eeeeee;border:1px solid #aeaeae; border-radius:%1px;}"
                                     "QLineEdit::focus{background-color:#ffffff;border:1px solid #6699cb; border-radius:%1px;}")
                .arg(WizSmartScaleUI(10));
    }

    m_editSearch->setStyleSheet(styleSheet);

    // avoid focus rect on OSX, this should be a bug of qt style sheet
    m_editSearch->setAttribute(Qt::WA_MacShowFocusRect, 0);    

    QHBoxLayout* layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_editSearch);
    layout->setStretch(1, 1);


    connect(m_editSearch, SIGNAL(returnPressed()), \
            SLOT(on_search_returnPressed()));
    connect(m_editSearch, SIGNAL(textChanged(QString)), \
            SLOT(on_searchTextChanged(QString)), Qt::QueuedConnection);
    connect(m_editSearch, SIGNAL(addCustomSearchRequest()),
            SIGNAL(addCustomSearchRequest()));
}

void WizSearchView::clear()
{
    m_editSearch->clear();
}

void WizSearchView::focus()
{
    m_editSearch->selectAll();
    m_editSearch->setFocus();
}

QSize WizSearchView::sizeHint() const
{
    return QSize(m_widthHint, height());
}

void WizSearchView::setWidthHint(int nWidth)
{
    m_widthHint = nWidth;
}

void WizSearchView::on_search_returnPressed()
{
    Q_EMIT doSearch(m_editSearch->text());
}

void WizSearchView::on_searchTextChanged(QString str)
{
//    if (str.isEmpty())
//    {
//        Q_EMIT doSearch("");
    //    }
}

#define SEARCH_ICON_SIZE  QSize(WizSmartScaleUI(20), WizSmartScaleUI(16))
#define DELETE_ICON_SIZE  QSize(WizSmartScaleUI(16), WizSmartScaleUI(16))
static const WizIconOptions ICON_OPTIONS(WIZ_TINT_COLOR, WizColorButtonIcon, WIZ_TINT_COLOR);


WizSearchEdit::WizSearchEdit(QWidget* parent)
    : QLineEdit(parent)
    , m_menu(new QMenu(this))
{    
    m_searchIcon = ::WizLoadSkinIcon(Utils::WizStyleHelper::themeName(), "mactoolbarsearch", SEARCH_ICON_SIZE, ICON_OPTIONS);
    m_deleteIcon = ::WizLoadSkinIcon(Utils::WizStyleHelper::themeName(), "mactoolbardelete", DELETE_ICON_SIZE, ICON_OPTIONS);

    m_menu->addAction(tr("Advanced search"), this, SLOT(on_actionAdvancedSearch()));
    m_menu->addAction(tr("Add custom search"), this, SLOT(on_addCustomSearch()));
}

void WizSearchEdit::on_actionAdvancedSearch()
{
    WizGetAnalyzer().logAction("advancedSearchOnSearchWidget");
    emit advanceSearchRequest();
}

void WizSearchEdit::on_addCustomSearch()
{
    WizGetAnalyzer().logAction("customSearchOnSearchWidget");
    emit addCustomSearchRequest();
}

void WizSearchEdit::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);

    QPainter pt(this);
    QSize szIcon = SEARCH_ICON_SIZE;
    QRect rcIcon(QPoint(WizSmartScaleUI(4), (height() - szIcon.height()) / 2 + 1),
                 szIcon);
    m_searchIcon.paint(&pt, rcIcon);

    if (!text().isEmpty())
    {
        szIcon = DELETE_ICON_SIZE;
        rcIcon = QRect(QPoint(width() - szIcon.width() - WizSmartScaleUI(4), (height() - szIcon.height()) / 2),
                       szIcon);
        m_deleteIcon.paint(&pt, rcIcon);
    }
}

void WizSearchEdit::mousePressEvent(QMouseEvent* event)
{
    QRect rcBtnSearch(geometry().topLeft(), SEARCH_ICON_SIZE);
    if (rcBtnSearch.contains(event->pos()))
    {
        event->accept();
        m_menu->popup(mapToGlobal(event->pos()));
        clearFocus();
        return;
    }

    if (!text().isEmpty())
    {
        QRect rect(QPoint(width() - DELETE_ICON_SIZE.width() - WizSmartScaleUI(4), (height() - DELETE_ICON_SIZE.height()) / 2), DELETE_ICON_SIZE);
        if (rect.contains(event->pos()))
        {
            setText("");
            //send returnPress signal to reset search status
            emit returnPressed();
        }
    }
    QLineEdit::mousePressEvent(event);
}

void WizSearchEdit::mouseMoveEvent(QMouseEvent* event)
{
    QLineEdit::mouseMoveEvent(event);

    QRect rcBtnSearch(geometry().topLeft(), SEARCH_ICON_SIZE);
    if (rcBtnSearch.contains(event->pos()))
    {
        if (cursor().shape() != Qt::ArrowCursor)
        {
            setCursor(QCursor(Qt::ArrowCursor));
        }
    }
    else
    {
        if (!text().isEmpty())
        {
            QRect rect(QPoint(width() - DELETE_ICON_SIZE.width() - WizSmartScaleUI(4), (height() - DELETE_ICON_SIZE.height()) / 2), DELETE_ICON_SIZE);
            if (rect.contains(event->pos()))
            {
                if (cursor().shape() != Qt::ArrowCursor)
                {
                    setCursor(QCursor(Qt::ArrowCursor));
                }
                return;
            }
        }

        if (cursor().shape() != Qt::IBeamCursor)
        {
            setCursor(QCursor(Qt::IBeamCursor));
        }
    }
}

#endif
