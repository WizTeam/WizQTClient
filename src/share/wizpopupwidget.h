#ifndef WIZPOPUPWIDGET_H
#define WIZPOPUPWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>

#include "wizui.h"

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
    QVector<QPoint> m_pointsPolygon;
    bool m_leftAlign;
    QPoint m_pos;

protected:
#ifndef Q_OS_MAC
    virtual void paintEvent(QPaintEvent* event);
#endif
    virtual void resizeEvent(QResizeEvent* event);

public:
    void setLeftAlign(bool b) { m_leftAlign = b; }
    void showAtPoint(const QPoint& pt);


public Q_SLOTS:
    void onAnimationStateChanged(QAbstractAnimation::State newState, \
                                 QAbstractAnimation::State oldState);

    void onAnimationTimeout();
};

#endif // WIZPOPUPWIDGET_H
