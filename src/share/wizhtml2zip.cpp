#include "wizhtml2zip.h"

#include "html/wizhtmlcollector.h"

#include "wizmisc.h"
#include "utils/pathresolve.h"

#include "../share/wizzip.h"


bool WizHtml2Zip(const QString& strUrl, const QString& strHtml, \
                 const QString& strResourcePath, long flags, \
                 const QString& strMetaText, const QString& strZipFileName)
{
    Q_UNUSED(flags);

    CWizHtmlCollector collector;

    QString strMainHtml(strHtml);
    if (!collector.Collect(strUrl, strMainHtml, true)) {
        return false;
    }

    return collector.Html2Zip(strResourcePath, strMetaText, strZipFileName);
}

bool WizHtml2Zip(const QString& strHtml, const CWizStdStringArray& arrayResource, \
                 const QString& strMetaText, const QString& strZipFileName)
{
    CWizZipFile zip;
    if (!zip.open(strZipFileName))
        return false;

    CString strIndexFileName = Utils::PathResolve::tempPath() + WizIntToStr(GetTickCount()) + ".html";
    //if (!::WizSaveUnicodeTextToUnicodeFile(strIndexFileName, strHtml))
    if (!::WizSaveUnicodeTextToUtf8File(strIndexFileName, strHtml))
        return false;

    CString strMetaFileName = Utils::PathResolve::tempPath() + WizIntToStr(GetTickCount()) + ".xml";
    if (!::WizSaveUnicodeTextToUtf8File(strMetaFileName, strMetaText))
        return false;

    if (!zip.compressFile(strIndexFileName, "index.html"))
        return false;

    int failed = 0;

    if (!zip.compressFile(strMetaFileName, "meta.xml"))
        failed++;

    for (CWizStdStringArray::const_iterator it = arrayResource.begin();
        it != arrayResource.end();
        it++)
    {
        CString strFileName = *it;
        CString strNameInZip = "index_files/" + WizExtractFileName(strFileName);
        if (!zip.compressFile(strFileName, strNameInZip))
        {
            failed++;
        }
    }

    return zip.close();
}
