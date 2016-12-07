#include "WizWindowsHelper.h"

#ifdef Q_OS_WIN

#include <windows.h>

int WizGetWindowStyle(HWND hwnd)
{
    return (int)::GetWindowLongW(hwnd, GWL_STYLE);
}

int WizGetWindowExStyle(HWND hwnd)
{
    return (int)::GetWindowLongW(hwnd, GWL_EXSTYLE);
}

void WizSetWindowStyle(HWND hwnd, int style)
{
    ::SetWindowLongW(hwnd, GWL_STYLE, style);
}

void WizSetWindowExStyle(HWND hwnd, int style)
{
    ::SetWindowLongW(hwnd, GWL_EXSTYLE, style);
}


#endif
