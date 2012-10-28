#ifndef WIZHTML2ZIP_H
#define WIZHTML2ZIP_H

#include "wizmisc.h"

bool WizHtml2Zip(const CString& strUrl, const CString& strHtml, \
                 const CString& strResourcePath, long flags, \
                 const CString& strMetaText, const CString& strZipFileName);

bool WizHtml2Zip(const CString& strHtml, const CWizStdStringArray& arrayResource, \
                 const CString& strMetaText, const CString& strZipFileName);

#endif // WIZHTML2ZIP_H
