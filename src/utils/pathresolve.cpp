#include "pathresolve.h"

#include <QtGlobal>
#include <QApplication>
#include <QDir>

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

QString PathResolve::dataStorePath()
{
    QString strPath = QDir::homePath();
    strPath += "/.wiznote/";

    ensurePathExists(strPath);
    return strPath;
}

QString PathResolve::cachePath()
{
    QString strCachePath = qgetenv("XDG_CACHE_HOME");
    if (strCachePath.isEmpty()) {
#ifdef Q_OS_LINUX
        strCachePath = qgetenv("HOME") + "/.cache/wiznote/";
#else
        strCachePath = dataStorePath() + "cache/";
#endif
    }

    ensurePathExists(strCachePath);
    return strCachePath;
}

QString PathResolve::logPath()
{
    QString strPath = dataStorePath() + "log/";
    ensurePathExists(strPath);
    return strPath;
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

QString PathResolve::userSettingsFilePath()
{
    return dataStorePath() + "wiznote.ini";
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
