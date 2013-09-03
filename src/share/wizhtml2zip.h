#ifndef WIZHTML2ZIP_H
#define WIZHTML2ZIP_H

#include "wizmisc.h"

bool WizHtml2Zip(const QString& strUrl, const QString& strHtml, \
                 const QString& strResourcePath, long flags, \
                 const QString& strMetaText, const QString& strZipFileName);

bool WizHtml2Zip(const QString &strHtml, const CWizStdStringArray& arrayResource, \
                 const QString &strMetaText, const QString &strZipFileName);

#endif // WIZHTML2ZIP_H
