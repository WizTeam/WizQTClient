#ifndef PATHRESOLVE_H
#define PATHRESOLVE_H

class QString;

namespace Utils {

class PathResolve
{
public:
    static QString appPath();
    static QString resourcesPath();
    static QString pluginsPath();
    static QString dataStorePath();
    static QString tempPath();

    // helpers
    static void addBackslash(QString& strPath);
    static void ensurePathExists(const QString& path);
};

} // namespace Utils

#endif // PATHRESOLVE_H
