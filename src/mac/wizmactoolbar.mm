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

#include <QtGui/QApplication>
#include <QtDeclarative/QDeclarativeView>
#include <QtCore/QtCore>
#include <QAction>
#include "wizmachelper.h"
#include "wizmactoolbar.h"
#include "wizmactoolbardelegate.h"

#import <AppKit/AppKit.h>

/*
NSString *toNSStandardItem(MacToolButton::StandardItem standardItem)
{
    if (standardItem == MacToolButton::Separator)
        return NSToolbarSeparatorItemIdentifier;
    else if (standardItem == MacToolButton::Space)
        return NSToolbarSpaceItemIdentifier;
    else if (standardItem == MacToolButton::FlexibleSpace)
        return NSToolbarFlexibleSpaceItemIdentifier;
    else if (standardItem == MacToolButton::ShowColors)
        return NSToolbarShowColorsItemIdentifier;
    else if (standardItem == MacToolButton::ShowFonts)
        return NSToolbarShowFontsItemIdentifier;
    else if (standardItem == MacToolButton::CustomizeToolbar)
        return NSToolbarCustomizeToolbarItemIdentifier;
    else if (standardItem == MacToolButton::PrintItem)
        return NSToolbarPrintItemIdentifier;
    return @"";
}

MacToolButton::MacToolButton()
{
   m_standardItem = NoItem;
   m_selectable = false;
   m_action = NULL;
   m_actionGroup = NULL;
}

MacToolButton::MacToolButton(QObject *parent)
    :QObject(parent)
{
    m_standardItem = NoItem;
    m_selectable = false;
    m_action = NULL;
    m_actionGroup = NULL;
}

MacToolButton::~MacToolButton()
{

}

QString MacToolButton::text() const
{
    return m_text;
}

void MacToolButton::setText(const QString &text)
{
    m_text = text;
}

QUrl MacToolButton::iconSource() const
{
    return m_iconSource;
}

void MacToolButton::setIconSource(const QUrl &iconSource)
{
    m_iconSource = iconSource;
}

bool MacToolButton::selectable() const
{
    return m_selectable;
}

void MacToolButton::setSelectable(bool selectable)
{
    m_selectable = selectable;
}

QString MacToolButton::toolTip() const
{
    return m_toolTip;
}

void MacToolButton::setToolTip(const QString &toolTip)
{
    m_toolTip = toolTip;
}

void MacToolButton::setAction(QAction *action)
{
    m_action = action;
    m_text = action->text();
    m_toolTip = action->toolTip();
    m_selectable = action->isCheckable();
}
void MacToolButton::setActionGroup(QActionGroup *actionGroup)
{
    m_actionGroup = actionGroup;
    m_text = "";
}

MacToolButton::StandardItem MacToolButton::standardItem() const
{
    return m_standardItem;
}

void MacToolButton::setStandardItem(StandardItem standardItem)
{
    m_standardItem = standardItem;
}

*/
class CWizMacToolBarPrivate
{
public:
    NSToolbar *toolbar;
    CWizMacToolBarDelegate *delegate;
    QWidget *m_targetWindow;
};

CWizMacToolBar::CWizMacToolBar(QObject *parent)
    : QObject(parent)
{
    //qDebug() << "CWizMacToolBar()";

    CWizNSAutoReleasePool pool;
    //
    d = new CWizMacToolBarPrivate();
    d->toolbar = [[NSToolbar alloc] initWithIdentifier:@"CWizMacToolBarr"];
    d->delegate = [[CWizMacToolBarDelegate alloc] initWithToolbar:d->toolbar qtToolBar:this];
    [d->toolbar setAllowsUserCustomization:YES];
    //  [d->Toolbar setAutosavesConfiguration:YES];
    [d->toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
    //[d->Toolbar setSizeMode: NSToolbarSizeModeSmall];
    [d->toolbar setDelegate: d->delegate];

    d->m_targetWindow = 0;
}

CWizMacToolBar::~CWizMacToolBar()
{
    CWizNSAutoReleasePool pool;
    //
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
    QTimer::singleShot(0, this, SLOT(showInTargetWindow()));
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
    NSWindow *macWindow = qt_mac_window_for(window);
//    qDebug() << "macWindow" << macWindow;


 //   qDebug() << "CWizMacToolBar NSWidnow setToolbar";
//    [macWindow orderOut: macWindow];
    [macWindow setToolbar: d->toolbar];
    [d->toolbar setVisible: YES];
    //[macWindow orderFront: macWindow];
    //    [macWindow setShowsToolbarButton:YES];

//    d->delegate
}

void CWizMacToolBar::addActionGroup(QActionGroup* actionGroup)
{
    [d->delegate addActionGroup:actionGroup];
}

void CWizMacToolBar::addAction(QAction* action)
{
    [d->delegate addAction:action];
}

void CWizMacToolBar::addStandardItem(StandardItem standardItem)
{
    [d->delegate addStandardItem:standardItem];
}
void CWizMacToolBar::addSearch()
{
    [d->delegate addSearch];
}


void CWizMacToolBar::onSearchEndEditing(const QString& str)
{
    emit doSearch(str);
}

