#include "wizScrollBar.h"

#include <QtGui>

CWizScrollBar::CWizScrollBar(QWidget* parent /* = 0 */)
    : QScrollBar(parent)
{
    setStyleSheet(
        "QScrollBar {\
            background: transparent;\
            width: 10px;\
        }\
        QScrollBar::handle {\
            background: rgba(85, 85, 85, 200);\
            border-radius: 4px;\
            min-height: 30px;\
        }\
        QScrollBar::handle:vertical {\
            margin: 0px 2px 0px 0px;\
        }\
        QScrollBar::handle:horizontal {\
            margin: 0px 0px 2px 0px;\
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
        }"\
    );

    setMouseTracking(true);

    connect(&m_timerScrollTimeout, SIGNAL(timeout()), this, SLOT(on_scrollTimeout()));
    connect(this, SIGNAL(valueChanged(int)), SLOT(on_valueChanged(int)));

    hide();
}

QSize CWizScrollBar::sizeHint() const
{
    return QSize(10, 1);
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

void CWizScrollBar::show()
{
    if (maximum() == minimum())
        return;

    QScrollBar::show();
    m_timerScrollTimeout.start(1000);
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

    show();
}

void CWizScrollBar::on_scrollTimeout()
{
    hide();
}
