#include "WizShadowEffect.h"

#include "WizUI.h"
#include "utils/WizStyleHelper.h"
#include "WizMisc.h"
#include <QPainter>
#include <QPaintEngine>



WizShadowEffect::WizShadowEffect()
    : m_shadowSize(10)
    , m_shadow(new WizSkin9GridImage())
{
    QString strShadow = Utils::WizStyleHelper::skinResourceFileName("shadow");
    m_shadow->setImage(strShadow, QPoint(m_shadowSize + 5, m_shadowSize + 5));
}

void WizShadowEffect::draw(QPainter *painter)
{
    PixmapPadMode mode = PadToEffectiveBoundingRect;
    if (painter->paintEngine()->type() == QPaintEngine::OpenGL2)
        mode = NoPad;

    // Draw pixmap in device coordinates to avoid pixmap scaling.
    QPoint offset(0, 0);
    const QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset, mode);
    if (pixmap.isNull())
        return;
    //
    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    painter->setBrush(QBrush(Qt::NoBrush));
    QSize sz = pixmap.size();
    painter->drawPixmap(offset.x(), offset.y(), pixmap);
    //
    QRect rcBound(offset.x() - 1, offset.y() - 1, sz.width() + 1, sz.height() + 1);
    m_shadow->drawBorder(painter, rcBound);
    //
    painter->setWorldTransform(restoreTransform);

}

QRectF WizShadowEffect::boundingRectFor(const QRectF &rect) const
{
    QRectF rc = rect;
    rc.adjust(-m_shadowSize, -m_shadowSize, m_shadowSize + 1, m_shadowSize + 1);
    return rc;
}


WizShadowWidget::WizShadowWidget(QWidget* parent)
    : QWidget(parent)
    , m_shadow(new WizSkin9GridImage())
{
    QString strShadow = Utils::WizStyleHelper::skinResourceFileName("shadow");
    m_shadow->setImage(strShadow, QPoint(12, 12));
}

void WizShadowWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    m_shadow->drawBorder(&painter, rect());
}
