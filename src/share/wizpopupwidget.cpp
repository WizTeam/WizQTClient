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
}

QSize CWizPopupWidget::sizeHint() const
{
#ifndef Q_OS_MAC
    return QSize(350, 450);
#else
    return QSize(300, 400);
#endif
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

void CWizPopupWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        event->accept();
        closeWidget();
    }
    else
    {
        event->ignore();
    }
}

void CWizPopupWidget::mousePressEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();
    //
    if (!mask().contains(pos))
    {
        event->accept();
        closeWidget();
    }
}

#endif

void CWizPopupWidget::resizeEvent(QResizeEvent* event)
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
    //remember background
    QRect rc = geometry();
    m_backgroundPixmap = QPixmap::grabWindow(QApplication::desktop()->winId(), rc.left(), rc.top(), rc.width(), rc.height());
#endif
    //
    show();
    activateWindow();

#ifndef Q_OS_MAC
    grabKeyboard();
    grabMouse();
#endif
}

#ifndef Q_OS_MAC
void CWizPopupWidget::closeWidget()
{
    hide();
    //
    releaseKeyboard();
    releaseMouse();
}

#endif
