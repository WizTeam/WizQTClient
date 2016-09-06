#include "WizCellButton.h"

#include <QString>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QSettings>
#include <QSize>
#include <QDebug>
#include <QFontMetrics>
#include <QPropertyAnimation>

#include "utils/WizStyleHelper.h"
#include "share/WizMisc.h"


WizCellButton::WizCellButton(ButtonType type, QWidget *parent)
    : QToolButton(parent)
    , m_state(0)
    , m_count(0)
    , m_buttonType(type)
    , m_iconSize(14, 14)
{    
}

void WizCellButton::setNormalIcon(const QIcon& icon, const QString& strTips)
{
    m_iconNomal = icon;
    m_strTipsNormal = strTips;

    setToolTip(strTips);
}

void WizCellButton::setCheckedIcon(const QIcon& icon, const QString& strTips)
{
    m_iconChecked = icon;
    m_strTipsChecked = strTips;

    setToolTip(strTips);
}

void WizCellButton::setBadgeIcon(const QIcon& icon, const QString& strTips)
{
    m_iconBadge = icon;
    m_strTipsBagde = strTips;

    setToolTip(strTips);
}

void WizCellButton::setState(int state)
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

void WizCellButton::setCount(int count)
{
    m_count = count;
    update();
}

const int nTextWidth = 14;
void WizCellButton::paintEvent(QPaintEvent *event)
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

QSize WizCellButton::sizeHint() const
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

QString WizCellButton::countInfo() const
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


WizRoundCellButton::WizRoundCellButton(QWidget* parent)
    : WizCellButton(ImageOnly, parent)
{
    m_iconSize = QSize(iconWidth(), RoundCellButtonConst::iconHeight);

    setMaximumWidth(0);
    m_animation = new QPropertyAnimation(this, "maximumWidth", this);
}

void WizRoundCellButton::setNormalIcon(const QIcon& icon, const QString& text, const QString& strTips)
{
    WizCellButton::setNormalIcon(icon, strTips);
    m_textNormal = text;
}

void WizRoundCellButton::setCheckedIcon(const QIcon& icon, const QString& text, const QString& strTips)
{
    WizCellButton::setCheckedIcon(icon, strTips);
    m_textChecked = text;
}

void WizRoundCellButton::setBadgeIcon(const QIcon& icon, const QString& text, const QString& strTips)
{
    WizCellButton::setBadgeIcon(icon, strTips);
    m_textBadge = text;
}

QString WizRoundCellButton::text() const
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
    return "";
}

int WizRoundCellButton::iconWidth() const
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

int WizRoundCellButton::buttonWidth() const
{
    QFont f;
    f.setPixelSize(RoundCellButtonConst::fontSize);
    QFontMetrics fm(f);
    int width = RoundCellButtonConst::margin * 2 + RoundCellButtonConst::spacing
            + iconWidth() + fm.width(text());
    return width;
}

void WizRoundCellButton::setState(int state)
{
    WizCellButton::setState(state);

    applyAnimation();
}

void WizRoundCellButton::paintEvent(QPaintEvent* /*event*/)
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

QSize WizRoundCellButton::sizeHint() const
{
    //NTOE: 设置一个最大宽度，实际宽度由animation通过maxWidth进行控制
    int maxWidth = 200;
    return QSize(maxWidth, RoundCellButtonConst::buttonHeight);
}

void WizRoundCellButton::applyAnimation()
{
    m_animation->stop();
    m_animation->setDuration(150);
    m_animation->setStartValue(maximumWidth());
    m_animation->setEndValue(buttonWidth());
    m_animation->setEasingCurve(QEasingCurve::InCubic);

    m_animation->start();
}



