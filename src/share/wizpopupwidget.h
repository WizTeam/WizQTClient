#ifndef WIZPOPUPWIDGET_H
#define WIZPOPUPWIDGET_H

#include <QWidget>
#include "wizui.h"

class QTimer;

class CWizPopupWidget : public QWidget
{
    Q_OBJECT
public:
    CWizPopupWidget(QWidget* parent);
public:
    virtual QSize sizeHint() const;
    virtual QRect getClientRect() const;
private:
    QVector<QPoint> m_pointsRegion;
    QVector<QPoint> m_pointsPolygon;
    bool m_leftAlign;
protected:
#ifndef Q_OS_MAC
    virtual void paintEvent(QPaintEvent* event);
#endif
    virtual void resizeEvent(QResizeEvent* event);
public:
    void setLeftAlign(bool b) { m_leftAlign = b; }
    void showAtPoint(const QPoint& pt);
};

#endif // WIZPOPUPWIDGET_H
