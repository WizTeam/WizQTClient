#include "wizPopupButton.h"

#include <QMenu>

#include "wizdef.h"
#include "share/wizsettings.h"
#include "wizDocumentListView.h"
#include <QApplication>

CWizPopupButton::CWizPopupButton(CWizExplorerApp& app, QWidget *parent)
    : QToolButton(parent)
    , m_app(app)
    , m_iconSize(23, 14)
{
    setPopupMode(QToolButton::InstantPopup);

//    QString strIconPath = ::WizGetSkinResourcePath(app.userSettings().skin()) + "arrow.png";
//    m_iconArraw.addFile(strIconPath);
}

void CWizPopupButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    // FIXME
    int nMargin = 4;

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QPainter p(this);
    p.setClipRect(opt.rect);

    // FIXME
    p.setPen("#787878");
    if (opt.state & QStyle::State_Sunken) {
        p.fillRect(opt.rect, QColor("#dadada"));
    }    

    if (!opt.icon.isNull()) {
        QRect rectIcon(opt.rect.x() + (opt.rect.width() - m_iconSize.width()) / 2,
                       opt.rect.y() + (opt.rect.height() - m_iconSize.height()) / 2, m_iconSize.width(), m_iconSize.height());

        if (opt.state & QStyle::State_Sunken) {
            opt.icon.paint(&p, rectIcon, Qt::AlignCenter, QIcon::Active);
        } else {
            opt.icon.paint(&p, rectIcon, Qt::AlignCenter, QIcon::Normal);
        }

        return;
    }

    if (!opt.text.isEmpty()) {
        QRect rectText = opt.rect;
        rectText.setLeft(rectText.left() + nMargin);
        rectText.setRight(rectText.right() - nMargin * 2);
        QString str = fontMetrics().elidedText(opt.text, Qt::ElideRight, rectText.width());
#ifdef Q_OS_MAC
        QFont font;
        font.setPixelSize(fontMetrics().height());
        p.setFont(font);
#endif
        p.drawText(rectText, Qt::AlignVCenter | Qt::AlignLeft, str);
        return;
    }
}

void CWizPopupButton::createAction(const QString& text, int type,
                                   QMenu* menu, QActionGroup* group)
{
    QAction* action = new QAction(text, menu);
    action->setData(type);
    action->setCheckable(true);
    menu->addAction(action);
    group->addAction(action);
    connect(action, SIGNAL(triggered()), SLOT(on_action_triggered()));
}

void CWizPopupButton::setActionChecked(const QMenu* menu, int type)
{
    QList<QAction*> listActions = menu->actions();
    for (int i = 0; i < listActions.size(); i++) {
        if (listActions.at(i)->data() == type) {
            listActions.at(i)->setChecked(true);
            setText(listActions.at(i)->text());
        }
    }
}


/* ------------------------ CWizViewTypePopupButton ------------------------ */
CWizViewTypePopupButton::CWizViewTypePopupButton(CWizExplorerApp& app, QWidget* parent)
    : CWizPopupButton(app, parent)
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);

    // only one action can be checked
    QActionGroup* group = new QActionGroup(this);
    group->setExclusive(true);

    QMenu* menu = new QMenu(this);
    createAction(tr("Thumbnail view"), CWizDocumentListView::TypeThumbnail, menu, group);
    createAction(tr("Two line view"), CWizDocumentListView::TypeTwoLine, menu, group);
    createAction(tr("One line view"), CWizDocumentListView::TypeOneLine, menu, group);
    setMenu(menu);

//    QString strSkinPath = ::WizGetSkinResourcePath(app.userSettings().skin());
//    m_iconOneLine.addFile(strSkinPath + "view_one_line.png");
//    m_iconTwoLine.addFile(strSkinPath + "view_two_line.png");
//    m_iconThumbnail.addFile(strSkinPath + "view_thumbnail.png");
    QIcon icon = ::WizLoadSkinIcon(app.userSettings().skin(), "documents_view_type");
    setIcon(icon);

    int type = m_app.userSettings().get("VIEW_TYPE").toInt();
    setActionChecked(menu, type);
//    setActionIcon(type);
}

QSize CWizViewTypePopupButton::sizeHint() const
{
    return QSize(32 + 10, fontMetrics().height() + 10);
}

void CWizViewTypePopupButton::on_action_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());

    action->setChecked(true);
    setText(action->text());

    int type = action->data().toInt();
//    setActionIcon(type);

    m_app.userSettings().set("VIEW_TYPE", QString::number(type));

    Q_EMIT viewTypeChanged(type);
}

//void CWizViewTypePopupButton::setActionIcon(int type)
//{
//    switch (type) {
//    case CWizDocumentListView::TypeOneLine:
//        setIcon(m_iconOneLine);
//        break;
//    case CWizDocumentListView::TypeTwoLine:
//        setIcon(m_iconTwoLine);
//        break;
//    case CWizDocumentListView::TypeThumbnail:
//        setIcon(m_iconThumbnail);
//        break;
//    default:
//        Q_ASSERT(0);
//    }
//}

void CWizViewTypePopupButton::on_viewTypeChanged(int type)
{
    setActionChecked(menu(), type);
//    setActionIcon(type);
}


/* ------------------------ CWizSortingPopupButton ------------------------ */
CWizSortingPopupButton::CWizSortingPopupButton(CWizExplorerApp& app, QWidget *parent)
    : CWizPopupButton(app, parent)
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);

    // only one action can be checked
    QActionGroup* group = new QActionGroup(this);
    group->setExclusive(true);

    QMenu* menu = new QMenu(this);

    createAction(tr("Sorting by created time"), SortingByCreatedTime, menu, group);
    createAction(tr("Sorting by updated time"), SortingByModifiedTime, menu, group);
    createAction(tr("Sorting by access time"), SortingByAccessedTime, menu, group);
    menu->addSeparator();
    createAction(tr("Sorting by title"), SortingByTitle, menu, group);
    createAction(tr("Sorting by location"), SortingByLocation, menu, group);
//    createAction(tr("Sorting by tag"), SortingByTag, menu, group);
    createAction(tr("Sorting by size"), SortingBySize, menu, group);

    menu->addSeparator();
    QActionGroup* orderGroup = new QActionGroup(this);
    createAction(tr("AscendingOrder"), SortingAsAscendingOrder, menu, orderGroup);
    createAction(tr("DescendingOrder"), SortingAsDescendingOrder, menu, orderGroup);

    setMenu(menu);

    int type = m_app.userSettings().get("SORT_TYPE").toInt();
    if (type > 0 && type < SortingAsAscendingOrder)
    {
        setActionChecked(menu, SortingAsDescendingOrder);
        setActionChecked(menu, type);
        m_isAscendingOrder = false;
    }
    else if (type < 0 && type > -SortingAsAscendingOrder)
    {
        setActionChecked(menu, SortingAsAscendingOrder);
        setActionChecked(menu, -type);
        m_isAscendingOrder = true;
    }
    else
    {
        setActionChecked(menu, SortingAsDescendingOrder);
        m_isAscendingOrder = false;
        type = SortingByCreatedTime;
        setActionChecked(menu, type);
        m_app.userSettings().set("SORT_TYPE", QString::number(type));
    }

    QIcon icon = ::WizLoadSkinIcon(app.userSettings().skin(), "documents_sort_type");
    setIcon(icon);
}

void CWizSortingPopupButton::on_sortingTypeChanged(int type)
{
    QMenu* myMenu = menu();
    setActionChecked(myMenu, type);
}

QSize CWizSortingPopupButton::sizeHint () const
{
//#ifdef Q_OS_MAC
//    return QSize(fontMetrics().width(text()) + 45, fontMetrics().height() + 16);
//#else
//    return QSize(fontMetrics().width(text()) + 45, fontMetrics().height() + 12);
//#endif

    return QSize(32 + 10, fontMetrics().height() + 10);
}

void CWizSortingPopupButton::on_action_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());

    action->setChecked(true);

    int type = action->data().toInt();
    if (SortingAsAscendingOrder == type || SortingAsDescendingOrder == type)
    {
        m_isAscendingOrder = (SortingAsAscendingOrder == type);
        int curType = m_app.userSettings().get("SORT_TYPE").toInt();
        type = qAbs(curType) * (m_isAscendingOrder ? -1 : 1);
    }
    else
    {
        type = type * (m_isAscendingOrder ? -1 : 1);
//        setText(action->text());
    }
    m_app.userSettings().set("SORT_TYPE", QString::number(type));

    Q_EMIT sortingTypeChanged(type);
}
