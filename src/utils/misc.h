#ifndef UTILS_MISC_H
#define UTILS_MISC_H

class QString;
class QDateTime;

namespace Utils {

class Misc
{
public:
    static QString time2humanReadable(const QDateTime& time);

    //static bool copyRes2Cache(const QString& strName, const QString& strFilePath);
};

} // namespace Utils

#endif // UTILS_MISC_H
