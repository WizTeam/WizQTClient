#include "wizButton.h"

#include <QStyleOptionToolButton>
#include <QPainter>

#include "wizdef.h"
#include "share/wizmisc.h"
#include "share/wizsettings.h"


CWizButton::CWizButton(QWidget* parent)
    : QToolButton(parent)
{
}

QSize CWizButton::sizeHint() const
{
#ifdef Q_OS_LINUX
    return QSize(32, 32);
#else
    return QSize(24, 24);
#endif
}

void CWizButton::setAction(QAction* action)
{
    setDefaultAction(action);
}

void CWizButton::paintEvent(QPaintEvent *event)
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

    opt.icon.paint(&p, opt.rect, Qt::AlignCenter, mode, state);
}
