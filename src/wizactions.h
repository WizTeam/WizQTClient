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
#define WIZACTION_GLOBAL_TOGGLE_CATEGORY    "actionViewToggleCategory"
#define WIZACTION_GLOBAL_TOGGLE_FULLSCREEN  "actionViewToggleFullscreen"

#define WIZACTION_EDITOR_UNDO               "actionEditingUndo"
#define WIZACTION_EDITOR_REDO               "actionEditingRedo"
#define WIZACTION_EDITOR_CUT                "actionEditingCut"
#define WIZACTION_EDITOR_COPY               "actionEditingCopy"
#define WIZACTION_EDITOR_PASTE              "actionEditingPaste"
#define WIZACTION_EDITOR_PASTE_PLAIN        "actionEditingPastePlain"
#define WIZACTION_EDITOR_DELETE             "actionEditingDelete"
#define WIZACTION_EDITOR_SELECT_ALL         "actionEditingSelectAll"

#define WIZACTION_FORMAT_BOLD               "actionFormatBold"
#define WIZACTION_FORMAT_ITALIC             "actionFormatItalic"
#define WIZACTION_FORMAT_UNDERLINE          "actionFormatUnderLine"
#define WIZACTION_FORMAT_STRIKETHROUGH      "actionFormatStrikeThrough"
#define WIZACTION_FORMAT_UNORDEREDLIST      "actionFormatInsertUnorderedList"
#define WIZACTION_FORMAT_ORDEREDLIST        "actionFormatInsertOrderedList"
#define WIZACTION_FORMAT_JUSTIFYLEFT        "actionFormatJustifyLeft"
#define WIZACTION_FORMAT_JUSTIFYRIGHT       "actionFormatJustifyRight"
#define WIZACTION_FORMAT_JUSTIFYCENTER      "actionFormatJustifyCenter"
#define WIZACTION_FORMAT_JUSTIFYJUSTIFY     "actionFormatJustifyJustify"
#define WIZACTION_FORMAT_INDENT             "actionFormatIndent"
#define WIZACTION_FORMAT_OUTDENT            "actionFormatOutdent"
#define WIZACTION_FORMAT_INSERT_TABLE       "actionFormatInsertTable"
#define WIZACTION_FORMAT_INSERT_LINK        "actionFormatInsertLink"
#define WIZACTION_FORMAT_INSERT_HORIZONTAL  "actionFormatInsertHorizontal"
#define WIZACTION_FORMAT_INSERT_DATE        "actionFormatInsertDate"
#define WIZACTION_FORMAT_INSERT_TIME        "actionFormatInsertTime"
#define WIZACTION_FORMAT_REMOVE_FORMAT      "actionFormatRemoveFormat"
#define WIZACTION_FORMAT_VIEW_SOURCE        "actionEditorViewSource"


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
};

#endif // WIZACTIONS_H
