#ifndef COCOAHELP_H
#define COCOAHELP_H

#include <QtGlobal>

#ifdef Q_OS_MAC

#include <Cocoa/Cocoa.h>

class QNSAutoReleasePool
{
private:
    void *_pool;
public:
    QNSAutoReleasePool();
    ~QNSAutoReleasePool();
    inline void *handle() const { return _pool; }
};

#ifdef __OBJC__

NSString *toNSString(const QString &string);
QString toQString(NSString *string);
NSArray *toNSArray(const QList<QString> &stringList);
NSImage *toNSImage(const QPixmap &pixmap);
NSImage *toNSImage(const QIcon &icon);

class QChangeCocoaImplementation
{
    public:
    QChangeCocoaImplementation(Class baseClass, SEL originalSel, Class proxyClass, SEL replacementSel, SEL backupSel, bool apply = true);
    ~QChangeCocoaImplementation();

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
