#ifndef WIZWINDOWSHELPER_H
#define WIZWINDOWSHELPER_H

#include <QtGlobal>

#ifdef Q_OS_WIN

#include <QtGui>

int WizGetWindowStyle(HWND hwnd);
int WizGetWindowExStyle(HWND hwnd);

void WizSetWindowStyle(HWND hwnd, int style);
void WizSetWindowExStyle(HWND hwnd, int style);

#ifndef WS_EX_NOACTIVATE
    #define WS_EX_NOACTIVATE        0x08000000L
#endif

#endif

#endif // WIZWINDOWSHELPER_H
