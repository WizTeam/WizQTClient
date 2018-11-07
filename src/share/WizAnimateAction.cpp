#include "WizAnimateAction.h"

#include "utils/WizStyleHelper.h"

#include "WizMisc.h"
#include "WizSettings.h"
#include "WizUIBase.h"

#include <QTimer>
#include <QAction>
#include <QToolButton>

WizAnimateAction::WizAnimateAction(QObject* parent)
    : QObject(parent)
    , m_target(NULL)
    , m_nIconIndex(-1)
    , m_timer(new QTimer())
{
    connect(m_timer, SIGNAL(timeout()), SLOT(on_timer_timeout()));
    m_timer->setInterval(100);
}

void WizAnimateAction::setAction(QAction* action)
{
    m_target = new WizAnimateActionContainer(action, this);
}

void WizAnimateAction::setToolButton(QToolButton* button)
{
    m_target = new WizAnimateButtonContainer(button, this);
}

void WizAnimateAction::setSingleIcons(const QString& strIconBaseName, QSize size)
{
    QString themeName = Utils::WizStyleHelper::themeName();
    int index = 1;
    while (1)
    {
        QString iconName = strIconBaseName + WizIntToStr(index);
        QIcon icon = WizLoadSkinIcon(themeName, iconName, size);
        if (icon.isNull())
            return;
        //
        m_icons.push_back(icon);

        index++;
    }
}

void WizAnimateAction::setTogetherIcon(const QString& strIconBaseName)
{
    CString strFileName;
    if (WizIsHighPixel()) {
        strFileName  = ::WizGetSkinResourceFileName(Utils::WizStyleHelper::themeName(), strIconBaseName + "@2x");
    } else {
        strFileName  = ::WizGetSkinResourceFileName(Utils::WizStyleHelper::themeName(), strIconBaseName);
    }

    if (strFileName.isEmpty())
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

void WizAnimateAction::nextIcon()
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

bool WizAnimateAction::isPlaying()
{
    return m_timer->isActive();
}

void WizAnimateAction::startPlay()
{
    if (!m_target)
        return;

    // for other class to determine animation status
    m_target->setProperty("animationStatus", 1);

    m_nIconIndex = -1;

    nextIcon();

    m_timer->start();
}

void WizAnimateAction::stopPlay()
{
    if (!m_target)
        return;

    m_target->setProperty("animationStatus", 0);

    m_target->setIcon(m_target->icon());
    m_timer->stop();
}

void WizAnimateAction::on_timer_timeout()
{
    nextIcon();
}


WizAnimateActionContainer::WizAnimateActionContainer(QAction* action, QObject* parent)
    : m_action(action)
    , WizAnimateContainerBase(parent)
{

}

QIcon WizAnimateActionContainer::icon()
{
    return m_action->icon();
}

void WizAnimateActionContainer::setIcon(const QIcon& icon)
{
    m_action->setIcon(icon);
}

bool WizAnimateActionContainer::setProperty(const char* name, const QVariant& value)
{
    return m_action->setProperty(name, value);
}


WizAnimateContainerBase::WizAnimateContainerBase(QObject* parent)
    : QObject(parent)
{

}


WizAnimateButtonContainer::WizAnimateButtonContainer(QToolButton* button, QObject* parent)
    : m_button(button)
    , WizAnimateContainerBase(parent)
{

}

QIcon WizAnimateButtonContainer::icon()
{
    return m_button->icon();
}

void WizAnimateButtonContainer::setIcon(const QIcon& icon)
{
    m_button->setIcon(icon);
}

bool WizAnimateButtonContainer::setProperty(const char* name, const QVariant& value)
{
    return m_button->setProperty(name, value);
}
