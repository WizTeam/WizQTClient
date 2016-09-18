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


enum WizWindowHitTestResult {wizTopLeft, wizTop, wizTopRight, wizLeft, wizClient, wizRight, wizBottomLeft, wizBottom, wizBottomRight};

class WizShadowWidget : public QWidget
{
public:
    WizShadowWidget(QWidget* parent, int shadowSize, bool canResize);
private:
    WizSkin9GridImage* m_shadow;
    int m_shadowSize;
    bool m_canResize;
    //
    WizWindowHitTestResult m_oldHitCode;
    QPoint m_oldPressPos;
    QRect m_oldGeometry;
    bool m_mousePressed;
protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    //
    virtual WizWindowHitTestResult hitTest(const QPoint& posOfWindow);
public:
    bool canResize() const { return m_canResize; }
    void setCanResize(bool b) { m_canResize = b; }
};

#endif // WIZSHADOWEFFECT_H
