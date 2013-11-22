#include "wizButton.h"

#include <QStyleOptionToolButton>
#include <QPainter>

#include "wizdef.h"
#include "share/wizmisc.h"
#include "share/wizsettings.h"


CWizButton::CWizButton(CWizExplorerApp& app, QWidget* parent /* = 0 */)
    : QToolButton(parent)
    , m_app(app)
{
}

QSize CWizButton::sizeHint() const
{
    return QSize(24, 24);
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



CWizUtilButton::CWizUtilButton(Position pos, CWizExplorerApp& app, QWidget* parent)
    : QToolButton(parent)
{
    QString strName;
    if (pos == Left) {
        strName = "utility_button_left";
    } else if (pos == Center) {
        strName = "utility_button_center";
    } else if (pos == Right) {
        strName = "utility_button_right";
    } else {
        Q_ASSERT(0);
    }
    m_pos = pos;

    m_backgroundIcon = ::WizLoadSkinIcon(app.userSettings().skin(), strName);
}

void CWizUtilButton::paintEvent(QPaintEvent *event)
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

    m_backgroundIcon.paint(&p, opt.rect, Qt::AlignCenter, mode, state);
    opt.icon.paint(&p, opt.rect, Qt::AlignCenter, mode, state);
}

QSize CWizUtilButton::sizeHint() const
{
    switch (m_pos) {
    case Left:
        return QSize(35, 26);
    case Center:
        return QSize(33, 26);
    case Right:
        return QSize(35, 26);
    }
}
