#include "WizCustomToolBar.h"
#include <QMouseEvent>

WizCustomToolBar::WizCustomToolBar(QWidget* hostWidget, QWidget *parent)
    : QWidget(parent)
    , m_hostWidget(hostWidget)
{
    setFixedHeight(68);
}

void WizCustomToolBar::mousePressEvent(QMouseEvent *event)
{
    m_mousePoint = event->globalPos();
}

void WizCustomToolBar::mouseMoveEvent(QMouseEvent *event)
{
    if(m_mousePoint != QPoint(0, 0))
    {
        m_hostWidget->move(m_hostWidget->geometry().x() + event->globalPos().x() -
                           m_mousePoint.x(), m_hostWidget->geometry().y() + event->globalPos().y() - m_mousePoint.y());
        m_mousePoint = event->globalPos();
    }
}

void WizCustomToolBar::mouseReleaseEvent(QMouseEvent *)
{
    m_mousePoint = QPoint(0, 0);
}

