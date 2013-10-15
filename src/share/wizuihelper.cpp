#include "wizuihelper.h"

#ifdef BUILD_WITH_QT5
#include <QtWidgets>
#endif

#include "share/wizmisc.h"
#include <QLabel>

QBrush WizGetLeftViewBrush()
{
    QPixmap pixmapBg;
    pixmapBg.load(::WizGetResourcesPath() + "skins/leftview_bg.png");
    return QBrush(pixmapBg);
}

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
    CWizSplitterHandle(Qt::Orientation orientation, QSplitter* parent)
        : QSplitterHandle(orientation, parent)
    {
        setMask(QRegion(contentsRect()));
        setAttribute(Qt::WA_MouseNoMask, true);
    }

    virtual void paintEvent(QPaintEvent *event)
    {
        QPainter painter(this);

        // FIXME: hard-coded
        QColor bgColor = QColor(165, 165, 165);
        painter.fillRect(event->rect(), bgColor);
    }

    virtual void resizeEvent(QResizeEvent *event)
    {
        if (orientation() == Qt::Horizontal)
            setContentsMargins(2, 0, 2, 0);
        else
            setContentsMargins(0, 2, 0, 2);
        setMask(QRegion(contentsRect()));
        QSplitterHandle::resizeEvent(event);
    }

    virtual QSize sizeHint() const
    {
        return QSize(1, 1);
    }
};


CWizSplitter::CWizSplitter(QWidget* parent /*= 0*/)
    : QSplitter(parent)
{
    setHandleWidth(1);
    setChildrenCollapsible(false);
}

QSplitterHandle *CWizSplitter::createHandle()
{
    return new CWizSplitterHandle(orientation(), this);
}
