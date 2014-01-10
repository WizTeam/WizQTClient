#include "wizPopupButton.h"

#include <QMenu>

#include "wizdef.h"
#include "share/wizsettings.h"
#include "wizDocumentListView.h"

CWizPopupButton::CWizPopupButton(CWizExplorerApp& app, QWidget *parent)
    : QToolButton(parent)
    , m_app(app)
{
    setPopupMode(QToolButton::InstantPopup);

    QString strIconPath = ::WizGetSkinResourcePath(app.userSettings().skin()) + "arrow.png";
    m_iconArraw.addFile(strIconPath);
}

void CWizPopupButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    // FIXME
    int nArrawWidth = 6;
    int nMargin = 8;

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QPainter p(this);
    p.setClipRect(opt.rect);

    // FIXME
    p.setPen("#787878");
    if (opt.state & QStyle::State_Sunken) {
        p.fillRect(opt.rect, QColor("#dadada"));
    }

    // draw arraw
    if (!m_iconArraw.isNull()) {
        QRect rectArrow = opt.rect;
        rectArrow.setLeft(rectArrow.right() - nArrawWidth - nMargin);
        m_iconArraw.paint(&p, rectArrow, Qt::AlignVCenter, QIcon::Normal);
    }

    if (!opt.icon.isNull()) {
        QRect rectIcon = opt.rect;
        rectIcon.setLeft(rectIcon.left() + nMargin);
        rectIcon.setRight(rectIcon.right() - nArrawWidth - nMargin * 2);

        if (opt.state & QStyle::State_Active) {
            opt.icon.paint(&p, rectIcon, Qt::AlignCenter, QIcon::Active);
        } else {
            opt.icon.paint(&p, rectIcon, Qt::AlignCenter, QIcon::Normal);
        }

        return;
    }

    if (!opt.text.isEmpty()) {
        QRect rectText = opt.rect;
        rectText.setLeft(rectText.left() + nMargin);
        rectText.setRight(rectText.right() - nArrawWidth - nMargin * 2);
        QString str = fontMetrics().elidedText(opt.text, Qt::ElideRight, rectText.width());
        p.drawText(rectText, Qt::AlignCenter, str);
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
    menu->setStyleSheet("\
                        QMenu::item { padding: 2px 20px 2px 20px; spacing: 20px;}\
                        QMenu::item:selected { border-color: darkblue; background: #0073c5; color:white;}\
                        ");
    createAction(tr("Thumbnail view"), CWizDocumentListView::TypeThumbnail, menu, group);
    createAction(tr("Two line view"), CWizDocumentListView::TypeTwoLine, menu, group);
    createAction(tr("One line view"), CWizDocumentListView::TypeOneLine, menu, group);
    setMenu(menu);

    QString strSkinPath = ::WizGetSkinResourcePath(app.userSettings().skin());
    m_iconOneLine.addFile(strSkinPath + "view_one_line.png");
    m_iconTwoLine.addFile(strSkinPath + "view_two_line.png");
    m_iconThumbnail.addFile(strSkinPath + "view_thumbnail.png");

    int type = m_app.userSettings().get("VIEW_TYPE").toInt();
    setActionChecked(menu, type);
    setActionIcon(type);
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
    setActionIcon(type);

    m_app.userSettings().set("VIEW_TYPE", QString::number(type));

    Q_EMIT viewTypeChanged(type);
}

void CWizViewTypePopupButton::setActionIcon(int type)
{
    switch (type) {
    case CWizDocumentListView::TypeOneLine:
        setIcon(m_iconOneLine);
        break;
    case CWizDocumentListView::TypeTwoLine:
        setIcon(m_iconTwoLine);
        break;
    case CWizDocumentListView::TypeThumbnail:
        setIcon(m_iconThumbnail);
        break;
    default:
        Q_ASSERT(0);
    }
}


/* ------------------------ CWizSortingPopupButton ------------------------ */
CWizSortingPopupButton::CWizSortingPopupButton(CWizExplorerApp& app, QWidget *parent)
    : CWizPopupButton(app, parent)
{
    setToolButtonStyle(Qt::ToolButtonTextOnly);

    // only one action can be checked
    QActionGroup* group = new QActionGroup(this);
    group->setExclusive(true);

    QMenu* menu = new QMenu(this);
    menu->setStyleSheet("\
                        QMenu::item { padding: 2px 20px 2px 20px; spacing: 20px;}\
                        QMenu::item:selected { border-color: darkblue; background: #0073c5; color:white;}\
                        ");

    createAction(tr("Sorting by created time"), SortingCreateTime, menu, group);
    createAction(tr("Sorting by updated time"), SortingUpdateTime, menu, group);
    menu->addSeparator();
    createAction(tr("Sorting by title"), SortingTitle, menu, group);
    createAction(tr("Sorting by location"), SortingLocation, menu, group);
    createAction(tr("Sorting by tag"), SortingTag, menu, group);
    createAction(tr("Sorting by size"), SortingSize, menu, group);

    setMenu(menu);

    int type = m_app.userSettings().get("SORT_TYPE").toInt();
    setActionChecked(menu, type);
}

QSize CWizSortingPopupButton::sizeHint () const
{
    return QSize(fontMetrics().width(text()) + 30, fontMetrics().height() + 10);
}

void CWizSortingPopupButton::on_action_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());

    action->setChecked(true);
    setText(action->text());

    int type = action->data().toInt();
    m_app.userSettings().set("SORT_TYPE", QString::number(type));

    Q_EMIT sortingTypeChanged(type);
}
