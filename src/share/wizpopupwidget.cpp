#include "wizpopupwidget.h"
#include "wizmisc.h"

#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QResizeEvent>
#include <QPolygon>
#include <QTimer>

#ifdef Q_OS_WIN
//#include "wizwindowshelper.h"
#endif


CWizPopupWidget::CWizPopupWidget(QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
    , m_leftAlign(false)
{
    setContentsMargins(8, 20, 8, 8);
    //
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xff));
    setPalette(pal);
}

QSize CWizPopupWidget::sizeHint() const
{
    return QSize(300, 400);
}

QRect CWizPopupWidget::getClientRect() const
{
    QMargins margins = contentsMargins();
    return QRect(margins.left(), margins.top(),
                 width() - margins.left() - margins.right(),
                 height() - margins.top() - margins.bottom());
}

#ifndef Q_OS_MAC

void CWizPopupWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QPen pen(QColor(0xd9, 0xdc, 0xdd));
    //pen.setWidth(3);
    painter.setPen(pen);
    //
    QVector<QPoint> points = m_points;
    int right = 0;
    int bottom = 0;
    for (int i = 0; i < points.size() - 1; i++)
    {
        right = std::max<int>(right, points[i].x());
        bottom = std::max<int>(bottom, points[i].y());
    }
    for (int i = 0; i < points.size() - 1; i++)
    {
        if (right == points[i].x())
        {
            points[i].setX(points[i].x() - 1);
        }
        if (bottom == points[i].y())
        {
            points[i].setY(points[i].y() - 1);
        }
    }
    for (int i = 0; i < points.size() - 1; i++)
    {
        QPoint pt1 = points[i];
        QPoint pt2 = points[i + 1];
        //
        if (pt1.x() < pt2.x()
            && pt1.y() < pt2.y())
        {
            points[i + 1].setX(points[i + 1].x() - 1);
            points.insert(i + 1, QPoint(pt1.x() - 1, pt1.y()));
            i++;
            break;
        }

    }
    //
    painter.drawPolygon(points);
}

#endif

void CWizPopupWidget::resizeEvent(QResizeEvent* event)
{
    QSize sz = event->size();
    //
    m_points.clear();
    //
    if (m_leftAlign)
    {
        QPoint pt1(1, 10);
        QPoint pt2(11, 10);
        QPoint pt3(21, 0);
        QPoint pt4(31, 10);
        QPoint pt5(sz.width() - 1, 10);
        QPoint pt6(sz.width(), 11);
        QPoint pt7(sz.width(), sz.height() - 1);
        QPoint pt8(sz.width() - 1, sz.height());
        QPoint pt9(1, sz.height());
        QPoint pt10(0, sz.height() - 1);
        QPoint pt11(0, 11);
        //
        m_points.push_back(pt1);
        m_points.push_back(pt2);
        m_points.push_back(pt3);
        m_points.push_back(pt4);
        m_points.push_back(pt5);
        m_points.push_back(pt6);
        m_points.push_back(pt7);
        m_points.push_back(pt8);
        m_points.push_back(pt9);
        m_points.push_back(pt10);
        m_points.push_back(pt11);
    }
    else
    {
        QPoint pt1(1, 10);
        QPoint pt2(sz.width() - 31, 10);
        QPoint pt3(sz.width() - 21, 0);
        QPoint pt4(sz.width() - 11, 10);
        QPoint pt5(sz.width() - 1, 10);
        QPoint pt6(sz.width(), 11);
        QPoint pt7(sz.width(), sz.height() - 1);
        QPoint pt8(sz.width() - 1, sz.height());
        QPoint pt9(1, sz.height());
        QPoint pt10(0, sz.height() - 1);
        QPoint pt11(0, 11);
        //
        m_points.push_back(pt1);
        m_points.push_back(pt2);
        m_points.push_back(pt3);
        m_points.push_back(pt4);
        m_points.push_back(pt5);
        m_points.push_back(pt6);
        m_points.push_back(pt7);
        m_points.push_back(pt8);
        m_points.push_back(pt9);
        m_points.push_back(pt10);
        m_points.push_back(pt11);
    }
    //
    QPolygon polygon(m_points);
    //
    QRegion region(polygon);
    //
    setMask(region);
}


void CWizPopupWidget::showAtPoint(const QPoint& pt)
{
    adjustSize();
    //
    QSize sz = geometry().size();
    //
    int xOffset = m_leftAlign ? 21 : sz.width() - 21;
    int yOffset = 4;
    //
    int left = pt.x() - xOffset;
    int top = pt.y() - yOffset;
    //
    move(QPoint(left, top));
    //
    show();
    //
    //activateWindow();
}

