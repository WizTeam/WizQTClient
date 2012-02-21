#ifndef WIZPOPUPWIDGET_H
#define WIZPOPUPWIDGET_H

#include <QWidget>
#include "wizuihelper.h"

class CWizPopupWidget : public QWidget
{
    Q_OBJECT;
public:
    CWizPopupWidget(QWidget* parent);
private:
    CWizSkin9GridImage m_backgroundImage;
    QPixmap m_backgroundPixmap;
public:
    virtual QSize sizeHint() const;
    virtual QRect getClientRect() const;
protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void keyPressEvent(QKeyEvent *);
public:
    void showAtPoint(const QPoint& pt);

public slots:
    void on_application_focusChanged(QWidget* old, QWidget* now) ;
};

#endif // WIZPOPUPWIDGET_H
