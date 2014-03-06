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
    QString strText2;
    QString strShortcut;
};


CWizActions::CWizActions(CWizExplorerApp& app, QObject* parent)
    : m_parent(parent)
    , m_app(app)
{
}

WIZACTION* CWizActions::actionsData()
{
    // useless, just for translation
    static WIZACTION arrayRoot[] =
    {
        // root
        {"actionFile", QObject::tr("&File")},
        {"actionEdit", QObject::tr("&Edit")},
        {"actionView", QObject::tr("&View")},
        {"actionFormat", QObject::tr("For&mat")},
        {"actionTools", QObject::tr("&Tools")},
        {"actionHelp", QObject::tr("&Help")},

        // sub
        {"actionText", QObject::tr("Text")},
        {"actionList", QObject::tr("List")},
        {"actionTable", QObject::tr("Table")},
        {"actionLink", QObject::tr("Link")},
        {"actionStyle", QObject::tr("Style")}
    };

    Q_UNUSED(arrayRoot);

    static WIZACTION arrayActions[] =
    {
        {"actionPreference",                QObject::tr("Preference"), "", ""},
        {"actionAbout",                     QObject::tr("About WizNote"), "", ""},
#ifdef QT_DEBUG
        {"actionAboutPlugins",              QObject::tr("About plugins"), "", ""},
#endif
        {"actionExit",                      QObject::tr("Exit"), "", ""},
        {"actionLogout",                    QObject::tr("Logout"), "", ""},
        {WIZACTION_GLOBAL_SYNC,             QObject::tr("Sync"), "", ""},
        {WIZACTION_GLOBAL_NEW_DOCUMENT,     QObject::tr("New Note"), "", "Ctrl+N"},
        //{WIZACTION_GLOBAL_VIEW_MESSAGES,    QObject::tr("View messages"), "", ""},
        {"actionGoBack",                    QObject::tr("Back"), "", ""},
        {"actionGoForward",                 QObject::tr("Forward"), "", ""},
        {"actionConsole",                   QObject::tr("Console"), "", ""},
        {"actionRebuildFTS",                QObject::tr("Rebuild full text search index"), "", ""},
        {"actionSearch",                    QObject::tr("Search note"), "", "Alt+Ctrl+F"},
        {"actionResetSearch",               QObject::tr("Reset search"), "", "Ctrl+R"},
        {"actionFeedback",                  QObject::tr("User support"), "", ""},

        // editing
        {WIZACTION_EDITOR_UNDO,             QObject::tr("Undo"), "", "Ctrl+Z"},
        {WIZACTION_EDITOR_REDO,             QObject::tr("Redo"), "", "Shift+Ctrl+Z"},
        {WIZACTION_EDITOR_CUT,              QObject::tr("Cut"), "", "Ctrl+X"},
        {WIZACTION_EDITOR_COPY,             QObject::tr("Copy"), "", "Ctrl+C"},
        {WIZACTION_EDITOR_PASTE,            QObject::tr("Paste"), "", "Ctrl+V"},
        {WIZACTION_EDITOR_PASTE_PLAIN,      QObject::tr("Paste as plain text"), "", "Shift+Ctrl+V"},
        {WIZACTION_EDITOR_DELETE,           QObject::tr("Delete"), "", ""},
        {WIZACTION_EDITOR_SELECT_ALL,       QObject::tr("Select all"), "", "Ctrl+A"},

        // view
        {WIZACTION_GLOBAL_TOGGLE_CATEGORY,      QObject::tr("Hide category view"), QObject::tr("Show category view"), "Ctrl+Alt+s"},
        {WIZACTION_GLOBAL_TOGGLE_FULLSCREEN,    QObject::tr("Enter Fullscreen"), QObject::tr("Leave Fullscreen"), "Ctrl+Meta+f"},

        // format
        {WIZACTION_FORMAT_JUSTIFYLEFT,          QObject::tr("Justify left"), "", "Ctrl+["},
        {WIZACTION_FORMAT_JUSTIFYRIGHT,         QObject::tr("Justify right"), "", "Ctrl+]"},
        {WIZACTION_FORMAT_JUSTIFYCENTER,        QObject::tr("Justify center"), "", "Ctrl+="},
        {WIZACTION_FORMAT_JUSTIFYJUSTIFY,       QObject::tr("Justify both side"), "", ""},
        {WIZACTION_FORMAT_INDENT,               QObject::tr("Indent"), "", ""},
        {WIZACTION_FORMAT_OUTDENT,              QObject::tr("Outdent"), "", ""},
        {WIZACTION_FORMAT_UNORDEREDLIST,        QObject::tr("Convert to unoredered list"), "", "Ctrl+Alt+U"},
        {WIZACTION_FORMAT_ORDEREDLIST,          QObject::tr("Convert to ordered list"), "", "Ctrl+Alt+O"},
        {WIZACTION_FORMAT_INSERT_TABLE,         QObject::tr("Insert table"), "", ""},
        {WIZACTION_FORMAT_INSERT_LINK,          QObject::tr("Insert link"), "", "Ctrl+K"},
        {WIZACTION_FORMAT_BOLD,                 QObject::tr("Bold"), "", "Ctrl+B"},
        {WIZACTION_FORMAT_ITALIC,               QObject::tr("Italic"), "", "Ctrl+I"},
        {WIZACTION_FORMAT_UNDERLINE,            QObject::tr("Underline"), "", "Ctrl+U"},
        {WIZACTION_FORMAT_STRIKETHROUGH,        QObject::tr("Strike through"), "", "Ctrl+Alt+K"},
        {WIZACTION_FORMAT_INSERT_HORIZONTAL,    QObject::tr("Insert horizontal"), "", "Shift+Ctrl+H"},
        {WIZACTION_FORMAT_INSERT_DATE,          QObject::tr("Insert date"), "", "Shift+Ctrl+D"},
        {WIZACTION_FORMAT_INSERT_TIME,          QObject::tr("Insert time"), "", "Shift+Ctrl+Alt+D"},
        {WIZACTION_FORMAT_REMOVE_FORMAT,        QObject::tr("Remove format"), "", ""},
        {WIZACTION_FORMAT_VIEW_SOURCE,          QObject::tr("View html source"), "", ""},

        {"", "", "", ""}
    };

    return arrayActions;
}

QAction* CWizActions::addAction(WIZACTION& action)
{   
    QString strText = action.strText;
    QString strIconName = action.strName;
    QString strShortcut = action.strShortcut;
    QString strSlot = "1on_" + action.strName + "_triggered()";

    QAction* pAction = new QAction(strText, m_parent);

    if (!strIconName.isEmpty()) {
        pAction->setIcon(::WizLoadSkinIcon(m_app.userSettings().skin(), strIconName));
    }

    if (!strShortcut.isEmpty()) {
        pAction->setShortcut(QKeySequence::fromString(strShortcut));
    }

    if (action.strName == "actionAbout")
        pAction->setMenuRole(QAction::AboutRole);
    else if (action.strName == "actionAboutPlugins")
        pAction->setMenuRole(QAction::ApplicationSpecificRole);
    else if (action.strName == "actionPreference")
        pAction->setMenuRole(QAction::PreferencesRole);
    else if (action.strName == "actionExit")
        pAction->setMenuRole(QAction::QuitRole);

    // Used for building menu from ini file
    pAction->setObjectName(action.strName);

    pAction->setShortcutContext(Qt::ApplicationShortcut);

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
    if (pAction) {
        return pAction;
    }

    WIZACTION data = {strActionName, strActionName};

    return addAction(data);
}

void CWizActions::toggleActionText(const QString& strActionName)
{
    int i = 0;
    WIZACTION* action;
    WIZACTION* arrayData = actionsData();
    while (1) {
        action = &arrayData[i];
        if (action->strName.isEmpty())
            return;

        if (action->strName == strActionName) {
            break;
        }

        i++;
    }

    if (action->strText2.isEmpty()) {
        return;
    }

    QAction* pAction = m_actions[strActionName];
    if (!pAction)
        return;

    if (pAction->text() == action->strText) {
        pAction->setText(action->strText2);
    } else {
        pAction->setText(action->strText);
    }
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

        // no fullscreen mode menu
#ifndef Q_OS_MAC
        if (strAction == WIZACTION_GLOBAL_TOGGLE_FULLSCREEN) {
            index++;
            continue;
        }
#endif

#ifndef QT_DEBUG
        if (strAction == "actionAboutPlugins") {
            index++;
            continue;
        }
#endif

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
