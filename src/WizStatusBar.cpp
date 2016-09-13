#include "WizStatusBar.h"

#include <QCursor>

#include "WizMainWindow.h"


WizStatusBar::WizStatusBar(WizExplorerApp& app, QWidget *parent)
    : QLabel(parent)
    , m_app(app)
{
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);
}

void WizStatusBar::adjustPosition()
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    int y1 = mainWindow->size().height() - size().height();
    move(0, y1);
}

void WizStatusBar::showText(const QString& strText /* = QString() */)
{
    setText(strText);
    adjustSize();
    adjustPosition();
    show();
    raise();
}

bool WizStatusBar::isCursorInside()
{
    int nMargin = 30;

    // determine cursor position is inside statusbar frame or not
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    QPoint pos = mainWindow->mapFromGlobal(QCursor::pos());
    int y = mainWindow->size().height() - size().height() - nMargin;
    QRect rect(0, y, size().width(), size().height() + nMargin);
    return rect.contains(pos);
}
