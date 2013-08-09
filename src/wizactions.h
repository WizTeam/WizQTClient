#ifndef WIZACTIONS_H
#define WIZACTIONS_H

#include "wizdef.h"
#include "share/wizqthelper.h"

class QAction;
class QMenuBar;
class QMenu;
class CWizSettings;
class CWizAnimateAction;
struct WIZACTION;

#define WIZACTION_GLOBAL_SYNC               "actionSync"
#define WIZACTION_GLOBAL_NEW_DOCUMENT       "actionNewNote"
#define WIZACTION_GLOBAL_VIEW_MESSAGES      "actionViewMessages"
#define WIZACTION_GLOBAL_TOGGLE_CATEGORY    "actionViewToggleCategory"
#define WIZACTION_GLOBAL_TOGGLE_FULLSCREEN  "actionViewToggleFullscreen"

#define WIZACTION_FORMAT_BOLD               "actionFormatBold"
#define WIZACTION_FORMAT_ITALIC             "actionFormatItalic"
#define WIZACTION_FORMAT_UNDERLINE          "actionFormatUnderLine"
#define WIZACTION_FORMAT_UNORDEREDLIST      "actionFormatInsertUnorderedList"
#define WIZACTION_FORMAT_ORDEREDLIST        "actionFormatInsertOrderedList"
#define WIZACTION_FORMAT_JUSTIFYLEFT        "actionFormatJustifyLeft"
#define WIZACTION_FORMAT_JUSTIFYRIGHT       "actionFormatJustifyRight"
#define WIZACTION_FORMAT_JUSTIFYCENTER      "actionFormatJustifyCenter"
#define WIZACTION_FORMAT_JUSTIFYJUSTIFY     "actionFormatJustifyJustify"


class CWizActions
{
public:
    CWizActions(CWizExplorerApp& app, QObject* parent);

private:
    QObject* m_parent;
    CWizExplorerApp& m_app;

    std::map<QString, QAction*> m_actions;
    WIZACTION* actionsData();
    QAction* addAction(WIZACTION& action);

public:
    void init();
    QAction* actionFromName(const QString& strActionName);
    void toggleActionText(const QString& strActionName);
    CWizAnimateAction* animateActionFromName(const QString& strActionName);

    QMenu* toMenu(QWidget* parent, CWizSettings& settings, const QString& strSection);
    void buildMenu(QMenu* pMenu, CWizSettings& settings, const QString& strSection);
    void buildMenuBar(QMenuBar* menuBar, const QString& strFileName);
//    void buildActionMenu(QAction* pAction, QWidget* parent, const QString& strFileName);
};

#endif // WIZACTIONS_H
