#include "wizpopupwidget.h"
#include "wizmisc.h"

#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QResizeEvent>
#include <QPolygon>


CWizPopupWidget::CWizPopupWidget(QWidget* parent)
#ifndef Q_OS_MAC
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint)
#else
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
#endif
{
#ifndef Q_OS_MAC
    setFocusPolicy(Qt::StrongFocus);
    //
    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(on_application_focusChanged(QWidget*,QWidget*)));
    m_backgroundImage.SetImage(::WizGetSkinResourceFileName("popup_bckground.png"), QPoint(80, 50));
    setContentsMargins(24, 26, 22, 27);
#else
    setContentsMargins(8, 20, 8, 8);
#endif
    //
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xff));
    setPalette(pal);
    //
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

void CWizPopupWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    //
    QPainter painter(this);
    //
    if (!m_backgroundPixmap.isNull())
    {
        painter.drawPixmap(0, 0, m_backgroundPixmap);
    }
    //
    m_backgroundImage.Draw(&painter, rect(), 0);
}
#endif

void CWizPopupWidget::resizeEvent(QResizeEvent *event)
{
    QSize sz = event->size();
    //
    QVector<QPoint> points;
    //
    QPoint pt1(0, 10);
    QPoint pt2(sz.width() - 54, 10);
    QPoint pt3(sz.width() - 41, 0);
    QPoint pt4(sz.width() - 28, 10);
    QPoint pt5(sz.width(), 10);
    QPoint pt6(sz.width(), sz.height());
    QPoint pt7(0, sz.height());
    //
    points.push_back(pt1);
    points.push_back(pt2);
    points.push_back(pt3);
    points.push_back(pt4);
    points.push_back(pt5);
    points.push_back(pt6);
    points.push_back(pt7);
    //
    QPolygon polygon(points);
    //
    QRegion region(polygon);
    //
    setMask(region);
}

void CWizPopupWidget::keyPressEvent(QKeyEvent * e)
{
    if (e->key() == Qt::Key_Escape)
    {
        hide();
        e->accept();
    }
    else
    {
        QWidget::keyPressEvent(e);
    }
}

void CWizPopupWidget::showAtPoint(const QPoint& pt)
{
    QSize sz = geometry().size();
    //
    int xOffset = sz.width() - 34;
    int yOffset = 4;
    //
    int left = pt.x() - xOffset;
    int top = pt.y() - yOffset;
    //
    move(QPoint(left, top));
    //
#ifndef Q_OS_MAC
    QRect rc = geometry();
    m_backgroundPixmap = QPixmap::grabWindow(QApplication::desktop()->winId(), rc.left(), rc.top(), rc.width(), rc.height());
#endif
    //
    show();
    setFocus();
    activateWindow();
}

#ifndef Q_OS_MAC

void CWizPopupWidget::on_application_focusChanged(QWidget* old, QWidget* now)
{
    if (!isVisible())
        return;
    if (!old && !now)
        return;
    //
    bool childWidget = false;
    QWidget* newWidget = now;
    while (newWidget)
    {
        if (newWidget == this)
        {
            childWidget = true;
            break;
        }
        //
        newWidget = newWidget->parentWidget();
    }
    //
    if (!childWidget)
    {
        hide();
    }
}

#endif
