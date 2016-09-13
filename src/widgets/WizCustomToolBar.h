#ifndef CWIZCUSTOMTOOLBAR_H
#define CWIZCUSTOMTOOLBAR_H

#include <QWidget>
#include <QPoint>

class WizCustomToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit WizCustomToolBar(QWidget* hostWidget, QWidget *parent = 0);

signals:

public slots:

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    QPoint m_mousePoint;
    QWidget* m_hostWidget;
};

#endif // CWIZCUSTOMTOOLBAR_H
