#include "wizPositionDelegate.h"
#include <QWidget>
#include <algorithm>
#include <QDebug>

void CWizPositionDelegate::addListener(QWidget* widget)
{
    m_widgetList.append(widget);
    connect(widget, SIGNAL(destroyed(QObject*)), SLOT(on_WidgetDeleted(QObject*)));
}

void CWizPositionDelegate::removeListener(QWidget* widget)
{
    m_widgetList.removeOne(widget);
    disconnect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(on_WidgetDeleted(QObject*)));
}

void CWizPositionDelegate::mainwindowPositionChanged(const QPoint& oldPos, const QPoint& newPos)
{
    int xOff = newPos.x() - oldPos.x();
    int yOff = newPos.y() - oldPos.y();

    for (QWidget* widget : m_widgetList)
    {
        QPoint pos(widget->pos().x() + xOff, widget->pos().y() + yOff);
        widget->move(pos);
    }
}

void CWizPositionDelegate::on_WidgetDeleted(QObject* obj)
{
    for (QWidget* wgt : m_widgetList)
    {
        if (wgt == obj)
        {
            m_widgetList.removeOne(wgt);
            return;
        }
    }
}
