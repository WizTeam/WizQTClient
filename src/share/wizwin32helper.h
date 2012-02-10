#ifndef WIZWIN32HELPER_H
#define WIZWIN32HELPER_H

#ifdef Q_WS_WIN32
int WizGetWindowsFontHeight();

class QApplication;
QFont WizCreateWindowsUIFont(const QApplication& a);

#endif  //Q_WS_WIN32

#endif // WIZWIN32HELPER_H
