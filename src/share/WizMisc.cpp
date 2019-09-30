#include "WizSettings.h"
#include "WizWebSettingsDialog.h"

#include <algorithm>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <QApplication>
#include <QTextDocument>
#include <QClipboard>
#include <QCursor>
#include <QBitmap>
#include <QPixmap>
#include <QPainter>
#include <QThread>
#include <QFileIconProvider>
#include <QSettings>
#include <QSvgRenderer>
#include <QDomDocument>
#include <QtCore>
#include <QNetworkConfigurationManager>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "utils/WizLogger.h"
#include "utils/WizPathResolve.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizMisc.h"
#include "mac/WizMacHelper.h"
#include "sync/WizApiEntry.h"
#include "share/WizAnalyzer.h"
#include "share/WizObjectOperator.h"
#include "share/WizEventLoop.h"
#include "share/WizUIBase.h"
#include "WizDatabaseManager.h"
#include "WizObjectDataDownloader.h"
#include "WizProgressDialog.h"


#ifndef MAX_PATH
#define MAX_PATH 200
#endif

QString WizGetFileSizeHumanReadalbe(const QString& strFileName)
{
    QFileInfo info(strFileName);

    int sz = info.size();

    if (sz < 1024) {
        return QString::number(sz) + "B";
    } else if (sz >= 1024 && sz < 1024*1024) {
        return QString::number(sz/1024) + "KB";
    } else if (sz >= 1024*1024 && sz < 1024*1024*1024) {
        return QString::number(sz/(1024*1024)) + "MB";
    } else {
        Q_ASSERT(0);
        return QString();
    }
}

void WizPathAddBackslash(QString& strPath)
{
    strPath.replace('\\', '/');

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

void WizEnsurePathExists(const CString& path)
{
    QDir dir;
    dir.mkpath(path);
}

void WizEnsureFileExists(const QString& strFileName)
{
    QFile file(strFileName);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite);
        file.close();
    }
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


void WizEnumFiles(const QString& path, const QString& strExts, CWizStdStringArray& arrayFiles, UINT uFlags)
{
    BOOL bIncludeSubDir = uFlags & EF_INCLUDESUBDIR;

    QString strPath(path);
    WizPathAddBackslash(strPath);

    QDir dir(strPath);

    QStringList nameFilters = strExts.split(";");
    QDir::Filters filters = QDir::Files;

    QStringList files = dir.entryList(nameFilters, filters);
    for (QStringList::const_iterator it = files.begin();
        it != files.end();
        it++)
    {
        arrayFiles.push_back(strPath + *it);
    }

    if (!bIncludeSubDir)
        return;

    CWizStdStringArray arrayFolder;
    WizEnumFolders(strPath, arrayFolder, 0);
    for (CWizStdStringArray::const_iterator it = arrayFolder.begin();
        it != arrayFolder.end();
        it++)
    {
        WizEnumFiles(*it, strExts, arrayFiles, uFlags);
    }
}

void WizEnumFolders(const QString& path, CWizStdStringArray& arrayFolders, UINT uFlags)
{
    BOOL bIncludeSubDir = uFlags & EF_INCLUDESUBDIR;

    QString strPath(path);
    WizPathAddBackslash(strPath);

    QDir dir(strPath);

    QDir::Filters filtersDir = QDir::Dirs;
    QStringList dirs = dir.entryList(filtersDir);

    for (QStringList::const_iterator it = dirs.begin();
        it != dirs.end();
        it++)
    {
        QString strName = *it;
        if (strName == "." || strName == "..")
            continue;

        QString strSubPath = strPath + strName + "/";
        arrayFolders.push_back(strSubPath);

        if (bIncludeSubDir)
            WizEnumFolders(strSubPath, arrayFolders, uFlags);
    }
}

QString WizFolderNameByPath(const QString& strPath)
{
    Q_ASSERT(!strPath.isEmpty());

    QStringList list = strPath.split("/");

    // the last item is null string, we return string before that.
    return list.at(list.size() - 2);
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
    if (!WizPathFileExists(strFileName))
        return;
    //
    CString strPath = Utils::WizMisc::extractFilePath(strFileName);
    CString strTitle = Utils::WizMisc::extractFileTitle(strFileName);
    CString strExt = Utils::WizMisc::extractFileExt(strFileName);
    //
    //
    const UINT nMaxLength = MAX_PATH - 10;
    if (strFileName.getLength() >= int(nMaxLength))
    {
        int nTitleLength = nMaxLength - (strPath.getLength() + strExt.getLength());
        if (nTitleLength <= 0)
        {
            TOLOG1("File name is too long: %1", strFileName);
            strTitle = WizIntToStr(WizGetTickCount());
        }
        else
        {
            ATLASSERT(strTitle.getLength() >= nTitleLength);
            if (strTitle.getLength() >= nTitleLength)
            {
                strTitle = strTitle.left(nTitleLength);
            }
        }
        //
        strFileName = strPath + strTitle + strExt;
    }
    //
    if (!WizPathFileExists(strFileName))
        return;
    //
    int nPos = strTitle.lastIndexOf('_');
    if (nPos != -1)
    {
        CString strTemp = strTitle.right(strTitle.getLength() - nPos - 1);
        if (strTemp == WizIntToStr(wiz_ttoi(strTemp)))
        {
            strTitle.remove(nPos, strTitle.getLength() - nPos);
        }
    }
    //
    int nIndex = 2;
    while (WizPathFileExists(strFileName))
    {
        strFileName.format("%s%s_%d%s", strPath.toUtf8().constData(), strTitle.toUtf8().constData(), nIndex, strExt.toUtf8().constData());
        nIndex++;
    }
}

QString WizEncryptPassword(const QString& strPassword)
{
    QString str;
    ::WizBase64Encode(strPassword.toUtf8(), str);
    return str;
}

QString WizDecryptPassword(const QString& strEncryptedText)
{
    QByteArray data;
    ::WizBase64Decode(strEncryptedText, data);
    return QString::fromUtf8(data);
}


QString WizGetAppFileName()
{
    QString strPath = QApplication::applicationFilePath();
    return strPath;
}

void WizGetTranslatedLocales(QStringList& locales)
{
    locales.append("zh_CN");
    locales.append("zh_TW");
    locales.append(WizGetDefaultTranslatedLocal());
}

QString WizGetTranslatedLocaleDisplayName(int index)
{
    switch (index) {
        case 0:
            return QObject::tr("Simplified Chinese");
        case 1:
            return QObject::tr("Traditional Chinese");
        case 2:
            return QObject::tr("English(US)");
        default:
            Q_ASSERT(index < 3);
            return QString();
    }
}

bool WizIsPredefinedLocation(const QString& strLocation)
{
    if (strLocation == "/Deleted Items/") {
        return true;
    }else if (strLocation == "/My Notes/") {
        return true;
    } else if (strLocation == "/My Journals/") {
        return true;
    } else if (strLocation == "/My Contacts/") {
        return true;
    } else if (strLocation == "/My Events/") {
        return true;
    } else if (strLocation == "/My Sticky Notes/") {
        return true;
    } else if (strLocation == "/My Emails/") {
        return true;
    } else if (strLocation == "/My Drafts/") {
        return true;
    } else if (strLocation == "/My Tasks/") {
        return true;
    } else if (strLocation == "/My Tasks/Inbox/") {
        return true;
    } else if (strLocation == "/My Tasks/Completed/") {
        return true;
    }

    return false;
}

QString WizLocation2Display(const QString& strLocation)
{
    QString strLoc = strLocation;
    if (strLocation.startsWith("/Deleted Items/")) {
        strLoc.replace("/Deleted Items/", QObject::tr("/Deleted Items/"));
    }
    else if (strLocation.startsWith("/My Notes/"))
    {
        strLoc.replace("/My Notes/", QObject::tr("/My Notes/"));
    }
    else if (strLocation.startsWith("/My Journals/"))
    {
        strLoc.replace("/My Journals/", QObject::tr("/My Journals/"));
    }
    else if (strLocation.startsWith("/My Contacts/"))
    {
        strLoc.replace("/My Contacts/", QObject::tr("/My Contacts/"));
    }
    else if (strLocation.startsWith("/My Events/"))
    {
        strLoc.replace("/My Events/", QObject::tr("/My Events/"));
    }
    else if (strLocation.startsWith("/My Sticky Notes/"))
    {
        strLoc.replace("/My Sticky Notes/", QObject::tr("/My Sticky Notes/"));
    }
    else if (strLocation.startsWith("/My Emails/"))
    {
        strLoc.replace("/My Emails/", QObject::tr("/My Emails/"));
    }
    else if (strLocation.startsWith("/My Drafts/"))
    {
        strLoc.replace("/My Drafts/", QObject::tr("/My Drafts/"));
    }
    else if (strLocation.startsWith("/My Tasks/"))
    {
        strLoc.replace("/My Tasks/", QObject::tr("/My Tasks/"));
    }
    else if (strLocation.startsWith("/My Tasks/Inbox/"))
    {
        strLoc.replace("/My Tasks/Inbox/", QObject::tr("/My Tasks/Inbox/"));
    }
    else if (strLocation.startsWith("/My Tasks/Completed/"))
    {
        strLoc.replace("/My Tasks/Completed/", QObject::tr("/My Tasks/Completed/"));
    }

    return strLoc;
}



QString WizGetLogTime()
{
    QDateTime strTime = QDateTime::currentDateTime();
    QString strCurrentTime = QString("%1.%2 %3").arg(\
                strTime.date().month()).arg(\
                strTime.date().day()).arg(\
                strTime.time().toString());

    return strCurrentTime;
}

QString WizGetEmailPrefix(const QString& strMail)
{
    int n = strMail.indexOf('@');
    if (n == -1) {
        return QString();
    }

    return strMail.left(n);
}

CString WizIntToStr(int n)
{
    CString str;
    str.format("%d", n);
    return str;
}

QString WizGetTimeStamp()
{
    return QString::number(QDateTime::currentDateTime().toTime_t());
}

WizOleDateTime WizGetCurrentTime()
{
    WizOleDateTime t;
    return t;
}

bool WizStringToDateTime(const QString& str, WizOleDateTime& t, QString& strError)
{
    std::string utf8 = ::WizBSTR2UTF8(str);
    const char* lpsz = utf8.c_str();
    //
    t = WizGetCurrentTime();
    //
    if (!lpsz)
    {
        strError = "NULL pointer";
        return TRUE;
    }
    //
    if (strlen(lpsz) != 19)
    {
        strError = "Invalid date time format";
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
    if (nYear < 0)
    {
        strError = "Invalid date time format (year)";
        return TRUE;
    }
    if (nMonth < 1 || nMonth > 12)
    {
        strError = "Invalid date time format (month)";
        return TRUE;
    }
    if (nDay < 1 || nDay > 31)
    {
        strError = "Invalid date time format (day)";
        return TRUE;
    }
    if (nHour < 0 || nHour > 23)
    {
        strError = "Invalid date time format (hour)";
        return TRUE;
    }
    if (nMin < 0 || nMin > 59)
    {
        strError = "Invalid date time format (minute)";
        return TRUE;
    }
    if (nSec < 0 || nSec > 59)
    {
        strError = "Invalid date time format (second)";
        return TRUE;
    }
    //
    t = WizOleDateTime(nYear, nMonth, nDay, nHour, nMin, nSec);
    //
    return TRUE;
}


WizOleDateTime WizStringToDateTime(const CString& str)
{
    WizOleDateTime t;
    CString strError;
    WizStringToDateTime(str, t, strError);
    return t;
}




BOOL WizIso8601StringToDateTime(CString str, WizOleDateTime& t, CString& strError)
{
    if (str.length() != 17)
    {
        strError = "Invalid date time format";
        return FALSE;
    }
    //xxxxxxxxTxx:xx:xx
    //01234567890123456
    str.setAt(8, ' ');
    str.insert(6, '-');
    str.insert(4, '-');
    return WizStringToDateTime(str, t, strError);
}

CString WizDateTimeToIso8601String(const WizOleDateTime& t)
{
    CString str;
    str.format("%04d%02d%02dT%02d:%02d:%02d", t.getYear(), t.getMonth(), t.getDay(), t.getHour(), t.getMinute(), t.getSecond());
    //
    return str;
}

CString WizIntToHexEx(int n, int nWidth, BOOL bWithPrefix /*= FALSE*/)
{
    CString strFormat = CString("%0") + WizIntToStr(nWidth) + "X";
    //
    CString strValue;
    strValue.sprintf(strFormat.toUtf8(), n);
    //
    if (bWithPrefix)
    {
        return CString("0x") + strValue;
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
        strText.remove(0, 2);
    //
    strText.trim();
    strText.makeLower();
    //
    int nValue = 0;
    for (int i = 0; i < strText.getLength(); i++)
    {
        QChar ch = strText[i];
        if (ch >= '0' && ch <= '9')
        {
            nValue = (nValue * 16) + ch.unicode() - '0';
        }
        else if (ch >= 'a' &&ch <= 'f')
        {
            nValue = (nValue * 16) + ch.unicode() - 'a' + 10;
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
CString WizColorToString(const QColor& cr)
{
    return WizIntToHexEx(cr.red() , 2, FALSE)
        + WizIntToHexEx(cr.green(), 2, FALSE)
        + WizIntToHexEx(cr.blue(), 2, FALSE);
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
    s.trim('#');
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

CString WizFormatString1(const CString& strFormat, int n)
{
    CString str(strFormat);
    str.replace("%1", QString::number(n));
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


time_t WizTimeGetTimeT(const WizOleDateTime& t)
{
    return t.toTime_t();
}

CString WizStringToSQL(const CString& str)
{
    if (str.isEmpty()) {
        return "NULL";
    }

    CString strSql = str;
    if (str.indexOf('\'') == -1) {
        strSql = "'" + strSql + "'";
    } else {
        strSql.replace("'", "''");
        strSql = "'" + strSql + "'";
    }

    return strSql;
}

CString WizDateTimeToString(const WizOleDateTime& t)
{
    CString str;
    str.format("%04d-%02d-%02d %02d:%02d:%02d", t.getYear(), t.getMonth(), t.getDay(), t.getHour(), t.getMinute(), t.getSecond());
    //
    return str;
}

CString WizTimeToSQL(const WizOleDateTime& t)
{
    CString str = WizDateTimeToString(t);
    return CString("'") + str + "'";
}

CString WizTimeToSQL(const QDateTime& t)
{
    WizOleDateTime t2(t);
    CString str = WizDateTimeToString(t2);
    return CString("'") + str + "'";
}



CString WizColorToSQL(COLORREF cr)
{
    CString str;
    str.format("'%s'",
               WizColorToString(cr).utf16()
               );
    return str;
}
CString WizColorToSQL(const QColor& cr)
{
    CString str;
    str.format("'%s'",
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
#ifdef WIN32
        if(_strnicmp(str, subStr, len) == 0)
#else
        if(strncasecmp(str, subStr, len) == 0)
#endif
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
        strMeta.remove(0, nPos + 8);
        int nPos1 = strMeta.find("\"");
        int nPos2 = strMeta.find(">");
        if (nPos1 == -1)
            nPos = nPos2;
        else if (nPos2 == -1)
            nPos = nPos1;
        else
            nPos = std::min<int>(nPos1, nPos2);
        if (nPos == -1)
            return CString();
        //
        CString strCharset(strMeta.mid(0, nPos));
        nPos = strCharset.find(" ");
        if (nPos == -1)
            return strCharset;
        strCharset = strCharset.mid(0, nPos);
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
    strMark.remove(0, nPos + 9);
    //
    strMark.replace('"', ' ');
    strMark.replace('\'', ' ');
    strMark.replace('?', ' ');
    //
    strMark.trim();
    //
    strCharset = CString(strMark);
    //
    int nPosEnd = strCharset.find(' ');
    if (-1 == nPosEnd)
        return strCharset;
    //
    return strCharset.left(nPosEnd);
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

bool WizHtmlAnsiToUnicode(const char* lpszText, QString& strText)
{
    CString strCharset = WizHTMLGetCharsetFromHTMLText(lpszText);
    if (strCharset.isEmpty())
    {
        strText = QString::fromLocal8Bit(lpszText);
    }
    else
    {
        strText = WizAnsiToUnicode(strCharset, lpszText);
    }
    return TRUE;
}

bool WizXmlAnsiToUnicode(const char* lpszText, QString& strText)
{
    CString strCharset = WizXMLGetCharsetFromXMLText(lpszText);
    if (strCharset.isEmpty())
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

bool WizLoadUnicodeTextFromBuffer2(char* pBuffer, size_t nLen, QString& strText, UINT nFlags, const QString& strFileName)
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

        //convert to unicode
        if (bUTF16)
        {
            const char* p = pBuffer;
            if (bUTF16AutoDetected) {
                p += 2;
            }
            strText = QString::fromUtf16((ushort *)p);
            bRet = true;
        }
        else if (bUTF8)
        {
            const char* p = pBuffer;
            if (bUTF8AutoDetected) {
                p += 3;
            }

            strText = QString::fromUtf8(p);
            bRet = true;
        }
        else
        {
            CString strExt = Utils::WizMisc::extractFileExt(CString(strFileName));
            strExt.makeLower();
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


bool WizLoadUnicodeTextFromFile(const QString& strFileName, QString& strText)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    strText = stream.readAll();
    file.close();

    return true;
}

bool WizLoadUtf8TextFromFile(const QString& strFileName, QString& strText)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    strText = stream.readAll();
    file.close();

    return true;
}


bool WizLoadTextFromResource(const QString& resourceName, QString& text)
{
    QFile data(resourceName);
    if (data.open(QFile::ReadOnly)) {
        QTextStream in(&data);
        text = in.readAll();
        return true;
    }
    return false;
}


bool WizSaveUnicodeTextToUtf16File(const CString& strFileName, const CString& strText)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write("\xff\xfe");
    file.write((const char*)strText.utf16(), strText.length() * 2);
    file.close();

    return true;
}

bool WizSaveUnicodeTextToUtf8File(const QString& strFileName, const QString& strText)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(true);
    stream << strText;
    stream.flush();
    file.close();

    return true;
}

bool WizSaveUnicodeTextToUtf8File(const QString& strFileName, const QByteArray& strText)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(true);
    stream << strText;
    stream.flush();
    file.close();

    return true;
}


bool WizSaveUnicodeTextToUtf8File(const QString& strFileName, const QString& strText, bool addBom)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(addBom ? true : false);
    stream << strText;
    stream.flush();
    file.close();

    return true;
}

bool WizSaveUnicodeTextToData(QByteArray& data, const QString& strText, bool addBom)
{
    QTextStream stream(&data);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(addBom ? true : false);
    stream << strText;
    stream.flush();
    return true;
}


BOOL WizSplitTextToArray(const CString& strText, QChar ch, CWizStdStringArray& arrayResult)
{
    QStringList strings = strText.split(ch, QString::SkipEmptyParts);
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
    if (!strSplitter.isEmpty())
    {
        nSplitterLen = strSplitter.getLength();
    }
    //
    int nTextLen = 0;
    //
    int nLineCount = (int)arrayText.size();
    //
    for (int i = 0; i < nLineCount; i++)
    {
        nTextLen += arrayText[i].getLength();
    }
    //
    size_t nCharCount = nSplitterLen * nLineCount + nTextLen + 1024;
    //
    int nBufferSize = int(nCharCount * sizeof(unsigned short));
    //
    WizBufferAlloc ba(nBufferSize);
    unsigned short* pBuffer = (unsigned short *)ba.getBuffer();
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
        memcpy(p, strLine.utf16(), strLine.getLength() * sizeof(unsigned short));
        //
        p += strLine.getLength();
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
    for (int i = 0; i < nCount; i++) {
        if (0 == arrayText[i].compare(strFind))
            return i;
    }

    return -1;
}

int WizFindInArrayNoCase(const CWizStdStringArray& arrayText, const CString& strFind)
{
    int nCount = (int)arrayText.size();
    for (int i = 0; i < nCount; i++)
    {
        if (0 == arrayText[i].compareNoCase(strFind))
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
        strLine.trim();
        if (strLine.isEmpty())
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
        if (0 == arrayText[i].compareNoCase(arrayText[i - 1]))
            arrayText.erase(arrayText.begin() + i);
    }
}

BOOL WizStringSimpleSplit(const CString& str, char ch, CString& strLeft, CString& strRight)
{
    int nPos = str.find(ch);
    if (-1 == nPos)
        return FALSE;
    //
    strLeft = str.left(nPos);
    strRight = str.right(str.getLength() - nPos - 1);
    return TRUE;
}

CString WizDateToLocalString(const WizOleDateTime& t)
{
    WizOleDateTime localDateTime = t.toLocalTime();
    return localDateTime.toString(Qt::DefaultLocaleShortDate);
}

intptr_t WizStrStrI_Pos(const CString& str, const CString& strFind, int nStart /*= 0*/)
{
    return str.indexOf(strFind, nStart, Qt::CaseInsensitive);
}

CString WizInt64ToStr(__int64 n)
{
    CString str;
    str.format("%lld", n);
    return str;
}

CString WizGenGUIDLowerCaseLetterOnly()
{
    QUuid guid = QUuid::createUuid();
    CString str = guid.toString();
    str.trim('{');
    str.trim('}');
    return str;
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
                if (-1 != strOld.FindOneOf("<>"))
                {
#ifdef __AFX_H__
                    ASSERT(FALSE);
                    TRACE0("Warning: Failed to format XML!");
#else
                    ATLASSERT(FALSE);
                    ATLTRACE("Warning: Failed to format XML!");
#endif
                    return FALSE;
                }
                //
                CString strNew = strOld;
                strNew.Trim();
                if (strNew != strOld)
                {
                    strXML.remove(nBegin, strOld.GetLength());
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
    const const CString& strEndLine = "\n";
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
            strOffset.remove(strOffset.GetLength() - 1);
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


WizBufferAlloc::WizBufferAlloc(int nInitSize)
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
WizBufferAlloc::~WizBufferAlloc()
{
    if (m_pBuffer)
    {
        delete [] m_pBuffer;
    }
    m_pBuffer = NULL;
}

BYTE* WizBufferAlloc::getBuffer()
{
    return m_pBuffer;
}

BOOL WizBufferAlloc::setNewSize(int nNewSize)
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

BOOL WizBase64Encode(const QByteArray& arrayData, QString& str)
{
    const QByteArray base64 = arrayData.toBase64();
    str = base64.constData();

    return TRUE;
}

BOOL WizBase64Decode(const QString& str, QByteArray& arrayData)
{
    arrayData = QByteArray::fromBase64(str.toUtf8());
    return true;
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

QString WizGetDefaultSkinName()
{
//#ifdef Q_OS_LINUX
//    return "ubuntu";
//#else
    return "default";
//#endif
}

QString WizGetSkinResourcePath(const QString& strSkinName)
{
    return Utils::WizPathResolve::skinResourcesPath(strSkinName);
}

void WizGetSkins(QStringList& skins)
{
    CWizStdStringArray folders;
    ::WizEnumFolders(Utils::WizPathResolve::resourcesPath() + "skins/", folders, 0);

    foreach (const CString& path, folders)
    {
        QString strSkinFileName = path + "skin.ini";
        if (!WizPathFileExists(strSkinFileName))
            continue;

        skins.append(Utils::WizMisc::extractLastPathName(path));
    }
}

#ifdef WIZNOTE_OBSOLETE

// FIXME: remove from wizmisc!
QString WizGetSkinDisplayName(const QString& strSkinName, const QString& strLocale)
{
    WizSettings settings(::WizGetSkinResourcePath(strSkinName) + "skin.ini");

    QString strSkinDisplayName = settings.getString("Common", "Name_" + strLocale);
    if (!strSkinDisplayName.isEmpty()) {
        return strSkinDisplayName;
    }

    strSkinDisplayName = settings.getString("Common", "Name");
    if (!strSkinDisplayName.isEmpty()) {
        return strSkinDisplayName;
    }

    return strSkinName;
}

#endif // WIZNOTE_OBSOLETE

QString WizGetSystemCustomSkinPath(const QString& strSkinName)
{
    return WizGetSkinResourcePath(strSkinName) +
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

QString WizGetSkinResourceFileName(const QString& strSkinName, const QString& strName)
{
    if (strSkinName.isEmpty()) {
        if (strName.indexOf("/") != -1)
            return strName;
        if (strName.indexOf("\\") != -1)
            return strName;
    }
    //
    QString arrayPath[] =
    {
        WizGetSystemCustomSkinPath(strSkinName),
        WizGetSkinResourcePath(strSkinName)
    };

    QStringList suffixList;

    QString ext = Utils::WizMisc::extractFileExt(strName);
    if (ext.isEmpty()) {
        suffixList << ".svg" << ".png";
    } else {
        suffixList << "";
    }
    //
    for (size_t i = 0; i < sizeof(arrayPath) / sizeof(QString); i++)
    {
        QStringList::const_iterator it;
        for (it = suffixList.begin(); it != suffixList.end(); it++) {
            QString strFileName = arrayPath[i] + strName + *it;
            if (::WizPathFileExists(strFileName)) {
                return strFileName;
            }
        }
    }

#ifdef QT_DEBUG
     qDebug() << strName;
#endif
    return QString();
}


void SetDomAttrRecur(QDomElement &elem, QString strtagname, QString strattr, QString strattrval)
{
    // if it has the tagname then overwritte desired attribute
    if (elem.tagName().compare(strtagname) == 0)
    {
        elem.setAttribute(strattr, strattrval);
    }
    // loop all children
    for (int i = 0; i < elem.childNodes().count(); i++)
    {
        if (!elem.childNodes().at(i).isElement())
        {
            continue;
        }
        //
        QDomElement childElem = elem.childNodes().at(i).toElement();
        SetDomAttrRecur(childElem, strtagname, strattr, strattrval);
    }
}

QPixmap svg2Pixmap(const QByteArray& svgContent,
   const QSize& size,
   QColor color = Qt::transparent,
   QPainter::CompositionMode mode = QPainter::CompositionMode_SourceOver)
{
    QDomDocument doc;
    doc.setContent(svgContent);
    //
    QDomElement root = doc.documentElement();
    if (color != Qt::transparent) {
        SetDomAttrRecur(root, "path", "fill", color.name());
        SetDomAttrRecur(root, "polygon", "fill", color.name());
    }
    //
    QSvgRenderer rr(doc.toByteArray());
    QImage image(size.width(), size.height(), QImage::Format_ARGB32);
    QPainter painter(&image);
    painter.setCompositionMode(mode);
    image.fill(Qt::transparent);
    rr.render(&painter);
    return QPixmap::fromImage(image);
}


void svgAddToIcon(QIcon& icon, const QByteArray& svgContent, const QSize& size, QColor color, QIcon::Mode mode)
{
    QPixmap p1 = svg2Pixmap(svgContent, size, color);
    icon.addPixmap(p1, mode);
    //
#ifdef Q_OS_MAC
    QSize sizeScaled = QSize(size.width() * 2, size.height() * 2);
    QPixmap p2 = svg2Pixmap(svgContent, sizeScaled, color);
    p2.setDevicePixelRatio(2);
    icon.addPixmap(p2, mode);
#else
//    static int rate = 0;
//    if (!rate) {
//        //
//        rate = WizSmartScaleUI(100);
//    }
//    //
//    QSize sizeScaled = QSize(WizSmartScaleUI(size.width()), WizSmartScaleUI(size.height()));
//    QPixmap p2 = svg2Pixmap(svgContent, sizeScaled, Qt::transparent);
//    p2.setDevicePixelRatio(rate / 100.0);
//    icon.addPixmap(p2, mode);
#endif
}

QIcon svg2Icon(const QByteArray& svgContent, const QSize& size, const WizIconOptions& options)
{
    QIcon icon;
    if (isDarkMode()) {
        //
        svgAddToIcon(icon, svgContent, size, options.darkColor, QIcon::Normal);
        if (options.darkSelectedColor != Qt::transparent) {
            svgAddToIcon(icon, svgContent, size, options.darkSelectedColor, QIcon::Selected);
            svgAddToIcon(icon, svgContent, size, options.darkSelectedColor, QIcon::Active);
        }
    } else {
        svgAddToIcon(icon, svgContent, size, Qt::transparent, QIcon::Normal);
        if (options.selectedColor != Qt::transparent) {
            svgAddToIcon(icon, svgContent, size, options.selectedColor, QIcon::Selected);
            svgAddToIcon(icon, svgContent, size, options.selectedColor, QIcon::Active);
        }
    }
    return icon;
}

QIcon svg2Icon(QString svgFile, const QSize& size, const WizIconOptions& options) {

    QFile file(svgFile);
    file.open(QFile::ReadOnly);
    QByteArray bytes = file.readAll();
    return svg2Icon(bytes, size, options);
}

void addPngToIcon(QIcon& icon, const QString& fileName, QColor color, QIcon::Mode mode)
{
    QPixmap pixmap(fileName);
    if (!pixmap.isNull()) {
        if (color != Qt::transparent) {
            pixmap = qpixmapWithTintColor(pixmap, color);
        }
        icon.addPixmap(pixmap, mode);
    }
    //
    QString x2FileName = Utils::WizMisc::extractFilePath(fileName) +
            Utils::WizMisc::extractFileTitle(fileName) + "@2x" + Utils::WizMisc::extractFileExt(fileName);
    //
    QPixmap x2Pixmap(x2FileName);
    if (!x2Pixmap.isNull()) {
        if (color != Qt::transparent) {
            x2Pixmap = qpixmapWithTintColor(x2Pixmap, color);
        }
        icon.addPixmap(x2Pixmap, mode);
    }
}

void WizScaleIconToSize(QIcon& icon, QSize size)
{
    QIcon::Mode states[] = {QIcon::Normal, QIcon::Selected, QIcon::Active};
    //
    for (auto mode : states) {
        //
        QPixmap pixmap = icon.pixmap(size, mode);
        if (pixmap.size() != size) {
            //
            if (pixmap.size().width() < size.width()) {
                auto sizes = icon.availableSizes();
                for (auto s : sizes) {
                    if (s.width() >= size.width()) {
                        pixmap = icon.pixmap(s);
                        break;
                    }
                }
            }
            //
            pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            icon.addPixmap(pixmap, mode);
        }
    }
}

QIcon WizLoadPngSkinIcon(const QString& fileName, const QSize& iconSize, const WizIconOptions& options)
{
    QIcon icon;
    if (isDarkMode()) {
        //
        addPngToIcon(icon, fileName, options.darkColor, QIcon::Normal);
        //
        if (options.darkSelectedColor != Qt::transparent) {
            addPngToIcon(icon, fileName, options.darkSelectedColor, QIcon::Selected);
            addPngToIcon(icon, fileName, options.darkSelectedColor, QIcon::Active);
        }
    } else {
        addPngToIcon(icon, fileName, Qt::transparent, QIcon::Normal);
        if (options.selectedColor != Qt::transparent) {
            addPngToIcon(icon, fileName, options.selectedColor, QIcon::Selected);
            addPngToIcon(icon, fileName, options.selectedColor, QIcon::Active);
        }
    }
    //
#ifndef Q_OS_MAC
    QString path = Utils::WizMisc::extractFilePath(fileName);
    QString name = Utils::WizMisc::extractFileTitle(fileName);
    QString x2FileName = path + name + "@2x.png";
    if (!QFile::exists(x2FileName))
        return icon;
    //
    if (isDarkMode()) {
        //
        addPngToIcon(icon, x2FileName, options.darkColor, QIcon::Normal);
        //
        if (options.darkSelectedColor != Qt::transparent) {
            addPngToIcon(icon, x2FileName, options.darkSelectedColor, QIcon::Selected);
            addPngToIcon(icon, x2FileName, options.darkSelectedColor, QIcon::Active);
        }
    } else {
        addPngToIcon(icon, x2FileName, Qt::transparent, QIcon::Normal);
        if (options.selectedColor != Qt::transparent) {
            addPngToIcon(icon, x2FileName, options.selectedColor, QIcon::Selected);
            addPngToIcon(icon, x2FileName, options.selectedColor, QIcon::Active);
        }
    }
    //
    if (!iconSize.isEmpty()) {
        //
        WizScaleIconToSize(icon, iconSize);
    }
#endif
    //
    //
    return icon;
}

QIcon WizLoadSkinIcon(const QString& strSkinName, const QString& strIconName, const QSize& iconSize, const WizIconOptions& options)
{
    QSize size = iconSize;
    if (size.isEmpty() || size.isNull() || !size.isValid()) {
        size = QSize(WizSmartScaleUI(16), WizSmartScaleUI(16));
    }
    //
    QString fileName = WizGetSkinResourceFileName(strSkinName, strIconName);
    if (!QFile::exists(fileName)) {
        return QIcon();
    }

    if (Utils::WizMisc::extractFileExt(fileName) == ".png") {
        return WizLoadPngSkinIcon(fileName, iconSize, options);
    }
    //
    QIcon icon = svg2Icon(fileName, size, options);
    //
    return icon;
}

// FIXME: obosolete, use CWizHtmlToPlainText class instead!
void WizHtml2Text(const QString& strHtml, QString& strText)
{
    QTextDocument doc;
    doc.setHtml(strHtml);
    strText = doc.toPlainText();
    QChar ch(0xfffc);
    strText.replace(ch, QChar(' '));
    return;
}

inline bool WizConvertTextToHTML_ForPaste_ProcessSpaceBeginEnd(int nBegin, QString& strLine, const QString& strTab)
{
    bool bRet = false;
    //
    int i = nBegin;
    while (1)
    {
        if (i >= strLine.length())
            return bRet;
        //
        QChar ch = strLine[i];
        //
        QChar chNext = 0;
        if (i < strLine.length() - 1)
        {
            chNext = strLine[i + 1];
        }
        //
        if (ch == ' ')
        {
            if (chNext != ' ' && chNext != '\t' && bRet)
            {
                return bRet;
            }
            //
            strLine.remove(i, 1);
            strLine.insert(i, "&nbsp;");
            i += 6;
            //
            bRet = true;
        }
        else if (ch == '\t')
        {
            strLine.remove(i, 1);
            strLine.insert(i, strTab);
            i += strTab.length();
            //
            bRet = true;
        }
        else
        {
            return bRet;
        }
    }
}

inline bool WizConvertTextToHTML_ForPaste_ProcessBeginSpace(QString& strLine, const QString& strTab)
{
    return WizConvertTextToHTML_ForPaste_ProcessSpaceBeginEnd(0, strLine, strTab);
}
inline bool WizConvertTextToHTML_ForPaste_ProcessEndSpace(QString& strLine, const QString& strTab)
{
    int nPos = -1;
    //
    int nLen = strLine.length();
    for (int i = nLen - 1; i >= 0; i--)
    {
        QChar ch = strLine[i];
        if (ch == '\t' || ch == ' ')
        {
            nPos = i;
        }
        else
        {
            break;
        }
    }
    //
    if (-1 == nPos)
        return false;
    //
    return WizConvertTextToHTML_ForPaste_ProcessSpaceBeginEnd(nPos, strLine, strTab);
}

inline bool WizConvertTextToHTML_ForPaste_Line(QString& strLine, const QString& strTab, const QString& strMidTab)
{
    strLine.replace("&", "&amp;");
    strLine.replace("<", "&lt;");
    strLine.replace(">", "&gt;");
    //
    WizConvertTextToHTML_ForPaste_ProcessBeginSpace(strLine, strTab);
    WizConvertTextToHTML_ForPaste_ProcessEndSpace(strLine, strTab);
    //
    strLine.replace("\t", strMidTab);
    //
    return true;
}

inline bool WizConvertTextToHTML_ForPaste(QString& strText, int nTabSize)
{
    CWizStdStringArray arrayText;
    WizSplitTextToArray(strText, '\n', arrayText);
    //
    QString strTab;
    for (int i = 0; i < nTabSize; i++)
    {
        strTab += "&nbsp;";
    }
    //
    QString strMidTab;
    for (int i = 0; i < nTabSize - 1; i++)
    {
        strMidTab += "&nbsp;";
    }
    strMidTab += _T(" ");
    //
    size_t nCount = arrayText.size();
    for (size_t i = 0; i < nCount; i++)
    {
        QString strLine = arrayText[i];
        //
        QString strTestLine = strLine;
        strTestLine = strTestLine.trimmed();
        //
        if (strLine.isEmpty() || strTestLine.isEmpty())
        {
            if (i != nCount - 1)
            {
                arrayText[i] = "<br />";//_T("<div>&nbsp;</div>");
            }
        }
        else
        {
            WizConvertTextToHTML_ForPaste_Line(strLine, strTab, strMidTab);
            //
            if (i != nCount - 1)
            {
                strLine += "<br />";
            }
            //
            arrayText[i] = strLine;
        }
    }
    //
    CString ret;
    WizStringArrayToText(arrayText, ret, CString("\r\n"));
    strText = ret;
    //
    return true;
}

QString WizText2Html(const QString& text)
{
    QString temp = text;
    WizConvertTextToHTML_ForPaste(temp, 4);
    return temp;
    /*
    QString html = text;
    html.replace("&", "&amp;");
    html.replace("<", "&lt;");
    html.replace(">", "&gt;");
    html.replace("\n", "<br />");
    //
    return "<pre>" + html + "</pre>";
    */
}


QString getImageHtmlLabelByFile(const QString& strImageFile)
{
    return QString("<div><img border=\"0\" src=\"file://%1\" /></div>").arg(strImageFile);
}

QString WizGetImageHtmlLabelWithLink(const QString& imageFile, const QSize& imgSize, const QString& linkHref)
{
    return QString("<div><a href=\"%1\"><img border=\"0\" width=\"%2px\" height=\"%3px\" src=\"file://%4\" /></a></div>")
            .arg(linkHref).arg(imgSize.width()).arg(imgSize.height()).arg(imageFile);
}

bool WizImage2Html(const QString& strImageFile, QString& strHtml, QString strDestImagePath)
{
    Utils::WizMisc::addBackslash(strDestImagePath);
    //
    QString srcPath = Utils::WizMisc::extractFilePath(strImageFile);
    //
    QString strDestFile = strImageFile;
    if (srcPath != strDestImagePath)
    {
        strDestFile = strDestImagePath + WizGenGUIDLowerCaseLetterOnly() + Utils::WizMisc::extractFileExt(strImageFile);
        //
        qDebug() << "[Editor] copy to: " << strDestFile;

        if (!QFile::copy(strImageFile, strDestFile)) {
            qDebug() << "[Editor] failed to copy image to: " << strDestFile;
            return false;
        }
        //
        QFile::setPermissions(strImageFile,
                              QFile::ReadOwner
                            | QFile::WriteOwner
                            | QFile::ReadUser
                            | QFile::WriteUser
                            | QFile::ReadGroup
                            | QFile::ReadOther
                            );
    }
    //
    strHtml = getImageHtmlLabelByFile(strDestFile);
    return true;
}


void WizDeleteFolder(const CString& strPath)
{
    QDir dir(strPath);
    dir.cdUp();

    if (dir.isRoot())
        return;

    dir = QDir(strPath);

    dir.removeRecursively();
}



#define WIZ_INVALID_CHAR_OF_FILE_NAME		"/\"?:*|<>,%\'\\\r\n\t"

BOOL WizIsValidFileNameNoPath(const CString& strFileName)
{
    if (strFileName.getLength() >= MAX_PATH)
        return FALSE;
    //
    return -1 == strFileName.findOneOf(WIZ_INVALID_CHAR_OF_FILE_NAME);
}


void WizMakeValidFileNameNoPath(CString& strFileName)
{
    if (WizIsValidFileNameNoPath(strFileName))
        return;
    //
    if (strFileName.getLength() >= MAX_PATH)
        strFileName = strFileName.left(MAX_PATH - 1);
    //
    while (1)
    {
        int nPos = strFileName.findOneOf(WIZ_INVALID_CHAR_OF_FILE_NAME);
        if (-1 == nPos)
            break;
        //
        strFileName.setAt(nPos,  '-');
    }
    //
    while (strFileName.find("--") != -1)
    {
        strFileName.replace("--", "-");
    }
    //
    strFileName.trim();
    strFileName.trim('-');
}

void WizMakeValidFileNameNoPathLimitLength(CString& strFileName, int nMaxTitleLength)
{
    WizMakeValidFileNameNoPath(strFileName);
    //
    CString strTitle = Utils::WizMisc::extractFileTitle(strFileName);
    CString strExt = Utils::WizMisc::extractFileExt(strFileName);
    //
    if (strTitle.getLength() > nMaxTitleLength)
    {
        strFileName = strTitle.left(nMaxTitleLength) + strExt;
    }
}

void WizMakeValidFileNameNoPathLimitFullNameLength(CString& strFileName, int nMaxFullNameLength)
{
    WizMakeValidFileNameNoPath(strFileName);
    //
    if (strFileName.getLength() <= nMaxFullNameLength)
        return;
    //
    CString strExt = Utils::WizMisc::extractFileExt(strFileName);
    //
    int nMaxTitleLength = nMaxFullNameLength - strExt.getLength();
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
    int nValueNameLen = strValueName.getLength();
    //
    size_t nCount = arrayText.size();
    for (size_t i = 0; i < nCount; i++)
    {
        CString strLine = arrayText[i];
        if (strLine.startsWith(strValueName))
        {
            CString str = strLine.right(strLine.getLength() - nValueNameLen);
            str.trim();
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
    strCommandLine.insert(0, ' ');
    //
    WizSplitTextToArray(strCommandLine, " /", FALSE, arrayLine);
}
CString WizGetCommandLineValue(const CString& strCommandLine, const CString& strKey)
{
    CWizStdStringArray arrayLine;
    WizCommandLineToStringArray(strCommandLine, arrayLine);
    //
    return WizStringArrayGetValue(arrayLine, strKey);
}

bool WizSaveDataToFile(const QString& strFileName, const QByteArray& arrayData)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        TOLOG("Can't open file when save data to file");
        return false;
    }

    QDataStream out(&file);
    int retLen = out.writeRawData(arrayData.constData(), arrayData.length());
    if (retLen == -1) {
        TOLOG("Error occured when save data to file");
        return false;
    } else if (retLen != arrayData.length()) {
        TOLOG("not all data saved when save data to file");
        return false;
    }

    return true;
}

bool WizLoadDataFromFile(const QString& strFileName, QByteArray& arrayData)
{
    QFile file(strFileName);
    if (!file.exists()) {
        TOLOG("File not exists when load data from file");
        return false;
    }

    if (!file.open(QFile::ReadOnly)) {
        TOLOG("Can't open file when load data from file");
        return false;
    }

    arrayData.clear();
    arrayData.append(file.readAll());
    file.close();

    return true;
}

WizWaitCursor::WizWaitCursor()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

WizWaitCursor::~WizWaitCursor()
{
    QApplication::restoreOverrideCursor();
}


WizTempFileGuard::WizTempFileGuard(const QString& fileName)
    : m_fileName(fileName)
{
}
WizTempFileGuard::WizTempFileGuard()
{
    m_fileName = Utils::WizPathResolve::tempPath() + ::WizGenGUIDLowerCaseLetterOnly();
}

WizTempFileGuard::~WizTempFileGuard()
{
    WizDeleteFile(m_fileName);
    if (WizPathFileExists(m_fileName))
    {
        TOLOG1(_T("Failed to delete temp file: %1"), m_fileName);
    }
}
//
QString WizTempFileGuard::fileName()
{
    return m_fileName;
}


void WizShowWebDialogWithToken(const QString& windowTitle, const QString& url, QWidget* parent, const QSize& sz, bool dialogResizable)
{
    QString strFuncName = windowTitle;
    strFuncName = "Dialog"+strFuncName.replace(" ", "");
    WizFunctionDurationLogger logger(strFuncName);

    WizWebSettingsWithTokenDialog* pDlg = new WizWebSettingsWithTokenDialog(url, sz, parent);
    if (dialogResizable)
    {
        pDlg->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    }
    pDlg->setWindowTitle(windowTitle);
    pDlg->exec();
    //
    pDlg->deleteLater();
}

void WizShowWebDialogWithTokenDelayed(const QString& windowTitle, const QString& url, QWidget* parent, const QSize& sz, bool dialogResizable)
{
    QString strFuncName = windowTitle;
    strFuncName = "Dialog"+strFuncName.replace(" ", "");
    WizFunctionDurationLogger logger(strFuncName);

    WizWebSettingsWithTokenDialog* pDlg = WizWebSettingsWithTokenDialog::delayShow(windowTitle, url, sz, parent);
    if (dialogResizable)
    {
        pDlg->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    }
}


bool WizIsOffline()
{
    QNetworkConfigurationManager mgr;
    return !mgr.isOnline();
}

class SleepThread : public QThread
{
 public :
     static void sleep(long iSleepTime)
     {
          QThread::sleep(iSleepTime);
     }
     static void msleep(long iSleepTime)
     {
          QThread::msleep(iSleepTime);
     }
};

void WizWaitForThread(QThread* pThread)
{
    pThread->disconnect();
    //
    while (pThread->isRunning())
    {
        SleepThread::msleep(100);
        QApplication::processEvents();
    }

}


QString WizGetDefaultTranslatedLocal()
{
    return "en_US";
}


bool IsWizKMURL(const QString& strURL)
{
    return strURL.left(5) == "wiz:/";
}


bool WizIsKMURLOpenDocument(const QString& strURL)
{
    if (IsWizKMURL(strURL))
    {
        return strURL.contains("open_document");
    }
    return false;
}


QString WizGetHtmlBodyContent(const QString& strHtml)
{
    QString strBody = strHtml;
    QRegExp regBodyContant("<body[^>]*>[\\s\\S]*</body>");
    int index = regBodyContant.indexIn(strBody);
    if (index > -1)
    {
        strBody = regBodyContant.cap(0);

        QRegExp regBody = QRegExp("</?body[^>]*>", Qt::CaseInsensitive);
        strBody.replace(regBody, "");
    }

    return strBody;
}

bool WizGetBodyContentFromHtml(QString& strHtml, bool bNeedTextParse)
{
    QRegExp regHead("</?head[^>]*>", Qt::CaseInsensitive);
    if (strHtml.contains(regHead))
    {
        if (bNeedTextParse)
        {
            QRegExp regHeadContant("<head[^>]*>[\\s\\S]*</head>");
            int headIndex = regHeadContant.indexIn(strHtml);
            if (headIndex > -1)
            {
                QString strHead = regHeadContant.cap(0);
                if (strHead.contains("Cocoa HTML Writer"))
                {
                    // convert mass html to rtf, then convert rft to html
                    QTextDocument textParase;
                    textParase.setHtml(strHtml);
                    strHtml = textParase.toHtml();
                }
            }
        }

        strHtml = WizGetHtmlBodyContent(strHtml);
    }

    return true;
}


bool WizHTMLIsInCommentsBlock(const QString& strHtml, int pos)
{
    if (pos <= 0)
        return false;
    if (pos >= strHtml.length())
        return false;
    //
    int commentBegin = strHtml.lastIndexOf("<!--", pos);
    if (-1 == commentBegin)
        return false;
    //
    int commentEnd = strHtml.indexOf("-->", commentBegin);
    if (commentEnd == -1)
        return false;
    //
    if (commentEnd < pos)
        return false;
    //
    return true;
}

void WizHTMLAppendTextInHead(const QString& strText, QString& strHTML)
{
    ptrdiff_t nPos = 0;
    int from = 0;
    while (1)
    {
        nPos = strHTML.indexOf("</head", from, Qt::CaseInsensitive);
        if (-1 == nPos)
        {
            nPos = 0;
            break;
        }
        //
        if (!WizHTMLIsInCommentsBlock(strHTML, nPos))
            break;
        //
        from = nPos + 6;
    }
    //
    strHTML.insert(int(nPos), strText);
}




bool WizCopyFolder(const QString& strSrcDir, const QString& strDestDir, bool bCoverFileIfExist)
{
    QDir sourceDir(strSrcDir);
    QDir targetDir(strDestDir);
    if(!targetDir.exists())
    {
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    foreach(QFileInfo fileInfo, fileInfoList)
    {
        if(fileInfo.isDir())
        {
            if(!WizCopyFolder(fileInfo.filePath(),
                              targetDir.filePath(fileInfo.fileName()),
                              bCoverFileIfExist))
                return false;
        }
        else
        {
            if(bCoverFileIfExist && targetDir.exists(fileInfo.fileName()))
            {
                targetDir.remove(fileInfo.fileName());
            }

            if(!QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName())))
                return false;
        }
    }
    return true;
}


void WizShowDocumentHistory(const WIZDOCUMENTDATA& doc, QWidget* parent)
{
    CString strExt = WizFormatString3("obj_guid=%1&kb_guid=%2&doc_guid=%3&obj_type=document",
                                      doc.strGUID, doc.strKbGUID, doc.strGUID);
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("document_history", WIZ_TOKEN_IN_URL_REPLACE_PART, strExt);
    WizShowWebDialogWithToken(QObject::tr("Note History"), strUrl, parent, QSize(1000, 500), true);
}


WizKMUrlType GetWizUrlType(const QString& strURL)
{
    if (IsWizKMURL(strURL))
    {
        if (strURL.contains("open_document"))
            return WizUrl_Document;
        else if (strURL.contains("open_attachment"))
            return WizUrl_Attachment;
    }
    return WizUrl_Invalid;
}

#define WIZ_SEARCH_SPLIT_CHAR  ' '

QChar getWizSearchSplitChar()
{
    return WIZ_SEARCH_SPLIT_CHAR;
}


double calScaleFactor()
{
    static double scaleFactor = 0;

    if (scaleFactor < 0.5)
    {
        scaleFactor = 1.0 * WizSmartScaleUI(100) / 100;
    }

    return scaleFactor;
}

bool WizIsHighPixel()
{
#ifdef Q_OS_MAC
    return qApp->devicePixelRatio() >= 2;
#endif
    return calScaleFactor() > 1.5;

//    return false;
}


QString GetParamFromWizKMURL(const QString& strURL, const QString& strParamName)
{
    int nindex = strURL.indexOf('?');
    if (nindex == -1)
        return QString();

    QString strParams = strURL;
    strParams.remove(0, nindex + 1);
    QStringList paramList = strParams.split('&');
    QString strParaFlag = strParamName + "=";
    foreach (QString strParam, paramList) {
        if (strParam.contains(strParaFlag)) {
            return strParam.remove(strParaFlag);
        }
    }

    return QString();
}


QString WizStr2Title(const QString& str)
{
    int idx = str.size() - 1;
    static QString eol("，。？~!#$%^&*()_+{}|:\"<>?,./;'[]\\-=\n\r\t"); // end of line
    foreach(QChar c, eol) {
        int i = str.indexOf(c, 0, Qt::CaseInsensitive);
        if (i != -1 && i < idx) {
            idx = i;
        }
    }

    return str.left(idx);
}

bool WizCreateThumbnailForAttachment(QImage& img, const QString& attachFileName, const QSize& iconSize)
{
    QFileInfo info(attachFileName);
    if (!info.exists())
        return false;

    // get info text and calculate width of image
    const int nMb = 1024 * 1024;
    int nIconMargin = 14;
    QString fileSize = info.size() > 1024 ? (info.size() > nMb ? QString(QString::number(qCeil(info.size() / (double)nMb)) + " MB")
                                                         : QString(QString::number(qCeil(info.size() / (double)1024)) + " KB")) :
                                      QString(QString::number(info.size()) + " B");
    QString dateInfo = QDate::currentDate().toString(Qt::ISODate) + " " + QTime::currentTime().toString();

    const int FONTSIZE = 12;
    bool isHighPix = WizIsHighPixel();    
    QFont font;
    font.setPixelSize(FONTSIZE);
    QFontMetrics fm(font);
    int nTextWidth = fm.width(dateInfo + fileSize);
    int nWidth = nTextWidth + nIconMargin * 4 - 4 + iconSize.width();
    int nHeight = iconSize.height() + nIconMargin * 2;

    // draw icon and text on image
    int nBgWidth = isHighPix ? 2 * nWidth : nWidth;
    int nBgHeight = isHighPix ? 2 * nHeight : nHeight;
    img = QImage(nBgWidth, nBgHeight, QImage::Format_RGB888);
    QPainter p(&img);
    QRect rcd = QRect(0, 0, nBgWidth, nBgHeight);
    p.fillRect(rcd, QBrush(QColor(Qt::white)));
    p.setPen(QPen(QColor("#E7E7E7")));
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(rcd.adjusted(1, 1, -2, -2), 8, 10);

    QFont f = p.font();
    f.setPixelSize(FONTSIZE);
    p.setFont(f);

    if (isHighPix)
    {
        // 如果是高分辨率的屏幕，则将坐标放大二倍进行绘制，使用时进行缩放，否则会造成图片模糊。
        Utils::WizStyleHelper::initPainterByDevice(&p);
    }
    QFileIconProvider ip;
    QIcon icon = ip.icon(info);
    QPixmap pixIcon = icon.pixmap(iconSize);
    p.drawPixmap(nIconMargin, (nHeight - iconSize.height()) / 2, pixIcon);

    //    
    p.setPen(QPen(QColor("#535353")));
    QRect titleRect(QPoint(nIconMargin * 2 - 3 + iconSize.width(), nIconMargin), QPoint(nWidth, nHeight / 2));
    QString strTitle = fm.elidedText(info.fileName(), Qt::ElideMiddle, titleRect.width() - nIconMargin * 2);
    p.drawText(titleRect, strTitle);
    //
    QRect infoRect(QPoint(nIconMargin * 2 - 3 + iconSize.width(), nHeight / 2 + 2),
                      QPoint(nWidth, nHeight));
    p.setPen(QColor("#888888"));
    p.drawText(infoRect, dateInfo);

    int dateWidth = fm.width(dateInfo);
    infoRect.adjust(dateWidth + 4, 0, 0, 0);
    QPixmap pixGreyPoint(Utils::WizStyleHelper::loadPixmap("document_grey_point"));
    QRect rcPix = infoRect.adjusted(0, 6, 0, 0);
    rcPix.setSize(QSize(4, 4));
    p.drawPixmap(rcPix, pixGreyPoint);

    infoRect.adjust(8, 0, 0, 0);
    p.drawText(infoRect, fileSize);

    return true;
}


WizOleDateTime WizIniReadDateTimeDef(const CString& strFile, const CString& strSection, const CString& strKey, WizOleDateTime defaultData)
{
    QSettings settings(strFile, QSettings::IniFormat);
    QString strStr = strSection.isEmpty() ? strKey : strSection + "/" + strKey;
    QDateTime dt = settings.value(strStr, defaultData).toDateTime();

    return dt;
}


CString WizIniReadStringDef(const CString& strFile, const CString& strSection, const CString& strKey)
{
    QSettings settings(strFile, QSettings::IniFormat);
    QString strStr = strSection.isEmpty() ? strKey : strSection + "/" + strKey;
    return settings.value(strStr).toString();
}


void WizIniWriteString(const CString& strFile, const CString& strSection, const CString& strKey, const CString& strValue)
{
    QSettings settings(strFile, QSettings::IniFormat);
    QString strStr = strSection.isEmpty() ? strKey : strSection + "/" + strKey;
    settings.setValue(strStr, strValue);
}


int WizIniReadIntDef(const CString& strFile, const CString& strSection, const CString& strKey, int defaultValue)
{
    QSettings settings(strFile, QSettings::IniFormat);
    QString strStr = strSection.isEmpty() ? strKey : strSection + "/" + strKey;
    return settings.value(strStr, defaultValue).toInt();
}


void WizIniWriteInt(const CString& strFile, const CString& strSection, const CString& strKey, int nValue)
{
    QSettings settings(strFile, QSettings::IniFormat);
    QString strStr = strSection.isEmpty() ? strKey : strSection + "/" + strKey;
    settings.setValue(strStr, nValue);
}


void WizIniWriteDateTime(const CString& strFile, const CString& strSection, const CString& strKey, WizOleDateTime dateTime)
{
    QSettings settings(strFile, QSettings::IniFormat);
    QString strStr = strSection.isEmpty() ? strKey : strSection + "/" + strKey;
    settings.setValue(strStr, dateTime);
}


WizIniFileEx::WizIniFileEx()
    : m_settings(0)
{
}

WizIniFileEx::~WizIniFileEx()
{
    if (m_settings)
        delete m_settings;
}

void WizIniFileEx::loadFromFile(const QString& strFile)
{
    m_settings = new QSettings(strFile, QSettings::IniFormat);
}

void WizIniFileEx::getSection(const QString& section, CWizStdStringArray& arrayData)
{
    m_settings->beginGroup(section);
    QStringList childList = m_settings->childKeys();

    foreach (QString child, childList) {
        QString strLine = child + "=" + m_settings->value(child).toString();
        arrayData.push_back(strLine);
    }
    m_settings->endGroup();
}

void WizIniFileEx::getSection(const QString& section, QMap<QString, QString>& dataMap)
{
    m_settings->beginGroup(section);
    QStringList childList = m_settings->childKeys();
    foreach (QString child, childList) {
        dataMap.insert(child, m_settings->value(child).toString());
    }
    m_settings->endGroup();
}

void WizShowAttachmentHistory(const WIZDOCUMENTATTACHMENTDATA& attach, QWidget* parent)
{
    CString strExt = WizFormatString3("obj_guid=%1&kb_guid=%2&doc_guid=%3&obj_type=attachment",
                                      attach.strGUID, attach.strKbGUID, attach.strDocumentGUID);
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("document_history", WIZ_TOKEN_IN_URL_REPLACE_PART, strExt);
    WizShowWebDialogWithToken(QObject::tr("Attachment History"), strUrl, parent, QSize(1000, 500), true);
}


bool WizIsNoteContainsFrameset(const WIZDOCUMENTDATA& doc)
{
    QStringList fileTypes;
    fileTypes << ".xls" << ".xlsx";
    return fileTypes.contains(doc.strFileType);
}

void WizMime2Note(const QByteArray& bMime, WizDatabaseManager& dbMgr, CWizDocumentDataArray& arrayDocument)
{
    QString strMime(QString::fromUtf8(bMime));
    QStringList lsNotes = strMime.split(";");
    for (int i = 0; i < lsNotes.size(); i++) {
        QStringList lsMeta = lsNotes[i].split(":");
        //qDebug()<<lsMeta;
        Q_ASSERT(lsMeta.size() == 2);

        WizDatabase& db = dbMgr.db(lsMeta[0]);

        WIZDOCUMENTDATA data;
        if (db.documentFromGuid(lsMeta[1], data))
            arrayDocument.push_back(data);
    }
}


bool WizMakeSureDocumentExistAndBlockWidthDialog(WizDatabase& db, const WIZDOCUMENTDATA& doc)
{
    if (doc.strGUID.isEmpty() || db.kbGUID() != doc.strKbGUID)
        return false;

    QString strFileName = db.getDocumentFileName(doc.strGUID);
    if (!db.isDocumentDownloaded(doc.strGUID) || !WizPathFileExists(strFileName))
    {
        WizObjectDownloaderHost* downloaderHost = WizObjectDownloaderHost::instance();
        if (!downloaderHost)
            return false;

        WizProgressDialog dlg(0, false);
        dlg.setActionString(QObject::tr("Download Note %1 ...").arg(doc.strTitle));
        dlg.setWindowTitle(QObject::tr("Downloading"));
        dlg.setProgress(100,0);
        QObject::connect(downloaderHost, SIGNAL(downloadProgress(QString,int,int)), &dlg, SLOT(setProgress(QString,int,int)));
        QObject::connect(downloaderHost, SIGNAL(finished()), &dlg, SLOT(accept()));

        downloaderHost->downloadData(doc);
        dlg.exec();
        //
        downloaderHost->disconnect(&dlg);
    }

    return WizPathFileExists(strFileName);
}


bool WizMakeSureDocumentExistAndBlockWidthEventloop(WizDatabase& db, const WIZDOCUMENTDATA& doc)
{
    if (doc.strGUID.isEmpty() || db.kbGUID() != doc.strKbGUID)
        return false;

    QString strFileName = db.getDocumentFileName(doc.strGUID);
    if (!db.isDocumentDownloaded(doc.strGUID) || !WizPathFileExists(strFileName))
    {
        WizObjectDownloaderHost* downloaderHost = WizObjectDownloaderHost::instance();
        if (!downloaderHost)
            return false;

        QEventLoop loop;
        QObject::connect(downloaderHost, SIGNAL(finished()), &loop, SLOT(quit()));        
        downloaderHost->downloadData(doc);
        loop.exec();
        //
    }

    return WizPathFileExists(strFileName);
}

bool WizMakeSureAttachmentExistAndBlockWidthEventloop(WizDatabase& db, const WIZDOCUMENTATTACHMENTDATAEX& attachData)
{
    if (attachData.strGUID.isEmpty() || attachData.strKbGUID != db.kbGUID())
        return false;

    QString strAttachmentFileName = db.getAttachmentFileName(attachData.strGUID);
    if (!db.isAttachmentDownloaded(attachData.strDocumentGUID) && !WizPathFileExists(strAttachmentFileName))
    {
        WizObjectDownloaderHost* downloaderHost = WizObjectDownloaderHost::instance();
        if (!downloaderHost)
            return false;

        QEventLoop loop;
        QObject::connect(downloaderHost, SIGNAL(finished()), &loop, SLOT(quit()));        
        downloaderHost->downloadData(attachData);
        loop.exec();
        //
    }

    return WizPathFileExists(strAttachmentFileName);
}


bool WizMakeSureAttachmentExistAndBlockWidthDialog(WizDatabase& db, const WIZDOCUMENTATTACHMENTDATAEX& attachData)
{
    if (attachData.strGUID.isEmpty() || attachData.strKbGUID != db.kbGUID())
        return false;

    QString strAttachmentFileName = db.getAttachmentFileName(attachData.strGUID);
    if (!db.isAttachmentDownloaded(attachData.strDocumentGUID) && !WizPathFileExists(strAttachmentFileName))
    {
        WizObjectDownloaderHost* downloaderHost = WizObjectDownloaderHost::instance();
        if (!downloaderHost)
            return false;

        WizProgressDialog dlg(0, false);
        dlg.setActionString(QObject::tr("Download Attachment %1 ...").arg(attachData.strName));
        dlg.setWindowTitle(QObject::tr("Downloading"));
        dlg.setProgress(100,0);

        QObject::connect(downloaderHost, SIGNAL(downloadProgress(QString,int,int)), &dlg, SLOT(setProgress(QString,int,int)));
        QObject::connect(downloaderHost, SIGNAL(finished()), &dlg, SLOT(accept()));
        downloaderHost->downloadData(attachData);
        dlg.exec();
        //
        downloaderHost->disconnect(&dlg);
    }

    return WizPathFileExists(strAttachmentFileName);
}


bool WizGetLocalUsers(QList<WizLocalUser>& userList)
{
    QString dataPath = Utils::WizPathResolve::dataStorePath();
    QDir dir(dataPath);
    QStringList folderList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (QString folder : folderList)
    {
        WizLocalUser user;
        QString dataBase = dataPath + folder + "/data/index.db";
        WizIndex db;
        if (!QFile::exists(dataBase) || !db.open(dataBase))
            continue;

        user.strDataFolderName = folder;
        user.strGuid = db.getMetaDef("ACCOUNT", "GUID");
        if (user.strGuid.isEmpty())
        {
            continue;
        }
        user.strUserId = db.getMetaDef("ACCOUNT", "USERID");
//        qDebug() << "load user id ; " << user.strUserId << "  folder : " << folder;
        user.strUserId.isEmpty() ? (user.strUserId = folder) : 0;
        user.nUserType = db.getMetaDef("QT_WIZNOTE", "SERVERTYPE").toInt();
        userList.append(user);
    }
    return true;
}


QString WizGetLocalUserId(const QList<WizLocalUser>& userList, const QString& strGuid)
{
    for (WizLocalUser user : userList)
    {
        if (strGuid == user.strGuid)
            return user.strUserId;
    }
    return "";
}


bool WizURLDownloadToFile(const QString& url, const QString& fileName, bool isImage)
{
    QString newUrl = url;
    QNetworkAccessManager netCtrl;
    QNetworkReply* reply;
    //
    bool redirect = false;
    QByteArray byData;
    do
    {
        QNetworkRequest request(newUrl);
        //
        reply = netCtrl.get(request);
        WizAutoTimeOutEventLoop loop(reply);
        loop.setTimeoutWaitSeconds(60 * 60);
        loop.exec();

        if (loop.error() != QNetworkReply::NoError)
        {
            return false;
        }
        //
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 301) {
            redirect = true;
            QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            newUrl = redirectUrl.toString();
        } else {
            redirect = false;
            byData = loop.result();
        }
        //
    }
    while (redirect);

    WizDeleteFile(fileName);

    if (isImage)
    {
        QPixmap pix;
        pix.loadFromData(byData);
        return pix.save(fileName);
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    file.write(byData);
    file.close();

    return true;
}

bool WizURLDownloadToData(const QString& url, QByteArray& data)
{
    QString newUrl = url;
    QNetworkAccessManager netCtrl;
    QNetworkReply* reply;
    //
    bool redirect = false;
    do
    {
        QNetworkRequest request(newUrl);
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        //
        reply = netCtrl.get(request);
        WizAutoTimeOutEventLoop loop(reply);
        loop.setTimeoutWaitSeconds(60 * 60);
        loop.exec();

        if (loop.error() != QNetworkReply::NoError)
        {
            return false;
        }

        //
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 301) {
            redirect = true;
            QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            newUrl = redirectUrl.toString();
        } else {
            redirect = false;
            data = loop.result();
        }
        //
    }
    while (redirect);
    //
    return true;
}

bool WizURLDownloadToData(const QString& url, QByteArray& data, QObject* receiver, const char *member)
{
    QString newUrl = url;
    QNetworkAccessManager netCtrl;
    QNetworkReply* reply;
    //
    bool redirect = false;
    do
    {
        QNetworkRequest request(newUrl);
        //
        reply = netCtrl.get(request);
        //
        WizAutoTimeOutEventLoop loop(reply);
        //
        QObject::connect(&loop, SIGNAL(downloadProgress(QUrl, qint64, qint64)), receiver, member);
        //
        loop.setTimeoutWaitSeconds(60 * 60);
        loop.exec();

        if (loop.error() != QNetworkReply::NoError)
        {
            TOLOG2("Failed to download data: %1, %2", WizIntToStr(loop.error()), loop.errorString());
            return false;
        }

        //
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 301) {
            redirect = true;
            QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            newUrl = redirectUrl.toString();
        } else {
            redirect = false;
            data = loop.result();
        }
        //
    }
    while (redirect);
    //
    return true;
}


QString WizGetLocalFolderName(const QList<WizLocalUser>& userList, const QString& strGuid)
{
    for (WizLocalUser user : userList)
    {
        if (strGuid == user.strGuid)
            return user.strDataFolderName;
    }
    return "";
}


bool WizIsChineseLanguage(const QString& local)
{
    if (local.toUpper() == "ZH_CN" || local.toUpper() == "ZH_TW")
        return true;

    return false;
}


bool WizIsMarkdownNote(const WIZDOCUMENTDATA& doc)
{
    if (doc.strTitle.indexOf(".md") == -1 && doc.strTitle.indexOf(".mj") == -1)
        return false;

    int nPointPos = doc.strTitle.length() - 3;
    if (doc.strTitle.lastIndexOf(".md") == nPointPos || doc.strTitle.lastIndexOf(".mj") == nPointPos)
        return true;

    if (doc.strTitle.indexOf(".md ") != -1 || doc.strTitle.indexOf(".md@") != -1 ||
            doc.strTitle.indexOf(".mj ") != -1|| doc.strTitle.indexOf(".mj@") != -1)
        return true;

    return false;
}

void WizCopyNoteAsInternalLink(const WIZDOCUMENTDATA& document)
{
    QString strHtml, strLink;
    WizNoteToHtmlLink(document, strHtml, strLink);
    //
    QClipboard* clip = QApplication::clipboard();

    QMimeData* data = new QMimeData();
    data->setHtml(strHtml);
    data->setText(strLink);
    clip->setMimeData(data);
}


void WizCopyNotesAsInternalLink(const QList<WIZDOCUMENTDATA>& documents)
{
    QString strHtml, strLink;
    WizNotesToHtmlLink(documents, strHtml, strLink);

    QMimeData* data = new QMimeData();
    data->setHtml(strHtml);
    data->setText(strLink);
    QClipboard* clip = QApplication::clipboard();
    clip->setMimeData(data);
}


QString WizNoteToWizKMURL(const WIZDOCUMENTDATA& document)
{
    WizDatabase& db = WizDatabaseManager::instance()->db(document.strKbGUID);
    //
    if (!db.isGroup())
    {
        return WizFormatString3("wiz://open_document?guid=%1&kbguid=%2&private_kbguid=%3", document.strGUID, "", db.kbGUID());
    }
    else
    {
        return WizFormatString2("wiz://open_document?guid=%1&kbguid=%2", document.strGUID, document.strKbGUID);
    }
    return QString();
}


void WizNoteToHtmlLink(const WIZDOCUMENTDATA& document, QString& strHtml, QString& strLink)
{
    strLink = WizNoteToWizKMURL(document);
    QString strTitle = document.strTitle.toHtmlEscaped();
    strTitle.replace("&", "&amp;");
    //
    strHtml = WizFormatString2("<a href=\"%1\">%2</a>", strLink, strTitle);
}


void WizNotesToHtmlLink(const QList<WIZDOCUMENTDATA>& documents, QString& strHtml, QString& strLink)
{
    for (int i = 0; i < documents.count(); i++)
    {
        QString strOneHtml, strOneLink;
        WizNoteToHtmlLink(documents.at(i), strOneHtml, strOneLink);
        strHtml += strOneHtml + "<br>";
        strLink += strOneLink + "\n";
    }
}


void WizCopyNoteAsWebClientLink(const WIZDOCUMENTDATA& document)
{
    // https://note.wiz.cn?dc={document_guid}&kb={kb_guid}&cmd=km%2C"}
    QString url = WizCommonApiEntry::getUrlByCommand("note_link");
    url.replace("{document_guid}", document.strGUID);
    url.replace("{kb_guid}", document.strKbGUID);

    QMimeData* data = new QMimeData();
    data->setHtml(url);
    data->setText(url);
    QClipboard* clip = QApplication::clipboard();
    clip->setMimeData(data);
}


void WizCopyNotesAsWebClientLink(const QList<WIZDOCUMENTDATA>& documents)
{
    QString url = WizCommonApiEntry::getUrlByCommand("note_link");
    QString link;
    for (int i = 0; i < documents.count(); i++)
    {
        QString tmp = url;
        tmp.replace("{document_guid}", documents.at(i).strGUID);
        tmp.replace("{kb_guid}", documents.at(i).strKbGUID);
        link.append((i == 0 ? "" : "\n") + tmp);
    }

    QMimeData* data = new QMimeData();
    data->setHtml(link);
    data->setText(link);
    QClipboard* clip = QApplication::clipboard();
    clip->setMimeData(data);
}
