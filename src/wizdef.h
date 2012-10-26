#ifndef WIZDEF_H
#define WIZDEF_H

#include "share/wizdatabase.h"
#include "share/wizsettings.h"
#include "wizupdater.h"

class CWizDatabase;
class CWizCategoryView;

class CWizExplorerApp
{
public:
    virtual QWidget* mainWindow() = 0;
    virtual QObject* object() = 0;
    virtual CWizDatabase& database() = 0;
    virtual CWizCategoryView& category() = 0;

    virtual CWizUserSettings& userSettings() = 0;
    virtual CWizUpdater* updater() = 0;
};

#endif // WIZDEF_H
