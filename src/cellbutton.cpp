#include "cellbutton.h"

#include <QString>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QSettings>
#include <QSize>
#include <QDebug>
#include <QFontMetrics>

#include <extensionsystem/pluginmanager.h>

#include "share/wizmisc.h"

using namespace Core::Internal;

CellButton::CellButton(ButtonType type, QWidget *parent)
    : QToolButton(parent)
    , m_state(0)
    , m_count(0)
    , m_buttonType(type)
{    
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

void CellButton::setCount(int count)
{
    m_count = count;
    update();
}

const int nTextWidth = 20;
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

    QSize size = iconSize();
    int nLeft = (opt.rect.width() - size.width()) / 2;
    if (WithCountInfo == m_buttonType)
    {
        nLeft = (opt.rect.width() - nTextWidth - size.width()) / 2;
    }

    QRect rcIcon(nLeft, (opt.rect.height() - size.height()) / 2, size.width(), size.height());
    if (opt.icon.isNull())
    {
        m_iconNomal.paint(&p, rcIcon, Qt::AlignCenter, mode, state);
    }
    else
    {
        opt.icon.paint(&p, rcIcon, Qt::AlignCenter, mode, state);
    }

    if (WithCountInfo == m_buttonType)
    {
        QRect rcText(rcIcon.right() + 5, opt.rect.y(), opt.rect.width() - rcIcon.width(), opt.rect.height());
        p.setPen(m_count == 0 ? QColor("#B6B6B6") : QColor("#5990EF"));
        p.drawText(rcText,Qt::AlignVCenter | Qt::AlignLeft, countInfo());
    }
}

QSize CellButton::sizeHint() const
{
    switch (m_buttonType) {
    case ImageOnly:
        return QSize(28, 26);
    case WithCountInfo:
    {        
        return QSize(28 + nTextWidth, 26);
    }
    }
}

QString CellButton::countInfo() const
{
    if (m_count > 99)
        return "99+";
    return QString::number(m_count);
}
