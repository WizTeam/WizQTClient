#include "wizanimateaction.h"

#include "utils/stylehelper.h"

#include "wizmisc.h"
#include "wizsettings.h"

#include <QTimer>
#include <QAction>
#include <QToolButton>

CWizAnimateAction::CWizAnimateAction(QObject* parent)
    : QObject(parent)
    , m_target(NULL)
    , m_nIconIndex(-1)
    , m_timer(new QTimer())
{
    connect(m_timer, SIGNAL(timeout()), SLOT(on_timer_timeout()));
    m_timer->setInterval(100);
}

void CWizAnimateAction::setAction(QAction* action)
{
    m_target = new CWizAnimateActionContainer(action, this);
    m_iconDefault = m_target->icon();
}

void CWizAnimateAction::setToolButton(QToolButton* button)
{
    m_target = new CWizAnimateButtonContainer(button, this);
    m_iconDefault = m_target->icon();
}

void CWizAnimateAction::setSingleIcons(const QString& strIconBaseName)
{
    int index = 1;
    while (1)
    {
        CString strFileName = ::WizGetSkinResourceFileName(Utils::StyleHelper::themeName(), strIconBaseName + WizIntToStr(index));
        if (strFileName.isEmpty())
            return;
        QIcon icon(strFileName);
        if (icon.isNull())
            return;

        m_icons.push_back(icon);

        index++;
    }
}

void CWizAnimateAction::setTogetherIcon(const QString& strIconBaseName)
{
    CString strFileName;
    if (WizIsHighPixel()) {
        strFileName  = ::WizGetSkinResourceFileName(Utils::StyleHelper::themeName(), strIconBaseName + "@2x");
    } else {
        strFileName  = ::WizGetSkinResourceFileName(Utils::StyleHelper::themeName(), strIconBaseName);
    }

    if (strFileName.IsEmpty())
        return;

    QPixmap pix(strFileName);
    int startX = 0;
    int pixHeight = pix.height();
    while (1)
    {
        QPixmap p =  pix.copy(startX, 0, pixHeight, pixHeight);
        if (p.isNull())
            return;

        QIcon icon(p);
        m_icons.push_back(icon);

        startX += pixHeight;
        if (startX >= pix.width())
            return;
    }
}

void CWizAnimateAction::nextIcon()
{
    if (!m_target)
        return;

    m_nIconIndex++;

    int iconCount = m_icons.size();
    if (iconCount <= 0)
        return;

    int index = m_nIconIndex % iconCount;

    m_target->setIcon(m_icons.at(index));
}

bool CWizAnimateAction::isPlaying()
{
    return m_timer->isActive();
}

void CWizAnimateAction::startPlay()
{
    if (!m_target)
        return;

    // for other class to determine animation status
    m_target->setProperty("animationStatus", 1);

    m_nIconIndex = -1;

    nextIcon();

    m_timer->start();
}

void CWizAnimateAction::stopPlay()
{
    if (!m_target)
        return;

    m_target->setProperty("animationStatus", 0);

    m_target->setIcon(m_iconDefault);
    m_timer->stop();
}

void CWizAnimateAction::on_timer_timeout()
{
    nextIcon();
}


CWizAnimateActionContainer::CWizAnimateActionContainer(QAction* action, QObject* parent)
    : m_action(action)
    , CWizAnimateContainerBase(parent)
{

}

QIcon CWizAnimateActionContainer::icon()
{
    return m_action->icon();
}

void CWizAnimateActionContainer::setIcon(const QIcon& icon)
{
    m_action->setIcon(icon);
}

bool CWizAnimateActionContainer::setProperty(const char* name, const QVariant& value)
{
    return m_action->setProperty(name, value);
}


CWizAnimateContainerBase::CWizAnimateContainerBase(QObject* parent)
    : QObject(parent)
{

}


CWizAnimateButtonContainer::CWizAnimateButtonContainer(QToolButton* button, QObject* parent)
    : m_button(button)
    , CWizAnimateContainerBase(parent)
{

}

QIcon CWizAnimateButtonContainer::icon()
{
    return m_button->icon();
}

void CWizAnimateButtonContainer::setIcon(const QIcon& icon)
{
    m_button->setIcon(icon);
}

bool CWizAnimateButtonContainer::setProperty(const char* name, const QVariant& value)
{
    return m_button->setProperty(name, value);
}
