#ifndef COCOAHELP_H
#define COCOAHELP_H

#include <QtGlobal>

#ifdef Q_OS_MAC

#include <Cocoa/Cocoa.h>

class CWizNSAutoReleasePool
{
private:
    void *_pool;
public:
    CWizNSAutoReleasePool();
    ~CWizNSAutoReleasePool();
    inline void *handle() const { return _pool; }
};

#ifdef __OBJC__

QString WizToQString(NSString *string);
NSString* WizToNSString(const QString &string);
NSArray* WizToNSArray(const QList<QString> &stringList);
NSImage* WizToNSImage(const QPixmap &pixmap);
NSImage* WizToNSImage(const QIcon &icon);
NSString* WizGenGUID();

class CWizChangeCocoaImplementation
{
    public:
    CWizChangeCocoaImplementation(Class baseClass, SEL originalSel, Class proxyClass, SEL replacementSel, SEL backupSel, bool apply = true);
    ~CWizChangeCocoaImplementation();

    static void change(Class baseClass, SEL originalSel, Class proxyClass, SEL replacementSel, SEL backupSel);
    static void changeBack(Class baseClass, SEL originalSel, SEL backupSel);

    Class _baseClass;
    SEL _originalSel;
    SEL _backupSel;
    bool _apply;
};

#endif // __OBJC__

#endif //Q_OS_MAC

#endif // COCOAHELP_H
