#include "wizButton.h"

#include <QAction>
#include <QPainter>
#include <QDebug>

#include "share/wizmisc.h"
#include "share/wizsettings.h"


CWizButton::CWizButton(CWizExplorerApp& app, QWidget* parent /* = 0 */)
    : QToolButton(parent)
    , m_app(app)
{
}

QSize CWizButton::sizeHint() const
{
    return QSize(24, 24);
}

void CWizButton::setAction(QAction* action)
{
    QString strIconName = action->objectName() + "_active";
    m_iconPressed = ::WizLoadSkinIcon(m_app.userSettings().skin(), strIconName);

    setDefaultAction(action);
}

void CWizButton::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    p.drawPixmap(0, 0, icon().pixmap(sizeHint().width(), sizeHint().height()));
}

void CWizButton::mousePressEvent(QMouseEvent* event)
{
    // if animation is on-going, return
    if (defaultAction()->property("animationStatus").toBool()) {
        return;
    }

    if (m_iconDefault.isNull()) {
        m_iconDefault = icon();
        setIcon(m_iconPressed);
    }

    QToolButton::mousePressEvent(event);
}

void CWizButton::leaveEvent(QEvent *event)
{
    if (!m_iconDefault.isNull()) {
        setIcon(m_iconDefault);
        m_iconDefault = QIcon();
    }

    QToolButton::leaveEvent(event);
}
