#ifndef WIZSHADOWEFFECT_H
#define WIZSHADOWEFFECT_H

#include <QGraphicsEffect>
#include <QWidget>

class WizSkin9GridImage;

class WizShadowEffect : public QGraphicsEffect
{
public:
    WizShadowEffect();
private:
    int m_shadowSize;
    WizSkin9GridImage* m_shadow;
protected:
    virtual void draw(QPainter *painter);
public:
    virtual QRectF boundingRectFor(const QRectF &rect) const;
};


class WizShadowWidget : public QWidget
{
public:
    WizShadowWidget(QWidget* parent);
private:
    WizSkin9GridImage* m_shadow;
protected:
    virtual void paintEvent(QPaintEvent *);
};

#endif // WIZSHADOWEFFECT_H
