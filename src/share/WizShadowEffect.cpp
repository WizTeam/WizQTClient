#include "WizShadowEffect.h"

#include "WizUI.h"
#include "utils/WizStyleHelper.h"
#include "WizMisc.h"
#include <QPainter>
#include <QPaintEngine>

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
    return int(abs((i * f) * f2));
}

QImage WizMakeShadow(int shadowSize, bool activated)
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





WizShadowWidget::WizShadowWidget(QWidget* parent, int shadowSize)
    : QWidget(parent)
    , m_shadow(new WizSkin9GridImage())
{

#if 0
    ////graphics effect 效率太低，禁用////
    WizCustomShadowEffect *bodyShadow = new WizCustomShadowEffect();
    bodyShadow->setBlurRadius(shadowSize);
    bodyShadow->setDistance(6.0);
    bodyShadow->setColor(QColor(0, 0, 0, 80));
    setGraphicsEffect(bodyShadow);
#endif

    QImage image = WizMakeShadow(shadowSize, true);
    m_shadow->setImage(image, QPoint(shadowSize + 1, shadowSize + 1));
}

void WizShadowWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    m_shadow->drawBorder(&painter, rect());
}
