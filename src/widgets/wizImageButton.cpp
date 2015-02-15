#include "wizImageButton.h"
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QPixmap>

wizImageButton::wizImageButton(QWidget* parent) :
    QPushButton(parent)
{
    m_lockNormalStatus = false;
}

void wizImageButton::setIcon(const QIcon& icon)
{
    m_normalIcon = icon.pixmap(size(), QIcon::Normal, QIcon::Off);
    m_hotIcon = icon.pixmap(size(), QIcon::Active, QIcon::On);
    m_downIcon = icon.pixmap(size(), QIcon::Active, QIcon::Off);

    m_currentIcon = m_normalIcon;
}

void wizImageButton::setIconNormal(const QString& icoFile)
{
    m_normalIcon = QPixmap(icoFile);
}

void wizImageButton::setIconHot(const QString& icoFile)
{
    m_hotIcon = QPixmap(icoFile);
}

void wizImageButton::setIconDown(const QString& icoFile)
{
    m_downIcon = QPixmap(icoFile);
}

void wizImageButton::setLockNormalStatus(bool lock)
{
    m_lockNormalStatus = lock;
}

void wizImageButton::setStatusHot()
{
    m_currentIcon = m_hotIcon;
    update();
}

void wizImageButton::setStatusNormal()
{
    m_currentIcon = m_normalIcon;
    update();
}

void wizImageButton::setStatusDown()
{
    m_currentIcon = m_downIcon;
    update();
}

void wizImageButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QStylePainter p(this);
    QStyleOptionButton opt;
    initStyleOption(&opt);

    p.setRenderHint(QPainter::Antialiasing);
    p.drawPixmap(opt.rect, m_currentIcon);
}

void wizImageButton::mousePressEvent(QMouseEvent* event)
{
    if (!m_lockNormalStatus) {
        m_oldIcon = m_currentIcon;
        setStatusDown();
    }

    QPushButton::mousePressEvent(event);
}

void wizImageButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_lockNormalStatus) {
        m_currentIcon = m_oldIcon;
    }

    QPushButton::mouseReleaseEvent(event);
}
