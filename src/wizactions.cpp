#include "wizactions.h"
#include <QAction>
#include <QMenuBar>
#include "share/wizmisc.h"
#include "share/wizsettings.h"
#include "share/wizanimateaction.h"


struct WIZACTION
{
    QString strName;
    QString strText;
};



CWizActions::CWizActions(CWizExplorerApp& app, QObject* parent)
    : m_parent(parent)
    , m_app(app)
{
}

WIZACTION* CWizActions::actionsData()
{
    //for menu translations
    static QString arrayMenuText [] =
    {
        QObject::tr("&File"),
        QObject::tr("&Tools"),
        QObject::tr("&Help"),
    };

    Q_UNUSED(arrayMenuText);

    static WIZACTION arrayActions[] =
    {
#ifndef Q_OS_MAC
        {"actionPopupMainMenu",     QObject::tr("Menu")},
        {"actionPreference",         QObject::tr("Preference")},
        {"actionAbout",             QObject::tr("About WizNote...")},
#else
        // we don't have to translate these items on mac, qt will do it for us.
        {"actionPreference",         "Preference"},
        {"actionAbout",             "About WizNote..."},
#endif // Q_OS_MAC
        {"actionLogout",            QObject::tr("Logout")},
        {"actionExit",              QObject::tr("Exit")},
        {"actionDeleteCurrentNote", QObject::tr("Delete Note")},
        {"actionSync",              QObject::tr("Sync")},
        {"actionNewNote",           QObject::tr("New Note")},
        //{"actionCaptureScreen",     QObject::tr("Capture Screen")},
        //{"actionCaptureScreenShow", QObject::tr("Capture Screen")},
        //{"actionCaptureScreenHide", QObject::tr("Capture Screen (Hide WizNote)")},
        {"actionGoBack",            QObject::tr("Back")},
        {"actionGoForward",         QObject::tr("Forward")},
        {"actionConsole",         QObject::tr("Console")},

        {"",                        ""}
    };

    return arrayActions;
}

QAction* CWizActions::addAction(WIZACTION& action)
{
    QString strText = action.strText;
    QString strIconName = action.strName;
    QString strSlot = "1on_" + action.strName + "_triggered()";

    QAction* pAction = new QAction(strText, m_parent);

    // Used for building menu from ini file
    pAction->setObjectName(action.strName);

    if (!strIconName.isEmpty())
    {
        pAction->setIcon(::WizLoadSkinIcon(m_app.userSettings().skin(), strIconName));
    }

    m_actions[action.strName] = pAction;
    QObject::connect(pAction, "2triggered()", m_parent, strSlot.toUtf8());

    return pAction;
}

void CWizActions::init()
{
    int index = 0;
    WIZACTION* arrayData = actionsData();
    while (1)
    {
        WIZACTION& action = arrayData[index];
        if (action.strName.isEmpty())
            break;

        addAction(action);
        index++;
    }
}

QAction* CWizActions::actionFromName(const QString& strActionName)
{
    QAction* pAction = m_actions[strActionName];
    if (pAction)
        return pAction;

    WIZACTION data = {strActionName, strActionName};

    return addAction(data);
}

CWizAnimateAction* CWizActions::animateActionFromName(const QString& strActionName)
{
    return dynamic_cast<CWizAnimateAction*>(actionFromName(strActionName));
}

QMenu* CWizActions::toMenu(QWidget* parent, CWizSettings& settings, const QString& strSection)
{
    QMenu* pMenu = new QMenu(parent);
    QString strLocalText = QObject::tr(strSection.toUtf8());

    pMenu->setTitle(strLocalText);
    buildMenu(pMenu, settings, strSection);

    return pMenu;
}

void CWizActions::buildMenu(QMenu* pMenu, CWizSettings& settings, const QString& strSection)
{
    int index = 0;
    while (true)
    {
        QString strKey = WizIntToStr(index);
        QString strAction = settings.GetString(strSection, strKey);

        if (strAction.isEmpty())
            break;

        if (strAction.startsWith("-"))
        {
            pMenu->addSeparator();
        }
        else if (strAction.startsWith("+"))
        {
            strAction.remove(0, 1);
            pMenu->addMenu(toMenu(pMenu, settings, strAction));
        }
        else
        {
            pMenu->addAction(actionFromName(strAction));
        }

        index++;
    }
}

void CWizActions::buildMenuBar(QMenuBar* menuBar, const QString& strFileName)
{
    CWizSettings settings(strFileName);

    int index = 0;
    while (true)
    {
        QString strKey = WizIntToStr(index);
        QString strAction = settings.GetString("MainMenu", strKey);

        if (strAction.isEmpty())
            break;

        if (strAction.startsWith("-"))
        {
            continue;
        }
        else if (strAction.startsWith("+"))
        {
            strAction.remove(0, 1);
            QString strLocalText = QObject::tr(strAction.toUtf8());
            QMenu* pMenu = menuBar->addMenu(strLocalText);

            buildMenu(pMenu, settings, strAction);
        }
        else
        {
            menuBar->addAction(actionFromName(strAction));
        }

        index++;
    }
}


//void CWizActions::buildActionMenu(QAction* pAction, QWidget* parent, const QString& strFileName)
//{
//    CWizSettings settings(strFileName);
//    //
//    QMenu* pMenu = toMenu(parent, settings, pAction->objectName());
//    pAction->setMenu(pMenu);
//}
