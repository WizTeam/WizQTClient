#include "wizpopupwidget.h"

#include <QtGui>

#include "wizmisc.h"
#include "utils/stylehelper.h"

#ifdef Q_OS_WIN
//#include "wizwindowshelper.h"
#endif


CWizPopupWidget::CWizPopupWidget(QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
    , m_leftAlign(false)
    , m_triangleMargin(10)
    , m_triangleWidth(16)
    , m_triangleHeight(8)
{
    setContentsMargins(8, 20, 8, 8);

    QPalette pal;
    pal.setBrush(backgroundRole(), QBrush("#FFFFFF"));
    setPalette(pal);
}

QSize CWizPopupWidget::sizeHint() const
{
    return QSize(320, 400);
}

QRect CWizPopupWidget::getClientRect() const
{
    QMargins margins = contentsMargins();
    return QRect(margins.left(), margins.top(),
                 width() - margins.left() - margins.right(),
                 height() - margins.top() - margins.bottom());
}

void CWizPopupWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    setMask(maskRegion());
}

void CWizPopupWidget::showAtPoint(const QPoint& pt)
{
    int xOffset = m_leftAlign ? (m_triangleMargin + m_triangleWidth / 2) : sizeHint().width() - (m_triangleMargin + m_triangleWidth / 2);
    int yOffset = 0;

    int left = pt.x() - xOffset;
    int top = pt.y() - yOffset;

#if 0
    m_pos.setX(left);
    m_pos.setY(top);

    QPropertyAnimation* anim1 = new QPropertyAnimation(this, "geometry");
    anim1->setDuration(200);
    anim1->setKeyValueAt(0, QRect(QPoint(m_pos.x() + sizeHint().width(), m_pos.y()), QSize(0, 0)));
    anim1->setKeyValueAt(0.5, QRect(QPoint(m_pos.x() - 50, m_pos.y()), QSize(sizeHint().width() + 100, sizeHint().height() + 100)));
    anim1->setKeyValueAt(1, QRect(QPoint(m_pos.x(), m_pos.y()), QSize(sizeHint().width(), sizeHint().height())));

    QPropertyAnimation* anim2 = new QPropertyAnimation(this, "windowOpacity");
    anim2->setStartValue(0.1);
    anim2->setEndValue(1.0);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(anim1);
    group->addAnimation(anim2);

    group->start();
#endif

    setGeometry(left, top, sizeHint().width(), sizeHint().height());
    show();
}

void CWizPopupWidget::setTriangleStyle(int triangleMargin, int triangleWidth, int triangleHeight)
{
    m_triangleMargin = triangleMargin;
    m_triangleWidth = triangleWidth;
    m_triangleHeight = triangleHeight;
}

QRegion CWizPopupWidget::maskRegion()
{
//    QSize sz = size();

    return Utils::StyleHelper::borderRadiusRegionWithTriangle(QRect(0, 0, size().width(), size().height()), m_leftAlign,
                                                       m_triangleMargin, m_triangleWidth, m_triangleHeight);

//    m_pointsRegion.clear();

//    if (m_leftAlign)
//    {
//        m_pointsRegion.push_back(QPoint(1, 10));
//        m_pointsRegion.push_back(QPoint(1 + m_triangleMargin, 10));
//        m_pointsRegion.push_back(QPoint(1 + m_triangleMargin + m_triangleWidth / 2, 0));
//        m_pointsRegion.push_back(QPoint(31, 10));
//        m_pointsRegion.push_back(QPoint(sz.width() - 1, 10));
//        m_pointsRegion.push_back(QPoint(sz.width(), 11));
//        m_pointsRegion.push_back(QPoint(sz.width(), sz.height()));
//        m_pointsRegion.push_back(QPoint(0, sz.height()));
//        m_pointsRegion.push_back(QPoint(0, 11));

////        m_pointsPolygon.push_back(QPoint(1, 10));
////        m_pointsPolygon.push_back(QPoint(11, 10));
////        for (int i = 0; i < 9; i++)
////        {
////            m_pointsPolygon.push_back(QPoint(11 + i, 10 - i));
////            m_pointsPolygon.push_back(QPoint(11 + i + 1, 10 - i));
////        }
////        m_pointsPolygon.push_back(QPoint(20, 1));
////        m_pointsPolygon.push_back(QPoint(21, 1));
////        for (int i = 0; i < 9; i++)
////        {
////            m_pointsPolygon.push_back(QPoint(21 + i, 2 + i));
////            m_pointsPolygon.push_back(QPoint(21 + i + 1, 2 + i));
////        }
////        m_pointsPolygon.push_back(QPoint(31, 10));
////        m_pointsPolygon.push_back(QPoint(sz.width() - 2, 10));
////        m_pointsPolygon.push_back(QPoint(sz.width() - 1, 11));
////        m_pointsPolygon.push_back(QPoint(sz.width() - 1, sz.height() - 1));
////        m_pointsPolygon.push_back(QPoint(0, sz.height() - 1));
////        m_pointsPolygon.push_back(QPoint(0, 11));
////        m_pointsPolygon.push_back(QPoint(1, 11));
//    }
//    else
//    {
//        m_pointsRegion.push_back(QPoint(1, 10));
//        m_pointsRegion.push_back(QPoint(sz.width() - 31, 10));
//        m_pointsRegion.push_back(QPoint(sz.width() - 21, 0));
//        m_pointsRegion.push_back(QPoint(sz.width() - 11, 10));
//        m_pointsRegion.push_back(QPoint(sz.width() - 1, 10));
//        m_pointsRegion.push_back(QPoint(sz.width(), 11));
//        m_pointsRegion.push_back(QPoint(sz.width(), sz.height()));
//        m_pointsRegion.push_back(QPoint(0, sz.height()));
//        m_pointsRegion.push_back(QPoint(0, 11));

////        m_pointsPolygon.push_back(QPoint(1, 10));
////        m_pointsPolygon.push_back(QPoint(sz.width() - 31, 10));
////        for (int i = 0; i < 9; i++)
////        {
////            m_pointsPolygon.push_back(QPoint(sz.width() - 31 + i, 10 - i));
////            m_pointsPolygon.push_back(QPoint(sz.width() - 31 + i + 1, 10 - i));
////        }
////        m_pointsPolygon.push_back(QPoint(sz.width() - 22, 1));
////        m_pointsPolygon.push_back(QPoint(sz.width() - 21, 1));
////        for (int i = 0; i < 9; i++)
////        {
////            m_pointsPolygon.push_back(QPoint(sz.width() - 21 + i, 2 + i));
////            m_pointsPolygon.push_back(QPoint(sz.width() - 21 + i + 1, 2 + i));
////        }
////        m_pointsPolygon.push_back(QPoint(sz.width() - 12, 10));
////        m_pointsPolygon.push_back(QPoint(sz.width() - 2, 10));
////        m_pointsPolygon.push_back(QPoint(sz.width() - 2, 11));
////        m_pointsPolygon.push_back(QPoint(sz.width() - 1, 11));
////        m_pointsPolygon.push_back(QPoint(sz.width() - 1, sz.height() - 1));
////        m_pointsPolygon.push_back(QPoint(0, sz.height() - 1));
////        m_pointsPolygon.push_back(QPoint(0, 11));
////        m_pointsPolygon.push_back(QPoint(1, 11));

//    }

//    QPolygon polygon(m_pointsRegion);

//    return QRegion(polygon);
}
