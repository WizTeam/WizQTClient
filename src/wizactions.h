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
    CWizAnimateAction* animateActionFromName(const QString& strActionName);

    QMenu* toMenu(QWidget* parent, CWizSettings& settings, const QString& strSection);
    void buildMenu(QMenu* pMenu, CWizSettings& settings, const QString& strSection);
//    void buildMenuBar(QMenuBar* menuBar, const QString& strFileName);
//    void buildActionMenu(QAction* pAction, QWidget* parent, const QString& strFileName);
};

#endif // WIZACTIONS_H
