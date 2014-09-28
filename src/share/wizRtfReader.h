#ifndef WIZRTFREADER_H
#define WIZRTFREADER_H

#include <QString>

class CWizRtfReader
{
public:
    CWizRtfReader();

    static bool load (const QString& strFile, QString& strText);
};

#endif // WIZRTFREADER_H
