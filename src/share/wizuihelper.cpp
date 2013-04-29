#include "wizuihelper.h"

#ifdef BUILD_WITH_QT5
#include <QtWidgets>
#endif

#include "share/wizmisc.h"
#include <QLabel>


CWizSpacer::CWizSpacer(QWidget *parent)
    :QWidget(parent)
{
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
}


CWizVerSpacer::CWizVerSpacer(QWidget* parent)
    :QWidget(parent)
{
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setSizePolicy(sizePolicy);
}


/* ------------------------------ CWizSplitter ------------------------------ */
class CWizSplitterHandle : public QSplitterHandle
{
public:
    CWizSplitterHandle(Qt::Orientation orientation, CWizSplitter *parent)
        : QSplitterHandle(orientation, parent)
        , m_splitter(parent)
    {
    }

    virtual void paintEvent(QPaintEvent *e)
    {
        QPainter painter(this);

        QRect rc = e->rect();
        rc.setWidth(1);

        // FIXME: generate shadow from background color
        QColor bgColor1 = QColor(205, 205, 205);
        QColor bgColor2 = QColor(195, 195, 195);
        QColor bgColor3 = QColor(150, 150, 150);

        painter.fillRect(rc, bgColor1);

        rc.setX(rc.x() + 1);
        painter.fillRect(rc, bgColor2);

        rc.setX(rc.x() + 1);
        painter.fillRect(rc, bgColor3);
    }

    virtual QSize sizeHint() const
    {
        return QSize(3, 1);
    }

private:
    CWizSplitter* m_splitter;
};


CWizSplitter::CWizSplitter(QWidget* parent /*= 0*/)
    : QSplitter(parent)
{
}

QSplitterHandle *CWizSplitter::createHandle()
{
    return new CWizSplitterHandle(orientation(), this);
}

/* ------------------------------ CWizScrollBar ------------------------------ */
CWizScrollBar::CWizScrollBar(QWidget* parent /* = 0 */)
    : QScrollBar(parent)
{
    setStyleSheet(
        "QScrollBar:vertical {\
            background: transparent;\
            width: 10px;\
        }\
        QScrollBar::handle:vertical {\
            background: rgba(85, 85, 85, 200);\
            margin: 0px 2px 0px 0px;\
            border-radius: 4px;\
            min-height: 25px;\
        }\
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\
            background: transparent;\
        }\
        QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {\
            background: transparent;\
        }\
        QScrollBar::add-line:vertical QScrollBar::sub-line:vertical {\
            background: transparent;\
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

void CWizScrollBar::paintEvent(QPaintEvent* event)
{
    QScrollBar::paintEvent(event);
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
    m_timerScrollTimeout.start(1000);
}

void CWizScrollBar::on_scrollTimeout()
{
    hide();
}
