#include <QPixmap>
#include <QIcon>

#import <AppKit/AppKit.h>
#import <objc/objc-class.h>

#include "cocoahelp_mac.h"

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
        [array addObject: toNSString(string)];
    }
    return array;
}

NSImage* WizToNSImage(const QPixmap &pixmap)
{
    NSBitmapImageRep *bitmapRep = [[NSBitmapImageRep alloc] initWithCGImage:pixmap.toMacCGImageRef()];
    NSImage *image = [[NSImage alloc] init];
    [image addRepresentation:bitmapRep];
    [bitmapRep release];
    return image;
}



NSImage* WizToNSImage(const QIcon &icon)
{
    QList<QSize> sizes = icon.availableSizes();
    if (sizes.empty())
        return nil;
    //
    QSize sz = sizes.at(0);
    for (int i = 1; i < sizes.size(); i++)
    {
        if (sizes.at(i).width() > sz.width())
        {
            sz = sizes.at(i);
        }
    }
    //
    QPixmap pixmap = icon.pixmap(sz);
    return toNSImage(pixmap);
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

