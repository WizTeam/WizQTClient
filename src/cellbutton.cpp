#include "cellbutton.h"

#include <QString>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QSettings>
#include <QDebug>

#include <extensionsystem/pluginmanager.h>

#include "share/wizmisc.h"

using namespace Core::Internal;

CellButton::CellButton(Position pos, QWidget *parent)
    : QToolButton(parent)
    , m_state(0)
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

    QString strTheme = ExtensionSystem::PluginManager::globalSettings()->value("theme", "default").toString();
    m_backgroundIcon = ::WizLoadSkinIcon(strTheme, strName);
}

void CellButton::setNormalIcon(const QIcon& icon, const QString& strTips)
{
    m_iconNomal = icon;
    m_strTipsNormal = strTips;

    setToolTip(strTips);
}

void CellButton::setCheckedIcon(const QIcon& icon, const QString& strTips)
{
    m_iconChecked = icon;
    m_strTipsChecked = strTips;

    setToolTip(strTips);
}

void CellButton::setBadgeIcon(const QIcon& icon, const QString& strTips)
{
    m_iconBadge = icon;
    m_strTipsBagde = strTips;

    setToolTip(strTips);
}

void CellButton::setState(int state)
{
    switch (state) {
    case Normal:
        setIcon(m_iconNomal);
        setToolTip(m_strTipsNormal);
        m_state = 0;
        break;
    case Checked:
        setIcon(m_iconChecked);
        setToolTip(m_strTipsChecked);
        m_state = 1;
        break;
    case Badge:
        setIcon(m_iconBadge);
        setToolTip(m_strTipsBagde);
        m_state = 2;
        break;
    default:
        Q_ASSERT(0);
    }
}

void CellButton::paintEvent(QPaintEvent *event)
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

    m_backgroundIcon.paint(&p, opt.rect, Qt::AlignCenter, QIcon::Normal, state);

    if (opt.icon.isNull()) {
        m_iconNomal.paint(&p, opt.rect, Qt::AlignCenter, mode, state);
    } else {
        opt.icon.paint(&p, opt.rect, Qt::AlignCenter, mode, state);
    }
}

QSize CellButton::sizeHint() const
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
