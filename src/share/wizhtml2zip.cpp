#include "wizhtml2zip.h"

#include "html/wizhtmlcollector.h"

#include "wizmisc.h"

#include <QDebug>

#include "../zip/wizzip.h"



BOOL WizHtml2Zip(const CString& strUrl, const CString& strHtml, const CString& strResourcePath, long flags, const CString& strMetaText, const CString& strZipFileName)
{
    CWizHtmlCollector collector;
    //
    CString strMainHtml = strHtml;
    collector.Collect(strUrl, strMainHtml, true);
    return collector.Html2Zip(strUrl, strHtml, strResourcePath, strMetaText, strZipFileName);
}

BOOL WizHtml2Zip(const CString& strHtml, const CWizStdStringArray& arrayResource, const CString& strMetaText, const CString& strZipFileName)
{
    CWizZipFile zip;
    if (!zip.open(strZipFileName))
        return FALSE;
    //
    CString strIndexFileName = WizGlobal()->GetTempPath() + WizIntToStr(GetTickCount()) + _T(".html");
    if (!::WizSaveUnicodeTextToUnicodeFile(strIndexFileName, strHtml))
        return FALSE;
    //
    CString strMetaFileName = ::WizGlobal()->GetTempPath() + WizIntToStr(GetTickCount()) + _T(".xml");
    if (!::WizSaveUnicodeTextToUnicodeFile(strMetaFileName, strMetaText))
        return FALSE;
    //
    if (!zip.compressFile(strIndexFileName, "index.html"))
        return FALSE;
    //
    int failed = 0;
    //
    if (!zip.compressFile(strMetaFileName, "meta.xml"))
        failed++;
    //
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
    //
    return zip.close();
}
