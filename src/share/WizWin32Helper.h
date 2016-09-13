#ifndef WIZWIN32HELPER_H
#define WIZWIN32HELPER_H

#include <QtGlobal>

#ifdef Q_OS_WIN

#include <QFont>

int WizGetWindowsFontHeight();
QString WizGetWindowsFontName();

class QApplication;
class QString;
QFont WizCreateWindowsUIFont(const QApplication& a, const QString& strDefaultFontName);

#endif  //Q_OS_WIN

#endif // WIZWIN32HELPER_H
