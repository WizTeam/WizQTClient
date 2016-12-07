#ifndef CWIZPOSITIONDELEGATE_H
#define CWIZPOSITIONDELEGATE_H

#include <QObject>
#include <QPoint>
#include <QList>

class QWidget;

class WizPositionDelegate : public QObject
{
    Q_OBJECT
public:
    static WizPositionDelegate& instance()
    {
        static WizPositionDelegate _instance;   //局部静态变量
        return _instance;
    }

    void addListener(QWidget* widget);

    void removeListener(QWidget* widget);


    void mainwindowPositionChanged(const QPoint& oldPos, const QPoint& newPos);

public slots:
    void on_WidgetDeleted(QObject * obj);

private:
    explicit WizPositionDelegate() {}
    WizPositionDelegate(const WizPositionDelegate &) {}
    WizPositionDelegate& operator= (const WizPositionDelegate &) { return *this; }

private:
    QList<QWidget*> m_widgetList;
};

#endif // CWIZPOSITIONDELEGATE_H
