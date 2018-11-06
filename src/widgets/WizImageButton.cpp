#include "WizImageButton.h"
#include "share/WizQtHelper.h"
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QPixmap>

WizImageButton::WizImageButton(QWidget* parent) :
    QPushButton(parent)
{
    m_lockNormalStatus = false;
}

void WizImageButton::setIcon(const QIcon& icon)
{
    m_normalIcon = icon.pixmap(size(), QIcon::Normal, QIcon::Off);
    m_hotIcon = icon.pixmap(size(), QIcon::Active, QIcon::On);
    m_downIcon = icon.pixmap(size(), QIcon::Active, QIcon::Off);

    m_currentIcon = m_normalIcon;
}

void WizImageButton::setIconNormal(const QPixmap& icoFile)
{
    m_normalIcon = QPixmap(icoFile);
}

void WizImageButton::setIconHot(const QPixmap& icoFile)
{
    m_hotIcon = QPixmap(icoFile);
}

void WizImageButton::setIconDown(const QPixmap& icoFile)
{
    m_downIcon = QPixmap(icoFile);
}

void WizImageButton::setLockNormalStatus(bool lock)
{
    m_lockNormalStatus = lock;
}

void WizImageButton::setStatusHot()
{
    m_currentIcon = m_hotIcon;
    update();
}

void WizImageButton::setStatusNormal()
{
    m_currentIcon = m_normalIcon;
    update();
}

void WizImageButton::setStatusDown()
{
    m_currentIcon = m_downIcon;
    update();
}

QSize WizImageButton::sizeHint() const
{
    int size = WizSmartScaleUI(16);
    return QSize(size, size);
}

void WizImageButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QStylePainter p(this);
    QStyleOptionButton opt;
    initStyleOption(&opt);

    p.setRenderHint(QPainter::Antialiasing);
    p.drawPixmap(opt.rect, m_currentIcon);
}

void WizImageButton::mousePressEvent(QMouseEvent* event)
{
    if (!m_lockNormalStatus) {
        m_oldIcon = m_currentIcon;
        setStatusDown();
    }

    QPushButton::mousePressEvent(event);
}

void WizImageButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_lockNormalStatus) {
        m_currentIcon = m_oldIcon;
    }

    QPushButton::mouseReleaseEvent(event);
}
