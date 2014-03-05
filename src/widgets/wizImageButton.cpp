#include "wizImageButton.h"
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QPixmap>

wizImageButton::wizImageButton(QWidget* parent) :
    QPushButton(parent)
{
    m_lockNormalStatus = false;
}

void wizImageButton::setIconNormal(const QString& icoFile)
{
    m_normalIcon = icoFile;
}

void wizImageButton::setIconHot(const QString& icoFile)
{
    m_hotIcon = icoFile;
}

void wizImageButton::setIconDown(const QString& icoFile)
{
    m_downIcon = icoFile;
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
    QRect arcRect(opt.rect.topLeft(), QSize(opt.rect.height(), opt.rect.height()));
    QPixmap pxIco(m_currentIcon);
    p.drawPixmap(arcRect, pxIco);
}

void wizImageButton::mousePressEvent(QMouseEvent* event)
{
    if (!m_lockNormalStatus) {
        setStatusDown();
    }

    QPushButton::mousePressEvent(event);
}

void wizImageButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_lockNormalStatus) {
        setStatusHot();
    }

    QPushButton::mouseReleaseEvent(event);
}
