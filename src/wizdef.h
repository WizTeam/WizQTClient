#ifndef WIZDEF_H
#define WIZDEF_H

#ifndef WIZDATABASE_H
#include "share/wizdatabase.h"
#endif

class CWizDatabase;
class CWizCategoryView;

class CWizExplorerApp
{
public:
    virtual QWidget* mainWindow() = 0;
    virtual QObject* object() = 0;
    virtual CWizDatabase& database() = 0;
    virtual CWizCategoryView& category() = 0;
};

#endif // WIZDEF_H
