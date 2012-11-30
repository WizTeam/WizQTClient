#include "wizpopupwidget.h"

#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QResizeEvent>
#include <QPolygon>
#include <QTimer>

#include "wizmisc.h"

#ifdef Q_OS_WIN
//#include "wizwindowshelper.h"
#endif


CWizPopupWidget::CWizPopupWidget(QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
    , m_leftAlign(false)
{
    setContentsMargins(8, 20, 8, 8);

    //QPalette pal = palette();
    //pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xff));
    //setPalette(pal);
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
    Q_UNUSED(event);

    QPainter painter(this);
    QPen pen(QColor(0xd9, 0xdc, 0xdd));
    //pen.setWidth(3);
    painter.setPen(pen);
    //painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPolygon(m_pointsPolygon);
}

#endif

void CWizPopupWidget::resizeEvent(QResizeEvent* event)
{
    QSize sz = event->size();

    m_pointsRegion.clear();

    if (m_leftAlign)
    {
        m_pointsRegion.push_back(QPoint(1, 10));
        m_pointsRegion.push_back(QPoint(11, 10));
        m_pointsRegion.push_back(QPoint(21, 0));
        m_pointsRegion.push_back(QPoint(31, 10));
        m_pointsRegion.push_back(QPoint(sz.width() - 1, 10));
        m_pointsRegion.push_back(QPoint(sz.width(), 11));
        m_pointsRegion.push_back(QPoint(sz.width(), sz.height()));
        m_pointsRegion.push_back(QPoint(0, sz.height()));
        m_pointsRegion.push_back(QPoint(0, 11));

        m_pointsPolygon.push_back(QPoint(1, 10));
        m_pointsPolygon.push_back(QPoint(11, 10));
        for (int i = 0; i < 9; i++)
        {
            m_pointsPolygon.push_back(QPoint(11 + i, 10 - i));
            m_pointsPolygon.push_back(QPoint(11 + i + 1, 10 - i));
        }
        m_pointsPolygon.push_back(QPoint(20, 1));
        m_pointsPolygon.push_back(QPoint(21, 1));
        for (int i = 0; i < 9; i++)
        {
            m_pointsPolygon.push_back(QPoint(21 + i, 2 + i));
            m_pointsPolygon.push_back(QPoint(21 + i + 1, 2 + i));
        }
        m_pointsPolygon.push_back(QPoint(31, 10));
        m_pointsPolygon.push_back(QPoint(sz.width() - 2, 10));
        m_pointsPolygon.push_back(QPoint(sz.width() - 1, 11));
        m_pointsPolygon.push_back(QPoint(sz.width() - 1, sz.height() - 1));
        m_pointsPolygon.push_back(QPoint(0, sz.height() - 1));
        m_pointsPolygon.push_back(QPoint(0, 11));
        m_pointsPolygon.push_back(QPoint(1, 11));
    }
    else
    {
        m_pointsRegion.push_back(QPoint(1, 10));
        m_pointsRegion.push_back(QPoint(sz.width() - 31, 10));
        m_pointsRegion.push_back(QPoint(sz.width() - 21, 0));
        m_pointsRegion.push_back(QPoint(sz.width() - 11, 10));
        m_pointsRegion.push_back(QPoint(sz.width() - 1, 10));
        m_pointsRegion.push_back(QPoint(sz.width(), 11));
        m_pointsRegion.push_back(QPoint(sz.width(), sz.height()));
        m_pointsRegion.push_back(QPoint(0, sz.height()));
        m_pointsRegion.push_back(QPoint(0, 11));

        m_pointsPolygon.push_back(QPoint(1, 10));
        m_pointsPolygon.push_back(QPoint(sz.width() - 31, 10));
        for (int i = 0; i < 9; i++)
        {
            m_pointsPolygon.push_back(QPoint(sz.width() - 31 + i, 10 - i));
            m_pointsPolygon.push_back(QPoint(sz.width() - 31 + i + 1, 10 - i));
        }
        m_pointsPolygon.push_back(QPoint(sz.width() - 22, 1));
        m_pointsPolygon.push_back(QPoint(sz.width() - 21, 1));
        for (int i = 0; i < 9; i++)
        {
            m_pointsPolygon.push_back(QPoint(sz.width() - 21 + i, 2 + i));
            m_pointsPolygon.push_back(QPoint(sz.width() - 21 + i + 1, 2 + i));
        }
        m_pointsPolygon.push_back(QPoint(sz.width() - 12, 10));
        m_pointsPolygon.push_back(QPoint(sz.width() - 2, 10));
        m_pointsPolygon.push_back(QPoint(sz.width() - 2, 11));
        m_pointsPolygon.push_back(QPoint(sz.width() - 1, 11));
        m_pointsPolygon.push_back(QPoint(sz.width() - 1, sz.height() - 1));
        m_pointsPolygon.push_back(QPoint(0, sz.height() - 1));
        m_pointsPolygon.push_back(QPoint(0, 11));
        m_pointsPolygon.push_back(QPoint(1, 11));

    }

    QPolygon polygon(m_pointsRegion);

    QRegion region(polygon);

    setMask(region);
}


void CWizPopupWidget::showAtPoint(const QPoint& pt)
{
    int xOffset = m_leftAlign ? 21 : sizeHint().width() - 21;
    int yOffset = 4;

    int left = pt.x() - xOffset;
    int top = pt.y() - yOffset;

    m_pos.setX(left);
    m_pos.setY(top);

    QPropertyAnimation* animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(100);
    animation->setStartValue(QRect(QPoint(m_pos.x() + sizeHint().width(), m_pos.y()), QSize(0, 0)));
    animation->setEndValue(QRect(QPoint(m_pos.x() - 100, m_pos.y()), QSize(sizeHint().width() + 100, sizeHint().height() + 100)));
    animation->start();

    connect(animation, SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)), \
            SLOT(onAnimationStateChanged(QAbstractAnimation::State, QAbstractAnimation::State)));

    show();
}

void CWizPopupWidget::onAnimationStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    Q_UNUSED(oldState);

    if (newState == QAbstractAnimation::Stopped && size() != sizeHint()) {
        hide();
        QTimer::singleShot(10, this, SLOT(onAnimationTimeout()));
    }
}

void CWizPopupWidget::onAnimationTimeout()
{
    setGeometry(QRect(m_pos, sizeHint()));
    show();
}
