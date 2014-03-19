#include "wizstatusbar.h"

#include <QCursor>

#include "wizmainwindow.h"

using namespace Core::Internal;

CWizStatusBar::CWizStatusBar(CWizExplorerApp& app, QWidget *parent)
    : QLabel(parent)
    , m_app(app)
{
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);
}

void CWizStatusBar::adjustPosition()
{
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    int y1 = mainWindow->size().height() - size().height();
    move(0, y1);
}

void CWizStatusBar::showText(const QString& strText /* = QString() */)
{
    setText(strText);
    adjustSize();
    adjustPosition();
    show();
    raise();
}

bool CWizStatusBar::isCursorInside()
{
    int nMargin = 30;

    // determine cursor position is inside statusbar frame or not
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    QPoint pos = mainWindow->mapFromGlobal(QCursor::pos());
    int y = mainWindow->size().height() - size().height() - nMargin;
    QRect rect(0, y, size().width(), size().height() + nMargin);
    return rect.contains(pos);
}
