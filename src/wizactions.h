#ifndef WIZACTIONS_H
#define WIZACTIONS_H

#include "share/wizqthelper.h"

class QAction;
class QMenuBar;
class QMenu;
class CWizSettings;
struct WIZACTION;

class CWizActions
{
public:
    CWizActions(QObject* parent);
private:
    QObject* m_parent;
    std::map<CString, QAction*> m_actions;
    //
    WIZACTION* actionsData();
private:
    QAction* addAction(WIZACTION& action);
public:
    void init();
    QAction* actionFromName(const CString& strActionName);
    //
    QMenu* toMenu(QWidget* parent, CWizSettings& settings, const CString& strSection);
    void buildMenu(QMenu* pMenu, CWizSettings& settings, const CString& strSection);
    void buildMenuBar(QMenuBar* menuBar, const CString& strFileName);
    void buildActionMenu(QAction* pAction, QWidget* parent, const CString& strFileName);
};

#endif // WIZACTIONS_H
