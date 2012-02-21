#ifndef WIZPOPUPWIDGET_H
#define WIZPOPUPWIDGET_H

#include <QWidget>
#include "wizui.h"


class CWizPopupWidget : public QWidget
{
    Q_OBJECT
public:
    CWizPopupWidget(QWidget* parent);
private:
#ifndef Q_OS_MAC
    CWizSkin9GridImage m_backgroundImage;
    QPixmap m_backgroundPixmap;
#endif
public:
    virtual QSize sizeHint() const;
    virtual QRect getClientRect() const;
protected:
#ifndef Q_OS_MAC
    virtual void paintEvent(QPaintEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
#endif
    virtual void resizeEvent(QResizeEvent* event);
public:
    void showAtPoint(const QPoint& pt);
#ifndef Q_OS_MAC
    void closeWidget();
#endif
public slots:
};

#undef Q_OS_WIN

#endif // WIZPOPUPWIDGET_H
