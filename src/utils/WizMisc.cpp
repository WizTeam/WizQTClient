#include "WizMisc.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTextCodec>

#include "WizDef.h"
#include "WizPinyin.h"

namespace Utils {

QString WizMisc::time2humanReadable(const QDateTime& time) {
    return time2humanReadable(time, "yy-MM-dd");
}

QString WizMisc::time2humanReadable(const QDateTime& time, const QString& formate)
{
    QDateTime t(QDateTime::currentDateTime());
    int nElapseSecs = time.secsTo(t);
    int nElapseDays = time.daysTo(t);

    if (nElapseDays == 1) {
        return QObject::tr("Yesterday");
    } else if (nElapseDays == 2) {
        return QObject::tr("The day before yesterday");
    } else if (nElapseDays > 2) {
        return time.toString(formate);
    }

    if (nElapseSecs < 60) {
        // less than 1 minutes: "just now"
        return QObject::tr("Just now");

    } else if (nElapseSecs >= 60 && nElapseSecs < 60 * 2) {
        // 1 minute: "1 minute ago"
        return QObject::tr("1 minute ago");

    } else if (nElapseSecs >= 120 && nElapseSecs < 60 * 60) {
        // 2-60 minutes: "x minutes ago"
        QString str = QObject::tr("%1 minutes ago");
        return str.arg(nElapseSecs/60);

    } else if (nElapseSecs >= 60 * 60 && nElapseSecs < 60 * 60 * 2) {
        // 1 hour: "1 hour ago"
        return QObject::tr("1 hour ago");

    } else if (nElapseSecs >= 60 * 60 * 2 && nElapseSecs < 60 * 60 * 24) {
        // 2-24 hours: "x hours ago"
        QString str = QObject::tr("%1 hours ago");
        return str.arg(nElapseSecs/60/60);
    }

    return QString("Error");
}

bool WizMisc::loadUnicodeTextFromFile(const QString& strFileName, QString& strText)
{
    QFile file(strFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    strText = stream.readAll();
    file.close();

    return true;
}

void WizMisc::addBackslash(QString& strPath)
{
    strPath.replace('\\', '/');

    if (strPath.endsWith('/'))
        return;

    strPath += '/';
}

void WizMisc::removeBackslash(CString& strPath)
{
    while (1)
    {
        if (!strPath.endsWith('/'))
            return;

        strPath.remove(strPath.length() - 1, 1);
    }
}

CString WizMisc::addBackslash2(const CString& strPath)
{
    CString str(strPath);
    addBackslash(str);
    return str;
}
CString WizMisc::removeBackslash2(const CString& strPath)
{
    CString str(strPath);
    removeBackslash(str);
    return str;
}

void WizMisc::ensurePathExists(const CString& path)
{
    QDir dir;
    dir.mkpath(path);
}

void WizMisc::ensureFileExists(const QString& strFileName)
{
    QFile file(strFileName);
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite);
        file.close();
    }
}

CString WizMisc::extractFilePath(const CString& strFileName)
{
    CString str = strFileName;
    str.replace('\\', '/');
    int index = str.lastIndexOf('/');
    if (-1 == index)
        return strFileName;
    //
    return str.left(index + 1); //include separator
}


CString WizMisc::extractLastPathName(const CString& strFileName)
{
    CString strPath = removeBackslash2(strFileName);
    return extractFileName(strPath);
}

QString WizMisc::extractFileName(const QString& strFileName)
{
    QString str = strFileName;
    str.replace('\\', '/');
    int index = str.lastIndexOf('/');
    if (-1 == index)
        return strFileName;

    return strFileName.right(str.length() - index - 1);
}

QString WizMisc::extractFileTitle(const QString &strFileName)
{
    QString strName = extractFileName(strFileName);

    int index = strName.lastIndexOf('.');
    if (-1 == index)
        return strName;

    return strName.left(index);
}

CString WizMisc::extractTitleTemplate(const CString& strFileName)
{
    return strFileName;
}

CString WizMisc::extractFileExt(const CString& strFileName)
{
    CString strName = extractFileName(strFileName);
    //
    int index = strName.lastIndexOf('.');
    if (-1 == index)
        return "";
    //
    return strName.right(strName.getLength() - index);  //include .
}

qint64 WizMisc::getFileSize(const CString& strFileName)
{
    QFileInfo info(strFileName);
    return info.size();
}

void WizMisc::deleteFile(const CString& strFileName)
{
    QDir dir(extractFilePath(strFileName));
    dir.remove(extractFileName(strFileName));
}

QString WizMisc::getHtmlBodyContent(QString strHtml)
{
    QRegExp regex("<body.*>([\\s\\S]*)</body>", Qt::CaseInsensitive);
    QString strBody;
    if (regex.indexIn(strHtml) != -1) {
        strBody = regex.cap(1);
    } else {
        strBody = strHtml;
    }
    return strBody;
}

void WizMisc::splitHtmlToHeadAndBody(const QString& strHtml, QString& strHead, QString& strBody)
{
    QRegExp regh("<head.*>([\\s\\S]*)</head>", Qt::CaseInsensitive);
    if (regh.indexIn(strHtml) != -1) {
        strHead = regh.cap(1).simplified();
    }

    QRegExp regex("<body.*>([\\s\\S]*)</body>", Qt::CaseInsensitive);
    if (regex.indexIn(strHtml) != -1) {
        strBody = regex.cap(1);
    } else {
        strBody = strHtml;
    }
}

void WizMisc::copyTextToClipboard(const QString& text)
{
    QClipboard* clip = QApplication::clipboard();
    QMimeData* data = new QMimeData();
    data->setHtml(text);
    data->setText(text);
    clip->setMimeData(data);
}

bool WizMisc::isChinese()
{
#ifdef QT_DEBUG
    return true;
#endif
    return isSimpChinese() || isTraditionChinese();
}

bool WizMisc::isSimpChinese()
{
    QLocale local;
    QString name = local.name().toLower();
    if (name == "zh_cn"
        || name == "zh-cn")
    {
        return true;
    }
    return false;
}

bool WizMisc::isTraditionChinese()
{
    QLocale local;
    QString name = local.name().toLower();
    if (name == "zh_tw"
        || name == "zh-tw")
    {
        return true;
    }
    return false;
}

bool WizMisc::localeAwareCompare(const QString& s1, const QString& s2)
{
    return WizToolsSmartCompare(s1, s2) < 0;
}

int WizMisc::getVersionCode()
{
    QString str(WIZ_CLIENT_VERSION);
    QStringList strList = str.split('.');
    Q_ASSERT(strList.count() >= 3);

    QString strNum = strList.first();
    QString strSec = strList.at(1);
    while (strSec.length() < 2) {
        strSec.insert(0, "0");
    }
    QString strThr = strList.at(2);
    while (strThr.length() < 3) {
        strThr.insert(0, "0");
    }

    strNum.append(strSec);
    strNum.append(strThr);

    return strNum.toInt();
}



} // namespace Utils
