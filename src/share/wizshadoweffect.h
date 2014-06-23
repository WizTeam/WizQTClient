#ifndef WIZSHADOWEFFECT_H
#define WIZSHADOWEFFECT_H

#include <QGraphicsEffect>
#include <QWidget>

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


class CWizShadowWidget : public QWidget
{
public:
    CWizShadowWidget(QWidget* parent);
private:
    CWizSkin9GridImage* m_shadow;
protected:
    virtual void paintEvent(QPaintEvent *);
};

#endif // WIZSHADOWEFFECT_H
