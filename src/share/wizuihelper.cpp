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
