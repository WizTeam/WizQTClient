#include "WizActions.h"
#include <QMenuBar>
#include <QKeySequence>
#include <QShortcut>
#include <QWidgetAction>
#include "share/WizMisc.h"
#include "share/WizSettings.h"
#include "share/WizAnimateAction.h"
#include "widgets/WizTableSelector.h"
#include "share/WizUIBase.h"

struct WIZACTION
{
    QString strName;
    QString strText;
    QString strText2;
    QKeySequence strShortcut;
};


WizActions::WizActions(WizExplorerApp& app, QObject* parent)
    : QObject(parent)
    , m_parent(parent)
    , m_app(app)
{
}

WIZACTION* WizActions::actionsData()
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
        {"actionInsert", QObject::tr("Insert")},
        {"actionCategoryOption", QObject::tr("Category Option")},
        {"actionSortBy", QObject::tr("Sort By")}
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
        {"actionNewNoteByTemplate",             QObject::tr("New Note by Template..."),      "",          QKeySequence()},
        {WIZACTION_GLOBAL_SAVE_AS_PDF,      QObject::tr("Save as PDF..."),             "",      QKeySequence()},
        {WIZACTION_GLOBAL_SAVE_AS_HTML,      QObject::tr("Save as Html..."),          "",      QKeySequence()},
        {WIZACTION_GLOBAL_SAVE_AS_MARKDOWN,      QObject::tr("Save as Markdown..."),          "",      QKeySequence()},
        {WIZACTION_GLOBAL_IMPORT_FILE,      QObject::tr("Import Files..."),          "",      QKeySequence()},
        {WIZACTION_GLOBAL_PRINT_MARGIN,      QObject::tr("PDF Page Margins..."),   "",     QKeySequence()},
        //{WIZACTION_GLOBAL_VIEW_MESSAGES,    QObject::tr("View messages"),     "",       QKeySequence()},
        {WIZACTION_GLOBAL_GOBACK,                    QObject::tr("Back"),                        "",       QKeySequence()},
        {WIZACTION_GLOBAL_GOFORWARD,                 QObject::tr("Forward"), "",                   QKeySequence()},
        {"actionConsole",                              QObject::tr("Console..."),                                   "",      QKeySequence()},
        {"actionSearch",                                QObject::tr("Search Note"),                             "",         QKeySequence("Ctrl+Alt+F")},
        {"actionResetSearch",                       QObject::tr("Reset Search"),                           "",         QKeySequence("Ctrl+R")},
        {"actionFeedback",                            QObject::tr("User Feedback..."),                      "",         QKeySequence()},
        {"actionSupport",                               QObject::tr("User Support..."),                         "",         QKeySequence()},
        {"actionManual",                                QObject::tr("User Manual..."),                        "",          QKeySequence()},
        {WIZACTION_EDITOR_FIND_REPLACE,        QObject::tr("Find and Replace..."), "",          QKeySequence("Ctrl+F")},

        // editing
        {WIZACTION_EDITOR_UNDO,                  QObject::tr("Undo"),        "",       QKeySequence()},
        {WIZACTION_EDITOR_REDO,                   QObject::tr("Redo"),       "",        QKeySequence()},
        {WIZACTION_EDITOR_CUT,                       QObject::tr("Cut"),         "",        QKeySequence()},
        {WIZACTION_EDITOR_COPY,                    QObject::tr("Copy"),      "",        QKeySequence()},
        {WIZACTION_EDITOR_PASTE,                  QObject::tr("Paste"),      "",        QKeySequence()},
        {WIZACTION_EDITOR_PASTE_PLAIN,      QObject::tr("Paste as Plain Text"), "", QKeySequence("Shift+Ctrl+V")},
        {WIZACTION_EDITOR_DELETE,                QObject::tr("Delete"),    "",          QKeySequence()},
        {WIZACTION_EDITOR_SELECT_ALL,        QObject::tr("Select All"), "",         QKeySequence()},

        // view
        {WIZACTION_GLOBAL_TOGGLE_CATEGORY,      QObject::tr("Hide Sidebar"),   QObject::tr("Show Sidebar"),    QKeySequence("Alt+Ctrl+S")},
        {WIZACTION_GLOBAL_SHOW_SUB_FOLDER_DOCUMENTS,      QObject::tr("Show sub folder documents"),   QObject::tr(""),    QKeySequence("")},
        {WIZACTION_GLOBAL_TOGGLE_FULLSCREEN,    QObject::tr("Enter Fullscreen"),       QObject::tr("Leave Fullscreen"),         QKeySequence("Ctrl+Meta+f")},

        {"actionViewMinimize",                                               QObject::tr("Minimize"),       QObject::tr(""),         QKeySequence("Ctrl+M")},
        {"actionZoom",                                                             QObject::tr("Zoom"),          QObject::tr(""),         QKeySequence()},
        {"actionBringFront",                                                     QObject::tr("Bring All to Front"),          QObject::tr(""),         QKeySequence()},

        //view
        {WIZCATEGORY_OPTION_MESSAGECENTER,                         QObject::tr("Message Center"),   QObject::tr(""), QKeySequence()},
        {WIZCATEGORY_OPTION_SHORTCUTS,                                   QObject::tr("Shortcuts"),              QObject::tr(""), QKeySequence()},
        {WIZCATEGORY_OPTION_QUICKSEARCH,                               QObject::tr("QuickSearch"),        QObject::tr(""), QKeySequence()},
        {WIZCATEGORY_OPTION_FOLDERS,                                         QObject::tr("Folders"),                 QObject::tr(""), QKeySequence()},
        {WIZCATEGORY_OPTION_TAGS,                                                QObject::tr("Tags"),                     QObject::tr(""), QKeySequence()},
        {WIZCATEGORY_OPTION_BIZGROUPS,                                     QObject::tr("Biz Groups"),           QObject::tr(""), QKeySequence()},
        {WIZCATEGORY_OPTION_PERSONALGROUPS,                       QObject::tr("Personal Groups"),  QObject::tr(""), QKeySequence()},
        {WIZCATEGORY_OPTION_THUMBNAILVIEW,                             QObject::tr("Thumbnail View"),    QObject::tr(""), QKeySequence()},
        {WIZCATEGORY_OPTION_TWOLINEVIEW,                                 QObject::tr("Two Line View"),     QObject::tr(""), QKeySequence()},
        {WIZCATEGORY_OPTION_ONELINEVIEW,                                 QObject::tr("One Line View"),      QObject::tr(""), QKeySequence()},
        {WIZDOCUMENT_SORTBY_CREATEDTIME,                               QObject::tr("Sort by Created Time"),    QObject::tr(""), QKeySequence()},
        {WIZDOCUMENT_SORTBY_UPDATEDTIME,                               QObject::tr("Sort by Updated Time"),    QObject::tr(""), QKeySequence()},
        {WIZDOCUMENT_SORTBY_ACCESSTIME,                                 QObject::tr("Sort by Access Time"),    QObject::tr(""), QKeySequence()},
        {WIZDOCUMENT_SORTBY_TITLE,                                               QObject::tr("Sort by Title"),    QObject::tr(""), QKeySequence()},
        {WIZDOCUMENT_SORTBY_FOLDER,                                          QObject::tr("Sort by Folder"),    QObject::tr(""), QKeySequence()},
        {WIZDOCUMENT_SORTBY_SIZE,                                                 QObject::tr("Sort by Size"),    QObject::tr(""), QKeySequence()},

        // format
        {WIZACTION_FORMAT_JUSTIFYLEFT,               QObject::tr("Justify Left"),                "",           QKeySequence("Ctrl+[")},
        {WIZACTION_FORMAT_JUSTIFYRIGHT,             QObject::tr("Justify Right"),              "",          QKeySequence("Ctrl+]")},
        {WIZACTION_FORMAT_JUSTIFYCENTER,          QObject::tr("Justify Center"),           "",          QKeySequence("Ctrl+=")},
        {WIZACTION_FORMAT_JUSTIFYJUSTIFY,          QObject::tr("Justify Both Side"),      "",           QKeySequence()},
        {WIZACTION_FORMAT_INDENT,                          QObject::tr("Indent"),                    "",            QKeySequence()},
        {WIZACTION_FORMAT_OUTDENT,                      QObject::tr("Outdent"),                   "",           QKeySequence()},
        {WIZACTION_FORMAT_UNORDEREDLIST,         QObject::tr("Convert to Unoredered List"), "", QKeySequence("Ctrl+Alt+U")},
        {WIZACTION_FORMAT_ORDEREDLIST,              QObject::tr("Convert to Ordered List"), "",       QKeySequence("Ctrl+Alt+O")},
        {WIZACTION_FORMAT_INSERT_LINK,                 QObject::tr("Insert Link"),                  "",          QKeySequence("Ctrl+K")},
        {WIZACTION_FORMAT_BOLD,                              QObject::tr("Bold"),                       "",             QKeySequence("Ctrl+B")},
        {WIZACTION_FORMAT_ITALIC,                             QObject::tr("Italic"),                        "",           QKeySequence("Ctrl+I")},
        {WIZACTION_FORMAT_UNDERLINE,                   QObject::tr("Underline"),                "",           QKeySequence("Ctrl+U")},
        {WIZACTION_FORMAT_STRIKETHROUGH,         QObject::tr("Strike Through"),         "",           QKeySequence("Ctrl+Alt+K")},
        {WIZACTION_FORMAT_INSERT_HORIZONTAL,  QObject::tr("Insert Horizontal"),       "",           QKeySequence("Shift+Ctrl+H")},
        {WIZACTION_FORMAT_INSERT_DATE,               QObject::tr("Insert Date"),                "",           QKeySequence("Shift+Ctrl+D")},
        {WIZACTION_FORMAT_INSERT_TIME,                QObject::tr("Insert Time"),               "",           QKeySequence("Shift+Ctrl+Alt+D")},
        {WIZACTION_FORMAT_INSERT_CHECKLIST,    QObject::tr("Insert Check List"),        "",           QKeySequence("Ctrl+O")},
        {WIZACTION_FORMAT_INSERT_CODE,              QObject::tr("Insert Code"),               "",           QKeySequence("Shift+Ctrl+C")},
        {WIZACTION_FORMAT_INSERT_IMAGE,             QObject::tr("Insert Image"),            "",           QKeySequence("Shift+Ctrl+I")},
        {WIZACTION_FORMAT_REMOVE_FORMAT,        QObject::tr("Remove Format"),       "",           QKeySequence()},
        {WIZACTION_FORMAT_SCREEN_SHOT,             QObject::tr("Screen Shot..."),           "",           QKeySequence()},


        {"", "", "", QKeySequence()}
    };

    return arrayActions;
}

const WizIconOptions ICON_OPTIONS = WizIconOptions(Qt::transparent, "#a6a6a6", Qt::transparent);

WizShortcutAction *WizActions::addAction(WIZACTION& action, bool bUseExtraShortcut)
{   
    QString strText = action.strText;
    QString strIconName = action.strName;
    QKeySequence strShortcut = action.strShortcut;
    QString strSlot = "1on_" + action.strName + "_triggered()";

    WizShortcutAction* pAction = new WizShortcutAction(strText, m_parent);

    if (!strIconName.isEmpty()) {
        pAction->setIcon(::WizLoadSkinIcon(m_app.userSettings().skin(), strIconName, QSize(), ICON_OPTIONS));
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

void WizActions::init(bool bUseExtraShortcut)
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

WizShortcutAction *WizActions::actionFromName(const QString& strActionName)
{
    WizShortcutAction* pAction = m_actions[strActionName];
    if (pAction) {
        return pAction;
    }

    WIZACTION data = {strActionName, strActionName};

    return addAction(data, false);
}

void WizActions::toggleActionText(const QString& strActionName)
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

WizAnimateAction* WizActions::animateActionFromName(const QString& strActionName)
{
    return dynamic_cast<WizAnimateAction*>(actionFromName(strActionName));
}

QMenu* WizActions::toMenu(QWidget* parent, WizSettings& settings, const QString& strSection)
{
    if (strSection == "Table")
    {
        QWidgetAction* tableAction = new QWidgetAction(parent);
        tableAction->setText(QObject::tr("Table"));
        WizTableSelectorWidget* tableWidget = new WizTableSelectorWidget(parent);
        tableAction->setDefaultWidget(tableWidget);
        //
        connect(tableWidget, SIGNAL(itemSelected(int,int)), SIGNAL(insertTableSelected(int,int)));
        //
        QMenu* menuTable = new QMenu(parent);
        menuTable->setTitle(tableAction->text());
        menuTable->addAction(tableAction);
        //
        return menuTable;
    }
    else
    {
        QMenu* pMenu = new QMenu(parent);
        QString strLocalText = QObject::tr(strSection.toUtf8());

        pMenu->setTitle(strLocalText);
        buildMenu(pMenu, settings, strSection, false);

        return pMenu;
    }
}

void WizActions::buildMenu(QMenu* pMenu, WizSettings& settings, const QString& strSection, bool bMenuBar)
{
    int index = 0;
    while (true)
    {
        QString strKey = WizIntToStr(index);
        QString strAction = settings.getString(strSection, strKey);

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

void WizActions::buildMenuBar(QMenuBar* menuBar, const QString& strFileName, QMenu*& windowsMenu)
{
    WizSettings settings(strFileName);

    int index = 0;
    while (true)
    {
        QString strKey = WizIntToStr(index);
        QString strAction = settings.getString("MainMenu", strKey);

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

void WizActions::buildMenu(QMenu* menu, const QString& strFileName)
{
    WizSettings settings(strFileName);

    int index = 0;
    while (true)
    {
        QString strKey = WizIntToStr(index);
        QString strAction = settings.getString("MainMenu", strKey);

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


void WizShortcutAction::setShortcut(QShortcut *shortcut)
{
    m_shortcut = shortcut;
}

void WizShortcutAction::setShortcut(const QKeySequence &shortcut)
{
    QAction::setShortcut(shortcut);
}

void WizShortcutAction::setEnabled(bool enable)
{
    QAction::setEnabled(enable);
    if (m_shortcut)
    {
        m_shortcut->setEnabled(enable);
    }
}
