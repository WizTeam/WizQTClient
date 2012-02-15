#include "wizmacactionhelper.h"
#include <QAction>

CWizMacActionHelper::CWizMacActionHelper(CWizMacToolBarItem* item, QAction* action, QObject* parent)
    : QObject(parent)
    , m_item(item)
{
    connect(action, SIGNAL(changed()), this, SLOT(on_action_changed()));
}
