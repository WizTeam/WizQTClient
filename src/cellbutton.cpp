#include "cellbutton.h"

#include <QString>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QSettings>
#include <QSize>
#include <QDebug>
#include <QFontMetrics>
#include <QPropertyAnimation>

#include <extensionsystem/pluginmanager.h>

#include "utils/stylehelper.h"
#include "share/wizmisc.h"

using namespace Core::Internal;

CellButton::CellButton(ButtonType type, QWidget *parent)
    : QToolButton(parent)
    , m_state(0)
    , m_count(0)
    , m_buttonType(type)
    , m_iconSize(14, 14)
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

const int nTextWidth = 14;
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

    QSize size = m_iconSize;// opt.icon.actualSize(iconSize());
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
        p.setPen(m_count == 0 ? QColor("#A7A7A7") : QColor("#5990EF"));
        p.drawText(rcText,Qt::AlignVCenter | Qt::AlignLeft, countInfo());
    }
}

QSize CellButton::sizeHint() const
{
    switch (m_buttonType)
    {
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


namespace RoundCellButtonConst {
    const int margin = 8;
    const int spacing = 8;
    const int iconHeight = 14;
    const int fontSize = 12;
    const int buttonHeight = 18;
}


RoundCellButton::RoundCellButton(QWidget* parent)
    : CellButton(ImageOnly, parent)
{
    m_iconSize = QSize(iconWidth(), RoundCellButtonConst::iconHeight);

    setMaximumWidth(0);
    m_animation = new QPropertyAnimation(this, "maximumWidth", this);
}

void RoundCellButton::setNormalIcon(const QIcon& icon, const QString& text, const QString& strTips)
{
    CellButton::setNormalIcon(icon, strTips);
    m_textNormal = text;
}

void RoundCellButton::setCheckedIcon(const QIcon& icon, const QString& text, const QString& strTips)
{
    CellButton::setCheckedIcon(icon, strTips);
    m_textChecked = text;
}

void RoundCellButton::setBadgeIcon(const QIcon& icon, const QString& text, const QString& strTips)
{
    CellButton::setBadgeIcon(icon, strTips);
    m_textBadge = text;
}

QString RoundCellButton::text() const
{
    switch (m_state) {
    case Normal:
        return m_textNormal;
        break;
    case Checked:
        return m_textChecked;
        break;
    case Badge:
        return m_textBadge;
        break;
    default:
        Q_ASSERT(0);
        break;
    }
}

int RoundCellButton::iconWidth() const
{
    return 14;

//    switch (m_state) {
//    case Normal:
//        return 12;
//        break;
//    case Checked:
//    case Badge:
//        return 13;
//        break;
//    default:
//        Q_ASSERT(0);
//        break;
//    }
}

int RoundCellButton::buttonWidth() const
{
    QFont f;
    f.setPixelSize(RoundCellButtonConst::fontSize);
    QFontMetrics fm(f);
    int width = RoundCellButtonConst::margin * 2 + RoundCellButtonConst::spacing
            + iconWidth() + fm.width(text());
    return width;
}

void RoundCellButton::setState(int state)
{
    CellButton::setState(state);

    applyAnimation();
}

void RoundCellButton::paintEvent(QPaintEvent* /*event*/)
{
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    QPainter p(this);

    QIcon::Mode mode = opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
    if (mode == QIcon::Normal && (opt.state & QStyle::State_HasFocus || opt.state & QStyle::State_Sunken))
        mode = QIcon::Active;
    QIcon::State state = QIcon::Off;
    if (opt.state & QStyle::State_On)
        state = QIcon::On;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor((mode & QIcon::Active) ? "#D3D3D3" : "#E6E6E6"));
    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawRoundedRect(opt.rect, 8, 10);

    QSize size = m_iconSize;
    int nLeft = RoundCellButtonConst::margin;

    QRect rcIcon(nLeft, (opt.rect.height() - size.height()) / 2, iconWidth(), size.height());
    if (opt.icon.isNull())
    {
        m_iconNomal.paint(&p, rcIcon, Qt::AlignCenter, mode, state);
    }
    else
    {
        opt.icon.paint(&p, rcIcon, Qt::AlignCenter, mode, state);
    }

    QRect rcText(rcIcon.right() + RoundCellButtonConst::spacing, opt.rect.y(),
                 opt.rect.right() - rcIcon.right() - RoundCellButtonConst::spacing, opt.rect.height());
    p.setPen(QColor("#535353"));
    QFont f = p.font();
    f.setPixelSize(RoundCellButtonConst::fontSize);
    p.setFont(f);
    p.drawText(rcText,Qt::AlignVCenter | Qt::AlignLeft, text());
}

QSize RoundCellButton::sizeHint() const
{
    //NTOE: 设置一个最大宽度，实际宽度由animation通过maxWidth进行控制
    int maxWidth = 200;
    return QSize(maxWidth, RoundCellButtonConst::buttonHeight);
}

void RoundCellButton::applyAnimation()
{
    m_animation->stop();
    m_animation->setDuration(150);
    m_animation->setStartValue(maximumWidth());
    m_animation->setEndValue(buttonWidth());
    m_animation->setEasingCurve(QEasingCurve::InCubic);

    m_animation->start();
}
