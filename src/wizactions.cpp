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
        {"actionExit",                      QObject::tr("Exit"), "", ""},
        {"actionLogout",                    QObject::tr("Logout"), "", ""},
        //{"actionDeleteCurrentNote",       QObject::tr("Delete Note"), "", ""},
        {WIZACTION_GLOBAL_SYNC,             QObject::tr("Sync"), "", ""},
        {WIZACTION_GLOBAL_NEW_DOCUMENT,     QObject::tr("New Note"), "", "Ctrl+N"},
        {"actionGoBack",                    QObject::tr("Back"), "", ""},
        {"actionGoForward",                 QObject::tr("Forward"), "", ""},
        {"actionConsole",                   QObject::tr("Console"), "", ""},
        {"actionRebuildFTS",                QObject::tr("Rebuild full text search index"), "", ""},
        {"actionSearch",                    QObject::tr("Search document"), "", "Alt+Ctrl+F"},
        {"actionResetSearch",               QObject::tr("Reset search"), "", "Ctrl+R"},
        {WIZACTION_SWITCH_CATEGORY_PRIVATE,     QObject::tr("Switch to Private Notes"), "", ""},
        {WIZACTION_SWITCH_CATEGORY_TAGS,        QObject::tr("Switch to Tags"), "", ""},
        {WIZACTION_SWITCH_CATEGORY_GROUPS,      QObject::tr("Switch to Groups"), "", ""},

        // editing
        {"actionEditingUndo",               QObject::tr("Undo"), "", "Ctrl+Z"},
        {"actionEditingRedo",               QObject::tr("Redo"), "", "Shift+Ctrl+Z"},

        // view
        {WIZACTION_GLOBAL_TOGGLE_CATEGORY,      QObject::tr("Hide category view"), QObject::tr("Show category view"), "Ctrl+Alt+s"},
        {WIZACTION_GLOBAL_TOGGLE_FULLSCREEN,    QObject::tr("Enter Fullscreen"), QObject::tr("Leave Fullscreen"), "Ctrl+Meta+f"},

        // format
        {WIZACTION_FORMAT_JUSTIFYLEFT,      QObject::tr("Justify left"), "", "Ctrl+["},
        {WIZACTION_FORMAT_JUSTIFYRIGHT,     QObject::tr("Justify right"), "", "Ctrl+]"},
        {WIZACTION_FORMAT_JUSTIFYCENTER,    QObject::tr("Justify center"), "", "Ctrl+="},
        {WIZACTION_FORMAT_JUSTIFYJUSTIFY,   QObject::tr("Justify both side"), "", ""},
        {"actionFormatIndent",              QObject::tr("Indent"), "", ""},
        {"actionFormatOutdent",             QObject::tr("Outdent"), "", ""},
        {WIZACTION_FORMAT_UNORDEREDLIST,    QObject::tr("Convert to unoredered list"), "", "Ctrl+Alt+U"},
        {WIZACTION_FORMAT_ORDEREDLIST,      QObject::tr("Convert to ordered list"), "", "Ctrl+Alt+O"},
        {"actionFormatInsertTable",         QObject::tr("Insert table"), "", ""},
        {"actionFormatInsertLink",          QObject::tr("Insert link"), "", "Ctrl+K"},
        {WIZACTION_FORMAT_BOLD,             QObject::tr("Bold"), "", "Ctrl+B"},
        {WIZACTION_FORMAT_ITALIC,           QObject::tr("Italic"), "", "Ctrl+I"},
        {WIZACTION_FORMAT_UNDERLINE,        QObject::tr("Underline"), "", "Ctrl+U"},
        {"actionFormatStrikeThrough",       QObject::tr("Strike through"), "", "Ctrl+Alt+K"},
        {"actionFormatInsertHorizontal",    QObject::tr("Insert horizontal"), "", "Shift+Ctrl+H"},
        {"actionFormatInsertDate",          QObject::tr("Insert date"), "", "Shift+Ctrl+D"},
        {"actionFormatInsertTime",          QObject::tr("Insert time"), "", "Shift+Ctrl+Alt+D"},
        {"actionFormatRemoveFormat",        QObject::tr("Remove format"), "", ""},

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
    else if (action.strName == "actionPreference")
        pAction->setMenuRole(QAction::PreferencesRole);
    else if (action.strName == "actionExit")
        pAction->setMenuRole(QAction::QuitRole);

    // Used for building menu from ini file
    pAction->setObjectName(action.strName);

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
#ifndef Q_WS_MAC
        if (strAction == WIZACTION_GLOBAL_TOGGLE_FULLSCREEN) {
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


//void CWizActions::buildActionMenu(QAction* pAction, QWidget* parent, const QString& strFileName)
//{
//    CWizSettings settings(strFileName);
//    //
//    QMenu* pMenu = toMenu(parent, settings, pAction->objectName());
//    pAction->setMenu(pMenu);
//}
