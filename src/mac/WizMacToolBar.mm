/****************************************************************************
 **
 ** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
 **     the names of its contributors may be used to endorse or promote
 **     products derived from this software without specific prior written
 **     permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOTgall
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#ifdef USECOCOATOOLBAR

#include <QApplication>
#include <QtCore>
#include <QAction>
#include "WizMacHelper.h"
#include "WizMacToolBar.h"
#include "WizMacToolBarDelegate.h"
#include "WizMacHelper_mm.h"
#include "WizSearchWidget_mm.h"

#import <AppKit/AppKit.h>


class QOpenGLContext;
class QScreen;
class QWindow;
class QPlatformWindow;
class QBackingStore;

class QPlatformNativeInterface_Fake : public QObject
{
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **);
private:
    struct QPrivateSignal {};
public:
    virtual void *nativeResourceForIntegration(const QByteArray &resource);
    virtual void *nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context);
    virtual void *nativeResourceForScreen(const QByteArray &resource, QScreen *screen);
    virtual void *nativeResourceForWindow(const QByteArray &resource, QWindow *window);
    virtual void *nativeResourceForBackingStore(const QByteArray &resource, QBackingStore *backingStore);
#ifndef QT_NO_CURSOR
    virtual void *nativeResourceForCursor(const QByteArray &resource, const QCursor &cursor);
#endif
    typedef void * (*NativeResourceForIntegrationFunction)();
    typedef void * (*NativeResourceForContextFunction)(QOpenGLContext *context);
    typedef void * (*NativeResourceForScreenFunction)(QScreen *screen);
    typedef void * (*NativeResourceForWindowFunction)(QWindow *window);
    typedef void * (*NativeResourceForBackingStoreFunction)(QBackingStore *backingStore);
    virtual NativeResourceForIntegrationFunction nativeResourceFunctionForIntegration(const QByteArray &resource);
    virtual NativeResourceForContextFunction nativeResourceFunctionForContext(const QByteArray &resource);
    virtual NativeResourceForScreenFunction nativeResourceFunctionForScreen(const QByteArray &resource);
    virtual NativeResourceForWindowFunction nativeResourceFunctionForWindow(const QByteArray &resource);
    virtual NativeResourceForBackingStoreFunction nativeResourceFunctionForBackingStore(const QByteArray &resource);
    virtual QFunctionPointer platformFunction(const QByteArray &function) const;
    virtual QVariantMap windowProperties(QPlatformWindow *window) const;
    virtual QVariant windowProperty(QPlatformWindow *window, const QString &name) const;
    virtual QVariant windowProperty(QPlatformWindow *window, const QString &name, const QVariant &defaultValue) const;
    virtual void setWindowProperty(QPlatformWindow *window, const QString &name, const QVariant &value);
Q_SIGNALS:
    void windowPropertyChanged(QPlatformWindow *window, const QString &propertyName);
};

inline QPlatformNativeInterface_Fake::NativeResourceForIntegrationFunction resolvePlatformFunction_Fake(const QByteArray &functionName)
{
    QPlatformNativeInterface_Fake *nativeInterface = (QPlatformNativeInterface_Fake *)QGuiApplication::platformNativeInterface();
    QPlatformNativeInterface_Fake::NativeResourceForIntegrationFunction function =
        nativeInterface->nativeResourceFunctionForIntegration(functionName);
    if (Q_UNLIKELY(!function))
         qWarning("Qt could not resolve function %s from "
                  "QGuiApplication::platformNativeInterface()->nativeResourceFunctionForIntegration()",
                  functionName.constData());
    return function;
}

class CWizMacToolBarPrivate
{
public:
    NSToolbar *toolbar;
    CWizMacToolBarDelegate *delegate;
    QWidget *m_targetWindow;
};

WizMacToolBar::WizMacToolBar(QWidget *parent)
    : QWidget(parent)
{
    WizNSAutoReleasePool pool;

    d = new CWizMacToolBarPrivate();
    d->toolbar = [[NSToolbar alloc] initWithIdentifier:@"CWizMacToolBarr"];
    d->delegate = [[CWizMacToolBarDelegate alloc] initWithToolbar:d->toolbar qtToolBar:this];
    [d->toolbar setAllowsUserCustomization:NO];
    //  [d->Toolbar setAutosavesConfiguration:YES];
    [d->toolbar setDisplayMode:NSToolbarDisplayModeIconOnly];
    //[d->toolbar setSizeMode: NSToolbarSizeModeSmall];
    [d->toolbar setDelegate: d->delegate];

    setFocusPolicy(Qt::StrongFocus);
    d->m_targetWindow = 0;

//    enableBlendingBlurOnOSX10_10(this);
}

WizMacToolBar::~WizMacToolBar()
{
    WizNSAutoReleasePool pool;

    [d->toolbar release];
    delete d;
}

WizMacToolBar::DisplayMode WizMacToolBar::displayMode() const
{
    return WizMacToolBar::DisplayMode([d->toolbar displayMode]);
}

void WizMacToolBar::setDisplayMode(DisplayMode displayMode)
{
   [d->toolbar setDisplayMode : NSToolbarDisplayMode(displayMode)];
}

WizMacToolBar::SizeMode WizMacToolBar::sizeMode() const
{
    return WizMacToolBar::SizeMode([d->toolbar sizeMode]);
}

void WizMacToolBar::setSizeMode(SizeMode sizeMode)
{
//    [d->toolbar setDisplayMode : NSToolbarSizeMode(sizeMode)];
}

// show the Toolbar in the given window, delayed
void WizMacToolBar::showInWindow(QWidget *window)
{
    d->m_targetWindow = window;
    //
    QPlatformNativeInterface_Fake::NativeResourceForIntegrationFunction function = resolvePlatformFunction_Fake("setNSToolbar");
    if (function) {
        typedef void (*SetNSToolbarFunction)(QWindow *window, void *nsToolbar);
        reinterpret_cast<SetNSToolbarFunction>(function)(window->windowHandle(), d->toolbar);
    } else {
        QTimer::singleShot(100, this, SLOT(showInTargetWindow()));
    }
}

void WizMacToolBar::setToolBarVisible(bool bVisible)
{
    [d->toolbar setVisible: bVisible];
}

// internal invokable, show the Toolbar in m_targetWindow
void WizMacToolBar::showInTargetWindow()
{
    showInWindowImpl(d->m_targetWindow);
}

// internal implementation: show the Toolbar in window
void WizMacToolBar::showInWindowImpl(QWidget *window)
{
    WizNSAutoReleasePool pool;

    NSView *nsview = (NSView *)window->winId();
    NSWindow *macWindow = [nsview window];

    [macWindow setToolbar: nil];
    [macWindow setToolbar: d->toolbar];
    [d->toolbar setVisible: YES];
}

void WizMacToolBar::addAction(QAction* action)
{
    [d->delegate addAction:action];
}

void WizMacToolBar::addStandardItem(StandardItem standardItem)
{
    [d->delegate addStandardItem:standardItem];
}

void WizMacToolBar::addSearch(const QString& label, const QString& tooltip, int width)
{
    [d->delegate addSearch:label tooltip:tooltip width: width];
}

void WizMacToolBar::addWidget(WizCocoaViewContainer* widget, const QString& label, const QString& tooltip)
{
    [d->delegate addCustomView:widget label:label tooltip:tooltip];
}

void WizMacToolBar::deleteAllToolBarItems()
{
    [d->delegate deleteAllToolBarItem];
}

void WizMacToolBar::onSearchEndEditing(const QString& str)
{
    emit doSearch(str);
}

WizSearchView* WizMacToolBar::getSearchWidget()
{
    return [d->delegate getSearchWidget];
}

void WizMacToolBar::adjustSearchWidgetWidth(int nWidth)
{
    NSToolbarItem* toolbarItem = [d->delegate getSearchToolBarItem];
    NSSize maxSize = [toolbarItem maxSize];
    [toolbarItem setMaxSize: NSMakeSize(nWidth, maxSize.height)];
    NSView* nsView = [toolbarItem view];
    NSRect f = nsView.frame;
    f.size.width = nWidth;
    nsView.frame = f;
}

/*
void CWizMacToolBar::adjustWidgetToolBarItemWidth(QWidget* widget, int nWidth)
{
   NSToolbarItem* toolbarItem = [d->delegate getWidgetToolBarItemByWidget: widget];
   if (toolbarItem)
   {
       NSSize maxSize = [toolbarItem maxSize];
       [toolbarItem setMaxSize: NSMakeSize(nWidth, maxSize.height)];
       NSView* nsView = [toolbarItem view];
       NSRect f = nsView.frame;
       f.size.width = nWidth;
       nsView.frame = f;
   }
}
*/

WizMacFixedSpacer::WizMacFixedSpacer(QSize sz, QWidget* parent)
    : m_sz(sz)
{
}

void WizMacFixedSpacer::adjustWidth(int width)
{
     m_sz.setWidth(width);
}


@interface WizButtonItem: NSButton
{
    WizMacToolBarButtonItem* m_pButtonWidget;
}

- (void)setButtonWidget:(WizMacToolBarButtonItem*)buttonWidget;
- (void)buttonPressed;
@end

@implementation WizButtonItem
- (void)setButtonWidget:(WizMacToolBarButtonItem*)buttonWidget
{
    m_pButtonWidget = buttonWidget;
}

- (void)buttonPressed
{
    m_pButtonWidget->buttonClicked();
}
@end

@interface WizSegmentedControl: NSSegmentedControl
{
    WizMacToolBarButtonItem* m_pButtonWidget;
}

- (void)setButtonWidget:(WizMacToolBarButtonItem*)buttonWidget;
- (void)buttonPressed;
@end

@implementation WizSegmentedControl
- (void)setButtonWidget:(WizMacToolBarButtonItem*)buttonWidget
{
    m_pButtonWidget = buttonWidget;
}

- (void)buttonPressed
{
    NSInteger intger = [self selectedSegment];

    if (0 == intger)
    {
        m_pButtonWidget->buttonClicked();
    }
    else if (1 == intger)
    {
        m_pButtonWidget->extraMenuClicked();
    }
}
@end


WizMacToolBarButtonItem::WizMacToolBarButtonItem(const QString& title, const QPixmap& extraMenuIcon, int width, QWidget* parent)
    : m_width(width)
{
//    WizButtonItem *myButton = [[WizButtonItem alloc] initWithFrame:NSMakeRect(0, 0, sizeHint().width(), sizeHint().height())];
//    [myButton setTitle: WizToNSString(title)];
//    [myButton setImage: [NSImage imageNamed: NSImageNameAddTemplate]];
//    [myButton setImagePosition: NSImageLeft];
//    [myButton setButtonType:NSButtonType(buttonType)]; //Set what type button You want
//    [myButton setBezelStyle:NSBezelStyle(bezelStyle)]; //Set what style You want

//    [myButton setButtonWidget: this];
//    [myButton setTarget:myButton];
//    [myButton setAction:@selector(buttonPressed)];

//    setCocoaView(myButton);

//    [myButton release];

    WizSegmentedControl* button = [[WizSegmentedControl alloc] initWithFrame:NSMakeRect(0, 0, sizeHint().width(), sizeHint().height())];
    [[button cell] setTrackingMode:NSSegmentSwitchTrackingMomentary];
    [button setSegmentCount:2];

    [button setLabel:WizToNSString(title) forSegment:0];
    [button setImage:[NSImage imageNamed: NSImageNameAddTemplate] forSegment:0];
    [button setWidth:(sizeHint().width() - 24) forSegment:0];
    //
    NSSize imageSize;
    int pixScale = qApp->devicePixelRatio() >= 2 ? 2 : 1;
    imageSize.width = (CGFloat)extraMenuIcon.width() / pixScale;
    imageSize.height = (CGFloat)extraMenuIcon.height() / pixScale;
    NSImage* image = WizToNSImage(extraMenuIcon);
    [image setSize:imageSize];
    [button setImage:image forSegment:1];
    [button setLabel:@"" forSegment:1];
    [button setWidth:17 forSegment:1];

    //
    [button setButtonWidget: this];
    [button setTarget:button];
    [button setAction:@selector(buttonPressed)];

    setCocoaView(button);

    [button release];
}

QSize WizMacToolBarButtonItem::sizeHint() const
{
     return  QSize(m_width, TOOLBARITEMHEIGHT);
}

QRect WizMacToolBarButtonItem::geometry()
{
    WizSegmentedControl* button = (WizSegmentedControl *)cocoaView();
    //
    NSRect frame = button.bounds;
    //
    frame = [button convertRect:frame toView:nil];
    //
    NSWindow* window = button.window;
    NSRect windowFrame = window.frame;
    CGFloat y = windowFrame.size.height - frame.origin.y;
    //
    return QRect(frame.origin.x, (int)y, frame.size.width, frame.size.height);
}


void WizMacToolBarButtonItem::buttonClicked()
{
    emit triggered(true);
}

void WizMacToolBarButtonItem::extraMenuClicked()
{
    emit showExtraMenuRequest();
}

#endif
