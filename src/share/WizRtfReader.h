#ifndef WIZRTFREADER_H
#define WIZRTFREADER_H

#include "WizQtHelper.h"

class CWizRtfReader
{
public:
    CWizRtfReader();

    static bool load (const QString& strFile, QString& strText);

    static bool rtf2hmlt(const QString& strRtf, QString& strHtml);
};


#endif // WIZRTFREADER_H
