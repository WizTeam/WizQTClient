#include "WizButton.h"

#include <QStyleOptionToolButton>
#include <QPainter>

#include "WizDef.h"
#include "share/WizMisc.h"
#include "share/WizSettings.h"


WizButton::WizButton(QWidget* parent)
    : QToolButton(parent)
{
}

QSize WizButton::sizeHint() const
{
    return QSize(WizSmartScaleUI(32), WizSmartScaleUI(32));
}

void WizButton::setAction(QAction* action)
{
    setDefaultAction(action);
}

void WizButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QPainter p(this);

    QIcon::Mode mode = opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
    if (mode == QIcon::Normal && (opt.state & QStyle::State_HasFocus || opt.state & QStyle::State_Sunken))
        mode = QIcon::Active;
    QIcon::State state = QIcon::Off;
    if (opt.state & QStyle::State_On)
        state = QIcon::On;
    //
    QRect rc = opt.rect;
    rc.setSize(iconSize());
    rc.moveCenter(opt.rect.center());

    opt.icon.paint(&p, rc, Qt::AlignCenter, mode, state);
}
