#include "wizButton.h"

#include <QtGui>

#include "share/wizmisc.h"
#include "share/wizsettings.h"


CWizButton::CWizButton(CWizExplorerApp& app, QWidget* parent /* = 0 */)
    : QToolButton(parent)
    , m_app(app)
{
}

QSize CWizButton::sizeHint() const
{
    return iconSize() + QSize(4, 4);
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
