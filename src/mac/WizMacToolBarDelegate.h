#ifndef WIZMACTOOLBARDELEGATE_H
#define WIZMACTOOLBARDELEGATE_H

#ifdef USECOCOATOOLBAR

#include <QtGlobal>

#import <AppKit/AppKit.h>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QAction>

#include "WizMacToolBar.h"

class WizMacToolBarItem;



#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_6
@protocol NSToolbarDelegate @end
#endif

@interface CWizMacToolBarDelegate : NSObject <NSToolbarDelegate, NSTextFieldDelegate>
{
@public
    NSToolbar *m_toolbar;
    WizMacToolBar* m_qtToolBar;

    QList<WizMacToolBarItem *> *items;
}

- (id)initWithToolbar:(NSToolbar*)tb qtToolBar:(WizMacToolBar*)qtToolBar;
//
- (NSToolbarItem *) toolbar: (NSToolbar *)toolbar itemForItemIdentifier: (NSString *) itemIdent willBeInsertedIntoToolbar:(BOOL) willBeInserted;
- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)tb;
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)Toolbar;
- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)Toolbar;

//- (BOOL)validateToolbarItem:(NSToolbarItem *)theItem;

- (BOOL) control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor;
- (void) searchUsingToolbarSearchField:(id) sender;

- (void)addAction:(QAction *)action;
- (void)addStandardItem:(WizMacToolBar::StandardItem)standardItem;
- (void)addSearch:(const QString&)label tooltip:(const QString&)tooltip width:(int)width;
- (void)addCustomView:(WizCocoaViewContainer *)container label:(const QString&)label tooltip:(const QString&)tooltip;

- (void)deleteAllToolBarItem;

- (WizMacToolBarItem*) itemFromItemIdentifier: (NSString*)itemIdentifier;
- (NSToolbarItem*) itemIdentifierToItem: (NSString*)itemIdentifier;

- (void) viewSizeChanged:(NSNotification*)notification;
- (IBAction)itemClicked:(id)sender;

- (WizSearchView*) getSearchWidget;
- (NSToolbarItem*) getSearchToolBarItem;
@end




class WizMacToolBarItem : public QObject
{
public:
    virtual NSString* itemIdentifier() const = 0;
    virtual NSToolbarItem* toItem() = 0;
    virtual void trigger() { }
    virtual void onActionChanged()  { }

    virtual bool isGroup() const { return false; }
    virtual int childCount() const { return 0; }
    virtual WizMacToolBarItem* childItem(int index) const { Q_UNUSED(index); return NULL; }
    virtual int indexOf(WizMacToolBarItem* item) const { Q_UNUSED(item); return -1; }
    virtual WizMacToolBarItem* childItemFromItemIdentifier(NSString* itemIdentifier)  { Q_UNUSED(itemIdentifier); return NULL; }
    virtual void childItemTriggerred(WizMacToolBarItem* itemChild) { Q_UNUSED(itemChild); }
    virtual void setChildItemEnabled(WizMacToolBarItem* itemChild, bool enabled) { Q_UNUSED(itemChild); Q_UNUSED(enabled); }
};




#endif  //Q_OS_MAC

#endif // WIZMACTOOLBARDELEGATE_H
