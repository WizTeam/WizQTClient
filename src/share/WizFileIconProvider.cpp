#include "WizFileIconProvider.h"
#include <QPixmapCache>
#include <QDir>


#if defined(Q_OS_WIN)
#  define _WIN32_IE 0x0500
#  include <qt_windows.h>
#  include <commctrl.h>
#  include <objbase.h>

#ifndef SHGFI_ADDOVERLAYS
#  define SHGFI_ADDOVERLAYS 0x000000020
#  define SHGFI_OVERLAYINDEX 0x000000040
#endif

#endif

#ifdef Q_OS_MAC
#include "WizMisc.h"
#endif

#include "utils/WizPathResolve.h"
#include "utils/WizMisc.h"


WizFileIconProvider::WizFileIconProvider()
{
}

#ifdef Q_OS_WIN
#if QT_VERSION >= 0x050000
extern Q_GUI_EXPORT QPixmap qt_pixmapFromWinHICON(HICON);
#endif
#endif

QIcon WizFileIconProvider::icon(const QString& strFilePath) const
{
    QFileInfo fi(strFilePath);
    if (!fi.exists())
        return QFileIconProvider::icon(QFileIconProvider::File);

    QIcon retIcon = QFileIconProvider::icon(fi);
    if (!retIcon.isNull())
        return retIcon;
    //
#if defined(Q_OS_WIN)
    for (int i = 0; i < 2; i++)
    {
        long flags = SHGFI_ICON|SHGFI_SYSICONINDEX|SHGFI_ADDOVERLAYS|SHGFI_OVERLAYINDEX | SHGFI_USEFILEATTRIBUTES;
        flags |= (i == 0 ? SHGFI_SMALLICON : SHGFI_LARGEICON);
        //
        SHFILEINFO info;
        memset(&info, 0, sizeof(SHFILEINFO));
        unsigned long val = SHGetFileInfo((const wchar_t *)QDir::toNativeSeparators(strFilePath).utf16(), 0, &info,
                            sizeof(SHFILEINFO), flags);
        //
        if (val && info.hIcon)
        {
            QPixmap pixmap = qt_pixmapFromWinHICON(info.hIcon);
            if (!pixmap.isNull())
            {
                retIcon.addPixmap(pixmap);
            }
            else
            {
              qWarning("QFileIconProviderPrivate::getWinIcon() no small icon found");
            }
            //
            DestroyIcon(info.hIcon);
        }
    }
#elif defined (Q_OS_MAC)
    QString strTempFileName = Utils::WizPathResolve::tempPath() + "test" + Utils::WizMisc::extractFileExt(strFilePath);
    ::WizSaveDataToFile(strTempFileName, QByteArray());
    retIcon = QFileIconProvider::icon(QFileInfo(strTempFileName));
    ::WizDeleteFile(strTempFileName);
#endif
    //
    return retIcon;
}


QString WizFileIconProvider::type(const QString& strFileName) const
{
#if defined(Q_OS_WIN)
    long flags = SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES;
    //
    SHFILEINFO info;
    memset(&info, 0, sizeof(SHFILEINFO));
    SHGetFileInfo((const wchar_t *)QDir::toNativeSeparators(strFileName).utf16(), 0, &info,
                        sizeof(SHFILEINFO), flags);
    //
    return QString::fromUtf16((const ushort*)info.szTypeName);
#else
    // FIXME: more types needed
    QFileInfo info(strFileName);
    QString suffix = info.suffix().toLower();
    if (suffix == "txt") {
        return QObject::tr("text document");
    } else if (suffix == "doc") {
        return QObject::tr("word document");
    } else if (suffix == "rar" || suffix == "zip" || suffix == "tar" || suffix == "gz") {
        return suffix.toUpper() + " " + QObject::tr("Compressed file");
    } else {
        return suffix.toUpper() + " " + QObject::tr("File");
    }
#endif
}



