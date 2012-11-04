#include "wizstatusbar.h"

CWizStatusBar::CWizStatusBar(CWizExplorerApp& app, QWidget *parent)
    : QLabel(parent)
    , m_app(app)
{
    setParent(m_app.mainWindow());
    setWindowFlags(Qt::CustomizeWindowHint|Qt::Tool);
}

void CWizStatusBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QRect msgRect(0, 0, sizeHint().width(), sizeHint().height());

    QPainter p(this);
    p.setPen(palette().foreground().color());
    p.drawText(msgRect, Qt::AlignLeading | Qt::AlignVCenter | Qt::TextSingleLine, text());

    QRect windowRect = m_app.mainWindow()->geometry();
    int x = windowRect.x();
    int y = windowRect.y() + windowRect.height() - sizeHint().height();
    int w = sizeHint().width();
    int h = sizeHint().height();
    setGeometry(x, y, w, h);
}
