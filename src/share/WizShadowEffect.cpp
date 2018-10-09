#include "WizShadowEffect.h"

#include "WizUI.h"
#include "utils/WizStyleHelper.h"
#include "WizMisc.h"
#include <QPainter>
#include <QPaintEngine>
#include <QMouseEvent>

#include <math.h>

#if 0
WizCustomShadowEffect::WizCustomShadowEffect(QObject *parent) :
    QGraphicsEffect(parent),
    _distance(4.0f),
    _blurRadius(10.0f),
    _color(0, 0, 0, 80)
{
}

QT_BEGIN_NAMESPACE
  extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
QT_END_NAMESPACE

void WizCustomShadowEffect::draw(QPainter* painter)
{
    // if nothing to show outside the item, just draw source
    if ((blurRadius() + distance()) <= 0) {
        drawSource(painter);
        return;
    }

    PixmapPadMode mode = QGraphicsEffect::PadToEffectiveBoundingRect;
    QPoint offset;
    const QPixmap px = sourcePixmap(Qt::DeviceCoordinates, &offset, mode);

    // return if no source
    if (px.isNull())
        return;

    // save world transform
    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());

    // Calculate size for the background image
    QSize szi(px.size().width() + 2 * distance(), px.size().height() + 2 * distance());

    QImage tmp(szi, QImage::Format_ARGB32_Premultiplied);
    QPixmap scaled = px.scaled(szi);
    tmp.fill(0);
    QPainter tmpPainter(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
    tmpPainter.drawPixmap(QPointF(-distance(), -distance()), scaled);
    tmpPainter.end();

    // blur the alpha channel
    QImage blurred(tmp.size(), QImage::Format_ARGB32_Premultiplied);
    blurred.fill(0);
    QPainter blurPainter(&blurred);
    qt_blurImage(&blurPainter, tmp, blurRadius(), false, true);
    blurPainter.end();

    tmp = blurred;

    // blacken the image...
    tmpPainter.begin(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tmpPainter.fillRect(tmp.rect(), color());
    tmpPainter.end();

    // draw the blurred shadow...
    painter->drawImage(offset, tmp);

    // draw the actual pixmap...
    painter->drawPixmap(offset, px, QRectF());

    // restore world transform
    painter->setWorldTransform(restoreTransform);
}

QRectF WizCustomShadowEffect::boundingRectFor(const QRectF& rect) const
{
    qreal delta = blurRadius() + distance();
    return rect.united(rect.adjusted(-delta, -delta, delta, delta));
}

#endif


inline unsigned char WizMakeAlpha(int i, double f, int nSize)
{
    if (i == nSize)
        f *= 1.2;
    //

    double f2 = 1 - cos((double)i / nSize * 3.14 / 2);
    //
    return int(fabs((i * f) * f2));
}

QImage WizMakeShadowImage(int shadowSize, bool activated)
{
    int size = shadowSize * 2 + 10;
    QImage image(size, size, QImage::Format_ARGB32);
    image.fill(QColor(Qt::black));
    //
    double f = activated ? 1.6 : 1.0;
    //
    QSize szImage = image.size();
    //
    //left
    for (int y = shadowSize; y < szImage.height() - shadowSize; y++)
    {
        for (int x = 0; x < shadowSize; x++)
        {
            int i = x + 1;
            int alpha = WizMakeAlpha(i, f, shadowSize);
            image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
        }
    }
    //right
    for (int y = shadowSize; y < szImage.height() - shadowSize; y++)
    {
        for (int x = szImage.width() - shadowSize - 1; x < szImage.width(); x++)
        {
            int i = szImage.width() - x;
            int alpha = WizMakeAlpha(i, f, shadowSize);
            image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
        }
    }
    //top
    for (int y = 0; y < shadowSize; y++)
    {
        int i = y + 1;
        for (int x = shadowSize; x < szImage.width() - shadowSize; x++)
        {
            int alpha = WizMakeAlpha(i, f, shadowSize);
            image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
        }
        //
    }
    //bottom
    for (int y = szImage.height() - shadowSize - 1; y < szImage.height(); y++)
    {
        int i = szImage.height() - y;
        for (int x = shadowSize; x < szImage.width() - shadowSize; x++)
        {
            int alpha = WizMakeAlpha(i, f, shadowSize);
            image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
        }
    }
    //
    int parentRoundSize = 3;
    //
    for (int x = 0; x < shadowSize + parentRoundSize; x++)
    {
        for (int y = 0; y < shadowSize + parentRoundSize; y++)
        {
            int xx = (shadowSize + parentRoundSize) - x;
            int yy = (shadowSize + parentRoundSize) - y;
            int i = int(sqrt(double(xx * xx + yy * yy)));
            i = std::min<int>(shadowSize + parentRoundSize, i);
            i -= parentRoundSize;
            i = shadowSize - i;
            //
            int alpha = WizMakeAlpha(i, f, shadowSize);
            image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
        }
    }
    //
    for (int x = szImage.width() - shadowSize - parentRoundSize; x < szImage.width(); x++)
    {
        for (int y = 0; y < shadowSize + parentRoundSize; y++)
        {
            int xx = (shadowSize + parentRoundSize) - (szImage.width() - x);
            int yy = (shadowSize + parentRoundSize) - y;
            int i = int(sqrt(double(xx * xx + yy * yy)));
            i = std::min<int>(shadowSize + parentRoundSize, i);
            i -= parentRoundSize;
            i = shadowSize - i;
            //
            int alpha = WizMakeAlpha(i, f, shadowSize);
            image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
        }
    }
    //
    for (int x = 0; x < shadowSize + parentRoundSize; x++)
    {
        for (int y = szImage.height() - shadowSize - parentRoundSize; y < szImage.height(); y++)
        {
            int xx = (shadowSize + parentRoundSize) - x;
            int yy = (shadowSize + parentRoundSize) - (szImage.height() - y);
            int i = int(sqrt(double(xx * xx + yy * yy)));
            i = std::min<int>(shadowSize + parentRoundSize, i);
            i -= parentRoundSize;
            i = shadowSize - i;
            //
            int alpha = WizMakeAlpha(i, f, shadowSize);
            image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
        }
    }
    //
    for (int x = szImage.width() - shadowSize - parentRoundSize; x < szImage.width(); x++)
    {
        for (int y = szImage.height() - shadowSize - parentRoundSize; y < szImage.height(); y++)
        {
            int xx = (shadowSize + parentRoundSize) - (szImage.width() - x);
            int yy = (shadowSize + parentRoundSize) - (szImage.height() - y);
            int i = int(sqrt(double(xx * xx + yy * yy)));
            i = std::min<int>(shadowSize + parentRoundSize, i);
            i -= parentRoundSize;
            i = shadowSize - i;
            //
            int alpha = WizMakeAlpha(i, f, shadowSize);
            image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
        }
    }
    //
    int borderR = 165;
    int borderG = 165;
    int borderB = 165;
    //
    if (activated)
    {
        borderR = 68;
        borderG = 138;
        borderB = 255;
    }
    //
    int borderSize = ::WizSmartScaleUI(1);
    //left
    for (int i = 0; i < borderSize; i++)
    {
        for (int y = shadowSize - 1; y < szImage.height() - shadowSize + 1; y++)
        {
            int x = shadowSize - i - 1;
            image.setPixelColor(x, y, QColor(borderR, borderG, borderB, 255));
        }
    }
    //right
    for (int i = 0; i < borderSize; i++)
    {
        for (int y = shadowSize - 1; y < szImage.height() - shadowSize + 1; y++)
        {
            int x = szImage.width() - shadowSize - 1 + i + 1;
            image.setPixelColor(x, y, QColor(borderR, borderG, borderB, 255));
        }
    }
    //top
    for (int i = 0; i < borderSize; i++)
    {
        for (int x = shadowSize; x < szImage.width() - shadowSize; x++)
        {
            int y = shadowSize - i - 1;
            image.setPixelColor(x, y, QColor(borderR, borderG, borderB, 255));
        }
    }
    //bottom
    for (int i = 0; i < borderSize; i++)
    {
        for (int x = shadowSize; x < szImage.width() - shadowSize; x++)
        {
            int y = szImage.height() - shadowSize - 1 + i + 1;
            image.setPixelColor(x, y, QColor(borderR, borderG, borderB, 255));
        }
    }
    //
    return image;
}





WizShadowWidget::WizShadowWidget(QWidget* parent, int shadowSize, bool canResize)
    : QWidget(parent)
    , m_shadow(new WizSkin9GridImage())
    , m_shadowSize(shadowSize)
    , m_canResize(canResize)
    , m_oldHitCode(wizClient)
    , m_mousePressed(false)
{

#if 0
    ////graphics effect 效率太低，禁用////
    WizCustomShadowEffect *bodyShadow = new WizCustomShadowEffect();
    bodyShadow->setBlurRadius(shadowSize);
    bodyShadow->setDistance(6.0);
    bodyShadow->setColor(QColor(0, 0, 0, 80));
    setGraphicsEffect(bodyShadow);
#endif

    if (m_canResize)
    {
        setMouseTracking(true);
    }
    //
    QImage image = WizMakeShadowImage(shadowSize, true);
    m_shadow->setImage(image, QPoint(shadowSize + 1, shadowSize + 1), "#666666");
}

void WizShadowWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    m_shadow->drawBorder(&painter, rect());
}


WizWindowHitTestResult WizShadowWidget::hitTest(const QPoint& posOfWindow)
{
    if (!m_canResize)
        return wizClient;
    //
    QPoint pos = posOfWindow;
    if (pos.x() < m_shadowSize)
    {
        if (pos.y() < m_shadowSize)
        {
            return wizTopLeft;
        }
        else if (pos.y() >= height() - m_shadowSize)
        {
            return wizBottomLeft;
        }
        else
        {
            return wizLeft;
        }
    }
    else if (pos.x() > width() - m_shadowSize)
    {
        if (pos.y() < m_shadowSize)
        {
            return wizTopRight;
        }
        else if (pos.y() >= height() - m_shadowSize)
        {
            return wizBottomRight;
        }
        else
        {
            return wizRight;
        }
    }
    else if (pos.y() < m_shadowSize)
    {
        return wizTop;
    }
    else if (pos.y() > height() - m_shadowSize)
    {
        return wizBottom;
    }
    else
    {
        return wizClient;
    }
}

void WizShadowWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mousePressed)
    {
        if (m_oldHitCode == wizClient)
            return;
        //
        QPoint pos = event->globalPos();
        int offsetX = pos.x() - m_oldPressPos.x();
        int offsetY = pos.y() - m_oldPressPos.y();
        //
        QRect rc = m_oldGeometry;
        //
        switch (m_oldHitCode)
        {
        case wizTopLeft:
            rc.adjust(offsetX, offsetY, 0, 0);
            break;
        case wizTop:
            rc.adjust(0, offsetY, 0, 0);
            break;
        case wizTopRight:
            rc.adjust(0, offsetY, offsetX, 0);
            break;
        case wizLeft:
            rc.adjust(offsetX, 0, 0, 0);
            break;
        case wizRight:
            rc.adjust(0, 0, offsetX, 0);
            break;
        case wizBottomLeft:
            rc.adjust(offsetX, 0, 0, offsetY);
            break;
        case wizBottom:
            rc.adjust(0, 0, 0, offsetY);
            break;
        case wizBottomRight:
            rc.adjust(0, 0, offsetX, offsetY);
            break;
        default:
            Q_ASSERT(false);
            break;
        }
        //
        parentWidget()->setGeometry(rc);
    }
    else
    {
        QPoint pos = event->pos();
        WizWindowHitTestResult hit = hitTest(pos);
        if (hit != wizClient)
        {
            event->accept();
        }
        //
        switch (hit)
        {
        case wizTopLeft:
            setCursor(QCursor(Qt::SizeFDiagCursor));
            break;
        case wizTop:
            setCursor(QCursor(Qt::SizeVerCursor));
            break;
        case wizTopRight:
            setCursor(QCursor(Qt::SizeBDiagCursor));
            break;
        case wizLeft:
            setCursor(QCursor(Qt::SizeHorCursor));
            break;
        case wizClient:
            setCursor(QCursor(Qt::ArrowCursor));
            break;
        case wizRight:
            setCursor(QCursor(Qt::SizeHorCursor));
            break;
        case wizBottomLeft:
            setCursor(QCursor(Qt::SizeBDiagCursor));
            break;
        case wizBottom:
            setCursor(QCursor(Qt::SizeVerCursor));
            break;
        case wizBottomRight:
            setCursor(QCursor(Qt::SizeFDiagCursor));
            break;
        }
    }
}

void WizShadowWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    WizWindowHitTestResult hit = hitTest(pos);
    //
    m_oldHitCode = hit;
    m_oldPressPos = event->globalPos();
    m_mousePressed = true;
    m_oldGeometry = parentWidget()->geometry();
    //
    QWidget::mousePressEvent(event);
}

void WizShadowWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_mousePressed = false;
    //
    QWidget::mouseReleaseEvent(event);
}
