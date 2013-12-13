#include "pathresolve.h"

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
