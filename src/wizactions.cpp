#include "wizactions.h"
#include <QMenuBar>
#include <QKeySequence>
#include <QShortcut>
#include "share/wizmisc.h"
#include "share/wizsettings.h"
#include "share/wizanimateaction.h"

struct WIZACTION
{
    QString strName;
    QString strText;
    QString strText2;
    QKeySequence strShortcut;
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
    #ifdef Q_OS_LINUX
        {"actionFile", QObject::tr("&File")},
        {"actionEdit", QObject::tr("&Edit")},
        {"actionView", QObject::tr("&View")},
        {"actionFormat", QObject::tr("For&mat")},
        {"actionTools", QObject::tr("&Tools")},
        {"actionWindow", QObject::tr("&Window")},
        {"actionHelp", QObject::tr("&Help")},
    #else
        // root
        {"actionFile", QObject::tr("File")},
        {"actionEdit", QObject::tr("Edit")},
        {"actionView", QObject::tr("View")},
        {"actionFormat", QObject::tr("Format")},
        {"actionTools", QObject::tr("Tools")},
        {"actionWindow", QObject::tr("Window")},
        {"actionHelp", QObject::tr("Help")},
    #endif

        // sub
        {"actionText", QObject::tr("Text")},
        {"actionList", QObject::tr("List")},
        {"actionTable", QObject::tr("Table")},
        {"actionLink", QObject::tr("Link")},
        {"actionStyle", QObject::tr("Style")},
        {"actionInsert", QObject::tr("Insert")}
    };

    Q_UNUSED(arrayRoot);

    static WIZACTION arrayActions[] =
    {
        {"actionPreference",                                 QObject::tr("Preference..."),               "",              QKeySequence()},
        {"actionAbout",                                         QObject::tr("About WizNote..."),         "",              QKeySequence()},
        {"actionExit",                                            QObject::tr("Exit"),                              "",               QKeySequence("Ctrl+Q")},
        {"actionClose",                                         QObject::tr("Close"),                             "",            QKeySequence("Ctrl+W")},
        {"actionLogout",                                        QObject::tr("Logout..."),                       "",            QKeySequence()},
        {WIZACTION_GLOBAL_SYNC,              QObject::tr("Sync"),                             "",             QKeySequence()},
        {WIZACTION_GLOBAL_NEW_DOCUMENT,     QObject::tr("New Note"),          "",             QKeySequence("Ctrl+N")},
        {"actionNewNoteByTemplate",             QObject::tr("New Note by template..."),      "",          QKeySequence()},
        {WIZACTION_GLOBAL_SAVE_AS_PDF,      QObject::tr("Save as PDF..."),             "",      QKeySequence()},
        {WIZACTION_GLOBAL_SAVE_AS_HTML,      QObject::tr("Save as Html..."),          "",      QKeySequence()},
        {WIZACTION_GLOBAL_IMPORT_FILE,      QObject::tr("Import Files..."),          "",      QKeySequence()},
        {WIZACTION_GLOBAL_PRINT,                    QObject::tr("Print..."),                           "",      QKeySequence("Ctrl+P")},
        {WIZACTION_GLOBAL_PRINT_MARGIN,      QObject::tr("Print page margins..."),   "",     QKeySequence()},
        //{WIZACTION_GLOBAL_VIEW_MESSAGES,    QObject::tr("View messages"),     "",       QKeySequence()},
        {WIZACTION_GLOBAL_GOBACK,                    QObject::tr("Back"),                        "",       QKeySequence()},
        {WIZACTION_GLOBAL_GOFORWARD,                 QObject::tr("Forward"), "",                   QKeySequence()},
        {"actionConsole",                              QObject::tr("Console..."),                                   "",      QKeySequence()},
        {"actionRebuildFTS",                        QObject::tr("Rebuild full text search index"),    "",        QKeySequence()},
        {"actionSearch",                                QObject::tr("Search note"),                             "",         QKeySequence("Alt+Ctrl+F")},
        {"actionAdvancedSearch",                QObject::tr("Advanced search"),                     "",         QKeySequence("Alt+Ctrl+A")},
        {"actionResetSearch",                       QObject::tr("Reset search"),                           "",         QKeySequence("Ctrl+R")},
        {"actionFeedback",                            QObject::tr("User feedback..."),                      "",         QKeySequence()},
        {"actionSupport",                               QObject::tr("User support..."),                         "",         QKeySequence()},
        {"actionManual",                                QObject::tr("User manual..."),                        "",          QKeySequence()},
        {WIZACTION_EDITOR_FIND_REPLACE,        QObject::tr("Find and replace..."), "",          QKeySequence("Ctrl+F")},

        // editing
        {WIZACTION_EDITOR_UNDO,                  QObject::tr("Undo"),        "",       QKeySequence("Ctrl+Z")},
        {WIZACTION_EDITOR_REDO,                   QObject::tr("Redo"),       "",        QKeySequence("Shift+Ctrl+Z")},
        {WIZACTION_EDITOR_CUT,                       QObject::tr("Cut"),         "",        QKeySequence("Ctrl+X")},
        {WIZACTION_EDITOR_COPY,                    QObject::tr("Copy"),      "",        QKeySequence("Ctrl+C")},
        {WIZACTION_EDITOR_PASTE,                  QObject::tr("Paste"),      "",        QKeySequence("Ctrl+V")},
        {WIZACTION_EDITOR_PASTE_PLAIN,      QObject::tr("Paste as plain text"), "", QKeySequence("Shift+Ctrl+V")},
        {WIZACTION_EDITOR_DELETE,                QObject::tr("Delete"),    "",          QKeySequence()},
        {WIZACTION_EDITOR_SELECT_ALL,        QObject::tr("Select all"), "",         QKeySequence("Ctrl+A")},

#ifdef USEWEBENGINE
        {"actionMoveToPageStart",          QObject::tr("Move to page start"),     "",     QKeySequence(QKeySequence::MoveToStartOfDocument)},
        {"actionMoveToPageEnd",           QObject::tr("Move to page end"),      "",     QKeySequence(QKeySequence::MoveToEndOfDocument)},
    #ifdef Q_OS_MAC
        {"actionMoveToLineStart",            QObject::tr("Move to line start"),       "",     QKeySequence(QKeySequence::MoveToStartOfLine)},
        {"actionMoveToLineEnd",             QObject::tr("Move to line end"),        "",     QKeySequence(QKeySequence::MoveToEndOfLine)},
//        {"actionMoveToLineEnd",             QObject::tr("Move to line end"),        "",     QKeySequence(QKeySequence::MoveToNextLine)},
//        {"actionMoveToLineEnd",             QObject::tr("Move to line end"),        "",     QKeySequence(QKeySequence::MoveToNextPage)},
//        {"actionMoveToLineEnd",             QObject::tr("Move to line end"),        "",     QKeySequence(QKeySequence::MoveToPreviousLine)},
//        {"actionMoveToLineEnd",             QObject::tr("Move to line end"),        "",     QKeySequence(QKeySequence::MoveToPreviousPage)},

    #endif
#endif

        // view
        {WIZACTION_GLOBAL_TOGGLE_CATEGORY,      QObject::tr("Hide Sidebar"),   QObject::tr("Show Sidebar"),    QKeySequence("Alt+Ctrl+S")},
        {WIZACTION_GLOBAL_TOGGLE_FULLSCREEN,    QObject::tr("Enter Fullscreen"),       QObject::tr("Leave Fullscreen"),         QKeySequence("Ctrl+Meta+f")},


        {"actionViewMinimize",                                               QObject::tr("Minimize"),       QObject::tr(""),         QKeySequence("Ctrl+M")},
        {"actionZoom",                                                             QObject::tr("Zoom"),          QObject::tr(""),         QKeySequence()},
        {"actionBringFront",                                                     QObject::tr("Bring All to Front"),          QObject::tr(""),         QKeySequence()},

        // format
        {WIZACTION_FORMAT_JUSTIFYLEFT,               QObject::tr("Justify left"),                "",           QKeySequence("Ctrl+[")},
        {WIZACTION_FORMAT_JUSTIFYRIGHT,             QObject::tr("Justify right"),              "",          QKeySequence("Ctrl+]")},
        {WIZACTION_FORMAT_JUSTIFYCENTER,          QObject::tr("Justify center"),           "",          QKeySequence("Ctrl+=")},
        {WIZACTION_FORMAT_JUSTIFYJUSTIFY,          QObject::tr("Justify both side"),      "",           QKeySequence()},
        {WIZACTION_FORMAT_INDENT,                          QObject::tr("Indent"),                    "",            QKeySequence()},
        {WIZACTION_FORMAT_OUTDENT,                      QObject::tr("Outdent"),                   "",           QKeySequence()},
        {WIZACTION_FORMAT_UNORDEREDLIST,         QObject::tr("Convert to unoredered list"), "", QKeySequence("Ctrl+Alt+U")},
        {WIZACTION_FORMAT_ORDEREDLIST,              QObject::tr("Convert to ordered list"), "",       QKeySequence("Ctrl+Alt+O")},
        {WIZACTION_FORMAT_INSERT_TABLE,             QObject::tr("Insert table"),                "",           QKeySequence()},
        {WIZACTION_FORMAT_INSERT_LINK,                 QObject::tr("Insert link"),                  "",          QKeySequence("Ctrl+K")},
        {WIZACTION_FORMAT_BOLD,                              QObject::tr("Bold"),                       "",             QKeySequence("Ctrl+B")},
        {WIZACTION_FORMAT_ITALIC,                             QObject::tr("Italic"),                        "",           QKeySequence("Ctrl+I")},
        {WIZACTION_FORMAT_UNDERLINE,                   QObject::tr("Underline"),                "",           QKeySequence("Ctrl+U")},
        {WIZACTION_FORMAT_STRIKETHROUGH,         QObject::tr("Strike through"),         "",           QKeySequence("Ctrl+Alt+K")},
        {WIZACTION_FORMAT_INSERT_HORIZONTAL,  QObject::tr("Insert horizontal"),       "",           QKeySequence("Shift+Ctrl+H")},
        {WIZACTION_FORMAT_INSERT_DATE,               QObject::tr("Insert date"),                "",           QKeySequence("Shift+Ctrl+D")},
        {WIZACTION_FORMAT_INSERT_TIME,                QObject::tr("Insert time"),               "",           QKeySequence("Shift+Ctrl+Alt+D")},
        {WIZACTION_FORMAT_INSERT_CHECKLIST,    QObject::tr("Insert check list"),        "",           QKeySequence("Ctrl+O")},
        {WIZACTION_FORMAT_INSERT_CODE,              QObject::tr("Insert code"),               "",           QKeySequence("Shift+Ctrl+C")},
        {WIZACTION_FORMAT_INSERT_IMAGE,             QObject::tr("Insert image"),            "",           QKeySequence("Shift+Ctrl+I")},
        {WIZACTION_FORMAT_REMOVE_FORMAT,        QObject::tr("Remove format"),       "",           QKeySequence()},
        {WIZACTION_FORMAT_PLAINTEXT,                    QObject::tr("Convert to plain text"), "",           QKeySequence()},
        {WIZACTION_FORMAT_VIEW_SOURCE,             QObject::tr("View html source..."),  "",           QKeySequence()},
        {WIZACTION_FORMAT_SCREEN_SHOT,             QObject::tr("Screen shot..."),           "",           QKeySequence()},
        {"actionDeveloper",                       QObject::tr("Developer mode"), "", QKeySequence()},


        {"", "", "", QKeySequence()}
    };

    return arrayActions;
}

CWizShortcutAction *CWizActions::addAction(WIZACTION& action, bool bUseExtraShortcut)
{   
    QString strText = action.strText;
    QString strIconName = action.strName;
    QKeySequence strShortcut = action.strShortcut;
    QString strSlot = "1on_" + action.strName + "_triggered()";

    CWizShortcutAction* pAction = new CWizShortcutAction(strText, m_parent);

    if (!strIconName.isEmpty()) {
        pAction->setIcon(::WizLoadSkinIcon(m_app.userSettings().skin(), strIconName));
    }

    pAction->setShortcut(strShortcut);

    if (action.strName == "actionAbout")
        pAction->setMenuRole(QAction::AboutRole);
    else if (action.strName == "actionPreference")
        pAction->setMenuRole(QAction::PreferencesRole);
    else if (action.strName == "actionExit")
        pAction->setMenuRole(QAction::QuitRole);

    // Used for building menu from ini file
    pAction->setObjectName(action.strName);

    pAction->setShortcutContext(Qt::ApplicationShortcut);

    m_actions[action.strName] = pAction;

    if (bUseExtraShortcut && !strShortcut.isEmpty()) {
        QShortcut *shortcut  = new QShortcut(strShortcut, m_app.mainWindow());
        QObject::connect(shortcut, SIGNAL(activated()), m_parent, strSlot.toUtf8());
        pAction->setShortcut(shortcut);
    }

    QObject::connect(pAction, "2triggered()", m_parent, strSlot.toUtf8());

    return pAction;
}

void CWizActions::init(bool bUseExtraShortcut)
{
    int index = 0;
    WIZACTION* arrayData = actionsData();
    while (1)
    {
        WIZACTION& action = arrayData[index];
        if (action.strName.isEmpty())
            break;

        addAction(action, bUseExtraShortcut);
        index++;
    }
}

CWizShortcutAction *CWizActions::actionFromName(const QString& strActionName)
{
    CWizShortcutAction* pAction = m_actions[strActionName];
    if (pAction) {
        return pAction;
    }

    WIZACTION data = {strActionName, strActionName};

    return addAction(data, false);
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
    buildMenu(pMenu, settings, strSection, false);

    return pMenu;
}

void CWizActions::buildMenu(QMenu* pMenu, CWizSettings& settings, const QString& strSection, bool bMenuBar)
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

        if (!bMenuBar && (strAction == "actionPreference" || strAction == "actionExit")) {
            index++;
            continue;
        }

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

void CWizActions::buildMenuBar(QMenuBar* menuBar, const QString& strFileName, QMenu*& windowsMenu)
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

            // there is no shortcut for menubar on macosx
#ifdef Q_OS_MAC
            QString strLocalText = strAction;
            strLocalText = QObject::tr(strLocalText.remove('&').toUtf8());
#else
            QString strLocalText = QObject::tr(strAction.toUtf8());
#endif

            QMenu* pMenu = menuBar->addMenu(strLocalText);
            buildMenu(pMenu, settings, strAction, true);

            if (strAction.remove('&') == "Window")
            {
                windowsMenu = pMenu;
            }
        }
        else
        {
            menuBar->addAction(actionFromName(strAction));
        }

        index++;
    }
}

void CWizActions::buildMenu(QMenu* menu, const QString& strFileName)
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
            QMenu* pMenu = menu->addMenu(strLocalText);

            buildMenu(pMenu, settings, strAction, false);
        }
        else
        {
            menu->addAction(actionFromName(strAction));
        }

        index++;
    }

    QAction * actionQuit = actionFromName("actionExit");
    QAction* actionOptions = actionFromName("actionPreference");

    menu->addSeparator();
    menu->addAction(actionOptions);
    menu->addSeparator();
    menu->addAction(actionQuit);
}


void CWizShortcutAction::setShortcut(QShortcut *shortcut)
{
    m_shortcut = shortcut;
}

void CWizShortcutAction::setShortcut(const QKeySequence &shortcut)
{
    QAction::setShortcut(shortcut);
}

void CWizShortcutAction::setEnabled(bool enable)
{
    QAction::setEnabled(enable);
    if (m_shortcut)
    {
        m_shortcut->setEnabled(enable);
    }
}
