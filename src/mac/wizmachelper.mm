#include "wizmachelper.h"
#include "wizmachelper_mm.h"

#include "wizmainwindow.h"

#include "share/wizRtfReader.h"

#include <QLocale>
#include <QMainWindow>
#include <QSize>
#include <QXmlStreamReader>
#include <QStringList>
#include <QWebPage>
#include <QWebElement>
#include <QWebElementCollection>
#include <QWebFrame>
#include <QDebug>

#import <WebKit/WebKit.h>

#if QT_VERSION >= 0x050200
#include <qmacfunctions.h>
#endif
//#ifndef QT_MAC_USE_COCOA
//float qt_mac_get_scalefactor(QWidget *window)
//{
//    Q_UNUSED(window);
//    return HIGetScaleFactor();
//}
//#endif


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

CWizNSAutoReleasePool::CWizNSAutoReleasePool() : _pool([[NSAutoreleasePool alloc] init])
{
}

CWizNSAutoReleasePool::~CWizNSAutoReleasePool()
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

CWizChangeCocoaImplementation::CWizChangeCocoaImplementation(Class baseClass, SEL originalSel,
     Class proxyClass, SEL replacementSel, SEL backupSel, bool apply)
    : _baseClass(baseClass), _originalSel(originalSel), _backupSel(backupSel), _apply(apply)
{
    if (_apply)
        change(baseClass, originalSel, proxyClass, replacementSel, backupSel);
}

CWizChangeCocoaImplementation::~CWizChangeCocoaImplementation()
{
    if (_apply)
        changeBack(_baseClass, _originalSel, _backupSel);
}

void CWizChangeCocoaImplementation::change(Class baseClass, SEL originalSel, Class proxyClass, SEL replacementSel, SEL backupSel)
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

void CWizChangeCocoaImplementation::changeBack(Class baseClass, SEL originalSel, SEL backupSel)
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
            Core::Internal::MainWindow *window = Core::Internal::MainWindow::instance();
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
            Core::Internal::MainWindow *window = Core::Internal::MainWindow::instance();
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

QString wizRtfConveter(NSData *rtfData)
{
    // Read RTF into to NSAttributedString, then convert the string to HTML
    NSAttributedString *string = [[NSAttributedString alloc] initWithData:rtfData
                                                                          options:[NSDictionary dictionaryWithObject:NSRTFTextDocumentType forKey:NSDocumentTypeDocumentAttribute]
                                                                          documentAttributes:nil
                                                                          error:nil];
    NSError *error;
    NSRange range = NSMakeRange(0, [string length]);
    NSDictionary *dict = [NSDictionary dictionaryWithObject:NSHTMLTextDocumentType forKey:NSDocumentTypeDocumentAttribute];
    NSData *htmlData = [string dataFromRange:range documentAttributes:dict error:&error];
    NSString *htmlString = [[NSString alloc] initWithData:htmlData encoding:NSUTF8StringEncoding];
    return WizToQString(htmlString);
}

bool processWebImageUrl(QString& strHtml, const QString& strUrl)
{
    QWebPage page;
    QWebFrame* frame = page.mainFrame();
    QUrl webUrl(strUrl);
    frame->setHtml(strHtml, webUrl);
    QWebElement document = frame->documentElement();
    QWebElementCollection collection = document.findAll("img");
    foreach (QWebElement paraElement, collection) {
        QString strSrc = paraElement.attribute("src");
        QUrl elemUrl(strSrc);
//        qDebug() << "origin image src :  "  << strSrc;
        if (strSrc.left(2) == "//" && elemUrl.scheme().isEmpty())
        {
            elemUrl.setScheme(webUrl.scheme());
        }
        else if (strSrc.left(1) == "/" && elemUrl.scheme().isEmpty())
        {
            elemUrl.setScheme(webUrl.scheme());
            elemUrl.setHost(webUrl.host());
        }
        paraElement.setAttribute("src", elemUrl.toString());
//        strSrc = paraElement.attribute("src");
//        qDebug() << "after change scheme image src :  "  << strSrc;
    }
    strHtml = document.toInnerXml();

//    qDebug() << "after process document inner xml : " << document.toInnerXml();
//    qDebug() << "after process document outer xml : " << document.toOuterXml();
//    qDebug() << "after process frame html : " << frame->toHtml();

    return true;
}

QString wizSystemClipboardData()
{
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];

    NSArray *typeArray = [pasteboard types];
    NSString *type = @"com.apple.webarchive";
    if ([typeArray containsObject:type])
    {
        NSData* data = [pasteboard dataForType:type];
        WebArchive *archive = [[WebArchive alloc] initWithData:data];
        WebResource *resource = archive.mainResource;
        NSString *string = [[NSString alloc] initWithData:resource.data encoding:NSUTF8StringEncoding];
//        NSLog(@"webresource url %@", [[resource URL] absoluteString]);
//        NSLog(@"%@", string);
        [archive release];

        QString strHtml = WizToQString(string);
        NSString* url = [[resource URL] absoluteString];
        QString strUrl = WizToQString(url);
        processWebImageUrl(strHtml, strUrl);
        return strHtml;
    }

    return "";

/*
//    NSArray *typeArray = [pasteboard types];
    for (int i = 0; i < [typeArray count]; i++) {
        NSString *type = [typeArray objectAtIndex:i];
        NSString *string = [pasteboard stringForType:type];
        QString strType = WizToQString(type);
        QString str = WizToQString(string);
        qDebug() << "Data type   :  " << strType  << "  Data  : " << str;

        if ([type isEqual:@"com.apple.webarchive"])
        {
            NSData* data = [pasteboard dataForType:type];
            WebArchive *archive = [[WebArchive alloc] initWithData:data];
            WebResource *resource = archive.mainResource;
            NSString *string = [[NSString alloc] initWithData:resource.data encoding:NSUTF8StringEncoding];
            NSLog(@"%@", string);
            [archive release];
        }

    }



    if ([[pasteboard types] containsObject:NSRTFPboardType]) {
        NSData *htmlData = [pasteboard dataForType:NSRTFPboardType];
        if (htmlData) {
            QString strHtml = wizRtfConveter(htmlData);
            qDebug() << "html data after convert from rtf" << strHtml;

            NSString *htmlString = [[[NSString alloc] initWithData:htmlData encoding:NSUTF8StringEncoding] autorelease];
//            NSString *htmlString = [pasteboard  stringForType:NSHTMLPboardType];
            if (htmlString) {
                QString str = WizToQString(htmlString);
                QString strHtml;
                qDebug() << "data before convert :  " << str;
                CWizRtfReader::rtf2hmlt(str, strHtml);
                qDebug() << "after convert data from rtf to html   :  " << strHtml;
//                QFile file("/Users/lxn/text.rtf");
//                if (file.open(QIODevice::WriteOnly | QIODevice::Text))
//                {
//                    QTextStream out(&file);
//                    out << str;
//                    file.close();
//                }
//                return str;
            }

            return strHtml;
        }
    }

    NSArray *classArray = [NSArray arrayWithObject:[NSString class]];
    NSDictionary *options = [NSDictionary dictionary];

    BOOL ok = [pasteboard canReadObjectForClasses:classArray options:options];
    if (ok) {
        NSArray *objectsToPaste = [pasteboard readObjectsForClasses:classArray options:options];
        NSString *string = [objectsToPaste objectAtIndex:0];
        QString str = WizToQString(string);
        return str;
    }
    return "";
    */
}


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

//HIDictionaryWindowShow ( DCSDictionaryRef dictionary, CFTypeRef textString,
//                         CFRange selectionRange, CTFontRef textFont, CGPoint textOrigin, Boolean verticalText, const CGAffineTransform *viewTransform );


void wizHIDictionaryWindowShow(const QString& strText, QRect rcText)
{
//    CFStringRef cfString = (CFStringRef)WizToNSString(strText);
//    [HIDictionaryWindowShow dictionary:NULL textString:cfString selectionRange:];
}

