#include "wizhtml2zip.h"

#include "html/wizhtmlcollector.h"

#include "../share/wizzip.h"

#include "wizmisc.h"
#include "utils/pathresolve.h"
#include "utils/misc.h"
#include <QDir>




bool WizHtml2Zip(const QString& strUrl, const QString& strHtml, \
                 const QString& strResourcePath, long flags, \
                 const QString& strMetaText, const QString& strZipFileName)
{
    Q_UNUSED(flags);

    CWizHtmlCollector collector;

    //
    QString strTempPath = Utils::PathResolve::tempPath();
    //
    QString strMainHtml(strHtml);
    if (!collector.Collect(strUrl, strMainHtml, true, strTempPath)) {
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

    QString strHtmlText = strHtml;
    if (strHtmlText.left(2) != " <!")
    {
        strHtmlText = "<!DOCTYPE html>" + strHtmlText;
    }
    CString strIndexFileName = Utils::PathResolve::tempPath() + WizIntToStr(GetTickCount()) + ".html";
    //if (!::WizSaveUnicodeTextToUnicodeFile(strIndexFileName, strHtml))
    if (!::WizSaveUnicodeTextToUtf8File(strIndexFileName, strHtmlText))
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
        CString strNameInZip = "index_files/" + Utils::Misc::extractFileName(strFileName);
        if (!zip.compressFile(strFileName, strNameInZip))
        {
            failed++;
        }
    }

    return zip.close();
}


bool WizFolder2Zip(const QString &strFolder, const QString &strMetaText,    \
                   const QString &strZipFileName, const QString &indexFile /*= "index.html"*/, \
                   const QString &strResourceFolder /*= "index_files"*/)
{
    CWizZipFile zip;
    if (!zip.open(strZipFileName))
        return false;

    CString strIndexFileName = strFolder + indexFile;

    CString strMetaFileName = Utils::PathResolve::tempPath() + WizIntToStr(GetTickCount()) + ".xml";
    if (!::WizSaveUnicodeTextToUtf8File(strMetaFileName, strMetaText))
        return false;

    if (!zip.compressFile(strIndexFileName, "index.html"))
        return false;

    int failed = 0;

    if (!zip.compressFile(strMetaFileName, "meta.xml"))
        failed++;

    QDir dir(strFolder + strResourceFolder);
    QStringList strResourceList = dir.entryList(QDir::Files, QDir::NoSort);
    for (QStringList::const_iterator it = strResourceList.begin(); it != strResourceList.end(); it++)
    {
        QString strFileName =dir.path() +"/" + *it;
        QString strNameInZip = "index_files/" + Utils::Misc::extractFileName(strFileName);
        if (!zip.compressFile(strFileName, strNameInZip))
        {
            failed++;
        }
    }

    return zip.close();
}
