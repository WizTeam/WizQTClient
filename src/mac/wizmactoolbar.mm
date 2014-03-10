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

#include <QApplication>
#include <QtCore>
#include <QAction>
#include "wizmachelper.h"
#include "wizmactoolbar.h"
#include "wizmactoolbardelegate.h"

#import <AppKit/AppKit.h>

class CWizNSAutoReleasePool
{
public:
    CWizNSAutoReleasePool()
    {
        pool = [[NSAutoreleasePool alloc] init];
    }

    ~CWizNSAutoReleasePool()
    {
       [pool release];
    }

private:
    NSAutoreleasePool* pool;
};

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
    [d->toolbar setDisplayMode : NSToolbarSizeMode(sizeMode)];
}

// show the Toolbar in the given window, delayed
void CWizMacToolBar::showInWindow(QWidget *window)
{
    d->m_targetWindow = window;
    QTimer::singleShot(100, this, SLOT(showInTargetWindow()));
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

void CWizMacToolBar::addSearch(const QString& label, const QString& tooltip)
{
    [d->delegate addSearch:label tooltip:tooltip];
}

void CWizMacToolBar::addWidget(QMacCocoaViewContainer* widget, const QString& label, const QString& tooltip)
{
    [d->delegate addWidget:widget label:label tooltip:tooltip];
}

void CWizMacToolBar::onSearchEndEditing(const QString& str)
{
    emit doSearch(str);
}

CWizSearchWidget* CWizMacToolBar::getSearchWidget()
{
    return [d->delegate getSearchWidget];
}
