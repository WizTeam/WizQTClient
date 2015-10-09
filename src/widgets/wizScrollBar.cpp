#include "wizScrollBar.h"

#include <QtGui>

CWizScrollBar::CWizScrollBar(QWidget* parent /* = 0 */)
    : QScrollBar(parent)
    , m_width(8)
{

    // FIXME:  hard code
    setHandleVisible(true);

    setMouseTracking(true);

    connect(&m_timerScrollTimeout, SIGNAL(timeout()), this, SLOT(on_scrollTimeout()));
    connect(this, SIGNAL(valueChanged(int)), SLOT(on_valueChanged(int)));

    m_timerScrollTimeout.setSingleShot(true);
    m_timerScrollTimeout.start(3000);
}

QSize CWizScrollBar::sizeHint() const
{
    return QSize(m_width, 1);
}

void CWizScrollBar::mouseMoveEvent(QMouseEvent* event)
{
    m_timerScrollTimeout.start(3000);

    QScrollBar::mouseMoveEvent(event);
}

void CWizScrollBar::syncWith(QScrollBar* source)
{
    setMinimum(source->minimum());
    setMaximum(source->maximum());
    setSliderPosition(source->sliderPosition());
    setPageStep(source->pageStep());
    setSingleStep(source->singleStep());

    connect(source, SIGNAL(valueChanged(int)), SLOT(on_sourceValueChanged(int)));
    connect(source, SIGNAL(rangeChanged(int, int)), SLOT(on_sourceRangeChanged(int, int)));

    m_scrollSyncSource = source;

}

void CWizScrollBar::setWidth(int width)
{
    m_width = width;
}

void CWizScrollBar::showHandle()
{
    if (maximum() == minimum())
        return;

    setHandleVisible(true);
    update();

//    QScrollBar::show();
    m_timerScrollTimeout.start(1000);
}

void CWizScrollBar::hideHandle()
{
    setHandleVisible(false);
    update();
}

void CWizScrollBar::on_sourceValueChanged(int value)
{
    setSliderPosition(value);
}

void CWizScrollBar::on_sourceRangeChanged(int min, int max)
{
    setMinimum(min);
    setMaximum(max);
    setPageStep(m_scrollSyncSource->pageStep());
    setSingleStep(m_scrollSyncSource->singleStep());
}

void CWizScrollBar::on_valueChanged(int value)
{
    if (m_scrollSyncSource) {
        m_scrollSyncSource->setSliderPosition(value);
    }

    showHandle();
}

void CWizScrollBar::on_scrollTimeout()
{
    hideHandle();
}

void CWizScrollBar::setHandleVisible(bool visible)
{
    setStyleSheet(
        QString("QScrollBar {\
            background: #F5F5F5;\
        }\
        QScrollBar::handle {\
            background: %1;\
            min-height: 30px;\
        }\
        QScrollBar::handle:vertical {\
            margin: 0px %2 0px 2px;\
        }\
        QScrollBar::add-page, QScrollBar::sub-page {\
            background: transparent;\
        }\
        QScrollBar::up-arrow, QScrollBar::down-arrow, QScrollBar::left-arrow, QScrollBar::right-arrow {\
            background: transparent;\
        }\
        QScrollBar::add-line, QScrollBar::sub-line {\
            height: 0px;\
            width: 0px;\
        }").arg(visible ? "#D8D8D8" : "transparent").arg(m_width - 6));
}


CWizListWidgetWithCustomScorllBar::CWizListWidgetWithCustomScorllBar(QWidget* parent)
    : QListWidget(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vScroll = new CWizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());
}

void CWizListWidgetWithCustomScorllBar::resizeEvent(QResizeEvent* event)
{
    // reset scrollbar
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);
    QListWidget::resizeEvent(event);
}
