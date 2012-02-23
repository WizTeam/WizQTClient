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
private:
#ifndef Q_OS_MAC
    CWizSkin9GridImage m_backgroundImage;
    QPixmap m_backgroundPixmap;
    QTimer* m_timer;
#endif
public:
    virtual QSize sizeHint() const;
    virtual QRect getClientRect() const;
protected:
#ifndef Q_OS_MAC
    virtual void paintEvent(QPaintEvent* event);
#endif
    virtual void resizeEvent(QResizeEvent* event);
public:
    void showAtPoint(const QPoint& pt);
#ifndef Q_OS_MAC
public slots:
    void on_timer_timeOut();
#endif
};

#endif // WIZPOPUPWIDGET_H
