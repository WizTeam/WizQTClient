#ifndef COCOAHELP_H
#define COCOAHELP_H

#include <QtGlobal>
#include <QRect>

#ifdef Q_OS_MAC

class QMainWindow;
class QWidget;

float qt_mac_get_scalefactor(QWidget *window);

void setupCocoa();
void setupFullScreenMode(QMainWindow* mainWindow);
void toggleFullScreenMode(QMainWindow* mainWindow);
QString WizMacGetOSVersion();

void wizMacHideCurrentApplication();
void wizMacShowCurrentApplication();
bool wizMacIsCurrentApplicationVisible();

void wizMacInitUncaughtExceptionHandler();

void wizMacRegisterSystemService();

void wizHIDictionaryWindowShow(const QString& strText, QRect rcText);

QString wizSystemClipboardData();

QString wizRrtConveter();

//path for yosemite
bool wizIsYosemiteFilePath(const QString& strPath);
QString wizConvertYosemiteFilePathToNormalPath(const QString& strYosePath);


#endif // Q_OS_MAC

#endif // COCOAHELP_H
