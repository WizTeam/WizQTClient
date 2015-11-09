#ifndef WIZPOPUPWIDGET_H
#define WIZPOPUPWIDGET_H

#include <QWidget>
//#include <QPropertyAnimation>

//#include "wizui.h"

class QTimer;

class CWizPopupWidget : public QWidget
{
    Q_OBJECT

public:
    CWizPopupWidget(QWidget* parent);

    virtual QSize sizeHint() const;
    virtual QRect getClientRect() const;

private:
    QVector<QPoint> m_pointsRegion;
//    QVector<QPoint> m_pointsPolygon;
    bool m_leftAlign;
    QPoint m_pos;
    QRegion maskRegion();

protected:
    int m_triangleMargin;
    int m_triangleWidth;
    int m_triangleHeight;


protected:
    virtual void paintEvent(QPaintEvent* event);

public:
    void setLeftAlign(bool b) { m_leftAlign = b; }
    void showAtPoint(const QPoint& pt);
    void setTriangleStyle(int triangleMargin, int triangleWidth, int triangleHeight);
};

#endif // WIZPOPUPWIDGET_H
