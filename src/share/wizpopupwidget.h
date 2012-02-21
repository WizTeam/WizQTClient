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
    CWizSkin9GridImage m_backgroundImage;
#ifdef Q_OS_WIN
    QPixmap m_backgroundPixmap;
#endif
public:
    virtual QSize sizeHint() const;
    virtual QRect getClientRect() const;
protected:
#ifdef Q_OS_WIN
    virtual void paintEvent(QPaintEvent *event);
#endif
    virtual void resizeEvent(QResizeEvent *event);
    virtual void keyPressEvent(QKeyEvent *);
public:
    void showAtPoint(const QPoint& pt);

public slots:
#ifdef Q_OS_WIN
    void on_application_focusChanged(QWidget* old, QWidget* now) ;
#endif
};

#endif // WIZPOPUPWIDGET_H
