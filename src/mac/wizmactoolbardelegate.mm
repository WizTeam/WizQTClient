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


#include <QDebug>
#include <QPixmap>
#include "wizmactoolbardelegate.h"
#include "wizmachelper.h"
#include "wizmacactionhelper.h"


void CWizMacActionHelper::on_action_changed()
{
    m_item->onActionChanged();
}


class CWizMacToolBarActionItem : public CWizMacToolBarItem
{
public:
    CWizMacToolBarActionItem(QAction* action)
        : m_action(action)
        , m_id(WizGenGUID())
        , m_item(nil)
    {
        new CWizMacActionHelper(this, action, this);
    }
private:
    QAction* m_action;
    NSString* m_id;
    NSToolbarItem* m_item;
public:
    virtual NSString* itemIdentifier() const
    {
        return m_id;
    }
    virtual NSToolbarItem* toItem(CWizMacToolBarDelegate* delegate)
    {
        NSString* itemId = itemIdentifier();
        //
        NSToolbarItem *item = [[[NSToolbarItem alloc] initWithItemIdentifier: itemId] autorelease];

        [item setLabel: WizToNSString(m_action->text())];
        [item setPaletteLabel:[item label]];
        [item setToolTip: WizToNSString(m_action->toolTip())];

        // a reference to and not a copy of the pixmap data.
        QIcon icon = m_action->icon();
        if (!icon.isNull())
        {
            NSImage* image = WizToNSImage(icon);
            [item setImage:image];
        }

        [item setTarget : delegate];
        [item setAction : @selector(itemClicked:)];

        m_item = item;
        //
        return item;
    }
    virtual bool isGroup() const
    {
        return false;
    }
    virtual int childCount() const
    {
        return 0;
    }
    virtual CWizMacToolBarItem* childItem(int index) const
    {
        Q_UNUSED(index);
        return nil;
    }
    virtual int indexOf(CWizMacToolBarItem* item) const
    {
        Q_UNUSED(item);
        return -1;
    }
    virtual CWizMacToolBarItem* itemFromItemIdentifier(NSString* itemIdentifier)
    {
        Q_UNUSED(itemIdentifier);
        return NULL;
    }
    virtual void trigger()
    {
        m_action->trigger();
    }
    virtual void childItemTriggerred(CWizMacToolBarItem* itemChild)
    {
        Q_UNUSED(itemChild);
    }
    virtual void onActionChanged()
    {
        if (m_item)
        {
            [m_item setEnabled: (m_action->isEnabled() ? YES : NO)];
        }
    }

};


class CWizMacToolBarActionGroupItem : public CWizMacToolBarItem
{
public:
    CWizMacToolBarActionGroupItem(QActionGroup* actionGroup)
        : m_actionGroup(actionGroup)
        , m_groupItem(nil)
        , m_id(WizGenGUID())
    {
    }
private:
    QActionGroup* m_actionGroup;
    NSToolbarItemGroup* m_groupItem;
    NSString* m_id;
    QList<CWizMacToolBarItem*> m_subItems;
public:
    virtual NSString* itemIdentifier() const
    {
        return m_id;
    }
    virtual NSToolbarItem* toItem(CWizMacToolBarDelegate* delegate)
    {
        m_subItems.clear();
        //
        NSString* itemId = itemIdentifier();
        //
        NSToolbarItemGroup *group = [[[NSToolbarItemGroup alloc] initWithItemIdentifier:itemId] autorelease];
        NSSegmentedControl* groupView = [[NSSegmentedControl alloc] init];
        [groupView setSegmentStyle:NSSegmentStyleTexturedRounded];

        NSMutableArray* groupItems = [NSMutableArray array];
        //
        QList<QAction*> actions = m_actionGroup->actions();

        int actionCount = actions.count();
        [groupView setSegmentCount:actionCount];
        //
        for (int i = 0; i < actionCount; i++)
        {
            QAction* action = actions.at(i);
            //
            CWizMacToolBarActionItem* item = new CWizMacToolBarActionItem(action);
            NSToolbarItem *toolbarItem = item->toItem(delegate);
            //
            [groupItems addObject:toolbarItem];
            //
            [groupView setWidth:40.0 forSegment:i];
            //
            if (!action->icon().isNull())
            {
                NSImage* image = [toolbarItem image];
                [groupView setImage:image forSegment:i];
            }
            //
            m_subItems.append(item);
        }
        //
        [group setSubitems:groupItems];
        //
        //
        int groupViewWidth = actionCount * 40 + 8;
        [group setMinSize:NSMakeSize(groupViewWidth, 26)];
        [group setMaxSize:NSMakeSize(groupViewWidth, 26)];
        //
        [group setView:groupView];
        //
        m_groupItem = group;
        //
        return group;
    }
    virtual bool isGroup() const
    {
        return true;
    }
    virtual int childCount() const
    {
        return m_actionGroup->actions().count();
    }
    virtual CWizMacToolBarItem* childItem(int index) const
    {
        if (!m_groupItem)
            return NULL;
        //
        if (index < 0)
            return NULL;
        if (index >= int(m_subItems.size()))
            return NULL;
        //
        return m_subItems.at(index);
    }
    virtual int indexOf(CWizMacToolBarItem* item) const
    {
        return m_subItems.indexOf(item);
    }
    virtual CWizMacToolBarItem* itemFromItemIdentifier(NSString* itemIdentifier)
    {
        foreach (CWizMacToolBarItem* item, m_subItems)
        {
            if ([item->itemIdentifier() isEqualToString:itemIdentifier])
                return item;
        }
        //
        return NULL;
    }
    virtual void trigger()
    {
    }
    virtual void childItemTriggerred(CWizMacToolBarItem* itemChild)
    {
        if (!m_groupItem)
            return;
        //
        int index = indexOf(itemChild);
        if (index == -1)
            return;
        //
        NSView* view = [m_groupItem view];
        if ([view isKindOfClass:[NSSegmentedControl class]])
        {
            NSSegmentedControl* control = (NSSegmentedControl *)view;
            //
            if (index < [control segmentCount])
            {
                [control setSelected:NO forSegment:index];
            }
            //
        }
    }
};


class CWizMacToolBarStandardItem : public CWizMacToolBarItem
{
public:
    CWizMacToolBarStandardItem(CWizMacToolBar::StandardItem item)
        : m_standardItem(item)
    {

    }
private:
    CWizMacToolBar::StandardItem m_standardItem;
public:
    virtual NSString* itemIdentifier() const
    {
        if (m_standardItem == CWizMacToolBar::Separator)
            return NSToolbarSeparatorItemIdentifier;
        else if (m_standardItem == CWizMacToolBar::Space)
            return NSToolbarSpaceItemIdentifier;
        else if (m_standardItem == CWizMacToolBar::FlexibleSpace)
            return NSToolbarFlexibleSpaceItemIdentifier;
        else if (m_standardItem == CWizMacToolBar::ShowColors)
            return NSToolbarShowColorsItemIdentifier;
        else if (m_standardItem == CWizMacToolBar::ShowFonts)
            return NSToolbarShowFontsItemIdentifier;
        else if (m_standardItem == CWizMacToolBar::CustomizeToolbar)
            return NSToolbarCustomizeToolbarItemIdentifier;
        else if (m_standardItem == CWizMacToolBar::PrintItem)
            return NSToolbarPrintItemIdentifier;
        //
        assert(false);
        //
        return @"";
    }
    virtual NSToolbarItem* toItem(CWizMacToolBarDelegate* delegate)
    {
        Q_UNUSED(delegate);
        //
        assert(false);
        return nil;
    }
    virtual bool isGroup() const
    {
        return false;
    }
    virtual int childCount() const
    {
        return 0;
    }
    virtual CWizMacToolBarItem* childItem(int index) const
    {
        Q_UNUSED(index);
        return nil;
    }
    virtual int indexOf(CWizMacToolBarItem* item) const
    {
        Q_UNUSED(item);
        return -1;
    }
    virtual CWizMacToolBarItem* itemFromItemIdentifier(NSString* itemIdentifier)
    {
        Q_UNUSED(itemIdentifier);
        return NULL;
    }
    virtual void trigger()
    {
    }
    virtual void childItemTriggerred(CWizMacToolBarItem* itemChild)
    {
        Q_UNUSED(itemChild);
    }
};



class CWizMacToolBarSearchItem : public CWizMacToolBarItem
{
public:
    CWizMacToolBarSearchItem(const QString& label, const QString& tooltip)
        : m_id(WizGenGUID())
        , m_searchFieldOutlet(nil)
        , m_strLabel(label)
        , m_strTooltip(tooltip)

    {
    }
private:
    NSString* m_id;
    NSSearchField* m_searchFieldOutlet;
    QString m_strLabel;
    QString m_strTooltip;
public:
    virtual NSString* itemIdentifier() const
    {
        return m_id;
    }
    virtual NSToolbarItem* toItem(CWizMacToolBarDelegate* delegate)
    {
        NSString* itemId = itemIdentifier();
        //
        NSToolbarItem* toolbarItem = [[[NSToolbarItem alloc] initWithItemIdentifier: itemId] autorelease];
        //
        NSString* labelString = WizToNSString(m_strLabel);
        NSString* tooltipString = WizToNSString(m_strTooltip);

        // Set up the standard properties
        [toolbarItem setLabel: labelString];
        [toolbarItem setPaletteLabel: labelString];
        [toolbarItem setToolTip: tooltipString];

        m_searchFieldOutlet = [[NSSearchField alloc] initWithFrame:NSRectFromString(@"{0, 0}, {150, 26}")];
        [m_searchFieldOutlet setDelegate: delegate];
        // Use a custom view, a text field, for the search item
        [toolbarItem setView: m_searchFieldOutlet];
        [toolbarItem setMinSize:NSMakeSize(30, NSHeight([m_searchFieldOutlet frame]))];
        [toolbarItem setMaxSize:NSMakeSize(250,NSHeight([m_searchFieldOutlet frame]))];
        //
        //
        return toolbarItem;
    }
    virtual bool isGroup() const
    {
        return false;
    }
    virtual int childCount() const
    {
        return 0;
    }
    virtual CWizMacToolBarItem* childItem(int index) const
    {
        Q_UNUSED(index);
        return nil;
    }
    virtual int indexOf(CWizMacToolBarItem* item) const
    {
        Q_UNUSED(item);
        return -1;
    }
    virtual CWizMacToolBarItem* itemFromItemIdentifier(NSString* itemIdentifier)
    {
        Q_UNUSED(itemIdentifier);
        return NULL;
    }
    virtual void trigger()
    {
    }
    virtual void childItemTriggerred(CWizMacToolBarItem* itemChild)
    {
        Q_UNUSED(itemChild);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

NSMutableArray *itemIdentifiers(const QList<CWizMacToolBarItem *> *items, bool cullUnselectable)
{
    NSMutableArray *array = [[[NSMutableArray alloc] init] autorelease];

    foreach (const CWizMacToolBarItem* item, *items)
    {
        if (!cullUnselectable)
        {
            [array addObject: item->itemIdentifier()];
        }
    }
    return array;
}


@implementation CWizMacToolBarDelegate

-(id)initWithToolbar:(NSToolbar*)tb qtToolBar:(CWizMacToolBar*)qtToolBar
{
    m_toolbar = tb;
    m_qtToolBar = qtToolBar;
    //
    self = [super init];
    if (self) {
        items = new QList<CWizMacToolBarItem *>();
    }
    return self;
}

- (void)dealloc
{
    delete items;
    [super dealloc];
}


- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)tb
{
    Q_UNUSED(tb);
    return itemIdentifiers(self->items, false);
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)tb
{
    Q_UNUSED(tb);
    return itemIdentifiers(self->items, false);
}

- (NSArray *)toolbarSelectableItemIdentifiers: (NSToolbar *)tb
{
    Q_UNUSED(tb);
    NSMutableArray *array = itemIdentifiers(self->items, true);
    return array;
}


- (NSToolbarItem *) toolbar: (NSToolbar *)tb itemForItemIdentifier: (NSString *) itemIdentifier willBeInsertedIntoToolbar:(BOOL) willBeInserted
{
    Q_UNUSED(tb);
    Q_UNUSED(willBeInserted);
    //
    return [self itemIdentifierToItem:itemIdentifier];
}

- (BOOL) control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
{
    Q_UNUSED(control);
    //
    if (!m_toolbar)
        return YES;
    //
    QString str = WizToQString([fieldEditor string]);
    //
    m_qtToolBar->onSearchEndEditing(str);
    //
    return YES;
}


- (IBAction)itemClicked:(id)sender
{
    NSToolbarItem *item = reinterpret_cast<NSToolbarItem *>(sender);
    //
    NSString* itemId = [item itemIdentifier];
    //
    CWizMacToolBarItem* barItem = [self itemFromItemIdentifierWithChildren: itemId];
    if (!barItem)
        return;
    //
    barItem->trigger();
    //
    CWizMacToolBarItem* itemGroup = [self findItemGroup: itemId];
    if (itemGroup)
    {
        itemGroup->childItemTriggerred(barItem);
    }
}

- (void) viewSizeChanged : (NSNotification*)notification
{
    Q_UNUSED(notification);
    // Noop for now.
}

- (void)addActionGroup:(QActionGroup *)actionGroup
{
    items->append(new CWizMacToolBarActionGroupItem(actionGroup));
}

- (void)addAction:(QAction *)action
{
    items->append(new CWizMacToolBarActionItem(action));
}


- (void)addStandardItem:(CWizMacToolBar::StandardItem) standardItem
{
    items->append(new CWizMacToolBarStandardItem(standardItem));
}

- (void)addSearch:(const QString&)label tooltip:(const QString&)tooltip
{
    items->append(new CWizMacToolBarSearchItem(label, tooltip));
}


- (CWizMacToolBarItem*) itemFromItemIdentifier: (NSString*)itemIdentifier
{
    foreach (CWizMacToolBarItem* item, *items)
    {
        if ([itemIdentifier isEqualToString:item->itemIdentifier()])
        {
            return item;
        }
    }
    //
    return NULL;
}

- (NSToolbarItem*) itemIdentifierToItem: (NSString*)itemIdentifier
{
    CWizMacToolBarItem* item = [self itemFromItemIdentifier: itemIdentifier];
    if (item == NULL)
        return nil;
    //
    return item->toItem(self);
}

- (CWizMacToolBarItem*) itemFromItemIdentifierWithChildren: (NSString*)itemIdentifier
{
    foreach (CWizMacToolBarItem* item, *items)
    {
        if ([itemIdentifier isEqualToString:item->itemIdentifier()])
        {
            return item;
        }
        //
        if (item->isGroup())
        {
            CWizMacToolBarItem* subItem = item->itemFromItemIdentifier(itemIdentifier);
            if (subItem)
                return subItem;
        }
    }
    //
    return NULL;
}

- (CWizMacToolBarItem*) findItemGroup: (NSString*)itemIdentifier
{
    foreach (CWizMacToolBarItem* item, *items)
    {
        if (!item->isGroup())
            continue;
        //
        CWizMacToolBarItem* subItem = item->itemFromItemIdentifier(itemIdentifier);
        if (subItem)
            return item;
    }
    //
    return NULL;
}


@end





