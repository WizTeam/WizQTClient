#ifndef COCOAHELP_H
#define COCOAHELP_H

#include <QtGlobal>
#include <QRect>
#include <QObject>

#ifdef Q_OS_MAC


enum WizMacDocumentType {
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
class QStringList;

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

bool wizDocumentToHtml(const QString& strFile, WizMacDocumentType type, QString& strHtml);

void wizMacSetClipboardText(const QString& strText);

void wizMacGetClipboardHtml(const QString& html, QString& url);

//path for yosemite
bool wizIsYosemiteFilePath(const QString& strPath);
QString wizConvertYosemiteFilePathToNormalPath(const QString& strYosePath);

void initCrashReporter();

int getSystemMajorVersion();
int getSystemMinorVersion();
int getSystemPatchVersion();

bool isDarkMode();
bool isMojaveOrHigher();

void updateShareExtensionAccount(const QString &userId, const QString &userGUID, const QString &myWiz, const QString &displayName);
void readShareExtensionAccount();

void adjustSubViews(QWidget* wgt);


Q_FORWARD_DECLARE_OBJC_CLASS(NSView);

class WizCocoaViewContainer : public QObject
{
public:
    WizCocoaViewContainer();
    //
    void setCocoaView(NSView* view);
    NSView* cocoaView() { return m_view; }
    //
    virtual QSize sizeHint() const { return QSize(); }
private:
    NSView* m_view;
};



#endif // Q_OS_MAC

#endif // COCOAHELP_H
