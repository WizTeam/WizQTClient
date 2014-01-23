#ifndef ITREEVIEW_H
#define ITREEVIEW_H

#include <QTreeWidget>
#include "core_global.h"

namespace Core {

class ITreeView : public QTreeWidget
{
    Q_OBJECT

public:
    explicit ITreeView(QWidget *parent = 0);
    virtual ~ITreeView();

    void updateItem(QTreeWidgetItem* pItem);
};

} // namespace Core

#endif // ITREEVIEW_H
