#include "wizmachelper.h"
#include "wizmachelper_mm.h"

#include "wizmainwindow.h"

#include "share/wizRtfReader.h"
#include "utils/pathresolve.h"

#include <QLocale>
#include <QMainWindow>
#include <QSize>
#include <QXmlStreamReader>
#include <QStringList>
#include <QWebPage>
#include <QWebElement>
#include <QWebElementCollection>
#include <QWebFrame>
#include <QEventLoop>
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
        if (elemUrl.scheme().isEmpty())
        {
            if (strSrc.left(2) == "//")
            {
                elemUrl.setScheme(webUrl.scheme());
            }
            else if (strSrc.left(1) == "/")
            {
                elemUrl.setScheme(webUrl.scheme());
                elemUrl.setHost(webUrl.host());
            }
            else if (strSrc.left(3) == "../")
            {
                elemUrl.setUrl(webUrl.scheme() + "://"+ webUrl.host() + strSrc.remove(0, 2));
            }
            else if (elemUrl.host().isEmpty())
            {
                elemUrl.setHost(webUrl.host());
                elemUrl.setScheme(webUrl.scheme());
            }
            else
            {
                elemUrl.setScheme(webUrl.scheme());
            }
//            qDebug() << "after reset url scheme , url " << elemUrl.toString();
        }
        paraElement.setAttribute("src", elemUrl.toString());
//        strSrc = paraElement.attribute("src");te
//        qDebug() << "after change scheme image src :  "  << strSrc;
    }
    strHtml = document.toInnerXml();

    return true;
}

bool processWebarchiveImageUrl(QString& strHtml, const QString& strFolderPath)
{
    QWebPage page;
    QWebFrame* frame = page.mainFrame();
    frame->setHtml(strHtml);
    QWebElement document = frame->documentElement();
    QWebElementCollection collection = document.findAll("img");
    foreach (QWebElement paraElement, collection) {
        QString strSrc = paraElement.attribute("src");
        qDebug() << "origin image src :  "  << strSrc;
        if (strSrc.left(8) == "file:///")
        {
            strSrc.remove(0, 8);
            strSrc = strFolderPath + strSrc;
        }
        paraElement.setAttribute("src", strSrc);
        strSrc = paraElement.attribute("src");
        qDebug() << "after change scheme image src :  "  << strSrc;
    }
    strHtml = document.toInnerXml();

    return true;
}



QString wizSystemClipboardData(QString& orignUrl)
{
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];

    NSArray *typeArray = [pasteboard types];
//    NSLog(@"clipboard types : %@", typeArray);
//    NSString* htmlType = @"public.html";
//    if ([typeArray containsObject:htmlType])
//    {
//        NSData* data = [pasteboard dataForType:htmlType];

//        WebArchive *archive = [[WebArchive alloc] initWithData:data];
//        WebResource *resource = archive.mainResource;
//        NSLog(@"webresource url %@", [[resource URL] absoluteString]);
//        [archive release];

//        NSString* url = [[resource URL] absoluteString];
//        orignUrl = WizToQString(url);
//    }
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
        orignUrl = WizToQString(url);
        processWebImageUrl(strHtml, orignUrl);
        return strHtml;
    }

    return "";
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


void wizHIDictionaryWindowShow(const QString& strText, QRect rcText)
{
//    CFStringRef cfString = (CFStringRef)WizToNSString(strText);
//    [HIDictionaryWindowShow dictionary:NULL textString:cfString selectionRange:];
}

NSString* getDoucmentType(documentType type)
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


QString wizWebarchiveToHtml(NSString *filePath)
{
    QString webFile = WizToQString(filePath);
    if (QFile::exists(webFile))
    {
        QFileInfo info(webFile);
        QString strFolder = Utils::PathResolve::tempPath() + WizGenGUIDLowerCaseLetterOnly() + "/";
        QString newFile = strFolder + info.fileName();
        QDir dir;
        dir.mkdir(strFolder);
        QFile::copy(webFile, newFile);

        // convert webarchive to html
        QProcess process;
        QEventLoop loop;
        QObject::connect(&process, SIGNAL(finished(int)), &loop, SLOT(quit()));
        process.start(QString("textutil -convert html %1").arg(newFile));
        loop.exec();
        newFile = strFolder + info.baseName() + ".html";

        qDebug() << "convert html file finished";

        QByteArray ba;
        WizLoadDataFromFile(newFile, ba);
        QString strHtml(ba);


        if (!strHtml.isEmpty())
        {
            processWebarchiveImageUrl(strHtml, strFolder);

            return strHtml;
        }
    }
    return "";
}

bool documentToHtml(const QString& strFile, documentType type, QString& strHtml)
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
