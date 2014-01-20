#include "wizmachelper.h"
#include "wizmachelper_mm.h"

#include <QLocale>
#include <QMainWindow>
#include <QSize>
#include <QDebug>
#include <qmacfunctions.h>

//#ifndef QT_MAC_USE_COCOA
//float qt_mac_get_scalefactor(QWidget *window)
//{
//    Q_UNUSED(window);
//    return HIGetScaleFactor();
//}
//#endif

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
    CGImageRef iref = QtMac::toCGImageRef(pixmap);
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
