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


CWizSplitter::CWizSplitter(QWidget* parent /*= 0*/)
    : QSplitter(parent)
#ifndef Q_OS_MAC
    , m_splitterWidth(handleWidth())
    , m_splitterColor(palette().color(QPalette::Window))
#endif
{
}



#ifndef Q_OS_MAC

class CWizSplitterHandle : public QSplitterHandle
{
    CWizSplitter* m_splitter;
public:
    CWizSplitterHandle(Qt::Orientation orientation, CWizSplitter *parent);
    void paintEvent(QPaintEvent *);
};



CWizSplitterHandle::CWizSplitterHandle(Qt::Orientation orientation, CWizSplitter *parent)
    : QSplitterHandle(orientation, parent)
    , m_splitter(parent)
{
}

// Paint the horizontal handle as a gradient, paint
// the vertical handle as a line.
void CWizSplitterHandle::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    //
    QRect rc(0, 0, width(), height());
    int splitterWidth = m_splitter->splitterWidth();
    rc.setLeft((rc.width() - splitterWidth) / 2);
    rc.setRight(rc.left() + splitterWidth - 1);
    //
    painter.fillRect(rc, m_splitter->splitterColor());
}

QSplitterHandle *CWizSplitter::createHandle()
{
    return new CWizSplitterHandle(orientation(), this);
}


void CWizSplitter::setSplitterWidth(int width)
{
    m_splitterWidth = width;
}

void CWizSplitter::setSplitterColor(const QColor& color)
{
    m_splitterColor = color;
}

#endif
