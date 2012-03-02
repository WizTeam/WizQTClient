#include "wizmisc.h"
#include "wizsettings.h"
#include <QDir>
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUuid>
#include <QDebug>
#include <stdlib.h>
#include <QTextDocumentFragment>
#include <QTextCodec>
#include <algorithm>
#include <QCursor>
#include <fstream>

#ifndef MAX_PATH
#define MAX_PATH 200
#endif


__int64 WizGetFileSize(const CString& strFileName)
{
    QFileInfo info(strFileName);
    if (!info.exists())
    {
        TOLOG1(_T("File does not exists: %1"), strFileName);
    }

    return info.size();
}

void WizPathAddBackslash(CString& strPath)
{
    strPath.replace('\\', '/');
    //
    if (strPath.endsWith('/'))
        return;
    strPath += '/';
}
void WizPathRemoveBackslash(CString& strPath)
{
    while (1)
    {
        if (!strPath.endsWith('/'))
            return;
        //
        strPath.remove(strPath.length() - 1, 1);
    }
}

CString WizPathAddBackslash2(const CString& strPath)
{
    CString str(strPath);
    WizPathAddBackslash(str);
    return str;
}
CString WizPathRemoveBackslash2(const CString& strPath)
{
    CString str(strPath);
    WizPathRemoveBackslash(str);
    return str;
}

BOOL WizEnsurePathExistsEx(const CString& path)
{
    QDir dir;
    return dir.mkpath(path);
}
void WizEnsurePathExists(const CString& strPath)
{
    WizEnsurePathExistsEx(strPath);
}

BOOL WizDeleteAllFilesInFolder(const CString& strPath)
{
    CWizStdStringArray arrayFile;
    ::WizEnumFiles(strPath, "*", arrayFile, EF_INCLUDESUBDIR);
    foreach (const QString& fileName, arrayFile)
    {
        ::WizDeleteFile(fileName);
    }
    //
    CWizStdStringArray arrayFolder;
    ::WizEnumFolders(strPath, arrayFolder, EF_INCLUDESUBDIR);
    std::sort(arrayFolder.begin(), arrayFolder.end());
    size_t folderCount = arrayFolder.size();
    for (intptr_t i = folderCount - 1; i >= 0; i--)
    {
        CString strPath = arrayFolder[i];
        ::WizDeleteFolder(strPath);
    }
    return TRUE;
}


CString WizExtractFilePath(const CString& strFileName)
{
    CString str = strFileName;
    str.Replace('\\', '/');
    int index = str.lastIndexOf('/');
    if (-1 == index)
        return strFileName;
    //
    return str.left(index + 1); //include separator
}


CString WizExtractLastPathName(const CString& strFileName)
{
    CString strPath = ::WizPathRemoveBackslash2(strFileName);
    return ::WizExtractFileName(strPath);
}


CString WizExtractFileName(const CString& strFileName)
{
    CString str = strFileName;
    str.Replace('\\', '/');
    int index = str.lastIndexOf('/');
    if (-1 == index)
        return strFileName;
    //
    return strFileName.right(str.GetLength() - index - 1);
}


CString WizExtractFileTitle(const CString& strFileName)
{
    CString strName = WizExtractFileName(strFileName);
    //
    int index = strName.lastIndexOf('.');
    if (-1 == index)
        return strName;
    //
    return strName.left(index);

}

CString WizExtractTitleTemplate(const CString& strFileName)
{
    return strFileName;
}

CString WizExtractFileExt(const CString& strFileName)
{
    CString strName = WizExtractFileName(strFileName);
    //
    int index = strName.lastIndexOf('.');
    if (-1 == index)
        return "";
    //
    return strName.right(strName.GetLength() - index);  //include .
}

void WizEnumFiles(const CString& path, const CString& strExts, CWizStdStringArray& arrayFiles, UINT uFlags)
{
    //BOOL bIncludeHiddenFile = uFlags & EF_INCLUDEHIDDEN;
    BOOL bIncludeSubDir = uFlags & EF_INCLUDESUBDIR;
    //
    CString strPath(path);
    WizPathAddBackslash(strPath);
    //
    QDir dir(strPath);
    //
    QStringList nameFilters = strExts.split(";");
    QDir::Filters filters = QDir::Files;
    //
    QStringList files = dir.entryList(nameFilters, filters);
    for (QStringList::const_iterator it = files.begin();
        it != files.end();
        it++)
    {
        arrayFiles.push_back(strPath + *it);
    }
    //
    if (!bIncludeSubDir)
        return;
    //
    CWizStdStringArray arrayFolder;
    WizEnumFolders(strPath, arrayFolder, 0);
    for (CWizStdStringArray::const_iterator it = arrayFolder.begin();
        it != arrayFolder.end();
        it++)
    {
        WizEnumFiles(*it, strExts, arrayFiles, uFlags);
    }
}

void WizEnumFolders(const CString& path, CWizStdStringArray& arrayFolders, UINT uFlags)
{
    //BOOL bIncludeHiddenFile = uFlags & EF_INCLUDEHIDDEN;
    BOOL bIncludeSubDir = uFlags & EF_INCLUDESUBDIR;
    //
    CString strPath(path);
    WizPathAddBackslash(strPath);
    //
    QDir dir(strPath);
    //
    QDir::Filters filtersDir = QDir::Dirs;
    QStringList dirs = dir.entryList(filtersDir);
    //
    for (QStringList::const_iterator it = dirs.begin();
        it != dirs.end();
        it++)
    {
        CString strName = *it;
        if (strName == "."
            || strName == "..")
            continue;
        //
        CString strSubPath = strPath + strName + "/";
        arrayFolders.push_back(strSubPath);
        //
        if (!bIncludeSubDir)
            continue;
        //
        WizEnumFolders(strSubPath, arrayFolders, uFlags);
    }
}
BOOL WizCopyFile(const CString& strSrcFileName, const CString& strDestFileName, BOOL bFailIfExists)
{
    QFile fileSrc(strSrcFileName);
    if (!fileSrc.exists())
        return FALSE;
    //
    QFile fileDest(strDestFileName);
    if (fileDest.exists())
    {
        if (bFailIfExists)
            return FALSE;
        //
        fileDest.remove();
    }
    //
    return fileSrc.copy(strDestFileName);
}
void WizGetNextFileName(CString& strFileName)
{
    if (!PathFileExists(strFileName))
        return;
    //
    CString strPath = WizExtractFilePath(strFileName);
    CString strTitle = WizExtractFileTitle(strFileName);
    CString strExt = WizExtractFileExt(strFileName);
    //
    //
    const UINT nMaxLength = MAX_PATH - 10;
    if (strFileName.GetLength() >= int(nMaxLength))
    {
        int nTitleLength = nMaxLength - (strPath.GetLength() + strExt.GetLength());
        if (nTitleLength <= 0)
        {
            TOLOG1(_T("File name is too long: %1"), strFileName);
            strTitle = WizIntToStr(GetTickCount());
        }
        else
        {
            ATLASSERT(strTitle.GetLength() >= nTitleLength);
            if (strTitle.GetLength() >= nTitleLength)
            {
                strTitle = strTitle.Left(nTitleLength);
            }
        }
        //
        strFileName = strPath + strTitle + strExt;
    }
    //
    if (!PathFileExists(strFileName))
        return;
    //
    int nPos = strTitle.lastIndexOf('_');
    if (nPos != -1)
    {
        CString strTemp = strTitle.Right(strTitle.GetLength() - nPos - 1);
        if (strTemp == WizIntToStr(_ttoi(strTemp)))
        {
            strTitle.Delete(nPos, strTitle.GetLength() - nPos);
        }
    }
    //
    int nIndex = 2;
    while (PathFileExists(strFileName))
    {
        strFileName.Format(_T("%s%s_%d%s"), strPath.utf16(), strTitle.utf16(), nIndex, strExt.utf16());
        nIndex++;
    }
}
CString WizEncryptPassword(const CString& strPassword)
{
    CString str;
    ::WizBase64Encode(strPassword.toUtf8(), str);
    return str;
}

CString WizDecryptPassword(const CString& strEncryptedText)
{
    QByteArray data;
    ::WizBase64Decode(strEncryptedText, data);
    return CString::fromUtf8(data);
}

CString WizGetAppPath()
{
    CString strPath = QApplication::applicationDirPath();
    ::WizPathAddBackslash(strPath);
    return strPath;
}

CString WizGetAppFileName()
{
    CString strPath = QApplication::applicationFilePath();
    return strPath;
}


CString WizGetResourcesPath()
{
#ifdef Q_OS_LINUX
    QDir dir(WizGetAppPath());
    dir.cdUp(); //../bin
    CString strPath = dir.path();
    WizPathAddBackslash(strPath);
    strPath += "share/wiznote/";
    return strPath;
#else
    return WizGetAppPath();
#endif

}


CString WizGetDataStorePath()
{
    QDir dir;
    CString strPath = dir.homePath();
    //
    ::WizPathAddBackslash(strPath);
    //
    strPath = strPath + "WizNote/";
    //
    ::WizEnsurePathExists(strPath);
    return strPath;
}

CString WizGetSettingsFileName()
{
    return WizGetDataStorePath() + _T("wiznote.ini");
}

CString IWizGlobal::GetTempPath()
{
    CString path = QDir::tempPath();
    WizPathAddBackslash(path);
    //
    path += "WizNote/";
    ::WizEnsurePathExists(path);
    //
    return path;
}


void IWizGlobal::WriteLog(const CString& str)
{
    qDebug() << str;
    //
    COleDateTime t = ::WizGetCurrentTime();
    CString strTime = t.toString(Qt::SystemLocaleLongDate);
    //
    CString strFileName = ::WizGetDataStorePath() + "wiznote.log";
    std::fstream outf(strFileName.toLocal8Bit().constData(), std::ios::app | std::ios::out);
    outf << strTime.toUtf8().constData() << ":\t" << str.toUtf8().constData() << std::endl;
    outf.close();
}

void IWizGlobal::WriteDebugLog(const CString& str)
{
    static int writeLog = -1;
    if (writeLog == -1)
    {
#ifdef QT_DEBUG
        writeLog = 1;
#else
        writeLog = 0;
        QStringList args = QCoreApplication::arguments();
        if (-1 != args.join(" ").toLower().indexOf("debug"))
        {
            writeLog = 1;
        }
#endif
    }
    if (writeLog != 1)
        return;
    //
    WriteLog(str);
}

IWizGlobal* WizGlobal()
{
    static IWizGlobal global;
    return &global;
}


CString WizIntToStr(int n)
{
    CString str;
    str.Format("%d", n);
    return str;
}


COleDateTime WizGetCurrentTime()
{
    COleDateTime t;
    return t;
}

BOOL WizStringToDateTime(const CString& str, COleDateTime& t, CString& strError)
{
    std::string utf8 = ::WizBSTR2UTF8(str);
    const char* lpsz = utf8.c_str();
    //
    t = WizGetCurrentTime();
    //
    if (!lpsz)
    {
        strError = _T("NULL pointer");
        return TRUE;
    }
    //
    if (strlen(lpsz) != 19)
    {
        strError = _T("Invalid date time format");
        return TRUE;
    }
    //xxxx-xx-xx xx:xx:xx
    int nYear = atoi(lpsz);
    int nMonth = atoi(lpsz + 5);
    int nDay = atoi(lpsz + 8);
    int nHour = atoi(lpsz + 11);
    int nMin = atoi(lpsz + 14);
    int nSec = atoi(lpsz + 17);
    //
    if (nYear < 1900 || nYear > 2100)
    {
        strError = _T("Invalid date time format (year)");
        return TRUE;
    }
    if (nMonth < 1 || nMonth > 12)
    {
        strError = _T("Invalid date time format (month)");
        return TRUE;
    }
    if (nDay < 1 || nDay > 31)
    {
        strError = _T("Invalid date time format (day)");
        return TRUE;
    }
    if (nHour < 0 || nHour > 23)
    {
        strError = _T("Invalid date time format (hour)");
        return TRUE;
    }
    if (nMin < 0 || nMin > 59)
    {
        strError = _T("Invalid date time format (minute)");
        return TRUE;
    }
    if (nSec < 0 || nSec > 59)
    {
        strError = _T("Invalid date time format (second)");
        return TRUE;
    }
    //
    t = COleDateTime(nYear, nMonth, nDay, nHour, nMin, nSec);
    //
    return TRUE;
}


COleDateTime WizStringToDateTime(const CString& str)
{
    COleDateTime t;
    CString strError;
    WizStringToDateTime(str, t, strError);
    return t;
}




BOOL WizIso8601StringToDateTime(CString str, COleDateTime& t, CString& strError)
{
    if (str.length() != 17)
    {
        strError = _T("Invalid date time format");
        return FALSE;
    }
    //xxxxxxxxTxx:xx:xx
    //01234567890123456
    str.SetAt(8, ' ');
    str.Insert(6, '-');
    str.Insert(4, '-');
    return WizStringToDateTime(str, t, strError);
}


CString WizDateTimeToIso8601String(const COleDateTime& t)
{
    CString str;
    str.Format(_T("%04d%02d%02dT%02d:%02d:%02d"), t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
    //
    return str;
}
CString WizIntToHexEx(int n, int nWidth, BOOL bWithPrefix /*= FALSE*/)
{
    CString strFormat = CString(_T("%0")) + WizIntToStr(nWidth) + _T("X");
    //
    CString strValue;
    strValue.sprintf(strFormat.toUtf8(), n);
    //
    if (bWithPrefix)
    {
        return CString(_T("0x")) + strValue;
    }
    else
    {
        return strValue;
    }
}

int WizHexToInt(const CString& str)
{
    CString strText(str);
    if (strText.startsWith("0x"), Qt::CaseInsensitive)
        strText.Delete(0, 2);
    //
    strText.Trim();
    strText.MakeLower();
    //
    int nValue = 0;
    for (int i = 0; i < strText.GetLength(); i++)
    {
        QChar ch = strText[i];
        if (ch >= '0' && ch <= '9')
        {
            nValue = (nValue * 16) + ch.toAscii() - '0';
        }
        else if (ch >= 'a' &&ch <= 'f')
        {
            nValue = (nValue * 16) + ch.toAscii() - 'a' + 10;
        }
        else
        {
            return nValue;
        }
    }
    //
    return nValue;
}

CString WizColorToString(COLORREF cr)
{
    return WizIntToHexEx(GetRValue(cr), 2, FALSE)
        + WizIntToHexEx(GetGValue(cr), 2, FALSE)
        + WizIntToHexEx(GetBValue(cr), 2, FALSE);
}

COLORREF WizStringToColor(const CString& str)
{
    int n = WizHexToInt(str.utf16());
    //
    int r = (LOBYTE((n)>>16));
    int g = (LOBYTE(((WORD)(n)) >> 8));
    int b = LOBYTE(n);

    return RGB(r, g, b);
}


QColor WizStringToColor2(const CString& str)
{
    CString s = str;
    s.Trim('#');
    int n = WizHexToInt(s);
    //
    int r = (LOBYTE((n)>>16));
    int g = (LOBYTE(((WORD)(n)) >> 8));
    int b = LOBYTE(n);
    //
    return QColor(r, g, b);
}

std::string WizBSTR2UTF8(const CString& str)
{
    QByteArray utf8 = str.toUtf8();
    std::string s(utf8.constData());
    return s;
}

CString WizFormatString0(const CString& str)
{
    return str;
}

CString WizFormatString1(const CString& strFormat, const CString& strParam1)
{
    CString str(strFormat);
    str.replace("%1", strParam1);
    return str;
}

CString WizFormatString2(const CString& strFormat, const CString& strParam1, const CString& strParam2)
{
    CString str(strFormat);
    str.replace("%1", strParam1);
    str.replace("%2", strParam2);
    return str;
}

CString WizFormatString3(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3)
{
    CString str(strFormat);
    str.replace("%1", strParam1);
    str.replace("%2", strParam2);
    str.replace("%3", strParam3);
    return str;
}

CString WizFormatString4(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4)
{
    CString str(strFormat);
    str.replace("%1", strParam1);
    str.replace("%2", strParam2);
    str.replace("%3", strParam3);
    str.replace("%4", strParam4);
    return str;
}

CString WizFormatString5(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5)
{
    CString str(strFormat);
    str.replace("%1", strParam1);
    str.replace("%2", strParam2);
    str.replace("%3", strParam3);
    str.replace("%4", strParam4);
    str.replace("%5", strParam5);
    return str;
}

CString WizFormatString6(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6)
{
    CString str(strFormat);
    str.replace("%1", strParam1);
    str.replace("%2", strParam2);
    str.replace("%3", strParam3);
    str.replace("%4", strParam4);
    str.replace("%5", strParam5);
    str.replace("%6", strParam6);
    return str;
}

CString WizFormatString7(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6, const CString& strParam7)
{
    CString str(strFormat);
    str.replace("%1", strParam1);
    str.replace("%2", strParam2);
    str.replace("%3", strParam3);
    str.replace("%4", strParam4);
    str.replace("%5", strParam5);
    str.replace("%6", strParam6);
    str.replace("%7", strParam7);
    return str;
}

CString WizFormatString8(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6, const CString& strParam7, const CString& strParam8)
{
    CString str(strFormat);
    str.replace("%1", strParam1);
    str.replace("%2", strParam2);
    str.replace("%3", strParam3);
    str.replace("%4", strParam4);
    str.replace("%5", strParam5);
    str.replace("%6", strParam6);
    str.replace("%7", strParam7);
    str.replace("%8", strParam8);
    return str;
}

CString WizFormatInt(__int64 n)
{
    // According to the Si standard KB is 1000 bytes, KiB is 1024
    // but on windows sizes are calulated by dividing by 1024 so we do what they do.
    const quint64 kb = 1024;
    const quint64 mb = 1024 * kb;
    const quint64 gb = 1024 * mb;
    const quint64 tb = 1024 * gb;
    quint64 bytes = n;
    if (bytes >= tb)
        return QObject::tr("%1 TB").arg(QLocale().toString(qreal(bytes) / tb, 'f', 3));
    if (bytes >= gb)
        return QObject::tr("%1 GB").arg(QLocale().toString(qreal(bytes) / gb, 'f', 2));
    if (bytes >= mb)
        return QObject::tr("%1 MB").arg(QLocale().toString(qreal(bytes) / mb, 'f', 1));
    if (bytes >= kb)
        return QObject::tr("%1 KB").arg(QLocale().toString(bytes / kb));
    return QObject::tr("%1 byte(s)").arg(QLocale().toString(bytes));
}


time_t WizTimeGetTimeT(const COleDateTime& t)
{
    return t.toTime_t();
}

CString WizStringToSQL(const CString& str)
{
    if (str.GetLength() == 0)
    {
        return CString(_T("NULL"));
    }
    else
    {
        if (str.Find(_T('\'')) == -1)
        {
            return CString(_T("'")) + str + CString(_T("'"));
        }
        else
        {
            CString strRet = str;
            strRet.replace("'", "''");
            //
            return CString(_T("'")) + strRet + CString(_T("'"));
        }
    }
}

CString WizDateTimeToString(const COleDateTime& t)
{
    CString str;
    str.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
    //
    return str;
}

CString WizTimeToSQL(const COleDateTime& t)
{
    CString str = WizDateTimeToString(t);
    return CString(_T("'")) + str + _T("'");
}


CString WizColorToSQL(COLORREF cr)
{
    CString str;
    str.Format("'%s'",
               WizColorToString(cr).utf16()
               );
    return str;
}


const char* strncasestr(const char* str, const char* subStr)
{
    int len = strlen(subStr);
    if(len == 0)
    {
        return NULL;
    }

    while(*str)
    {
        if(strncasecmp(str, subStr, len) == 0)
        {
            return str;
        }
        str++;
    }
    return NULL;
}


CString WizHTMLGetCharsetFromHTMLText(const char* pBuffer)
{
    const char* pBegin = pBuffer;
    const char* p = pBegin;
    //
    while (1)
    {
        const char* p1 = ::strncasestr(p, "<meta");
        if (!p1)
            return CString();
        //
        const char* p2 = strchr(p1, '>');
        if (!p2)
            return CString();
        p2++;
        //
        std::string meta(p1, p2 - p1);
        CString strMeta(meta.c_str());
        //
        p = p2;
        //
        int nPos = (int)WizStrStrI_Pos(strMeta, "charset=");
        if (nPos == -1)
            continue;
        strMeta.Delete(0, nPos + 8);
        int nPos1 = strMeta.Find("\"");
        int nPos2 = strMeta.Find(">");
        if (nPos1 == -1)
            nPos = nPos2;
        else if (nPos2 == -1)
            nPos = nPos1;
        else
            nPos = std::min<int>(nPos1, nPos2);
        if (nPos == -1)
            return CString();
        //
        CString strCharset(strMeta.Mid(0, nPos));
        nPos = strCharset.Find(_T(" "));
        if (nPos == -1)
            return strCharset;
        strCharset = strCharset.Mid(0, nPos);
        return strCharset;
    }
}
CString WizXMLGetCharsetFromXMLText(const char* pBuffer)
{
    const char* pBegin = pBuffer;
    const char* p = pBegin;
    //
    CString strCharset;
    //
    const char* p1 = strncasestr(p, "<?xml");
    if (!p1)
        return strCharset;
    const char* p2 = strchr(p1, '>');
    if (!p2)
        return strCharset;
    //
    std::string mark(p1, p2 - p1);
    CString strMark(mark.c_str());
    //
    p = p2;
    //
    int nPos = (int)WizStrStrI_Pos(strMark, "encoding=");
    if (nPos == -1)
        return strCharset;
    //
    strMark.Delete(0, nPos + 9);
    //
    strMark.Replace('"', ' ');
    strMark.Replace('\'', ' ');
    strMark.Replace('?', ' ');
    //
    strMark.Trim();
    //
    strCharset = CString(strMark);
    //
    int nPosEnd = strCharset.Find(' ');
    if (-1 == nPosEnd)
        return strCharset;
    //
    return strCharset.Left(nPosEnd);
}

CString WizAnsiToUnicode(const CString& strCharset, const char* lpszText)
{
    QTextCodec* codec = QTextCodec::codecForName(strCharset.toUtf8());
    if (!codec)
    {
        return QString::fromLocal8Bit(lpszText);
    }
    //
    return codec->toUnicode(lpszText);
}

BOOL WizHtmlAnsiToUnicode(const char* lpszText, CString& strText)
{
    CString strCharset = WizHTMLGetCharsetFromHTMLText(lpszText);
    if (strCharset.IsEmpty())
    {
        strText = QString::fromLocal8Bit(lpszText);
    }
    else
    {
        strText = WizAnsiToUnicode(strCharset, lpszText);
    }
    return TRUE;
}

BOOL WizXmlAnsiToUnicode(const char* lpszText, CString& strText)
{
    CString strCharset = WizXMLGetCharsetFromXMLText(lpszText);
    if (strCharset.IsEmpty())
    {
        strText = QString::fromLocal8Bit(lpszText);
    }
    else
    {
        strText = WizAnsiToUnicode(strCharset, lpszText);
    }
    return TRUE;
}


template<class T>
void WizProcessText(char* pBuffer, size_t nBufferLen, BOOL bNoRemoveCr)
{
    BOOL bRemoveCr = !bNoRemoveCr;
    //
    T* pEnd = (T*)(pBuffer + nBufferLen);
    //
    T* p1 = (T*)pBuffer;
    T* p2 = p1;
    //
    while (p2 < pEnd)
    {
        if (*p2 == '\0')
        {
            p2++;
        }
        else if (*p2 == '\r')
        {
            T chNext = *(p2 + 1);
            if (chNext != '\n')
            {
                *p1 = '\n';
                p1++;
                p2++;
            }
            else
            {
                if (bRemoveCr)
                {
                    p2++;
                }
                else
                {
                    *p1 = *p2;
                    p1++;
                    p2++;
                }
            }
        }
        else
        {
            *p1 = *p2;
            p1++;
            p2++;
        }
    }
    *p1 = 0;
}

#define			WIZ_LTFF_FORCE_HTML				0x01
#define			WIZ_LTFF_FORCE_UTF8				0x02
#define			WIZ_LTFF_FORCE_UTF16			0x04
#define			WIZ_LTFF_NO_REMOVE_CR			0x08
#define			WIZ_LTFF_FORCE_XML				0x10

BOOL WizLoadUnicodeTextFromBuffer2(char* pBuffer, size_t nLen, CString& strText, UINT nFlags, const CString& strFileName)
{
    ATLASSERT(pBuffer);
    //
    BOOL bForceHTML		=	(nFlags & WIZ_LTFF_FORCE_HTML)	?	TRUE : FALSE;
    BOOL bForceUTF8		=	(nFlags & WIZ_LTFF_FORCE_UTF8)	?	TRUE : FALSE;
    BOOL bForceUTF16	=	(nFlags & WIZ_LTFF_FORCE_UTF16) ?	TRUE : FALSE;
    BOOL bNoRemoveCr	=	(nFlags & WIZ_LTFF_NO_REMOVE_CR)	?	TRUE : FALSE;
    BOOL bForceXML		=	(nFlags & WIZ_LTFF_FORCE_XML)	?	TRUE : FALSE;
    //
    ATLASSERT(!(bForceUTF8 && bForceUTF16));
    //
    BOOL bRet = FALSE;
    try
    {
        //check file type
        BYTE* pMark = (BYTE*)pBuffer;
        BOOL bUTF16AutoDetected = (pMark[0] == 0xFF && pMark[1] == 0xFE);
        BOOL bUTF8AutoDetected = (pMark[0] == 0xEF && pMark[1] == 0xBB && pMark[2] == 0xBF);
        //
        BOOL bUTF16 = (nLen >= 2) && (bForceUTF16 || bUTF16AutoDetected);
        BOOL bUTF8 =  (nLen >= 3) && (bForceUTF8  || bUTF8AutoDetected);
        //
        //remove \r \0
        if (bUTF16)
        {
            WizProcessText<unsigned short>(pBuffer, nLen, bNoRemoveCr);
        }
        else
        {
            WizProcessText<char>(pBuffer, nLen, bNoRemoveCr);
        }
        //
        //convert to unicode
        if (bUTF16)
        {
            const char* p = pBuffer;
            if (bUTF16AutoDetected)
            {
                p += 2;
            }
            //
            strText = QString::fromUtf16((unsigned short *)p);
            //
            bRet = TRUE;
        }
        else if (bUTF8)
        {
            const char* p = pBuffer;
            if (bUTF8AutoDetected)
            {
                p += 3;
            }
            strText = CString::fromUtf8(p);
            //
            bRet = TRUE;
        }
        else
        {
            CString strExt = WizExtractFileExt(CString(strFileName));
            strExt.MakeLower();
            if (bForceHTML || strExt.startsWith(".htm"))
            {
                bRet = WizHtmlAnsiToUnicode(pBuffer, strText);
            }
            else if (bForceXML || strExt.startsWith(".xml"))
            {
                bRet = WizXmlAnsiToUnicode(pBuffer, strText);
            }
            else if (strExt.startsWith(".php"))
            {
                strText = CString::fromUtf8(pBuffer);
                bRet = TRUE;
            }
            else
            {
                strText = CString::fromLocal8Bit(pBuffer);
                //
                bRet = TRUE;
            }
        }
    }
    catch(...)
    {
    }
    return bRet;
}

BOOL WizLoadUnicodeTextFromBuffer(const char* pBuffer, size_t nLen, CString& strText, UINT nFlags /* = 0 */, const CString& strFileName /*= ""*/)
{
    size_t nNewBufferLen = nLen + 4;
    char * pBufferNew = new char [nNewBufferLen];
    if (!pBufferNew)
        return FALSE;
    //
    memset(pBufferNew, 0, nNewBufferLen);
    //
    memcpy(pBufferNew, pBuffer, nLen);
    //
    BOOL bRet = WizLoadUnicodeTextFromBuffer2(pBufferNew, nLen, strText, nFlags, strFileName);
    //
    delete [] pBufferNew;
    //
    return bRet;
}


BOOL WizLoadUnicodeTextFromFile(const CString& strFileName, CString& strText)
{
    QFile file(strFileName);
    //
    __int64 size = file.size();
    //
     if (!file.open(QIODevice::ReadOnly))
         return false;
     //
     char* pBuffer = new char[size + 4];
     memset(pBuffer, 0, size + 4);
     file.read(pBuffer, size);
     file.close();
     //
     BOOL bRet = ::WizLoadUnicodeTextFromBuffer2(pBuffer, size, strText, 0, strFileName);
     //
     delete [] pBuffer;
     //
    return bRet;
}

BOOL WizSaveUnicodeTextToUnicodeFile(const CString& strFileName, const CString& strText)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    //
    file.write("\xff\xfe");
    file.write((const char*)strText.utf16(), strText.length() * 2);
    //
    file.close();
    //
    return TRUE;
}
BOOL WizSaveUnicodeTextToUtf8File(const CString& strFileName, const CString& strText)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    //
    file.write("\xef\xbb\xbf");
    file.write(strText.toUtf8());
    //
    file.close();
    //
    return TRUE;
}


BOOL WizSplitTextToArray(const CString& strText, QChar ch, CWizStdStringArray& arrayResult)
{
    QStringList strings = strText.split(ch);
    arrayResult.assign(strings.begin(), strings.end());
    return TRUE;
}

BOOL WizSplitTextToArray(CString strText, const CString& strSplitterText, BOOL bMatchCase, CWizStdStringArray& arrayResult)
{
    QStringList strings = strText.split(strSplitterText, QString::KeepEmptyParts, bMatchCase ? Qt::CaseSensitive : Qt::CaseInsensitive);
    arrayResult.assign(strings.begin(), strings.end());
    return TRUE;
}




void WizStringArrayToText(const CWizStdStringArray& arrayText, CString& strText, const CString& strSplitter)
{
    size_t nSplitterLen = 0;
    if (!strSplitter.IsEmpty())
    {
        nSplitterLen = strSplitter.GetLength();
    }
    //
    int nTextLen = 0;
    //
    int nLineCount = (int)arrayText.size();
    //
    for (int i = 0; i < nLineCount; i++)
    {
        nTextLen += arrayText[i].GetLength();
    }
    //
    size_t nCharCount = nSplitterLen * nLineCount + nTextLen + 1024;
    //
    int nBufferSize = int(nCharCount * sizeof(unsigned short));
    //
    CWizBufferAlloc ba(nBufferSize);
    unsigned short* pBuffer = (unsigned short *)ba.GetBuffer();
    if (!pBuffer)
        return;
    //
    unsigned short* pBufferEnd = pBuffer + nCharCount;
    //
    unsigned short* p = pBuffer;
    //
    for (int i = 0; i < nLineCount; i++)
    {
        const CString& strLine = arrayText[i];
        //
        ATLASSERT(p < pBufferEnd);
        memcpy(p, strLine.utf16(), strLine.GetLength() * sizeof(unsigned short));
        //
        p += strLine.GetLength();
        //
        if (i < nLineCount - 1)
        {
            ATLASSERT(p < pBufferEnd);
            memcpy(p, strSplitter.utf16(), nSplitterLen * sizeof(unsigned short));
            //
            p += nSplitterLen;
        }
    }
    //
    strText = pBuffer;
}

int WizFindInArray(const CWizStdStringArray& arrayText, const CString& strFind)
{
    int nCount = (int)arrayText.size();
    for (int i = 0; i < nCount; i++)
    {
        if (0 == arrayText[i].Compare(strFind))
            return i;
    }
    return -1;
}
int WizFindInArrayNoCase(const CWizStdStringArray& arrayText, const CString& strFind)
{
    int nCount = (int)arrayText.size();
    for (int i = 0; i < nCount; i++)
    {
        if (0 == arrayText[i].CompareNoCase(strFind))
            return i;
    }
    return -1;
}
void WizStringArrayEraseEmptyLine(CWizStdStringArray& arrayText)
{
    int nCount = (int)arrayText.size();
    for (int i = nCount - 1; i >= 0; i--)
    {
        CString strLine = arrayText[i];
        strLine.Trim();
        if (strLine.IsEmpty())
        {
            arrayText.erase(arrayText.begin() + i);
        }
    }
}


bool WizStringArrayCompareElementAsc(const CString& elem1, const CString& elem2)
{
    return elem2.compare( elem1) > 0;
}

bool WizStringArrayCompareElementDes(const CString& elem1, const CString& elem2)
{
    return elem1.compare(elem2) > 0;
}

void WizStringArraySort(CWizStdStringArray& arrayText, BOOL bAscending /*= TRUE*/)
{
    if (bAscending)
    {
        std::sort(arrayText.begin(), arrayText.end(), WizStringArrayCompareElementAsc);
    }
    else
    {
        std::sort(arrayText.begin(), arrayText.end(), WizStringArrayCompareElementDes);
    }
}

void WizStringArrayRemoveMultiElement(CWizStdStringArray& arrayText)
{
    WizStringArraySort(arrayText, true);
    //
    size_t nCount = arrayText.size();
    for (intptr_t i = nCount - 1; i > 0; i--)
    {
        if (arrayText[i] == arrayText[i - 1])
            arrayText.erase(arrayText.begin() + i);
    }
}
void WizStringArrayRemoveMultiElementNoCase(CWizStdStringArray& arrayText)
{
    WizStringArraySort(arrayText, true);
    //
    size_t nCount = arrayText.size();
    for (intptr_t i = nCount - 1; i > 0; i--)
    {
        if (0 == arrayText[i].CompareNoCase(arrayText[i - 1]))
            arrayText.erase(arrayText.begin() + i);
    }
}

BOOL WizStringSimpleSplit(const CString& str, char ch, CString& strLeft, CString& strRight)
{
    int nPos = str.Find(ch);
    if (-1 == nPos)
        return FALSE;
    //
    strLeft = str.Left(nPos);
    strRight = str.Right(str.GetLength() - nPos - 1);
    return TRUE;
}

CString WizDateToLocalString(const COleDateTime& t)
{
    Q_UNUSED(t);
    return CString();
}

intptr_t WizStrStrI_Pos(const CString& str, const CString& strFind, int nStart /*= 0*/)
{
    return str.indexOf(strFind, nStart, Qt::CaseInsensitive);
}

CString WizInt64ToStr(__int64 n)
{
    CString str;
    str.Format("%lld", n);
    return str;
}

CString WizGenGUIDLowerCaseLetterOnly()
{
    QUuid guid = QUuid::createUuid();
    CString str = guid.toString();
    str.Trim('{');
    str.Trim('}');
    return str;
}


CString WizGetComputerName()
{
    return "xxx";
}




/////////////////////////////////


CWizXMLNode::CWizXMLNode()
{
}


CWizXMLNode::CWizXMLNode(const QDomNode& node)
{
    InitData(node);
}

CWizXMLNode::CWizXMLNode(const CWizXMLNode& nodeSrc)
{
    if (nodeSrc.Valid())
    {
        InitData(nodeSrc.m_node);
    }
    else
    {
        Clear();
    }
}

CWizXMLNode::~CWizXMLNode()
{
}
CWizXMLNode& CWizXMLNode::operator = (const CWizXMLNode& right)
{
    if (!right.m_node.isNull())
    {
        InitData(right.m_node);
    }
    else
    {
        Clear();
    }
    //
    return *this;
}

void CWizXMLNode::InitData(const QDomNode& node)
{
    m_node = node;
}

BOOL CWizXMLNode::GetName(CString& strName)
{
    strName = m_node.nodeName();
    return TRUE;
}
CString CWizXMLNode::GetName()
{
    CString strName;
    GetName(strName);
    return strName;
}
CString CWizXMLNode::GetType()
{
    if (m_node.isNull())
        return CString();
    //
    switch (m_node.nodeType())
    {
    case QDomNode::ElementNode              :
        return "element";
    case QDomNode::AttributeNode            :
        return "attribute";
    case QDomNode::TextNode                 :
        return "text";
    case QDomNode::CDATASectionNode         :
        return "cdatasection";
    case QDomNode::EntityReferenceNode      :
        return "entityreference";
    case QDomNode::EntityNode               :
        return "entity";
    case QDomNode::ProcessingInstructionNode:
        return "processinginstruction";
    case QDomNode::CommentNode              :
        return "comment";
    case QDomNode::DocumentNode             :
        return "document";
    case QDomNode::DocumentTypeNode         :
        return "documenttype";
    case QDomNode::DocumentFragmentNode     :
        return "documentfragment";
    case QDomNode::NotationNode             :
        return "notation";
    case QDomNode::BaseNode                 :
        return "BaseNode";
    case QDomNode::CharacterDataNode        :
        return "CharacterDataNode";
    }
    return CString();
}


CString CWizXMLNode::GetText(const CString& strDefault /* = "" */)
{
    CString str;
    if (!GetText(str))
    {
        str = strDefault;
    }
    //
    return str;
}
BOOL CWizXMLNode::GetText(CString& strText)
{
    QDomNodeList nodes =  m_node.childNodes();
    if (nodes.isEmpty())
        return FALSE;
    //
    if (nodes.size() != 1)
        return FALSE;
    //
    QDomNode node = nodes.item(0);
    if (node.nodeType() != QDomNode::TextNode)
        return FALSE;
    //
    strText = node.nodeValue();
    return TRUE;
}
BOOL CWizXMLNode::SetText(const CString& strText)
{
    ATLASSERT(!m_node.isNull());
    //
    QDomNode child = m_node.ownerDocument().createTextNode(strText);
    //
    m_node.appendChild(child);
    return TRUE;
}
BOOL CWizXMLNode::GetAttributeText(const CString& strName, CString& strVal)
{
    QDomNamedNodeMap nodes = m_node.attributes();
    QDomNode node = nodes.namedItem(strName);
    if (node.isNull())
        return FALSE;
    //
    strVal = node.nodeValue();
    //
    return TRUE;
}
BOOL CWizXMLNode::GetAttributeInt(const CString& strName, int& nVal)
{
    CString strRet;
    if (!GetAttributeText(strName, strRet))
        return FALSE;
    //
    nVal = _ttoi(strRet);
    //
    return TRUE;
}
BOOL CWizXMLNode::GetAttributeInt64(const CString& strName, __int64& nVal)
{
    CString strRet;
    if (!GetAttributeText(strName, strRet))
        return FALSE;
    //
    nVal = _ttoi64(strRet);
    //
    return TRUE;
}
__int64 CWizXMLNode::GetAttributeInt64Def(const CString& strName, __int64 nDefault)
{
    CString strRet;
    if (!GetAttributeText(strName, strRet))
        return nDefault;
    //
    return _ttoi64(strRet);
}
BOOL CWizXMLNode::GetAttributeUINT(const CString& strName, UINT& nVal)
{
    int n;
    if (!GetAttributeInt(strName, n))
        return FALSE;
    //
    nVal = n;
    //
    return TRUE;
}
BOOL CWizXMLNode::GetAttributeTimeT(const CString& strName, time_t& nVal)
{
    __int64 n64;
    if (!GetAttributeInt64(strName, n64))
        return FALSE;
    //
    nVal = time_t(n64);
    //
    return TRUE;
}
BOOL CWizXMLNode::GetAttributeTimeString(const CString& strName, COleDateTime& t)
{
    CString str;
    if (!GetAttributeText(strName, str))
        return FALSE;
    //
    CString strError;
    if (!WizStringToDateTime(str, t, strError))
    {
        TOLOG(strError);
        return FALSE;
    }
    //
    return TRUE;
}

BOOL CWizXMLNode::GetAttributeDWORD(const CString& strName, DWORD& dwVal)
{
    int nVal = 0;
    if (!GetAttributeInt(strName, nVal))
        return FALSE;
    //
    dwVal = DWORD(nVal);
    //
    return TRUE;
}
BOOL CWizXMLNode::GetAttributeBool(const CString& strName, BOOL& bVal)
{
    CString strRet;
    if (!GetAttributeText(strName, strRet))
        return FALSE;
    //
    strRet.Trim();
    //
    bVal = strRet.Compare(_T("1")) == 0;
    //
    return TRUE;
}
//
CString CWizXMLNode::GetAttributeTextDef(const CString& strName, const CString& strDefault)
{
    CString strVal;
    if (!GetAttributeText(strName, strVal))
    {
        strVal = strDefault;
    }
    return strVal;
}
int CWizXMLNode::GetAttributeIntDef(const CString& strName, int nDefault)
{
    int nVal = 0;
    if (!GetAttributeInt(strName, nVal))
    {
        nVal = nDefault;
    }
    return nVal;
}
BOOL CWizXMLNode::GetAttributeBoolDef(const CString& strName, BOOL bDefault)
{
    BOOL bVal = FALSE;
    if (!GetAttributeBool(strName, bVal))
    {
        bVal = bDefault;
    }
    return bVal;
}
BOOL CWizXMLNode::SetAttributeText(const CString& strName, const CString& strText)
{
    ATLASSERT(!m_node.isNull());
    //
    QDomAttr att = m_node.ownerDocument().createAttribute(strName);
    att.setNodeValue(strText);
    //
    QDomNamedNodeMap nodes = m_node.attributes();
    nodes.setNamedItem(att);
    //
    return TRUE;
}
BOOL CWizXMLNode::SetAttributeInt(const CString& strName, int nVal)
{
    return SetAttributeText(strName, WizIntToStr(nVal));
}
BOOL CWizXMLNode::SetAttributeInt64(const CString& strName, __int64 nVal)
{
    return SetAttributeText(strName, WizInt64ToStr(nVal));
}
BOOL CWizXMLNode::SetAttributeTime(const CString& strName, const COleDateTime& t)
{
    return SetAttributeText(strName, WizDateTimeToString(t));
}

BOOL CWizXMLNode::SetAttributeBool(const CString& strName, BOOL bVal)
{
    return SetAttributeText(strName, bVal ? _T("1") : _T("0"));
}
BOOL CWizXMLNode::FindChildNode(const CString& strNodeName, CWizXMLNode& nodeChild)
{
    QDomNodeList nodes = m_node.childNodes();
    QDomNode node;
    if (!FindChildNode(nodes, strNodeName, node))
        return FALSE;
    //
    nodeChild.InitData(node);
    return TRUE;
}
BOOL CWizXMLNode::GetAllChildNodes(CWizStdStringArray& arrayNodeName)
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);
        CString strName = node.nodeName();
        //
        arrayNodeName.push_back(strName);
    }
    return TRUE;
}
BOOL CWizXMLNode::GetAllChildNodes(std::deque<CWizXMLNode>& arrayNodes)
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);
        arrayNodes.push_back(node);
    }
    //
    return TRUE;
}
BOOL CWizXMLNode::GetFirstChildNode(CWizXMLNode& nodeChild)
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    if (nCount <= 0)
        return FALSE;
    //
    CWizXMLNode node = nodes.item(0);
    //
    CString str = node.GetType();
    if (str != _T("element"))
        return FALSE;
    //
    nodeChild = node;
    //
    return TRUE;
}

CString CWizXMLNode::GetFirstChildNodeText(const CString& strDef /* = NULL */)
{
    CWizXMLNode nodeChild;
    if (!GetFirstChildNode(nodeChild))
        return strDef;
    //
    return nodeChild.GetText(strDef);
}

BOOL CWizXMLNode::DeleteChild(const CString& strChildName)
{
    CWizXMLNode node;
    if (!FindChildNode(strChildName, node))
        return TRUE;
    //
    m_node.removeChild(node.m_node);
    //
    return TRUE;
}
BOOL CWizXMLNode::DeleteChild(CWizXMLNode& nodeChild)
{
    m_node.removeChild(nodeChild.m_node);
    return TRUE;
}
BOOL CWizXMLNode::DeleteAllChild(const CString& strExceptNodeName1, const CString& strExceptNodeName2, const CString& strExceptNodeName3)
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    for (int i = nCount - 1; i >= 0; i--)
    {
        QDomNode node = nodes.item(i);
        //
        if (!strExceptNodeName1.IsEmpty()
            || !strExceptNodeName2.IsEmpty()
            || !strExceptNodeName3.IsEmpty()
            )
        {
            CString strName = node.nodeName();
            //
            if (!strExceptNodeName1.IsEmpty()
                && strName == strExceptNodeName1)
                continue;
            //
            if (!strExceptNodeName2.IsEmpty()
                && strName == strExceptNodeName2)
                continue;
            //
            if (!strExceptNodeName3.IsEmpty()
                && strName == strExceptNodeName3)
                continue;
        }
        //
        m_node.removeChild(node);
    }
    return TRUE;
}
BOOL CWizXMLNode::HasChildNode()
{
    return m_node.hasChildNodes();
}

int CWizXMLNode::GetChildNodesCount()
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    int nChildCount = nCount;
    //
    for (int i = 0; i < nCount; i++)
    {
        CWizXMLNode node = nodes.item(i);
        //
        nChildCount += node.GetChildNodesCount();
        //
    }
    return nChildCount;
}


BOOL CWizXMLNode::AppendChild(const CString& strNodeName, CWizXMLNode& nodeChild)
{
    QDomNode node = m_node.ownerDocument().createElement(strNodeName);
    //
    nodeChild = m_node.appendChild(node);
    //
    return TRUE;
}
BOOL CWizXMLNode::AppendChild(const CString& strNodeName, const CString& strChildNodeText)
{
    CWizXMLNode nodeChild;
    if (!AppendChild(strNodeName, nodeChild))
        return FALSE;
    //
    return nodeChild.SetText(strChildNodeText);
}

BOOL CWizXMLNode::SetChildNodeText(const CString& strNodeName, const CString& strText)
{
    CWizXMLNode nodeChild;
    if (!GetChildNode(strNodeName, nodeChild))
        return FALSE;
    //
    return nodeChild.SetText(strText);
}

BOOL CWizXMLNode::GetChildNodeText(const CString& strNodeName, CString& strText)
{
    CWizXMLNode nodeChild;
    if (!GetChildNode(strNodeName, nodeChild))
        return FALSE;
    //
    return nodeChild.GetText(strText);
}
CString CWizXMLNode::GetChildNodeTextDef(const CString& strNodeName, const CString& strDef)
{
    CString str;
    if (GetChildNodeText(strNodeName, str))
        return str;
    //
    return strDef;
}

BOOL CWizXMLNode::GetChildNode(const CString& strNodeName, CWizXMLNode& nodeChild)
{
    if (FindChildNode(strNodeName, nodeChild))
        return TRUE;
    return AppendChild(strNodeName, nodeChild);
}
BOOL CWizXMLNode::FindChildNode(const QDomNodeList& nodes, const CString& strName, QDomNode& nodeRet)
{
    long nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);
        CString strNodeName = node.nodeName();
        //
        if (strNodeName.CompareNoCase(strName) == 0)
        {
            nodeRet = node;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CWizXMLNode::FindNodeByPath(const CString& strPath, CWizXMLNode& nodeRet)
{
    CWizStdStringArray arrayText;
    if (!WizSplitTextToArray(strPath, '/', arrayText))
        return FALSE;
    //
    WizStringArrayEraseEmptyLine(arrayText);
    //
    if (arrayText.empty())
        return FALSE;
    //
    CWizXMLNode node = *this;
    //
    for (size_t i = 0; i < arrayText.size(); i++)
    {
        CString strNodeName = arrayText[i];
        //
        strNodeName.Trim();
        //
        if (strNodeName.IsEmpty())
            return FALSE;
        //
        CWizXMLNode nodeChild;
        if (!node.FindChildNode(strNodeName, nodeChild))
            return FALSE;
        //
        node = nodeChild;
    }
    //
    ATLASSERT(node.Valid());
    //
    nodeRet = node;
    //
    return TRUE;
}

BOOL CWizXMLNode::AppendNodeByPath(const CString& strPath, CWizXMLNode& nodeRet)
{
    CWizStdStringArray arrayText;
    if (!WizSplitTextToArray(strPath, '/', arrayText))
        return FALSE;
    //
    WizStringArrayEraseEmptyLine(arrayText);
    //
    if (arrayText.empty())
        return FALSE;
    //
    CWizXMLNode node = *this;
    //
    for (size_t i = 0; i < arrayText.size(); i++)
    {
        CString strNodeName = arrayText[i];
        //
        strNodeName.Trim();
        //
        if (strNodeName.IsEmpty())
            return FALSE;
        //
        CWizXMLNode nodeChild;
        if (!node.AppendChild(strNodeName, nodeChild))
        {
            TOLOG1(_T("Failed to append child node: %1"), strNodeName);
            return FALSE;
        }
        //
        node = nodeChild;
    }
    //
    ATLASSERT(node.Valid());
    //
    nodeRet = node;
    //
    return TRUE;
}

BOOL CWizXMLNode::AppendNodeSetTextByPath(const CString& strPath, const CString& strText)
{
    CWizXMLNode nodeRet;
    if (!AppendNodeByPath(strPath, nodeRet))
    {
        TOLOG1(_T("Failed to append node by path: %!"), strPath);
        return FALSE;
    }
    //
    BOOL bRet = nodeRet.SetText(strText);
    if (!bRet)
    {
        TOLOG(_T("Failed to set node text"));
        return FALSE;
    }
    //
    return TRUE;
}



BOOL CWizXMLNode::FindNodeTextByPath(const CString& strPath, CString& strRet)
{
    CWizXMLNode node;
    if (!FindNodeByPath(strPath, node))
        return FALSE;
    //
    return node.GetText(strRet);
}
BOOL CWizXMLNode::GetElementNodeByValue(const CString& strElementName, const CString& strValueName, const CString& strValue, CWizXMLNode& nodeRet)
{
    std::deque<CWizXMLNode> arrayNodes;
    if (!GetAllChildNodes(arrayNodes))
        return FALSE;
    //
    size_t nCount = arrayNodes.size();
    for (size_t i = 0; i < nCount; i++)
    {
        CWizXMLNode& node = arrayNodes[i];
        //
        if (node.GetName() != strElementName)
            continue;
        //
        CWizXMLNode nodeValue;
        if (!node.FindNodeByPath(strValueName, nodeValue))
            continue;
        //
        if (0 == nodeValue.GetText().CompareNoCase(strValue))
        {
            nodeRet = node;
            return TRUE;
        }
    }
    //
    return FALSE;
}
BOOL CWizXMLNode::GetElementOtherNodeByValue(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, CWizXMLNode& nodeRet)
{
    CWizXMLNode nodeElement;
    if (!GetElementNodeByValue(strElementName, strValueName, strValue, nodeElement))
        return FALSE;
    //
    return nodeElement.FindNodeByPath(strOtherNodePath, nodeRet);
}
BOOL CWizXMLNode::GetElementOtherNodeByValueReturnString(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, CString& strRet)
{
    CWizXMLNode nodeRet;
    if (!GetElementOtherNodeByValue(strElementName, strValueName, strValue, strOtherNodePath, nodeRet))
        return FALSE;
    //
    strRet = nodeRet.GetText();
    //
    return TRUE;
}
BOOL CWizXMLNode::GetElementOtherNodeByValueReturnInt(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, int& nRet)
{
    CString strRet;
    if (!GetElementOtherNodeByValueReturnString(strElementName, strValueName, strValue, strOtherNodePath, strRet))
        return FALSE;
    //
    strRet.Trim();
    //
    int nTemp = _ttoi(strRet);
    if (WizIntToStr(nTemp) == strRet)
    {
        nRet = nTemp;
        return TRUE;
    }
    //
    return FALSE;
}
BOOL CWizXMLNode::GetElementOtherNodeByValueReturnBool(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, BOOL& bRet)
{
    CString strRet;
    if (!GetElementOtherNodeByValueReturnString(strElementName, strValueName, strValue, strOtherNodePath, strRet))
        return FALSE;
    //
    strRet.Trim();
    strRet.MakeLower();
    //
    if (strRet == _T("0")
        || strRet == _T("false"))
    {
        bRet = FALSE;
        //
        return TRUE;
    }
    else if (strRet == _T("1")
        || strRet == _T("true"))
    {
        bRet = TRUE;
        //
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    //
    return TRUE;
}


//////////////////////////////////////////////////////////////////////

CWizXMLDocument::CWizXMLDocument()
{
    Create();
}

CWizXMLDocument::~CWizXMLDocument()
{
}
BOOL CWizXMLDocument::Create()
{
    return TRUE;
}
BOOL CWizXMLDocument::LoadXML(const CString& strXML)
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;
    if (m_doc.setContent(strXML, &errorMsg, &errorLine, &errorColumn))
        return true;
    //
    return false;
}
BOOL CWizXMLDocument::LoadFromFile(const CString& strFileName, BOOL bPromptError /*= TRUE*/)
{
    Q_UNUSED(bPromptError);

    if (!PathFileExists(strFileName))
        return FALSE;
    //
    CString strXml;
    if (!::WizLoadUnicodeTextFromFile(strFileName, strXml))
        return FALSE;
    //
    return LoadXML(strXml);
}
void CWizXMLDocument::Clear()
{
    m_doc.clear();
    Create();
}

//
BOOL CWizXMLDocument::FindChildNode(const CString& strName, CWizXMLNode& nodeChild)
{
    if (m_doc.isNull())
        return FALSE;
    //
    QDomNodeList nodes = m_doc.childNodes();
    //
    QDomNode node;
    if (!CWizXMLNode::FindChildNode(nodes, strName, node))
        return FALSE;
    //
    nodeChild.InitData(node);
    //
    return TRUE;
}
//
BOOL CWizXMLDocument::IsFail()
{
    return FALSE;
}
BOOL CWizXMLDocument::AppendChild(const CString& strNodeName, CWizXMLNode& nodeChild)
{
    if (m_doc.isNull())
    {
        m_doc = QDomDocument(strNodeName);
    }
    //
    QDomNode node = m_doc.createElement(strNodeName);
    nodeChild = m_doc.appendChild(node);
    //
    return TRUE;
}
BOOL CWizXMLDocument::GetChildNode(const CString& strName, CWizXMLNode& nodeChild)
{
    if (m_doc.isNull())
    {
        m_doc = QDomDocument(strName);
    }
    //
    if (FindChildNode(strName, nodeChild))
        return TRUE;
    return AppendChild(strName, nodeChild);
}

BOOL CWizXMLDocument::ToXML(CString& strText, BOOL bFormatText)
{
    if (m_doc.isNull())
        return FALSE;
    //
    strText = m_doc.toString();
    //
    if (bFormatText)
    {
        //WizFormatXML(strText);
    }
    //
    return TRUE;
}
BOOL CWizXMLDocument::ToUnicodeFile(const CString& strFileName)
{
    if (m_doc.isNull())
        return FALSE;
    //
    CString strText;
    if (!ToXML(strText, TRUE))
        return FALSE;
    //
    return WizSaveUnicodeTextToUnicodeFile(strFileName, strText);
}

BOOL CWizXMLDocument::GetAllChildNodes(CWizStdStringArray& arrayNodeName)
{
    if (m_doc.isNull())
        return FALSE;
    //
    QDomNodeList nodes = m_doc.childNodes();
    int nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);
        CString strName = node.nodeName();
        //
        arrayNodeName.push_back(strName);
    }
    return TRUE;
}
BOOL CWizXMLDocument::GetAllChildNodes(std::deque<CWizXMLNode>& arrayNodes)
{
    if (m_doc.isNull())
        return FALSE;
    //
    QDomNodeList nodes = m_doc.childNodes();
    int nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);
        //
        arrayNodes.push_back(node);
    }
    //
    return TRUE;
}
BOOL CWizXMLDocument::FindNodeByPath(const CString& strPath, CWizXMLNode& nodeRet)
{
    CWizStdStringArray arrayText;
    if (!WizSplitTextToArray(strPath, '/', arrayText))
        return FALSE;
    //
    WizStringArrayEraseEmptyLine(arrayText);
    //
    if (arrayText.empty())
        return FALSE;
    //
    CWizXMLNode nodeRoot;
    if (!FindChildNode(arrayText[0], nodeRoot))
        return FALSE;
    //
    CWizXMLNode node = nodeRoot;
    //
    for (size_t i = 1; i < arrayText.size(); i++)
    {
        CString strNodeName = arrayText[i];
        //
        strNodeName.Trim();
        //
        if (strNodeName.IsEmpty())
            return FALSE;
        //
        CWizXMLNode nodeChild;
        if (!node.FindChildNode(strNodeName, nodeChild))
            return FALSE;
        //
        node = nodeChild;
    }
    //
    ATLASSERT(node.Valid());
    //
    nodeRet = node;
    //
    return TRUE;
}

BOOL CWizXMLDocument::GetNodeTextByPath(const CString& strPath, CString& strRet)
{
    CWizXMLNode node;
    if (!FindNodeByPath(strPath, node))
        return FALSE;
    //
    return node.GetText(strRet);
}

BOOL CWizXMLDocument::SettingsGetSectionNode(const CString& strRootName, const CString& strNodeName, CWizXMLNode& node)
{
    CWizXMLNode nodeRoot;
    if (!GetChildNode(strRootName, nodeRoot))
    {
        //TOLOG1(_T("Failed to get root node by name: %1"), strRootName);
        return FALSE;
    }
    //
    if (!nodeRoot.GetChildNode(strNodeName, node))
    {
        //TOLOG1(_T("Failed to get section node by name: %1"), strNodeName);
        return FALSE;
    }
    //
    return TRUE;
}
BOOL CWizXMLDocument::SettingsFindSectionNode(const CString& strRootName, const CString& strNodeName, CWizXMLNode& node)
{
    CWizXMLNode nodeRoot;
    if (!FindChildNode(strRootName, nodeRoot))
    {
        //TOLOG1(_T("Failed to get root node by name: %1"), strRootName);
        return FALSE;
    }
    //
    if (!nodeRoot.FindChildNode(strNodeName, node))
    {
        //TOLOG1(_T("Failed to get section node by name: %1"), strNodeName);
        return FALSE;
    }
    //
    return TRUE;
}

BOOL CWizXMLDocument::SettingsGetChildNode(const CString& strRootName, const CString& strNodeName, const CString& strSubNodeName, CWizXMLNode& node)
{
    CWizXMLNode nodeRoot;
    if (!GetChildNode(strRootName, nodeRoot))
    {
        //TOLOG1(_T("Failed to get root node by name: %1"), strRootName);
        return FALSE;
    }
    //
    CWizXMLNode nodeParent;
    if (!nodeRoot.GetChildNode(strNodeName, nodeParent))
    {
        //TOLOG1(_T("Failed to get section node by name: %1"), strNodeName);
        return FALSE;
    }
    //
    if (!nodeParent.GetChildNode(strSubNodeName, node))
    {
        TOLOG1(_T("Failed to get key node by name: %1"), strSubNodeName);
        return FALSE;
    }
    //
    return TRUE;
}

BOOL CWizXMLDocument::SettingsGetStringValue(const CString& strRootName, const CString& strNodeName, const CString& strSubNodeName, const CString& strDefault, CString& strValue)
{
    CWizXMLNode node;
    if (!SettingsGetChildNode(strRootName, strNodeName, strSubNodeName, node))
    {
        TOLOG(_T("Failed to get value node"));
        return FALSE;
    }
    //
    strValue = node.GetText(strDefault);
    //
    return TRUE;
}
BOOL CWizXMLDocument::SettingsSetStringValue(const CString& strRootName, const CString& strNodeName, const CString& strSubNodeName, const CString& strValue)
{
    CWizXMLNode node;
    if (!SettingsGetChildNode(strRootName, strNodeName, strSubNodeName, node))
    {
        TOLOG(_T("Failed to get value node"));
        return FALSE;
    }
    //
    return node.SetText(strValue);
}

/*
BOOL WizFormatXML(CString& strXML)
{
    if (strXML.GetLength() > 10 * 1024)
        return TRUE;
    //
    int nBegin = -1;
    int nEnd = -1;
    for (int i = 1; i < strXML.GetLength(); i++)
    {
        if (strXML[i] == '>')
            nBegin = i + 1;
        else if (strXML[i] == '<')
            nEnd = i - 1;
        //
        if (nBegin != -1 && nEnd != -1 && nEnd >= nBegin)
        {
            CString strOld = strXML.Mid(nBegin, nEnd - nBegin + 1);
            if (!strOld.IsEmpty())
            {
                if (-1 != strOld.FindOneOf(_T("<>")))
                {
#ifdef __AFX_H__
                    ASSERT(FALSE);
                    TRACE0("Warning: Failed to format XML!");
#else
                    ATLASSERT(FALSE);
                    ATLTRACE(_T("Warning: Failed to format XML!"));
#endif
                    return FALSE;
                }
                //
                CString strNew = strOld;
                strNew.Trim();
                if (strNew != strOld)
                {
                    strXML.Delete(nBegin, strOld.GetLength());
                    if (!strNew.IsEmpty())
                    {
                        strXML.Insert(nBegin, strNew);
                    }
                    //
                    i = nBegin + strNew.GetLength();
                }
            }
            //
            nBegin = -1;
            nEnd = -1;
        }
    }
    //
    const const CString& strEndLine = _T("\n");
    //
    #define WizFormatXML_ADDINDENT2(str, strOffset)\
        str.Append(lpszEndLine);\
        str.Append(strOffset);
    //
    if (strXML.IsEmpty())
        return TRUE;

    CString strOffset(_T('\t'));
    //
    CString strRet;
    strRet.AppendChar(strXML[0]);
    //
    int nXMLLength = strXML.GetLength();
    for (int i = 1; i < nXMLLength; i++)
    {
        TCHAR ch = strXML[i];
        TCHAR chPrev = strXML[i - 1];
        TCHAR chNext = 0;
        if (i < nXMLLength - 1)
            chNext = strXML[i + 1];
        //
        BOOL bSelfClose = (ch == '>' && chPrev == '/');				//	/>
        BOOL bCloseTag  = (ch == '<' && chNext == '/');				//	 </
        //
        if (bCloseTag || bSelfClose)
        {
            strOffset.Delete(strOffset.GetLength() - 1);
            //
            if (bCloseTag)
            {
                //
                TCHAR chPrev1 = chPrev;
                TCHAR chPrev2 = 0;
                if (i >= 2)
                {
                    chPrev2 = strXML[i - 2];
                }
                //
                BOOL bPrevIsSelfClose = (chPrev2 == '/' && chPrev1 == '>');
                if (bPrevIsSelfClose)
                {
                    //<b><a/>Here</b>
                    WizFormatXML_ADDINDENT2(strRet, strOffset);
                }
                else
                {
                    //<b><a></a>Here</b>
                    int nPrevTagBegin = strRet.ReverseFind('<');
                    //
                    ATLASSERT(nPrevTagBegin != -1);
                    //
                    if (nPrevTagBegin != -1)
                    {
                        ATLASSERT(strRet.GetLength() > nPrevTagBegin + 1);
                        //
                        if (strRet[nPrevTagBegin + 1] == '/')
                        {
                            WizFormatXML_ADDINDENT2(strRet, strOffset);
                        }
                    }
                }
            }
        }
        else if (ch == '<')
        {
            WizFormatXML_ADDINDENT2(strRet, strOffset);
            //
            strOffset += '\t';
        }
        strRet.AppendChar(strXML[i]);
    }
    //
    strXML = strRet;
    //
    return TRUE;
}
*/




CWizBufferAlloc::CWizBufferAlloc(int nInitSize)
{
    m_nSize = 0;
    m_pBuffer = NULL;
    if (nInitSize > 0)
    {
        m_pBuffer = new BYTE[nInitSize];
        memset(m_pBuffer, 0, nInitSize);
        m_nSize = nInitSize;
    }
}
CWizBufferAlloc::~CWizBufferAlloc()
{
    if (m_pBuffer)
    {
        delete [] m_pBuffer;
    }
    m_pBuffer = NULL;
}

BYTE* CWizBufferAlloc::GetBuffer()
{
    return m_pBuffer;
}

BOOL CWizBufferAlloc::SetNewSize(int nNewSize)
{
    if (!m_pBuffer)
    {
        m_pBuffer = new BYTE[nNewSize];
        memset(m_pBuffer, 0, nNewSize);
        m_nSize = nNewSize;
    }
    else
    {
        if (m_nSize < nNewSize)
        {
            delete [] m_pBuffer;
            m_pBuffer = NULL;
            m_pBuffer = new BYTE[nNewSize];
            memset(m_pBuffer, 0, nNewSize);
            m_nSize = nNewSize;
        }
    }
    return !m_pBuffer;
}




BOOL WizBase64Encode(const QByteArray& arrayData, CString& str)
{
    const QByteArray base64 = arrayData.toBase64();
    //
    str = base64.constData();
    //
    return TRUE;
}

BOOL WizBase64Decode(const CString& str, QByteArray& arrayData)
{
    arrayData = QByteArray::fromBase64(str.toUtf8());
    //
    return TRUE;
}
CString WizStringToBase64(const CString& strSource)
{
    QByteArray arrayData = strSource.toUtf8();
    CString strRet;
    WizBase64Encode(arrayData, strRet);
    return strRet;
}

CString WizStringFromBase64(const CString& strBase64)
{
    QByteArray arrayData;
    WizBase64Decode(strBase64, arrayData);
    return CString::fromUtf8(arrayData);
}


CString WizGetSkinName()
{
#ifdef Q_OS_MAC
    return "default";
#else

#ifdef Q_OS_LINUX
    static CString strSkinName = WizGetString("skin", "Name", "ubuntu");
    if (strSkinName.isEmpty())
        strSkinName = "ubuntu";
#else
    static CString strSkinName = WizGetString("skin", "Name", "default");
    if (strSkinName.isEmpty())
        strSkinName = "default";
#endif
    //
    CString strPath = ::WizGetResourcesPath() + "skins/" + strSkinName + "/";
    if (!PathFileExists(strPath))
    {
        strSkinName = "default";
    }
    //
    return strSkinName;
#endif
}

#ifndef Q_OS_MAC
void WizSetSkinName(const CString& strName)
{
    WizSetString("skin", "Name", strName);
}
void WizSetSkinDisplayName(const CString& strDisplayName)
{
    std::map<CString, CString> skins;
    typedef std::map<CString, CString>::value_type SKIN;
    ::WizGetSkins(skins);
    foreach (const SKIN& skin, skins)
    {
        if (strDisplayName == skin.second)
        {
            WizSetSkinName(skin.first);
            return;
        }
    }
}

#endif


CString WizGetSkinPath()
{
    return WizGetResourcesPath() + "skins/" + WizGetSkinName() + "/";
}

#ifndef Q_OS_MAC
void WizGetSkins(std::map<CString, CString>& skins)
{
    CWizStdStringArray folders;
    ::WizEnumFolders(::WizGetResourcesPath() + "skins/", folders, 0);
    //
    QString localName = QLocale::system().name();

    foreach (const CString& path, folders)
    {
        CString strSkinFileName = path + "skin.ini";
        if (!PathFileExists(strSkinFileName))
            continue;
        //
        CString strSkinName = WizExtractLastPathName(path);
        //
        CWizSettings settings(strSkinFileName);
        CString strSkinDisplayName = settings.GetString("Common", "Name_" + localName);
        if (strSkinDisplayName.IsEmpty())
        {
            strSkinDisplayName = settings.GetString("Common", "Name");
        }
        if (strSkinDisplayName.IsEmpty())
        {
            strSkinDisplayName = strSkinName;
        }
        //
        skins[strSkinName] = strSkinDisplayName;
    }
}

#endif

CString WizGetSystemCustomSkinPath()
{
    return WizGetSkinPath() +
        #if defined(Q_OS_WIN32)
            "windows/";
        #elif defined(Q_OS_LINUX)
            "linux/";
        #elif defined(Q_OS_MAC)
            "mac/";
        #else
            "";
        #endif
}


CString WizGetSkinResourceFileName(const CString& strName)
{
    CString arrayPath[] =
    {
        WizGetSystemCustomSkinPath(),
        WizGetSkinPath()
    };
    //
    for (size_t i = 0; i < sizeof(arrayPath) / sizeof(CString); i++)
    {
        CString strFileName = arrayPath[i] + strName;

        if (!strFileName.endsWith(".png"))
        {
            strFileName.append(".png");
        }
        //
        if (::PathFileExists(strFileName))
        {
            return strFileName;
        }
    }
    //
    return CString();
}

QIcon WizLoadSkinIcon(const CString& strIconName)
{
    CString strFileName = WizGetSkinResourceFileName(strIconName);
    if (strFileName.isEmpty())
        return QIcon();
    //
    return QIcon(strFileName);
}

void WizHtml2Text(const CString& strHtml, CString& strText)
{
    QTextDocumentFragment doc = QTextDocumentFragment::fromHtml(strHtml);
    strText = doc.toPlainText();
    QChar ch(0xfffc);
    strText.replace(ch, QChar(' '));
}
void WizDeleteFolder(const CString& strPath)
{
    QDir dir(strPath);
    dir.cdUp();
    //
    if (dir.isRoot())
        return;
    //
    dir.rmdir(::WizExtractLastPathName(strPath));
}
void WizDeleteFile(const CString& strFileName)
{
    QDir dir(::WizExtractFilePath(strFileName));
    dir.remove(WizExtractFileName(strFileName));
}





#define WIZ_INVALID_CHAR_OF_FILE_NAME		"/\"?:*|<>\r\n\t,%\'"

BOOL WizIsValidFileNameNoPath(const CString& strFileName)
{
    if (strFileName.GetLength() >= MAX_PATH)
        return FALSE;
    //
    return -1 == strFileName.FindOneOf(WIZ_INVALID_CHAR_OF_FILE_NAME);
}


void WizMakeValidFileNameNoPath(CString& strFileName)
{
    if (WizIsValidFileNameNoPath(strFileName))
        return;
    //
    if (strFileName.GetLength() >= MAX_PATH)
        strFileName = strFileName.Left(MAX_PATH - 1);
    //
    while (1)
    {
        int nPos = strFileName.FindOneOf(WIZ_INVALID_CHAR_OF_FILE_NAME);
        if (-1 == nPos)
            break;
        //
        strFileName.SetAt(nPos,  _T('-'));
    }
    //
    while (strFileName.Find(_T("--")) != -1)
    {
        strFileName.Replace(_T("--"), _T("-"));
    }
    //
    strFileName.Trim();
    strFileName.Trim('-');
}

void WizMakeValidFileNameNoPathLimitLength(CString& strFileName, int nMaxTitleLength)
{
    WizMakeValidFileNameNoPath(strFileName);
    //
    CString strTitle = WizExtractFileTitle(strFileName);
    CString strExt = WizExtractFileExt(strFileName);
    //
    if (strTitle.GetLength() > nMaxTitleLength)
    {
        strFileName = strTitle.Left(nMaxTitleLength) + strExt;
    }
}

void WizMakeValidFileNameNoPathLimitFullNameLength(CString& strFileName, int nMaxFullNameLength)
{
    WizMakeValidFileNameNoPath(strFileName);
    //
    if (strFileName.GetLength() <= nMaxFullNameLength)
        return;
    //
    CString strExt = WizExtractFileExt(strFileName);
    //
    int nMaxTitleLength = nMaxFullNameLength - strExt.GetLength();
    //
    WizMakeValidFileNameNoPathLimitLength(strFileName, nMaxTitleLength);
}

CString WizMakeValidFileNameNoPathReturn(const CString& strFileName)
{
    CString strNew(strFileName);
    WizMakeValidFileNameNoPath(strNew);
    //
    return strNew;
}



CString WizStringArrayGetValue(const CWizStdStringArray& arrayText, const CString& valueName)
{
    CString strValueName = valueName + "=";
    int nValueNameLen = strValueName.GetLength();
    //
    size_t nCount = arrayText.size();
    for (size_t i = 0; i < nCount; i++)
    {
        CString strLine = arrayText[i];
        if (strLine.startsWith(strValueName))
        {
            CString str = strLine.Right(strLine.GetLength() - nValueNameLen);
            str.Trim();
            return str;
        }
    }
    //
    return CString();
}

void WizCommandLineToStringArray(const CString& commandLine, CWizStdStringArray& arrayLine)
{
    CString strCommandLine(commandLine);
    //
    strCommandLine.Insert(0, ' ');
    //
    WizSplitTextToArray(strCommandLine, _T(" /"), FALSE, arrayLine);
}
CString WizGetCommandLineValue(const CString& strCommandLine, const CString& strKey)
{
    CWizStdStringArray arrayLine;
    WizCommandLineToStringArray(strCommandLine, arrayLine);
    //
    return WizStringArrayGetValue(arrayLine, strKey);
}

BOOL WizSaveDataToFile(const CString& strFileName, const QByteArray& arrayData)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return FALSE;
    }
    //
    QDataStream out(&file);   // we will serialize the data into the file
    //
    out << arrayData;
    return TRUE;
}

BOOL WizLoadDataFromFile(const CString& strFileName, QByteArray& arrayData)
{
    arrayData.clear();
    if (!PathFileExists(strFileName))
        return FALSE;
    if (0 == ::WizGetFileSize(strFileName))
        return TRUE;
    //
    QFile file(strFileName);
    if (!file.open(QFile::ReadOnly))
        return FALSE;
    //
    arrayData = file.readAll();
    //
    return TRUE;
}

CWaitCursor::CWaitCursor()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

CWaitCursor::~CWaitCursor()
{
    QApplication::restoreOverrideCursor();
}
