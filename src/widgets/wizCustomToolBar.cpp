#include "wizCustomToolBar.h"
#include <QMouseEvent>

CWizCustomToolBar::CWizCustomToolBar(QWidget* hostWidget, QWidget *parent)
    : QWidget(parent)
    , m_hostWidget(hostWidget)
{
    setFixedHeight(68);
}

void CWizCustomToolBar::mousePressEvent(QMouseEvent *event)
{
    m_mousePoint = event->globalPos();
}

void CWizCustomToolBar::mouseMoveEvent(QMouseEvent *event)
{
    if(m_mousePoint != QPoint(0, 0))
    {
        m_hostWidget->move(m_hostWidget->geometry().x() + event->globalPos().x() -
                           m_mousePoint.x(), m_hostWidget->geometry().y() + event->globalPos().y() - m_mousePoint.y());
        m_mousePoint = event->globalPos();
    }
}

void CWizCustomToolBar::mouseReleaseEvent(QMouseEvent *)
{
    m_mousePoint = QPoint(0, 0);
}

