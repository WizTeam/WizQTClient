#include "wizhtml2zip.h"

#include "html/wizhtmlcollector.h"

#include "wizmisc.h"

#include "../zip/wizzip.h"


bool WizHtml2Zip(const CString& strUrl, const CString& strHtml, \
                 const CString& strResourcePath, long flags, \
                 const CString& strMetaText, const CString& strZipFileName)
{
    Q_UNUSED(flags);

    CWizHtmlCollector collector;

    CString strMainHtml = strHtml;
    collector.Collect(strUrl, strMainHtml, true);

    return collector.Html2Zip(strUrl, strHtml, strResourcePath, strMetaText, strZipFileName);
}

bool WizHtml2Zip(const CString& strHtml, const CWizStdStringArray& arrayResource, \
                 const CString& strMetaText, const CString& strZipFileName)
{
    CWizZipFile zip;
    if (!zip.open(strZipFileName))
        return false;

    CString strIndexFileName = WizGlobal()->GetTempPath() + WizIntToStr(GetTickCount()) + ".html";
    //if (!::WizSaveUnicodeTextToUnicodeFile(strIndexFileName, strHtml))
    if (!::WizSaveUnicodeTextToUtf8File(strIndexFileName, strHtml))
        return false;

    CString strMetaFileName = ::WizGlobal()->GetTempPath() + WizIntToStr(GetTickCount()) + ".xml";
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
