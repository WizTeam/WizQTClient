#ifndef WIZMACACTIONHELPER_H
#define WIZMACACTIONHELPER_H

#include <QObject>

class CWizMacToolBarItem;
class QAction;

class CWizMacActionHelper : public QObject
{
    Q_OBJECT

public:
    CWizMacActionHelper(CWizMacToolBarItem* item, QAction* action, QObject* parent);

private:
    CWizMacToolBarItem* m_item;

public Q_SLOTS:
    void on_action_changed();
};

#endif // WIZMACACTIONHELPER_H
