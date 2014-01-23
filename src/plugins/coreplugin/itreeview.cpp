#include "itreeview.h"
#include <extensionsystem/pluginmanager.h>

namespace Core {

ITreeView::ITreeView(QWidget* parent) : QTreeWidget(parent)
{
}

ITreeView::~ITreeView()
{
}

void ITreeView::updateItem(QTreeWidgetItem* pItem)
{
    update(indexFromItem(pItem, 0));
}


} // namespace Core
