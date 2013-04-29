#include "wizanimateaction.h"

#include "wizmisc.h"
#include "wizsettings.h"

#include <QTimer>
#include <QAction>

CWizAnimateAction::CWizAnimateAction(CWizExplorerApp& app, QObject* parent)
    : QObject(parent)
    , m_app(app)
    , m_action(NULL)
    , m_nIconIndex(-1)
    , m_timer(new QTimer())
{
    connect(m_timer, SIGNAL(timeout()), SLOT(on_timer_timeout()));
    m_timer->setInterval(100);
}

void CWizAnimateAction::setAction(QAction* action)
{
    m_action = action;
    m_iconDefault = m_action->icon();
}

void CWizAnimateAction::setIcons(const QString& strIconBaseName)
{
    int index = 1;
    while (1)
    {
        CString strFileName = ::WizGetSkinResourceFileName(m_app.userSettings().skin(), strIconBaseName + WizIntToStr(index));
        if (strFileName.isEmpty())
            return;
        QIcon icon(strFileName);
        if (icon.isNull())
            return;

        m_icons.push_back(icon);

        index++;
    }
}

void CWizAnimateAction::nextIcon()
{
    if (!m_action)
        return;

    m_nIconIndex++;

    int iconCount = m_icons.size();
    if (iconCount <= 0)
        return;

    int index = m_nIconIndex % iconCount;

    m_action->setIcon(m_icons.at(index));
}

void CWizAnimateAction::startPlay()
{
    if (!m_action)
        return;

    // for other class to determine animation status
    m_action->setProperty("animationStatus", 1);

    m_nIconIndex = -1;

    nextIcon();

    m_timer->start();
}

void CWizAnimateAction::stopPlay()
{
    if (!m_action)
        return;

    m_action->setProperty("animationStatus", 0);

    m_action->setIcon(m_iconDefault);
    m_timer->stop();
}

void CWizAnimateAction::on_timer_timeout()
{
    nextIcon();
}
