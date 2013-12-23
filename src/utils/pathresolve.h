#ifndef UTILS_PATHRESOLVE_H
#define UTILS_PATHRESOLVE_H

class QString;

namespace Utils {

class PathResolve
{
public:
    static QString appPath();
    static QString resourcesPath();
    static QString pluginsPath();
    static QString dataStorePath();
    static QString cachePath();
    static QString tempPath();
    static QString logPath();
    static QString userSettingsFilePath();

    // helpers
    static void addBackslash(QString& strPath);
    static void ensurePathExists(const QString& path);
};

} // namespace Utils

#endif // UTILS_PATHRESOLVE_H
