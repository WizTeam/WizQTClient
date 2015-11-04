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
#include "wizmachelper.h"
#include "wizmactoolbar.h"
#include "wizmactoolbardelegate.h"
#include "wizmachelper_mm.h"
#include "wizSearchWidget_mm.h"

#import <AppKit/AppKit.h>

class CWizMacToolBarPrivate
{
public:
    NSToolbar *toolbar;
    CWizMacToolBarDelegate *delegate;
    QWidget *m_targetWindow;
};

CWizMacToolBar::CWizMacToolBar(QWidget *parent)
    : QWidget(parent)
{
    CWizNSAutoReleasePool pool;

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

CWizMacToolBar::~CWizMacToolBar()
{
    CWizNSAutoReleasePool pool;

    [d->toolbar release];
    delete d;
}

CWizMacToolBar::DisplayMode CWizMacToolBar::displayMode() const
{
    return CWizMacToolBar::DisplayMode([d->toolbar displayMode]);
}

void CWizMacToolBar::setDisplayMode(DisplayMode displayMode)
{
   [d->toolbar setDisplayMode : NSToolbarDisplayMode(displayMode)];
}

CWizMacToolBar::SizeMode CWizMacToolBar::sizeMode() const
{
    return CWizMacToolBar::SizeMode([d->toolbar sizeMode]);
}

void CWizMacToolBar::setSizeMode(SizeMode sizeMode)
{
//    [d->toolbar setDisplayMode : NSToolbarSizeMode(sizeMode)];
}

// show the Toolbar in the given window, delayed
void CWizMacToolBar::showInWindow(QWidget *window)
{
    d->m_targetWindow = window;
    QTimer::singleShot(100, this, SLOT(showInTargetWindow()));
}

void CWizMacToolBar::setToolBarVisible(bool bVisible)
{
    [d->toolbar setVisible: bVisible];
}

// internal invokable, show the Toolbar in m_targetWindow
void CWizMacToolBar::showInTargetWindow()
{
    showInWindowImpl(d->m_targetWindow);
}

// internal implementation: show the Toolbar in window
void CWizMacToolBar::showInWindowImpl(QWidget *window)
{
    CWizNSAutoReleasePool pool;

    NSView *nsview = (NSView *)window->winId();
    NSWindow *macWindow = [nsview window];

    [macWindow setToolbar: nil];
    [macWindow setToolbar: d->toolbar];
    [d->toolbar setVisible: YES];
}

void CWizMacToolBar::addAction(QAction* action)
{
    [d->delegate addAction:action];
}

void CWizMacToolBar::addStandardItem(StandardItem standardItem)
{
    [d->delegate addStandardItem:standardItem];
}

void CWizMacToolBar::addSearch(const QString& label, const QString& tooltip, int width)
{
    [d->delegate addSearch:label tooltip:tooltip width: width];
}

void CWizMacToolBar::addWidget(QMacCocoaViewContainer* widget, const QString& label, const QString& tooltip)
{
    [d->delegate addWidget:widget label:label tooltip:tooltip];
}

void CWizMacToolBar::deleteAllToolBarItems()
{
    [d->delegate deleteAllToolBarItem];
}

void CWizMacToolBar::onSearchEndEditing(const QString& str)
{
    emit doSearch(str);
}

CWizSearchWidget* CWizMacToolBar::getSearchWidget()
{
    return [d->delegate getSearchWidget];
}

void CWizMacToolBar::adjustSearchWidgetWidth(int nWidth)
{
    NSToolbarItem* toolbarItem = [d->delegate getSearchToolBarItem];
    NSSize maxSize = [toolbarItem maxSize];
    [toolbarItem setMaxSize: NSMakeSize(nWidth, maxSize.height)];
    NSView* nsView = [toolbarItem view];
    NSRect f = nsView.frame;
    f.size.width = nWidth;
    nsView.frame = f;
}

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

CWizMacFixedSpacer::CWizMacFixedSpacer(QSize sz, QWidget* parent)
    : QMacCocoaViewContainer(nil, parent)
    , m_sz(sz)
{
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
}

void CWizMacFixedSpacer::adjustWidth(int width)
{
     m_sz.setWidth(width);
     setFixedWidth(width);
//     setMinimumWidth(width);
}


@interface WizButtonItem: NSButton
{
    CWizMacToolBarButtonItem* m_pButtonWidget;
}

- (void)setButtonWidget:(CWizMacToolBarButtonItem*)buttonWidget;
- (void)buttonPressed;
@end

@implementation WizButtonItem
- (void)setButtonWidget:(CWizMacToolBarButtonItem*)buttonWidget
{
    m_pButtonWidget = buttonWidget;
}

- (void)buttonPressed
{
    m_pButtonWidget->buttonClicked();
}
@end


CWizMacToolBarButtonItem::CWizMacToolBarButtonItem(const QString& title,
                                                           int buttonType, int bezelStyle, int width, QWidget* parent)
    : QMacCocoaViewContainer(nil, parent)
    , m_width(width)
{
    WizButtonItem *myButton = [[WizButtonItem alloc] initWithFrame:NSMakeRect(0, 0, sizeHint().width(), sizeHint().height())];
    [myButton setTitle: WizToNSString(title)];
    [myButton setImage: [NSImage imageNamed: NSImageNameAddTemplate]];
    [myButton setImagePosition: NSImageLeft];
    [myButton setButtonType:NSButtonType(buttonType)]; //Set what type button You want
    [myButton setBezelStyle:NSBezelStyle(bezelStyle)]; //Set what style You want

    [myButton setButtonWidget: this];
    [myButton setTarget:myButton];
    [myButton setAction:@selector(buttonPressed)];

    setCocoaView(myButton);   

    [myButton release];
}

QSize CWizMacToolBarButtonItem::sizeHint() const
{
     return  QSize(m_width, TOOLBARITEMHEIGHT);
}

void CWizMacToolBarButtonItem::buttonClicked()
{
    emit triggered(true);
}

#endif
