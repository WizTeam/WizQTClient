#ifndef WIZSHADOWEFFECT_H
#define WIZSHADOWEFFECT_H

#include <QGraphicsEffect>
#include <QWidget>

class WizSkin9GridImage;

#if 0
class WizCustomShadowEffect : public QGraphicsEffect
{
    Q_OBJECT
public:
    explicit WizCustomShadowEffect(QObject *parent = 0);

    void draw(QPainter* painter);
    QRectF boundingRectFor(const QRectF& rect) const;

    inline void setDistance(qreal distance) { _distance = distance; updateBoundingRect(); }
    inline qreal distance() const { return _distance; }

    inline void setBlurRadius(qreal blurRadius) { _blurRadius = blurRadius; updateBoundingRect(); }
    inline qreal blurRadius() const { return _blurRadius; }

    inline void setColor(const QColor& color) { _color = color; }
    inline QColor color() const { return _color; }

private:
    qreal  _distance;
    qreal  _blurRadius;
    QColor _color;
};

#endif


class WizShadowWidget : public QWidget
{
public:
    WizShadowWidget(QWidget* parent, int shadowSize);
private:
    WizSkin9GridImage* m_shadow;
protected:
    virtual void paintEvent(QPaintEvent *);
};

#endif // WIZSHADOWEFFECT_H
