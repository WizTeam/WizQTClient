#ifndef WIZMACACTIONHELPER_H
#define WIZMACACTIONHELPER_H


#ifdef USECOCOATOOLBAR

#include <QObject>

class WizMacToolBarItem;
class QAction;

class WizMacActionHelper : public QObject
{
    Q_OBJECT

public:
    WizMacActionHelper(WizMacToolBarItem* item, QAction* action, QObject* parent);

private:
    WizMacToolBarItem* m_item;

public Q_SLOTS:
    void on_action_changed();
};

#endif

#endif // WIZMACACTIONHELPER_H
