#ifndef WIZSHADOWEFFECT_H
#define WIZSHADOWEFFECT_H

#include <QGraphicsEffect>

class CWizSkin9GridImage;

class CWizShadowEffect : public QGraphicsEffect
{
public:
    CWizShadowEffect();
private:
    int m_shadowSize;
    CWizSkin9GridImage* m_shadow;
protected:
    virtual void draw(QPainter *painter);
public:
    virtual QRectF boundingRectFor(const QRectF &rect) const;
};

#endif // WIZSHADOWEFFECT_H
