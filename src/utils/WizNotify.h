#ifndef NOTIFY_H
#define NOTIFY_H

#include <QtGlobal>

class QString;

namespace Utils {

class WizNotify
{
public:
    static void sendNotify(const QString& strTile, const QString& strText);
#ifdef Q_OS_MAC
    static void setDockBadge(int n);
#endif
};

}

#endif // NOTIFY_H
