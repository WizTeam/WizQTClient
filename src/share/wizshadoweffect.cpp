#include "wizshadoweffect.h"

#include "wizui.h"
#include "utils/stylehelper.h"
#include "wizmisc.h"
#include <QPainter>
#include <QPaintEngine>



CWizShadowEffect::CWizShadowEffect()
    : m_shadowSize(10)
    , m_shadow(new CWizSkin9GridImage())
{
    QString strShadow = Utils::StyleHelper::skinResourceFileName("shadow");
    m_shadow->SetImage(strShadow, QPoint(m_shadowSize + 5, m_shadowSize + 5));
}

void CWizShadowEffect::draw(QPainter *painter)
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
    m_shadow->DrawBorder(painter, rcBound);
    //
    painter->setWorldTransform(restoreTransform);

}

QRectF CWizShadowEffect::boundingRectFor(const QRectF &rect) const
{
    QRectF rc = rect;
    rc.adjust(-m_shadowSize, -m_shadowSize, m_shadowSize + 1, m_shadowSize + 1);
    return rc;
}


CWizShadowWidget::CWizShadowWidget(QWidget* parent)
    : QWidget(parent)
    , m_shadow(new CWizSkin9GridImage())
{
    QString strShadow = Utils::StyleHelper::skinResourceFileName("shadow");
    m_shadow->SetImage(strShadow, QPoint(12, 12));
}

void CWizShadowWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    m_shadow->DrawBorder(&painter, rect());
}
