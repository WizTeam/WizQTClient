#include "pathresolve.h"

#include <QtGlobal>
#include <QApplication>
#include <QDir>
#include <QDebug>

namespace Utils {

QString PathResolve::appPath()
{
    QString strPath = QApplication::applicationDirPath();
    addBackslash(strPath);
    return strPath;
}

QString PathResolve::resourcesPath()
{
#ifdef Q_OS_MAC
    QDir dir(appPath());
    dir.cdUp();
    dir.cd("Resources");
    QString strPath = dir.path();
    addBackslash(strPath);
    return strPath;
#elif defined(Q_OS_LINUX)
    QDir dir(appPath());
    dir.cdUp();
    dir.cd("share/wiznote");
    QString strPath = dir.path();
    addBackslash(strPath);
    return strPath;
#else
    return appPath();
#endif
}

QString PathResolve::themePath(const QString& strThemeName)
{
    return resourcesPath() + "skins/" + strThemeName + "/";
}

QString PathResolve::skinResourcesPath(const QString &strSkinName)
{
    Q_ASSERT(!strSkinName.isEmpty());
    return resourcesPath() + "skins/" + strSkinName + "/";
}

QString PathResolve::builtinTemplatePath()
{
    return resourcesPath() + "templates/";
}

QString PathResolve::downloadedTemplatesPath()
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

QString PathResolve::dataStorePath()
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
    strPath += "/.wiznote/";
#endif
    ensurePathExists(strPath);
    return strPath;
}

QString PathResolve::cachePath()
{
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

    ensurePathExists(strCachePath);
    return strCachePath;
}

QString PathResolve::avatarPath()
{
    QString strPath = cachePath() + "avatar/";
    ensurePathExists(strPath);
    return strPath;
}

QString PathResolve::logFilePath()
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

QString PathResolve::logFile()
{
    QString strLogfile = logFilePath() + "wiznote.log";
    return strLogfile;
}

QString PathResolve::pluginsPath()
{
    return resourcesPath() + "plugins/";
}

QString PathResolve::tempPath()
{
    QString path = QDir::tempPath() + "/WizNote/";

    ensurePathExists(path);
    return path;
}

QString PathResolve::tempDocumentFolder(const QString& strGuid)
{
    QString strTempFolder = tempPath() + strGuid + "/";
    ensurePathExists(strTempFolder);

    return strTempFolder;
}

QString PathResolve::upgradePath()
{
    QString strPath = dataStorePath() + "/update/";
    ensurePathExists(strPath);
    return strPath;
}

QString PathResolve::globalSettingsFile()
{
    QString strConfigHome = dataStorePath();

    ensurePathExists(strConfigHome);
    return strConfigHome + "wiznote.ini";
}

QString PathResolve::userSettingsFile(const QString strAccountFolderName)
{
    return dataStorePath() + strAccountFolderName + "/wiznote.ini";
}

QString PathResolve::qtLocaleFileName(const QString &strLocale)
{
    return resourcesPath() + "locales/qt_" + strLocale + ".qm";
}

QString PathResolve::localeFileName(const QString &strLocale)
{
    return resourcesPath() + "locales/wiznote_" + strLocale + ".qm";
}

QString PathResolve::introductionNotePath()
{
    return resourcesPath() + "files/introduction/";
}

void PathResolve::addBackslash(QString& strPath)
{
    strPath.replace('\\', '/');

    if (strPath.endsWith('/'))
        return;

    strPath += '/';
}

void PathResolve::ensurePathExists(const QString& path)
{
    QDir dir;
    dir.mkpath(path);
}

} // namespace Utils
