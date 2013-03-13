#include "wizstatusbar.h"
#include "wizmainwindow.h"

CWizStatusBar::CWizStatusBar(CWizExplorerApp& app, QWidget *parent)
    : QLabel(parent)
    , m_app(app)
{
    setFrameShadow(QFrame::Plain);
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_MacShowFocusRect, true);
}

void CWizStatusBar::autoShow(const QString& strText)
{
    setText(strText);
    adjustSize();

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    int y = mainWindow->size().height() - size().height();

    move(0, y);
    show();
    raise();
}

