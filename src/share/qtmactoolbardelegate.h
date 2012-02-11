#ifdef Q_OS_MAC

#import <AppKit/AppKit.h>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtGui/QAction>

#include "qtmactoolbar.h"


#ifndef QTMACTOOLBARDELEGATE_H
#define QTMACTOOLBARDELEGATE_H

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_6
@protocol NSToolbarDelegate @end
#endif

@interface QtMacToolbarDelegate : NSObject <NSToolbarDelegate>
{
@public
    NSToolbar *toolbar;

    QList<QObject *> *items;
    QList<QObject *> *allowedItems;
}

- (id)init;
- (NSToolbarItem *) toolbar: (NSToolbar *)toolbar itemForItemIdentifier: (NSString *) itemIdent willBeInsertedIntoToolbar:(BOOL) willBeInserted;
- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)tb;
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar;
- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar;

- (void)addActionGroup:(QActionGroup *)actionGroup;
- (void)addAction:(QAction *)action;
- (QAction *)addActionWithText:(const QString *)text;
- (QAction *)addActionWithText:(const QString *)text icon:(const QIcon *)icon;
- (QAction *)addStandardItem:(MacToolButton::StandardItem)standardItem;

- (QAction *)addAllowedActionWithText:(const QString *)text;
- (QAction *)addAllowedActionWithText:(const QString *)text icon:(const QIcon *)icon;
- (QAction *)addAllowedStandardItem:(MacToolButton::StandardItem)standardItem;

- (void) viewSizeChanged:(NSNotification*)notification;
- (IBAction)itemClicked:(id)sender;
- (NSToolbarItemGroup* )findItemGroup:(NSToolbarItem*)item itemIndex:(int*)itemIndex;
@end


#endif // QTMACTOOLBARDELEGATE_H

#endif
