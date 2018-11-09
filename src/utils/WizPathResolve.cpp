#include "WizPathResolve.h"

#include <QtGlobal>
#include <QApplication>
#include <QDir>
#include <QDebug>

namespace Utils {

QString WizPathResolve::appPath()
{
    QString strPath = QApplication::applicationDirPath();
    addBackslash(strPath);
    return strPath;
}

QString WizPathResolve::resourcesPath()
{
#ifdef Q_OS_MAC
    QDir dir(appPath());
    dir.cdUp();
    dir.cd("Resources");
    QString strPath = dir.path();
    addBackslash(strPath);
    return strPath;
#else
    QDir dir(appPath());
    dir.cdUp();
    dir.cd("share/wiznote");
    QString strPath = dir.path();
    addBackslash(strPath);
    return strPath;
#endif
}

QString WizPathResolve::themePath(const QString& strThemeName)
{
    return resourcesPath() + "skins/" + strThemeName + "/";
}

QString WizPathResolve::pluginsPath()
{
    return resourcesPath() + "plugins/";
}


QString WizPathResolve::skinResourcesPath(const QString &strSkinName)
{
    Q_ASSERT(!strSkinName.isEmpty());
    return resourcesPath() + "skins/" + strSkinName + "/";
}

QString WizPathResolve::builtinTemplatePath()
{
    return resourcesPath() + "templates/";
}

QString WizPathResolve::customNoteTemplatesPath()
{
    QString strPath;
#ifdef Q_OS_MAC
    #ifdef BUILD4APPSTORE
        strPath = QDir::homePath() + "/Library/Templates/";
    #else
        strPath = dataStorePath() + "templates/";
    #endif
#else
    strPath = dataStorePath() + "templates/";
#endif

    return strPath;
}

QString WizPathResolve::wizTemplateJsFilePath()
{
    return Utils::WizPathResolve::customNoteTemplatesPath() + "wiz_template.js";
}

QString WizPathResolve::wizTemplateJsonFilePath()
{
    return Utils::WizPathResolve::customNoteTemplatesPath() + "wiz_template.json";
}

QString WizPathResolve::wizTemplatePurchaseRecordFile()
{
    return Utils::WizPathResolve::customNoteTemplatesPath() + "wiz_templateRecord.json";
}

QString WizPathResolve::customMarkdownTemplatesPath()
{
    QString strPath;
#ifdef Q_OS_MAC
    #ifdef BUILD4APPSTORE
        strPath = QDir::homePath() + "/Library/MarkdownTemplates/";
    #else
        strPath = dataStorePath() + "markdownTemplates/";
    #endif
#else
    strPath = dataStorePath() + "markdownTemplates/";
#endif

    return strPath;
}

QString WizPathResolve::dataStorePath()
{
    QString strPath;
    strPath= QDir::homePath();
#ifdef Q_OS_MAC
    #ifdef BUILD4APPSTORE
        strPath += "/Documents/";
    #else
        strPath += "/.wiznote/";
    #endif
#else
#ifdef Q_OS_WIN
    strPath += "/WizNote/";
#else
    strPath += "/.wiznote/";
#endif
#endif
    ensurePathExists(strPath);
    //
    return strPath;
}

QString WizPathResolve::cachePath()
{
#ifdef Q_OS_WIN
    QString strCachePath = dataStorePath() + "cache/";
#else
    QString strCachePath = qgetenv("XDG_CACHE_HOME");
    if (strCachePath.isEmpty()) {
#ifdef Q_OS_MAC
    #ifdef BUILD4APPSTORE
        strCachePath = QDir::homePath() + "/Library/Caches/";
    #else
        strCachePath = dataStorePath() + "cache/";
    #endif
#else
        strCachePath = qgetenv("HOME") + "/.cache/wiznote/";
#endif
    } else {
        strCachePath += "/wiznote/";
    }

#endif
    ensurePathExists(strCachePath);
    return strCachePath;
}

QString WizPathResolve::avatarPath()
{
    QString strPath = cachePath() + "avatar/";
    ensurePathExists(strPath);
    return strPath;
}

QString WizPathResolve::logFilePath()
{
    QString strPath;
#ifdef Q_OS_MAC
    #ifdef BUILD4APPSTORE
        strPath = QDir::homePath() + "/Library/Logs/";
    #else
        strPath = dataStorePath() + "log/";
    #endif
#else
    strPath = dataStorePath() + "log/";
#endif

    ensurePathExists(strPath);
    return strPath;
}

QString WizPathResolve::logFile()
{
    QString strLogfile = logFilePath() + "wiznote.log";
    return strLogfile;
}


QString WizPathResolve::tempPath()
{
    QString path = QDir::tempPath() + "/WizNote/";

    ensurePathExists(path);
    return path;
}

QString WizPathResolve::tempDocumentFolder(const QString& strGuid)
{
    QString strTempFolder = tempPath() + strGuid + "/";
    ensurePathExists(strTempFolder);

    return strTempFolder;
}

QString WizPathResolve::upgradePath()
{
    QString strPath = dataStorePath() + "/update/";
    ensurePathExists(strPath);
    return strPath;
}

QString WizPathResolve::globalSettingsFile()
{
    QString strConfigHome = dataStorePath();

    ensurePathExists(strConfigHome);
    return strConfigHome + "wiznote.ini";
}

QString WizPathResolve::userSettingsFile(const QString strAccountFolderName)
{
    return dataStorePath() + strAccountFolderName + "/wiznote.ini";
}

QString WizPathResolve::qtLocaleFileName(const QString &strLocale)
{
    return resourcesPath() + "locales/qt_" + strLocale + ".qm";
}

QString WizPathResolve::localeFileName(const QString &strLocale)
{
    return resourcesPath() + "locales/wiznote_" + strLocale + ".qm";
}

QString WizPathResolve::introductionNotePath()
{
    return resourcesPath() + "files/introduction/";
}

void WizPathResolve::addBackslash(QString& strPath)
{
    strPath.replace('\\', '/');

    if (strPath.endsWith('/'))
        return;

    strPath += '/';
}

void WizPathResolve::ensurePathExists(const QString& path)
{
    QDir dir;
    dir.mkpath(path);
}

} // namespace Utils
