#include "wizstatusbar.h"

#include <QCursor>

#include "wizmainwindow.h"

using namespace Core::Internal;

CWizStatusBar::CWizStatusBar(CWizExplorerApp& app, QWidget *parent)
    : QLabel(parent)
    , m_app(app)
    , m_bIsVisible(false)
{
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);

    m_animation = new QPropertyAnimation(this, "pos");
    m_animation->setDuration(200);
    connect(m_animation, SIGNAL(finished()), SLOT(on_animation_finished()));

    m_animationTimer.setInterval(100);
    connect(&m_animationTimer, SIGNAL(timeout()), SLOT(on_animationTimer_timeout()));

    m_hideTimer.setInterval(3 * 1000);
    m_hideTimer.setSingleShot(true);
    connect(&m_hideTimer, SIGNAL(timeout()), SLOT(on_hideTimer_timeout()));
}

void CWizStatusBar::enterEvent(QEvent* event)
{
    Q_UNUSED(event);

    if (m_animation->state() != QAbstractAnimation::Stopped) {
        return;
    }

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    int y2 = mainWindow->size().height();
    m_animation->setEndValue(QPoint(0, y2));
    m_animation->start();

    m_bIsVisible = false;
}

void CWizStatusBar::on_animation_finished()
{
    if (!m_bIsVisible)
        m_animationTimer.start();
}

void CWizStatusBar::on_animationTimer_timeout()
{
    if (m_animation->state() != QAbstractAnimation::Stopped || isCursorInside()) {
        return;
    }

    m_animationTimer.stop();

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    int y1 = mainWindow->size().height() - size().height();
    m_animation->setEndValue(QPoint(0, y1));
    m_animation->start();

    m_bIsVisible = true;
}

void CWizStatusBar::adjustPosition()
{
    if (m_animation->state() != QAbstractAnimation::Stopped || isCursorInside()) {
        return;
    }

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

    m_hideTimer.start();

    m_bIsVisible = true;
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

void CWizStatusBar::on_hideTimer_timeout()
{
    hide();
}
