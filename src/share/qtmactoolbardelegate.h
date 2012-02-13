#ifndef QTMACTOOLBARDELEGATE_H
#define QTMACTOOLBARDELEGATE_H

#include <QtGlobal>

#ifdef Q_OS_MAC

#import <AppKit/AppKit.h>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtGui/QAction>

#include "qtmactoolbar.h"


class CWizMacToolBarItem
{
public:
    CWizMacToolBarItem();
public:
    virtual NSString* toIdentifier() = 0;
    virtual NSToolbarItem* toItem() = 0;
    virtual bool isGroup() = 0;
    virtual int childCount() = 0;
    virtual NSToolbarItem* childItem(int index) = 0;
    virtual int indexOf(NSToolbarItem* item) = 0;
};


#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_6
@protocol NSToolbarDelegate @end
#endif

@interface CWizMacToolbarDelegate : NSObject <NSToolbarDelegate>
{
@public
    NSToolbar *toolbar;

    QList<CWizMacToolBarItem *> *items;
}

- (id)initWithToolbar:(NSToolbar*)tb;
- (NSToolbarItem *) toolbar: (NSToolbar *)toolbar itemForItemIdentifier: (NSString *) itemIdent willBeInsertedIntoToolbar:(BOOL) willBeInserted;
- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)tb;
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar;
- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar;

- (void)addActionGroup:(QActionGroup *)actionGroup;
- (void)addAction:(QAction *)action;
- (CWizMacToolBarItem *)addStandardItem:(MacToolButton::StandardItem)standardItem;

- (NSToolbarItem*) itemIdentifierToItem: (NSString*)itemIdentifier;

- (void) viewSizeChanged:(NSNotification*)notification;
- (IBAction)itemClicked:(id)sender;
//- (NSToolbarItemGroup* )findItemGroup:(NSToolbarItem*)item itemIndex:(int*)itemIndex;
@end



#endif

#endif // QTMACTOOLBARDELEGATE_H
