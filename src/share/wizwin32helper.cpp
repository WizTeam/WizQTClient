#include "wizwin32helper.h"

#include <qglobal.h>

#ifdef Q_WS_WIN32

#include <QApplication>
#include <QFont>
#include <Windows.h>

int WizGetWindowsFontHeight()
{
    NONCLIENTMETRICS ncm;
    ZeroMemory(&ncm, sizeof(NONCLIENTMETRICS));
#if (WINVER >= 0x0600)
    ncm.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(ncm.iPaddedBorderWidth);
#else
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
#endif
    //
    ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
    //
    return ncm.lfMenuFont.lfHeight;
}

QString WizGetWindowsFontName()
{
    NONCLIENTMETRICS ncm;
    ZeroMemory(&ncm, sizeof(NONCLIENTMETRICS));
#if (WINVER >= 0x0600)
    ncm.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(ncm.iPaddedBorderWidth);
#else
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
#endif
    //
    ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
    //
    return QString::fromUtf16((const unsigned short *)ncm.lfMenuFont.lfFaceName);
}

QFont WizCreateWindowsUIFont(const QApplication& a)
{
    QFont f = a.font();
    //
    f.setFamily(WizGetWindowsFontName());
    //
    int fontHeight = WizGetWindowsFontHeight();

    if (fontHeight < 0)
    {
        f.setPixelSize(-fontHeight);
    }
    else
    {
        f.setPointSize(fontHeight);
    }
    //
    return f;
}

#endif
