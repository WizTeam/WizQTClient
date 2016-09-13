#ifndef WIZMACHELPER_MM_H
#define WIZMACHELPER_MM_H

#include <QList>
#include <QPixmap>
#include <QSize>
#include <QIcon>

#import <AppKit/AppKit.h>
#import <objc/objc-class.h>

class WizNSAutoReleasePool
{
private:
    void *_pool;
public:
    WizNSAutoReleasePool();
    ~WizNSAutoReleasePool();
    inline void *handle() const { return _pool; }
};


QString WizToQString(NSString *string);
NSString* WizToNSString(const QString &string);
NSArray* WizToNSArray(const QList<QString> &stringList);
NSImage* WizToNSImage(const QPixmap &pixmap);
NSImage* WizToNSImage(const QIcon &icon, const QSize &size = QSize());
NSString* WizGenGUID();

class WizChangeCocoaImplementation
{
    public:
    WizChangeCocoaImplementation(Class baseClass, SEL originalSel, Class proxyClass, SEL replacementSel, SEL backupSel, bool apply = true);
    ~WizChangeCocoaImplementation();

    static void change(Class baseClass, SEL originalSel, Class proxyClass, SEL replacementSel, SEL backupSel);
    static void changeBack(Class baseClass, SEL originalSel, SEL backupSel);

    Class _baseClass;
    SEL _originalSel;
    SEL _backupSel;
    bool _apply;
};



#endif // WIZMACHELPER_MM_H
