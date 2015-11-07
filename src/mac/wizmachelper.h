#ifndef COCOAHELP_H
#define COCOAHELP_H

#include <QtGlobal>
#include <QRect>

#ifdef Q_OS_MAC


enum documentType {
    RTFTextDocumentType,                            //Rich text format document.
    RTFDTextDocumentType,                         //Rich text format with attachments document.
    MacSimpleTextDocumentType,            //Macintosh SimpleText document.
    HTMLTextDocumentType,                         //Hypertext Markup Language (HTML) document.
    DocFormatTextDocumentType,                 //Microsoft Word document.
    WordMLTextDocumentType,                     //Microsoft Word XML (WordML schema) document.
    WebArchiveTextDocumentType,               //Web Kit WebArchive document.
    OfficeOpenXMLTextDocumentType,         //ECMA Office Open XML text document format.
    OpenDocumentTextDocumentType,         //OASIS Open Document text document format.

};

class QMainWindow;
class QWidget;
class QMacCocoaViewContainer;

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

QString wizSystemClipboardData(QString& orignUrl);

bool documentToHtml(const QString& strFile, documentType type, QString& strHtml);

//path for yosemite
bool wizIsYosemiteFilePath(const QString& strPath);
QString wizConvertYosemiteFilePathToNormalPath(const QString& strYosePath);

void initCrashReporter();

void enableWidgetBehindBlur(QWidget* wgt);
//void enableWidgetBlendingBlur(QWidget* wgt);

bool systemWidgetBlurAvailable();

int getSystemMajorVersion();
int getSystemMinorVersion();
int getSystemPatchVersion();

void adjustSubViews(QWidget* wgt);

QMacCocoaViewContainer* createViewContainer(QWidget* wgt);

#endif // Q_OS_MAC

#endif // COCOAHELP_H
