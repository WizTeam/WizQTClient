#ifndef COCOAHELP_H
#define COCOAHELP_H

#include <QtGlobal>

#ifdef Q_OS_MAC

class QMainWindow;
class QWidget;

float qt_mac_get_scalefactor(QWidget *window);

void setupCocoa();
void setupFullScreenMode(QMainWindow* mainWindow);
void toggleFullScreenMode(QMainWindow* mainWindow);
QString WizMacGetOSVersion();


#endif // Q_OS_MAC

#endif // COCOAHELP_H
