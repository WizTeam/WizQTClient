#ifndef UTILS_MISC_H
#define UTILS_MISC_H

#include <QtGlobal>
#include "share/WizQtHelper.h"

class QString;
class QDateTime;

namespace Utils {

class WizMisc
{
public:
    static QString time2humanReadable(const QDateTime& time);
    static QString time2humanReadable(const QDateTime& time, const QString& formate);
    static bool loadUnicodeTextFromFile(const QString& strFileName, QString& strText);

    //
    //  location format
    static void addBackslash(QString& strPath);
    static void removeBackslash(CString& strPath);
    static CString addBackslash2(const CString& strPath);
    static CString removeBackslash2(const CString& strPath);
    static void ensurePathExists(const CString& strPath);
    static void ensureFileExists(const QString& strFileName);

    static CString extractFilePath(const CString& strFileName);
    static CString extractLastPathName(const CString& strFileName);
    static QString extractFileName(const QString& strFileName);
    static QString extractFileTitle(const QString& strFileName);
    static CString extractTitleTemplate(const CString& strFileName);
    static CString extractFileExt(const CString& strFileName);

    // file operations
    static qint64 getFileSize(const CString& strFileName);
    static void deleteFile(const CString& strFileName);

    //html process
    static QString getHtmlBodyContent(QString strHtml);
    static void splitHtmlToHeadAndBody(const QString& strHtml, QString& strHead, QString& strBody);

    //
    static void copyTextToClipboard(const QString& text);

    //
    static bool isChinese();
    static bool isSimpChinese();
    static bool isTraditionChinese();

    //
    static bool localeAwareCompare(const QString &s1, const QString &s2);

    static int getVersionCode();
};

} // namespace Utils

#endif // UTILS_MISC_H
