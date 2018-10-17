#include "WizPopupWidget.h"

#include <QtGui>

#include "WizMisc.h"
#include "utils/WizStyleHelper.h"
#include "share/WizUIBase.h"

#ifdef Q_OS_WIN
//#include "WizWindowsHelper.h"
#endif


WizPopupWidget::WizPopupWidget(QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
    , m_leftAlign(false)
    , m_triangleMargin(10)
    , m_triangleWidth(16)
    , m_triangleHeight(8)
{
    setContentsMargins(8, 20, 8, 8);

#ifdef Q_OS_LINUX
    setStyleSheet("QWidget{background:#D7D7D7;}");
#elif defined(Q_OS_MAC)
    if (isDarkMode()) {
        setStyleSheet("QWidget{background:#666666;}");
    } else {
        setStyleSheet("QWidget{background:#F7F7F7;}");
    }
#endif
}

QSize WizPopupWidget::sizeHint() const
{
    return QSize(320, 400);
}

QRect WizPopupWidget::getClientRect() const
{
    QMargins margins = contentsMargins();
    return QRect(margins.left(), margins.top(),
                 width() - margins.left() - margins.right(),
                 height() - margins.top() - margins.bottom());
}

void WizPopupWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

}

void WizPopupWidget::showAtPoint(const QPoint& pt)
{
    int xOffset = m_leftAlign ? (m_triangleMargin + m_triangleWidth / 2) : sizeHint().width() - (m_triangleMargin + m_triangleWidth / 2);
    int yOffset = 0;

    int left = pt.x() - xOffset;
    int top = pt.y() - yOffset;

    setGeometry(left, top, sizeHint().width(), sizeHint().height());
    QRegion regin = mask();
    if (regin.isNull()) {
        setMask(maskRegion());
    }
    show();
}

void WizPopupWidget::setTriangleStyle(int triangleMargin, int triangleWidth, int triangleHeight)
{
    m_triangleMargin = triangleMargin;
    m_triangleWidth = triangleWidth;
    m_triangleHeight = triangleHeight;
}

QRegion WizPopupWidget::maskRegion()
{
    return Utils::WizStyleHelper::borderRadiusRegionWithTriangle(QRect(0, 0, size().width(), size().height()), m_leftAlign,
                                                       m_triangleMargin, m_triangleWidth, m_triangleHeight);
}
