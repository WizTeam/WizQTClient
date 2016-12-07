#include "WizHtml2Zip.h"

#include "html/WizHtmlCollector.h"

#include "../share/WizZip.h"

#include "WizMisc.h"
#include "utils/WizPathResolve.h"
#include "utils/WizMisc.h"
#include <QDir>




bool WizHtml2Zip(const QString& strUrl, const QString& strHtml, \
                 const QString& strResourcePath, long flags, \
                 const QString& strZipFileName)
{
    Q_UNUSED(flags);

    WizHtmlCollector collector;

    //
    QString strTempPath = Utils::WizPathResolve::tempPath();
    //
    QString strMainHtml(strHtml);
    if (!collector.collect(strUrl, strMainHtml, true, strTempPath)) {
        return false;
    }

    return collector.html2Zip(strResourcePath, strZipFileName);
}

bool WizHtml2Zip(const QString& strHtml, const CWizStdStringArray& arrayResource, \
                 const QString& strZipFileName)
{
    WizZipFile zip;
    if (!zip.open(strZipFileName))
        return false;

    QString strHtmlText = strHtml;
    if (strHtmlText.left(2) != "<!")
    {
        strHtmlText = "<!DOCTYPE html>" + strHtmlText;
    }
    CString strIndexFileName = Utils::WizPathResolve::tempPath() + WizIntToStr(WizGetTickCount()) + ".html";
    //if (!::WizSaveUnicodeTextToUnicodeFile(strIndexFileName, strHtml))
    if (!::WizSaveUnicodeTextToUtf8File(strIndexFileName, strHtmlText))
        return false;

    if (!zip.compressFile(strIndexFileName, "index.html"))
        return false;

    int failed = 0;

    for (CWizStdStringArray::const_iterator it = arrayResource.begin();
        it != arrayResource.end();
        it++)
    {
        CString strFileName = *it;
        CString strNameInZip = "index_files/" + Utils::WizMisc::extractFileName(strFileName);
        if (!zip.compressFile(strFileName, strNameInZip))
        {
            failed++;
        }
    }

    return zip.close();
}


bool WizFolder2Zip(const QString &strFolder,    \
                   const QString &strZipFileName, const QString &indexFile /*= "index.html"*/, \
                   const QString &strResourceFolder /*= "index_files"*/)
{
    WizZipFile zip;
    if (!zip.open(strZipFileName))
        return false;

    CString strIndexFileName = strFolder + indexFile;

    if (!zip.compressFile(strIndexFileName, "index.html"))
        return false;

    int failed = 0;

    QDir dir(strFolder + strResourceFolder);
    QStringList strResourceList = dir.entryList(QDir::Files, QDir::NoSort);
    for (QStringList::const_iterator it = strResourceList.begin(); it != strResourceList.end(); it++)
    {
        QString strFileName =dir.path() +"/" + *it;
        QString strNameInZip = "index_files/" + Utils::WizMisc::extractFileName(strFileName);
        if (!zip.compressFile(strFileName, strNameInZip))
        {
            failed++;
        }
    }

    return zip.close();
}
