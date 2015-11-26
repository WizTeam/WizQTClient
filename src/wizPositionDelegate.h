#ifndef CWIZPOSITIONDELEGATE_H
#define CWIZPOSITIONDELEGATE_H

#include <QObject>
#include <QPoint>
#include <QList>
#include <QWidget>

class CWizPositionDelegate
{
public:
    static CWizPositionDelegate& instance()
    {
        static CWizPositionDelegate _instance;   //局部静态变量
        return _instance;
    }

    void addListener(QWidget* widget)
    {
        m_widgetList.append(widget);
    }

    void removeListener(QWidget* widget)
    {
        m_widgetList.removeOne(widget);
    }

    void mainwindowPositionChanged(const QPoint& oldPos, const QPoint& newPos)
    {
        int xOff = newPos.x() - oldPos.x();
        int yOff = newPos.y() - oldPos.y();

        for (QWidget* widget : m_widgetList)
        {
            QPoint pos(widget->pos().x() + xOff, widget->pos().y() + yOff);
            widget->move(pos);
        }
    }

private:
    explicit CWizPositionDelegate() {}
    CWizPositionDelegate(const CWizPositionDelegate &) {}
    CWizPositionDelegate& operator= (const CWizPositionDelegate &) { return *this; }

private:
    QList<QWidget*> m_widgetList;
};

#endif // CWIZPOSITIONDELEGATE_H
