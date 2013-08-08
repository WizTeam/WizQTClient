#include "wizPopupButton.h"

#include "wizdef.h"
#include "share/wizsettings.h"

CWizPopupButton::CWizPopupButton(CWizExplorerApp& app, QWidget *parent)
    : QToolButton(parent)
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
    if (opt.state & QStyle::State_Sunken) {
        p.setPen("#FFFFFF"); // draw text
        p.fillRect(opt.rect, QColor("#1F62B5"));
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
        p.drawText(rectText, Qt::AlignCenter, opt.text);
        return;
    }
}



CWizViewTypePopupButton::CWizViewTypePopupButton(CWizExplorerApp& app, QWidget* parent)
    : CWizPopupButton(app, parent)
{
    QMenu* menu = new QMenu(this);
    QAction* action = NULL;

    action = new QAction(tr("One line view"), menu);
    action->setData(TypeOneLine);
    action->setCheckable(true);
    menu->addAction(action);

    action->setChecked(true);

    action = new QAction(tr("Two line view"), menu);
    action->setData(TypeTwoLine);
    action->setCheckable(true);
    menu->addAction(action);

    action = new QAction(tr("Thumbnail view"), menu);
    action->setData(TypeTwoLine);
    action->setCheckable(true);
    menu->addAction(action);

    setMenu(menu);

    QString strIconPath = ::WizGetSkinResourcePath(app.userSettings().skin()) + "view.png";
    setIcon(QIcon(strIconPath));
    setToolButtonStyle(Qt::ToolButtonIconOnly);
}

QSize CWizViewTypePopupButton::sizeHint() const
{
    return QSize(32 + 10, fontMetrics().height() + 10);
}



CWizSortingPopupButton::CWizSortingPopupButton(CWizExplorerApp& app, QWidget *parent)
    : CWizPopupButton(app, parent)
{
    // only one action can be checked
    QActionGroup* group = new QActionGroup(this);
    group->setExclusive(true);

    QMenu* menu = new QMenu(this);
    createAction(tr("Sorting by created time"), SortingCreateTime, menu, group);
    createAction(tr("Sorting by updated time"), SortingUpdateTime, menu, group);
    menu->addSeparator();
    createAction(tr("Sorting by title"), SortingTitle, menu, group);
    createAction(tr("Sorting by location"), SortingLocation, menu, group);
    createAction(tr("Sorting by tag"), SortingTag, menu, group);
    createAction(tr("Sorting by size"), SortingSize, menu, group);

    setMenu(menu);

    // FIXME: save user sorting type and restore
    QList<QAction*> listActions = menu->actions();
    for (int i = 0; i < listActions.size(); i++) {
        if (listActions.at(i)->data() == SortingCreateTime) {
            listActions.at(i)->setChecked(true);
            setText(listActions.at(i)->text());
        }
    }

    setToolButtonStyle(Qt::ToolButtonTextOnly);
}

QSize CWizSortingPopupButton::sizeHint () const
{
    return QSize(fontMetrics().width(text()) + 30, fontMetrics().height() + 10);
}

void CWizSortingPopupButton::createAction(const QString& text, SortingType type,
                                           QMenu* menu, QActionGroup* group)
{
    QAction* action = new QAction(text, menu);
    action->setData(type);
    action->setCheckable(true);
    menu->addAction(action);
    group->addAction(action);
    connect(action, SIGNAL(triggered()), SLOT(on_sortingTypeChanged()));
}


void CWizSortingPopupButton::on_sortingTypeChanged()
{
    QAction* action = qobject_cast<QAction*>(sender());

    action->setChecked(true);
    setText(action->text());
}
