#include "wizstatusbar.h"

CWizStatusBar::CWizStatusBar(CWizExplorerApp& app, QWidget *parent)
    : QStatusBar(parent)
    , m_app(app)
{
    addWidget(&m_label);
    //setParent(m_app.mainWindow());
    //setWindowFlags(Qt::CustomizeWindowHint|Qt::Tool);
}

//void CWizStatusBar::paintEvent(QPaintEvent* event)
//{
//    Q_UNUSED(event);
//
//    QRect msgRect(0, 0, sizeHint().width(), sizeHint().height());
//
//    QPainter p(this);
//    p.setPen(palette().foreground().color());
//    p.drawText(msgRect, Qt::AlignLeading | Qt::AlignVCenter | Qt::TextSingleLine, m_label.text());
//
//    QRect windowRect = m_app.mainWindow()->geometry();
//    int x = windowRect.x();
//    int y = windowRect.y() + windowRect.height() - m_label.sizeHint().height();
//    int w = m_label.sizeHint().width();
//    int h = m_label.sizeHint().height();
//    setGeometry(x, y, w, h);
//}

void CWizStatusBar::setText(const QString& text)
{
    m_label.setText(text);
}
