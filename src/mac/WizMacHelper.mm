#include "WizMacHelper.h"
#include "WizMacHelper_mm.h"


#include <QLocale>
#include <QMainWindow>
#include <QSize>
#include <QXmlStreamReader>
#include <QStringList>
#include <QEventLoop>
#include <QDebug>
#include <QPixmap>
#include <qmacfunctions.h>
#include "html/WizHtmlCollector.h"

#import <WebKit/WebKit.h>

#ifdef UsePLCrashReporter
#import <CrashReporter/CrashReporter.h>
#import <CrashReporter/PLCrashReporterConfig.h>
#import <CrashReporter/PLCrashReportTextFormatter.h>
#endif

#include "WizMainWindow.h"

#include "share/WizRtfReader.h"
#include "utils/WizPathResolve.h"
#include "widgets/WizCrashReportDialog.h"

#if QT_VERSION >= 0x050200
#include <qmacfunctions.h>
#endif



#define WizShareSettingsName    @"KCS8N3QJ92.cn.wiz.extension"




@interface DBSCustomView: NSView

- (void)drawRect:(NSRect)dirtyRect;
@end

@implementation DBSCustomView

- (void)drawRect:(NSRect)dirtyRect
{
   // Drawing code here.
    [super drawRect: dirtyRect];  //父类，
    NSColor* textColor = [NSColor colorWithDeviceRed:0x78/255.0 green:0x78/255.0 blue:0x78/255.0 alpha:0.2];
    [textColor set];  //设置颜色
    NSRectFill(dirtyRect);//填充rect区域
}
@end


@interface CreateNoteService : NSObject

- (void) serviceCreateNote:(NSPasteboard *)pboard
                  userData:(NSString *)userData
                     error:(NSString **)error;

@end


//#ifdef QT_MAC_USE_COCOA
float qt_mac_get_scalefactor(QWidget *window)
    {
    // No high-dpi support on 10.6 and below
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_7) {
        if (window == 0) {
            // If there is no window given we answer the question
            // "Are there any HiDPI screens connected?" by returning
            // the highest scale factor found.
            CGFloat highestScaleFactor = 1.0;
            NSArray *screens = [NSScreen screens];
            for (id screen in screens) {
                highestScaleFactor = qMax(highestScaleFactor, [screen backingScaleFactor]);
            }
            return highestScaleFactor;
        } else {
            //return [qt_mac_window_for(window) backingScaleFactor];
            return [[NSScreen mainScreen] backingScaleFactor];
        }
    } else
#endif
    {
        return 1.0; // return 1.0 when compiled on or running on 10.6 and lower.
    }
}
//#endif

void setupCocoa()
{
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    NSDictionary* globalDomain = [defaults persistentDomainForName:@"NSGlobalDomain"];
    //NSArray* languages = [globalDomain objectForKey:@"AppleLanguages"];
    NSString* locale = [globalDomain objectForKey:@"AppleLocale"];
    QLocale::setDefault(WizToQString(locale));
    //[defaults setObject:languages forKey:@"AppleLanguages"];
    //[defaults setObject:locale forKey:@"AppleLocale"];
    //NSLog(@"%@\n", languages);
    //NSLog(@"%@\n", locale);
}

void setupFullScreenMode(QMainWindow* mainWindow)
{
    NSView *nsview = (NSView *) mainWindow->winId();
    NSWindow *nswindow = [nsview window];
    [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
}

void toggleFullScreenMode(QMainWindow* mainWindow)
{
    NSView *nsview = (NSView *) mainWindow->winId();
    NSWindow *nswindow = [nsview window];
    [nswindow toggleFullScreen:nil];
}

QString WizMacGetOSVersion()
{
    NSString *versionString = [[NSProcessInfo processInfo] operatingSystemVersionString];
    return WizToQString(versionString);
}

WizNSAutoReleasePool::WizNSAutoReleasePool() : _pool([[NSAutoreleasePool alloc] init])
{
}

WizNSAutoReleasePool::~WizNSAutoReleasePool()
{
    [static_cast<NSAutoreleasePool*>(_pool) release];
}

NSString* WizToNSString(const QString &string)
{
    return [NSString
        stringWithCharacters: reinterpret_cast<const UniChar *>(string.unicode())
        length: string.length()];
}

QString WizToQString(NSString *string)
{
    if (!string)
        return QString();

    QString qstring;
    qstring.resize([string length]);
    [string getCharacters: reinterpret_cast<unichar*>(qstring.data()) range : NSMakeRange(0, [string length])];

    return qstring;
}

NSArray* WizToNSArray(const QList<QString> &stringList)
{
    NSMutableArray *array = [[[NSMutableArray alloc] init] autorelease];
    foreach (const QString &string, stringList) {
        [array addObject: WizToNSString(string)];
    }
    return array;
}

NSImage* WizToNSImage(const QPixmap &pixmap)
{
    if (pixmap.isNull())
    {
        qDebug() << "WizToNSImage failed: pixmap is null";
        return nil;
    }
#if QT_VERSION >= 0x050200
    CGImageRef iref = QtMac::toCGImageRef(pixmap);
#else
    CGImageRef iref = pixmap.toMacCGImageRef();
#endif
    NSSize sz = NSMakeSize(pixmap.width(), pixmap.height());
    NSImage *image = [[NSImage alloc] initWithCGImage:iref size:sz];
    return image;
}

NSImage* WizToNSImage(const QIcon &icon, const QSize& size)
{
    QSize sz = size;
    if (sz.isNull() || sz.width() <= 0 || sz.height() <= 0) {
        QList<QSize> sizes = icon.availableSizes();
        if (sizes.empty())
            return nil;

        QSize sz1 = sizes.at(0);
        for (int i = 1; i < sizes.size(); i++)
        {
            if (sizes.at(i).width() > sz1.width())
            {
                sz1 = sizes.at(i);
            }
        }

        sz = sz1;
    }

    QPixmap pixmap = icon.pixmap(sz);
    return WizToNSImage(pixmap);
}

NSString* WizGenGUID()
{
    CFUUIDRef theUUID = CFUUIDCreate(NULL);
    CFStringRef string = CFUUIDCreateString(NULL, theUUID);
    CFRelease(theUUID);
    //
    NSString* str = [NSString stringWithString:(NSString*)string];
    //
    CFRelease(string);
    //
    return [str lowercaseString];
}

WizChangeCocoaImplementation::WizChangeCocoaImplementation(Class baseClass, SEL originalSel,
     Class proxyClass, SEL replacementSel, SEL backupSel, bool apply)
    : _baseClass(baseClass), _originalSel(originalSel), _backupSel(backupSel), _apply(apply)
{
    if (_apply)
        change(baseClass, originalSel, proxyClass, replacementSel, backupSel);
}

WizChangeCocoaImplementation::~WizChangeCocoaImplementation()
{
    if (_apply)
        changeBack(_baseClass, _originalSel, _backupSel);
}

void WizChangeCocoaImplementation::change(Class baseClass, SEL originalSel, Class proxyClass, SEL replacementSel, SEL backupSel)
{
#ifndef QT_MAC_USE_COCOA
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5)
#endif
    {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
        // The following code replaces the _implementation_ for the selector we want to hack
        // (originalSel) with the implementation found in proxyClass. Then it creates
        // a new 'backup' method inside baseClass containing the old, original,
        // implementation. You can let replacementSel
        // call backupSel if needed (similar approach to calling a super class implementation).
        // backupSel must also be implemented in proxyClass, as the signature is used
        // as template for the method one we add into baseClass.
        // NB: You will typically never create any instances of proxyClass; we use it
        // only for stealing its contents and put it into baseClass.
        if (!replacementSel)
            replacementSel = originalSel;

        Method originalMethod = class_getInstanceMethod(baseClass, originalSel);
        Method replacementMethod = class_getInstanceMethod(proxyClass, replacementSel);
        IMP originalImp = method_setImplementation(originalMethod, method_getImplementation(replacementMethod));

        if (backupSel) {
            Method backupMethod = class_getInstanceMethod(proxyClass, backupSel);
            class_addMethod(baseClass, backupSel, originalImp, method_getTypeEncoding(backupMethod));
        }
#endif
    }
}

void WizChangeCocoaImplementation::changeBack(Class baseClass, SEL originalSel, SEL backupSel)
{
#ifndef QT_MAC_USE_COCOA
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5)
#endif
    {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
        Method originalMethod = class_getInstanceMethod(baseClass, originalSel);
        Method backupMethodInBaseClass = class_getInstanceMethod(baseClass, backupSel);
        method_setImplementation(originalMethod, method_getImplementation(backupMethodInBaseClass));
#endif
    }
}

void wizMacHideCurrentApplication()
{
    NSRunningApplication *app = [NSRunningApplication currentApplication];
    [app hide];
}

void wizMacShowCurrentApplication()
{
    NSRunningApplication *app = [NSRunningApplication currentApplication];
    [app unhide];
}

bool wizMacIsCurrentApplicationVisible()
{
    NSRunningApplication *app = [NSRunningApplication currentApplication];
    return YES != app.hidden;
}

void WizMacNSUncaughtExceptionHandler(NSException *exception)
{
    NSString* msg = [exception reason];
    QString err = WizToQString(msg);
    qDebug() << err;
}

void wizMacInitUncaughtExceptionHandler()
{
    NSSetUncaughtExceptionHandler(WizMacNSUncaughtExceptionHandler);
}



void wizMacRegisterSystemService()
{
    CreateNoteService *noteService = [[CreateNoteService alloc] init];
    [NSApp setServicesProvider:noteService];
}

void convertYosemiteFileListToNormalList(QStringList& fileList)
{
    int nCount = fileList.count();
    for (int i = 0;i < nCount; i++)
    {
        if (wizIsYosemiteFilePath(fileList.at(i)))
        {
            fileList[i] = wizConvertYosemiteFilePathToNormalPath(fileList.at(i));
        }
    }
}

@implementation CreateNoteService

- (void)serviceCreateNote:(NSPasteboard *)pboard
                 userData:(NSString *)userData
                    error:(NSString **)error
{
    for (NSString *type in [pboard types])
    {
        //NSLog(@"types %@", type);
        qDebug() << "avaliable pboard type : " << WizToQString(type);
    }

    // Get string.
    NSString *pFileNamesString = [pboard stringForType:NSFilenamesPboardType];
    QString strXml = WizToQString(pFileNamesString);
    if (!strXml.isEmpty())
    {
        QStringList strFileList;
        QXmlStreamReader xmlReader(strXml);
        while (xmlReader.readNextStartElement())
        {
            if (xmlReader.name() == "string")
            {
                QString strFile = xmlReader.readElementText();
                strFileList.append(strFile);
            }
        }

        if (strFileList.count() > 0)
        {
            WizMainWindow *window = WizMainWindow::instance();
            if (window)
            {
                convertYosemiteFileListToNormalList(strFileList);
                foreach (QString strFile, strFileList) {
                    qDebug() << "create document with : " << strFile;
                }
                window->createNoteWithAttachments(strFileList);
            }
        }
    }
    else
    {
        NSString *pString = [pboard stringForType:NSStringPboardType];
        QString strText = WizToQString(pString);
        if (!strText.isEmpty())
        {
            qDebug() << "[service] : text string finded : " << strText;
            WizMainWindow *window = WizMainWindow::instance();
            if (window)
            {
                window->createNoteWithText(strText);
            }
        }
        else
        {
            qDebug() << "[Service] : nothing usefull in pasteboard finded.";
        }
    }

    // Write the encrypted string onto the pasteboard.
    [pboard clearContents];


}

@end

bool wizIsYosemiteFilePath(const QString& strPath)
{
    return strPath.indexOf("file:///.file/id") == 0 || strPath.indexOf("///.file/id") == 0;
}

QString wizConvertYosemiteFilePathToNormalPath(const QString& strYosePath)
{
    QString strFilePath = strYosePath;
    if (strFilePath.left(3) == "///")
    {
        strFilePath = "file:" + strFilePath;
    }
    NSString *fileIdURL = WizToNSString(strFilePath);
    NSString *goodURL = [[[NSURL URLWithString:fileIdURL] filePathURL] path];
    return WizToQString(goodURL);
}


void wizHIDictionaryWindowShow(const QString& strText, QRect rcText)
{
//    CFStringRef cfString = (CFStringRef)WizToNSString(strText);
//    [HIDictionaryWindowShow dictionary:NULL textString:cfString selectionRange:];
}

NSString* getDoucmentType(WizMacDocumentType type)
{
   // NSString* temp = @"NULL";
    switch (type) {
    case RTFTextDocumentType:
        return @"NSRTFTextDocumentType";
        break;
    case RTFDTextDocumentType:
        return @"NSRTFDTextDocumentType";
        break;
    case MacSimpleTextDocumentType:
        return @"NSMacSimpleTextDocumentType";
        break;
    case HTMLTextDocumentType:
        return @"NSHTMLTextDocumentType";
        break;
    case DocFormatTextDocumentType:
        return @"NSDocFormatTextDocumentType";
        break;
    case WordMLTextDocumentType:
        return @"NSWordMLTextDocumentType";
        break;
    case WebArchiveTextDocumentType:
        return @"NSWebArchiveTextDocumentType";
        break;
    case OfficeOpenXMLTextDocumentType:
        return @"NSOfficeOpenXMLTextDocumentType";
        break;
    case OpenDocumentTextDocumentType:
        return @"NSOpenDocumentTextDocumentType";
        break;
    default:
        return @"NSPlainTextDocumentType";
        break;
    }
    return @"NULL";
}

QString wizAttributedStringToHtml(NSAttributedString *string)
{
    NSError *error;
    NSRange range = NSMakeRange(0, [string length]);
    NSDictionary *dict = [NSDictionary dictionaryWithObject:NSHTMLTextDocumentType forKey:NSDocumentTypeDocumentAttribute];
    NSData *htmlData = [string dataFromRange:range documentAttributes:dict error:&error];
    NSString *htmlString = [[NSString alloc] initWithData:htmlData encoding:NSUTF8StringEncoding];
    return WizToQString(htmlString);
}

QString wizDataToHtml(NSData *data, NSString* dataType)
{
    // Read RTF into to NSAttributedString, then convert the string to HTML
    NSAttributedString *string = [[NSAttributedString alloc] initWithData:data
                                                                          options:[NSDictionary dictionaryWithObject:dataType forKey:NSDocumentTypeDocumentAttribute]
                                                                          documentAttributes:nil
                                                                          error:nil];

    return wizAttributedStringToHtml(string);
}

QString wizUrlToHtml(NSString* url)
{
    NSAttributedString *string = [[NSAttributedString alloc] initWithPath:url
                                                                               documentAttributes:nil];
    return wizAttributedStringToHtml(string);
}



QString wizRtfToHtml(NSData *data)
{
    NSAttributedString *string = [[NSAttributedString alloc] initWithRTF:data
                                                                               documentAttributes:nil];
    return wizAttributedStringToHtml(string);
}

QString wizDocToHtml(NSData *data)
{
    NSAttributedString *string = [[NSAttributedString alloc] initWithDocFormat:data
                                                                               documentAttributes:nil];
    return wizAttributedStringToHtml(string);
}

bool processWebarchiveImageUrl(const QString& strFileName, QString& strHtml, const QString& strResourcePath)
{
    class CProcessWebarchiveHtmlCollector : public WizHtmlCollector
    {
    public:
        CProcessWebarchiveHtmlCollector(const QString& strResourcePath)
            : m_strResourcePath(strResourcePath)
        {

        }

        virtual void startTag(WizHtmlTag *pTag, DWORD dwAppData, bool &bAbort)
        {
            QString tagName = pTag->getTagName();
            tagName = tagName.toUpper();
            //
            if (tagName == "IMG"
                    || tagName == "SCRIPT"
                    || tagName == "STYLE")
            {
                processTagValue(pTag, "src");
            }
            else if (tagName == "LINK")
            {
                processTagValue(pTag, "href");
                //
                qDebug() << pTag->getTag();
            }
            //
            processTagValue(pTag, "background");
            //
            m_ret.push_back(pTag->getTag());
        }
        //
        void processTagValue(WizHtmlTag* pTag, const QString& valueName)
        {
            QString value = pTag->getValueFromName(valueName);
            if (value.isEmpty())
                return;
            //
            qDebug() << "value before: " << value;
            //
            if (value.startsWith("file:///"))
            {
                value.remove(0, 8);
                value = "file://" + m_strResourcePath + value;
                //
                qDebug() << "value result: " << value;
                //
                pTag->setValueToName(valueName, value);
            }
        }
    private:
        QString m_strResourcePath;
    };
    //
    CProcessWebarchiveHtmlCollector collector(strResourcePath);
    //
    collector.collect(strFileName, strHtml, true, strResourcePath);

    return true;
}


QString wizWebarchiveToHtml(NSString *filePath)
{
    QString webFile = WizToQString(filePath);
    if (!QFile::exists(webFile))
        return QString();
    //
    QFileInfo info(webFile);
    QString strFolder = Utils::WizPathResolve::tempPath() + WizGenGUIDLowerCaseLetterOnly() + "/";
    QString newFile = strFolder + info.fileName();
    QDir dir;
    dir.mkdir(strFolder);
    if (!QFile::copy(webFile, newFile))
        return QString();
    //
    QString htmlFile = strFolder + info.baseName() + ".html";
    QString commandLine = QString("textutil -convert html \"%1\" -output \"%2\"").arg(newFile).arg(htmlFile);

    // convert webarchive to html
    QProcess process;
    QEventLoop loop;
    QObject::connect(&process, SIGNAL(finished(int)), &loop, SLOT(quit()));
    process.start(commandLine);
    loop.exec();

    qDebug() << "convert html file finished";

    QString strHtml;
    ::WizLoadUnicodeTextFromFile(htmlFile, strHtml);

    if (strHtml.isEmpty())
        return QString();
    //
    //qDebug() << strHtml;
    //
    processWebarchiveImageUrl(newFile, strHtml, strFolder);

    return strHtml;
}

bool wizDocumentToHtml(const QString& strFile, WizMacDocumentType type, QString& strHtml)
{
    NSString* filePath = WizToNSString(strFile);

    if([[NSFileManager defaultManager] fileExistsAtPath:filePath])
    {
       NSData *data = [[NSFileManager defaultManager] contentsAtPath:filePath];
       //NSLog(@"document data loaded : %@", data);
       switch (type) {
       case DocFormatTextDocumentType:
       case RTFTextDocumentType:
           strHtml = wizUrlToHtml(filePath);
           break;
       case WebArchiveTextDocumentType:
           strHtml = wizWebarchiveToHtml(filePath);
           break;
       default:
           NSString* docType = getDoucmentType(type);
           strHtml = wizDataToHtml(data, docType);
           break;
       }
       return true;
    }
    else
    {
       NSLog(@"File not exits");
       return false;
    }

    return true;
}

void wizMacSetClipboardText(const QString& strText)
{
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    if (!pasteboard)
        return;
    //
    [pasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
    [pasteboard setString:WizToNSString(strText) forType:NSStringPboardType];
}


void wizMacGetClipboardHtml(const QString& html, QString& url)
{
    NSArray* arr = [[NSPasteboard generalPasteboard] types];
    for (int i = 0; i < arr.count; i++)
    {
        NSString* type = arr[i];
        NSLog(@"%@", type);
    }
    //
    NSString* WEB_ARCHIVE = @"Apple Web Archive pasteboard type";
    if ([[[NSPasteboard generalPasteboard] types] containsObject:WEB_ARCHIVE]) {
        NSData* archiveData = [[NSPasteboard generalPasteboard] dataForType:WEB_ARCHIVE];
        if (archiveData)
        {
            NSError* error = nil;
            id webArchive = [NSPropertyListSerialization propertyListWithData:archiveData options:NSPropertyListImmutable format:NULL error:&error];
            if (error) {
                return;
            }
            NSArray *subItems = [NSArray arrayWithArray:[webArchive objectForKey:@"WebSubresources"]];
            NSPredicate *iPredicate = [NSPredicate predicateWithFormat:@"WebResourceMIMEType like 'image*'"];
            NSArray *imagesArray = [subItems filteredArrayUsingPredicate:iPredicate];
            for (int i=0; i<[imagesArray count]; i++) {
                NSDictionary *sItem = [NSDictionary dictionaryWithDictionary:[imagesArray objectAtIndex:i]];
                //NSImage *sImage = [NSImage imageWithData:[sItem valueForKey:@"WebResourceData"]];
                // handle images
            }
        }
    }
    //
    //TODO: not finished

}


#ifdef UsePLCrashReporter
//
// Called to handle a pending crash report.
//
void handleCrashReport(PLCrashReporter *crashReporter)
{
    NSData *crashData;
    NSError *error;

    // Try loading the crash report
    crashData = [crashReporter loadPendingCrashReportDataAndReturnError: &error];
    if (crashData == nil) {
        NSLog(@"Could not load crash report: %@", error);

        // Purge the report
        [crashReporter purgePendingCrashReport];
        return;
    }

    // We could send the report from here, but we'll just print out
    // some debugging info instead
    PLCrashReport *report = [[[PLCrashReport alloc] initWithData: crashData error: &error] autorelease];
    if (report == nil) {
        NSLog(@"Could not parse crash report");

        // Purge the report
        [crashReporter purgePendingCrashReport];
        return;
    }

    NSLog(@"Crashed founded. on %@", report.systemInfo.timestamp);
    NSLog(@"Crashed with signal %@ (code %@, address=0x%\" PRIx64 \")", report.signalInfo.name, report.signalInfo.code, report.signalInfo.address);


    ///////// save to local file

//    NSFileManager *fm = [NSFileManager defaultManager];

//    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
//    NSString *documentsDirectory = [paths objectAtIndex:0];
//    if (![fm createDirectoryAtPath: documentsDirectory withIntermediateDirectories: YES attributes:nil error: &error]) {
//        NSLog(@"Could not create documents directory: %@", error);
//        return;
//    }

//    QString time = QDateTime::currentDateTime().toString();
//    NSString* nsTime = WizToNSString(time);
//    NSString *outputPath = [documentsDirectory stringByAppendingPathComponent: nsTime];
//    if (![crashData writeToFile: outputPath atomically: YES]) {
//        NSLog(@"Failed to write crash report");
//    }

//    NSLog(@"Saved crash report to: %@", outputPath);


    //////////
    /* Verify that the format is supported. Only one is actually supported currently */
    PLCrashReportTextFormat textFormat = PLCrashReportTextFormatiOS;

    /* Format the report */
    NSString* reports = [PLCrashReportTextFormatter stringValueForCrashReport: report withTextFormat: textFormat];
//    fprintf(output, "%s", [reports UTF8String]);
//    NSLog(@"report : %@", reports);
    QString strReport = WizToQString(reports);
    qDebug() << "report size : " << strReport.toUtf8().size();

    //
    [crashReporter purgePendingCrashReport];

    WizCrashReportDialog dlg(strReport);
    dlg.exec();
}

void initCrashReporter()
{
    PLCrashReporterConfig* config = [[[PLCrashReporterConfig alloc] initWithSignalHandlerType: PLCrashReporterSignalHandlerTypeBSD
    symbolicationStrategy: PLCrashReporterSymbolicationStrategySymbolTable] autorelease];
    PLCrashReporter *crashReporter = [[[PLCrashReporter alloc] initWithConfiguration: config] autorelease];


    // Check if we previously crashed
    if ([crashReporter hasPendingCrashReport])
    {
        handleCrashReport(crashReporter);
    }

    NSError *error = nil;
    // Enable the Crash Reporter
    if (![crashReporter enableCrashReporterAndReturnError: &error])
        NSLog(@"Warning: Could not enable crash reporter: %@", error);

}

#else
void initCrashReporter()
{}
#endif



void adjustSubViews(QWidget* wgt)
{
    qDebug() << "wgt size : " << wgt->size() << " wgt pos : " << wgt->mapToGlobal(QPoint(0, 0));
    NSView *nsview = (NSView *) wgt->winId();
    NSArray* subviewArray = [nsview subviews];
    for (NSView* subview in subviewArray)
    {
        NSLog(@"self bounds %f, %f  ; pos %f %f", subview.frame.size.width, subview.frame.size.height,
              subview.frame.origin.x, subview.frame.origin.y);

        if (subview.frame.size.width == 640)
        {
            NSRect f = subview.frame;
            f.origin.x = 20;
            f.origin.y = 40;
            f.size.width = 300;
            f.size.height = 100;
            subview.frame = f;
        }
    }
}


int getSystemMajorVersion()
{
    SInt32 major;
    Gestalt(gestaltSystemVersionMajor, &major);

    return major;
}

int getSystemMinorVersion()
{
    SInt32 minor;
    Gestalt(gestaltSystemVersionMinor, &minor);
    return minor;
}

int getSystemPatchVersion()
{
    SInt32 bugfix;
    Gestalt(gestaltSystemVersionBugFix, &bugfix);
    return bugfix;
}

bool isDarkMode()
{
    static bool first = true;
    static bool ret = false;
    if (first) {
        first = false;
        //
        int major = getSystemMajorVersion();
        int minor = getSystemMinorVersion();
        //return false;
        if ((major >= 11) || (major == 10 && minor >= 14)) {
            NSDictionary *dict = [[NSUserDefaults standardUserDefaults] persistentDomainForName:NSGlobalDomain];
            id style = [dict objectForKey:@"AppleInterfaceStyle"];
            bool darkModeOn = ( style && [style isKindOfClass:[NSString class]] && NSOrderedSame == [style caseInsensitiveCompare:@"dark"] );
            ret = darkModeOn;
        }
    }
    return ret;
}

bool isMojaveOrHigher()
{
    static bool isMojave = false;
    static bool first = true;
    if (first) {
        first = false;
        int major = getSystemMajorVersion();
        int minor = getSystemMinorVersion();
        if ((major >= 11) || (major == 10 && minor >= 14)) {
            isMojave = true;
        }
    }
    return isMojave;
}


void updateShareExtensionAccount(const QString& userId, const QString& userGUID, const QString& myWiz, const QString& displayName)
{
    NSURL *containerURL = [[NSFileManager defaultManager] containerURLForSecurityApplicationGroupIdentifier:WizShareSettingsName];
    NSLog(@"app group:\n%@",containerURL.path);

    NSError *err = nil;
    containerURL = [containerURL URLByAppendingPathComponent:@"Library/Application Support/UserAccount"];

    QString userAccount = userId + ";" + userGUID + ";" + myWiz + ";" + displayName;
    NSString *value = WizToNSString(userAccount);

    BOOL result = [value writeToURL:containerURL atomically:YES encoding:NSUTF8StringEncoding error:&err];

    if (!result) {
        NSLog(@"%@",err);
    } else {
        NSLog(@"save value:%@ success.",value);
    }




//    NSUserDefaults* shared = [[NSUserDefaults alloc] initWithSuiteName:WizShareSettingsName];
//    if (!shared)
//        return;
//    //
//    NSString* nsUserId = WizToNSString(userId);
//    NSString* nsUserGUID = WizToNSString(userGUID);
//    NSString* nsMyWiz = WizToNSString(myWiz);
//    NSString* nsDisplayName = WizToNSString(displayName);
//    [shared setObject:nsUserId forKey:@"userId"];
//    [shared setObject:nsUserGUID forKey:@"userGuid"];
//    [shared setObject:nsMyWiz forKey:@"mywizEmail"];
//    [shared setObject:nsDisplayName forKey:@"displayName"];
//    //
//    bool ret = [shared synchronize];
//    qDebug() << "update extension user account : " << ret;
}


void readShareExtensionAccount()
{
    NSURL *containerURL = [[NSFileManager defaultManager] containerURLForSecurityApplicationGroupIdentifier:WizShareSettingsName];
    NSLog(@"app group:\n%@",containerURL.path);

    NSError *err = nil;
    containerURL = [containerURL URLByAppendingPathComponent:@"Library/Application Support/UserAccount"];

    NSString *value = [NSString stringWithContentsOfURL:containerURL encoding:NSUTF8StringEncoding error:&err];
    QString userAccount = WizToQString(value);
    QStringList userDataList = userAccount.split(';');
    if (userDataList.size() < 4)
        return;

    qDebug() << "user id : " << userDataList.at(0);
    qDebug() << "user guid : " << userDataList.at(1);
    qDebug() << "mywiz : " << userDataList.at(2);


//    NSUserDefaults* shared = [[NSUserDefaults alloc] initWithSuiteName:WizShareSettingsName];
//    if (!shared)
//        return;
//    //
//    NSString* nsUserId = [shared objectForKey:@"userId"];
//    NSString* nsUserGUID = [shared objectForKey:@"userGuid"];
//    NSString* nsMyWiz = [shared objectForKey:@"mywizEmail"];
//    NSString* nsDisplayName = [shared objectForKey:@"displayName"];

////    NSLog(@"user account :  %@ %@ %@ %@", nsUserId, nsUserGUID, nsMyWiz, nsDisplayName);
//    qDebug() << "user id : " << WizToQString(nsUserId);
//    qDebug() << "user guid : " << WizToQString(nsUserGUID);
//    qDebug() << "mywiz : " << WizToQString(nsMyWiz);
}


WizCocoaViewContainer::WizCocoaViewContainer()
    : m_view(nil)
{

}
void WizCocoaViewContainer::setCocoaView(NSView* view)
{
    m_view = view;
    [m_view retain];
}


QList<WizWindowInfo> WizGetActiveWindows()
{
    QList<WizWindowInfo> windowTitles;

    // get frontmost process for currently active application
    ProcessSerialNumber psn = { 0L, 0L };
    OSStatus err = GetFrontProcess(&psn);

    CFStringRef processName = NULL;
    err = CopyProcessName(&psn, &processName);

    NSString *pname = (NSString *)processName;

    // loop through all application windows
    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    for (NSMutableDictionary* entry in (NSArray*)windowList)
    {
        NSString* ownerName = [entry objectForKey:(id)kCGWindowOwnerName];
        NSString *name = [entry objectForKey:@"kCGWindowName" ];
        NSInteger ownerPID = [[entry objectForKey:(id)kCGWindowOwnerPID] integerValue];
        NSInteger layer = [[entry objectForKey:@"kCGWindowLayer"] integerValue];
        if(layer == 0)
        {
            if([ownerName isEqualToString:pname])
            {
                NSRange range;
                range.location = 0;
                range.length = [ownerName length];

                unichar *chars = new unichar[range.length];
                [ownerName getCharacters:chars range:range];
                QString owner = QString::fromUtf16(chars, range.length);

                range.length = [name length];

                chars = new unichar[range.length];
                [name getCharacters:chars range:range];
                QString windowTitle = QString::fromUtf16(chars, range.length);
                delete[] chars;

                long pid = (long)ownerPID;

                WizWindowInfo wi;
                wi.processName = owner;
                wi.windowTitle = windowTitle;
                wi.pid = pid;
                windowTitles.append(wi);
            }
        }
    }
    //
    if (windowList) {
        CFRelease(windowList);
    }
    //
    if (processName) {
        CFRelease(processName);
    }

    return windowTitles;
}
