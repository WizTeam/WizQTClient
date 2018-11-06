#ifndef WIZDEF_H
#define WIZDEF_H

#include <QtGlobal>

#define WIZ_CLIENT_VERSION  "2.6.6"
#define WIZNOTE_FTS_VERSION "5"
#define WIZNOTE_THUMB_VERSION "3"
#define WIZ_NEW_FEATURE_GUIDE_VERSION "4"
#define WIZ_TABLE_STRUCTURE_VERSION "5"

#define USER_SETTINGS_SECTION "QT_WIZNOTE"

#if defined Q_OS_MAC
#define WIZ_CLIENT_TYPE     "QTMAC"
#elif defined Q_OS_LINUX
#define WIZ_CLIENT_TYPE     "QTLINUX"
#elif defined Q_OS_WIN
#define WIZ_CLIENT_TYPE     "QTWIN"
#endif

class QWidget;
class QObject;
class WizDatabaseManager;
class WizCategoryBaseView;
class WizUserSettings;

class WizExplorerApp
{
public:
    virtual QWidget* mainWindow() = 0;
    virtual QObject* object() = 0;
    virtual WizDatabaseManager& databaseManager() = 0;
    virtual WizCategoryBaseView& category() = 0;
    virtual WizUserSettings& userSettings() = 0;
};

#endif // WIZDEF_H
