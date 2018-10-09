#include "WizUIHelper.h"

#include <QtWidgets>

#include "share/WizMisc.h"
#include "utils/WizPathResolve.h"
#include "WizUIBase.h"
#include <QLabel>

QBrush WizGetLeftViewBrush()
{
    QPixmap pixmapBg;
    pixmapBg.load(Utils::WizPathResolve::resourcesPath() + "skins/leftview_bg.png");
    return QBrush(pixmapBg);
}

WizSpacer::WizSpacer(QWidget *parent)
    :QWidget(parent)
{
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
}


WizVerSpacer::WizVerSpacer(QWidget* parent)
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

        QColor bgColor = QColor(isDarkMode() ? "#000000" : "#dbdbdb");
        painter.setPen(bgColor);
        painter.setBrush(bgColor);
        painter.fillRect(event->rect(), bgColor);
    }

    virtual void resizeEvent(QResizeEvent *event)
    {
//        if (orientation() == Qt::Horizontal)
//            setContentsMargins(2, 0, 2, 0);
//        else
//            setContentsMargins(0, 2, 0, 2);
        setMask(QRegion(contentsRect()));
        QSplitterHandle::resizeEvent(event);
    }

    virtual QSize sizeHint() const
    {
        return QSize(1, 1);
    }
protected:
    void enterEvent(QEvent* ev)
    {
        QWidget::enterEvent(ev);
        setCursor(Qt::SplitHCursor);
    }

    void leaveEvent(QEvent* ev)
    {
        QWidget::leaveEvent(ev);
        setCursor(Qt::ArrowCursor);
    }
};

WizSplitter::WizSplitter(QWidget* parent /*= 0*/)
    : QSplitter(parent)
{
    setHandleWidth(1);
    setChildrenCollapsible(false);
}

QSplitterHandle *WizSplitter::createHandle()
{
    CWizSplitterHandle* spliter =  new CWizSplitterHandle(orientation(), this);
    return spliter;
}
