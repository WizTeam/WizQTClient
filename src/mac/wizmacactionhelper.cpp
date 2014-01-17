#include "wizmacactionhelper.h"
#include <QAction>

#if 0
CWizMacActionHelper::CWizMacActionHelper(CWizMacToolBarItem* item, QAction* action, QObject* parent)
    : QObject(parent)
    , m_item(item)
{
    connect(action, SIGNAL(changed()), SLOT(on_action_changed()));
}

#endif
