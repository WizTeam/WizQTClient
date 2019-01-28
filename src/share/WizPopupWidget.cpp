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
    , m_triangleMargin(WizSmartScaleUI(10))
    , m_triangleWidth(WizSmartScaleUI(16))
    , m_triangleHeight(WizSmartScaleUI(8))
{
    setContentsMargins(WizSmartScaleUI(8), WizSmartScaleUI(20), WizSmartScaleUI(8), WizSmartScaleUI(8));

#ifdef Q_OS_LINUX
    if (isDarkMode()) {
        setStyleSheet("QWidget{background:#373737;}");
    } else {
        setStyleSheet("QWidget{background:#D7D7D7;}");
    }
#elif defined(Q_OS_MAC)
    if (isDarkMode()) {
        setStyleSheet("QWidget{background:#272727;}");
    } else {
        setStyleSheet("QWidget{background:#F7F7F7;}");
    }
#endif
}

QSize WizPopupWidget::sizeHint() const
{
    return QSize(WizSmartScaleUI(320), WizSmartScaleUI(400));
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

void WizPopupWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    //
    qDebug() << event->size() << "\n" << event->oldSize();
    //
    setMask(maskRegion());
}

void WizPopupWidget::updateRegion()
{
    setMask(maskRegion());
}


void WizPopupWidget::showAtPoint(const QPoint& pt)
{
    QSize sz = sizeHint();
    int xOffset = m_leftAlign ? (m_triangleMargin + m_triangleWidth / 2) : sz.width() - (m_triangleMargin + m_triangleWidth / 2);
    int yOffset = 0;

    int left = pt.x() - xOffset;
    int top = pt.y() - yOffset;

    setGeometry(left, top, sz.width(), sz.height());
    setMask(maskRegion());
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
    QSize sz = size();
    return Utils::WizStyleHelper::borderRadiusRegionWithTriangle(QRect(0, 0, sz.width(), sz.height()), m_leftAlign,
                                                       m_triangleMargin, m_triangleWidth, m_triangleHeight);
}
