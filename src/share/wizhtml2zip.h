#ifndef WIZHTML2ZIP_H
#define WIZHTML2ZIP_H

#include "wizmisc.h"

bool WizHtml2Zip(const QString& strUrl, const QString& strHtml, \
                 const QString& strResourcePath, long flags, \
                 const QString& strMetaText, const QString& strZipFileName);

bool WizHtml2Zip(const QString &strHtml, const CWizStdStringArray& arrayResource, \
                 const QString &strMetaText, const QString &strZipFileName);

//should make sure the folder contains index file   \
//and all resource files placed in a child resource folder  before use this func
bool WizFolder2Zip(const QString &strFolder, const QString &strMetaText, \
                   const QString &strZipFileName, const QString &indexFile = "index.html", \
                   const QString &strResourceFolder = "index_files");

#endif // WIZHTML2ZIP_H
