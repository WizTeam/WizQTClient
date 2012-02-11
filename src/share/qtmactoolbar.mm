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

#include "qtmactoolbar.h"
#include "qtmactoolbardelegate.h"
#include <QtGui/QApplication>
#include <QtDeclarative/QDeclarativeView>
#include <QtCore/QtCore>
#include <QAction>
#include "cocoahelp_mac.h"
#import <AppKit/AppKit.h>

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
    m_iconPixmap = action->icon().pixmap(16, 16);
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

class QtMacToolBarPrivate
{
public:
    NSToolbar *toolbar;
    QtMacToolbarDelegate *delegate;
    QWidget *m_targetWindow;
};

QtMacToolBar::QtMacToolBar(QObject *parent)
    : QObject(parent)
{
    //qDebug() << "QtMacToolBar()";

    QNSAutoReleasePool pool;
    //
    Q_UNUSED(pool);
    //
    d = new QtMacToolBarPrivate();
    d->toolbar = [[NSToolbar alloc] initWithIdentifier:@"QtMacToolBarr"];
    d->delegate = [[QtMacToolbarDelegate alloc] init];
    [d->toolbar setAllowsUserCustomization:YES];
    //  [d->toolbar setAutosavesConfiguration:YES];
    [d->toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
    //[d->toolbar setSizeMode: NSToolbarSizeModeSmall];
    [d->toolbar setDelegate: d->delegate];

    d->m_targetWindow = 0;
}

QtMacToolBar::~QtMacToolBar()
{
    QNSAutoReleasePool pool;
    [d->toolbar release];
    delete d;
}

void QtMacToolBar::classBegin()
{

}

void QtMacToolBar::componentComplete()
{
//    qDebug() << "item count" << QList<QObject *>(m_buttons).count();
    *(d->delegate->items) = QList<QObject *>(m_buttons);
    *(d->delegate->allowedItems) = QList<QObject *>(m_allowedButtons);
    showInMainWindow();
}

QDeclarativeListProperty<QObject> QtMacToolBar::buttons()
{
    return QDeclarativeListProperty<QObject>(this, m_buttons);
}

QDeclarativeListProperty<QObject> QtMacToolBar::allowedButtons()
{
    return QDeclarativeListProperty<QObject>(this, m_allowedButtons);
}

QtMacToolBar::DisplayMode QtMacToolBar::displayMode() const
{
    return QtMacToolBar::DisplayMode([d->toolbar displayMode]);
}

void QtMacToolBar::setDisplayMode(DisplayMode displayMode)
{
   [d->toolbar setDisplayMode : NSToolbarDisplayMode(displayMode)];
}

QtMacToolBar::SizeMode QtMacToolBar::sizeMode() const
{
    return QtMacToolBar::SizeMode([d->toolbar sizeMode]);
}

void QtMacToolBar::setSizeMode(SizeMode sizeMode)
{
    [d->toolbar setDisplayMode : NSToolbarSizeMode(sizeMode)];
}

// show the toolbar in the main declarative window, delay/retry until found
void QtMacToolBar::showInMainWindow()
{

    QNSAutoReleasePool pool;

    // Heuristics for finding the main top-level window:
    QList<QWidget *> candidates = QApplication::topLevelWidgets();
    QWidget *topLevel = 0;

    // 1) Look for a widget with classname "QTopLevelWindow"
    foreach (QWidget *w, candidates) {
        if (qstrcmp(w->metaObject()->className(), "QTopLevelWindow") == 0) {
            topLevel = w;
            break;
        }
    }
    // 2) Look for a widget with classname "QDeclarativeViewer"
    if (!topLevel) {
        foreach (QWidget *w, candidates) {
            if (qstrcmp(w->metaObject()->className(), "QDeclarativeViewer") == 0) {
                topLevel = w;
                break;
            }
        }
    }

    if (topLevel) {
        topLevel->window()->winId();
        d->m_targetWindow = topLevel;
        showInTargetWindow();
        return;
    }

    // INVARIANT: No window found. So we wait:
    QTimer::singleShot(100, this, SLOT(showInMainWindow()));
}

// show the toolbar in the given window, delayed
void QtMacToolBar::showInWindow(QWidget *window)
{
    d->m_targetWindow = window;
    QTimer::singleShot(0, this, SLOT(showInTargetWindow()));
}

// internal invokable, show the toolbar in m_targetWindow
void QtMacToolBar::showInTargetWindow()
{
    showInWindowImpl(d->m_targetWindow);
}

// internal implementation: show the toolbar in window
void QtMacToolBar::showInWindowImpl(QWidget *window)
{
    QNSAutoReleasePool pool;
    NSWindow *macWindow = qt_mac_window_for(window);
//    qDebug() << "macWindow" << macWindow;


 //   qDebug() << "QtMacToolBar NSWidnow setToolbar";
//    [macWindow orderOut: macWindow];
    [macWindow setToolbar: d->toolbar];
    [d->toolbar setVisible: YES];
    //[macWindow orderFront: macWindow];
    //    [macWindow setShowsToolbarButton:YES];

//    d->delegate
}

void QtMacToolBar::addActionGroup(QActionGroup* actionGroup)
{
    [d->delegate addActionGroup:actionGroup];
}

void QtMacToolBar::addAction(QAction* action)
{
    [d->delegate addAction:action];
}



QAction *QtMacToolBar::addAction(const QString &text)
{
    return [d->delegate addActionWithText:&text];
}

QAction *QtMacToolBar::addAction(const QIcon &icon, const QString &text)
{
    return [d->delegate addActionWithText:&text icon:&icon];
}

QAction *QtMacToolBar::addStandardItem(MacToolButton::StandardItem standardItem)
{
    return [d->delegate addStandardItem:standardItem];
}

QAction *QtMacToolBar::addAllowedAction(const QString &text)
{
    return [d->delegate addAllowedActionWithText:&text];
}

QAction *QtMacToolBar::addAllowedAction(const QIcon &icon, const QString &text)
{
    return [d->delegate addAllowedActionWithText:&text icon:&icon];
}

QAction *QtMacToolBar::addAllowedStandardItem(MacToolButton::StandardItem standardItem)
{
    return [d->delegate addAllowedStandardItem:standardItem];
}


