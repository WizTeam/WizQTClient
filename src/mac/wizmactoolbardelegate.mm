#include <QDebug>
#include <QPixmap>

#include "wizmactoolbardelegate.h"
#include "wizmachelper_mm.h"
#include "wizmacactionhelper.h"
#include "wizSearchWidget_mm.h"


void CWizMacActionHelper::on_action_changed()
{
    m_item->onActionChanged();
}


class CWizMacToolBarActionItem : public CWizMacToolBarItem
{
public:
    CWizMacToolBarActionItem(CWizMacToolBarDelegate* delegate, QAction* action)
        : m_delegate(delegate)
        , m_action(action)
        , m_id(WizGenGUID())
        , m_item(nil)
    {
        m_nsImages = [[NSMutableDictionary alloc] init];
        //
        new CWizMacActionHelper(this, action, this);
    }
    virtual ~CWizMacToolBarActionItem()
    {
        [m_nsImages release];
    }

private:
    CWizMacToolBarDelegate* m_delegate;
    QAction* m_action;
    NSString* m_id;
    NSToolbarItem* m_item;
    NSMutableDictionary* m_nsImages;
public:
    virtual NSString* itemIdentifier() const
    {
        return m_id;
    }
    virtual NSToolbarItem* toItem()
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

        [item setTarget : m_delegate];
        [item setAction : @selector(itemClicked:)];
        [item setEnabled: (m_action->isEnabled() ? YES : NO)];

        m_item = item;
        //
        return item;
    }
    virtual void trigger()
    {
        m_action->trigger();
    }
    virtual void onActionChanged()
    {
        if (m_item)
        {
            [m_item setEnabled: (m_action->isEnabled() ? YES : NO)];
            //
            NSString* itemId = [m_item itemIdentifier];
            //qDebug() << WizToQString(itemId);
            CWizMacToolBarItem* item = [m_delegate findItemGroup: itemId];
            if (item)
            {
                CWizMacToolBarItem* itemChild = item->childItemFromItemIdentifier(itemId);
                if (itemChild)
                {
                    item->setChildItemEnabled(itemChild, m_action->isEnabled());
                }
            }
            //
            QIcon icon = m_action->icon();
            //qDebug() << icon.cacheKey();
            //
            NSImage* img = iconToNSImage(icon);
            if (img)
            {
                [m_item setImage:img];
            }
        }
    }
    NSImage* iconToNSImage(const QIcon& icon)
    {
        qint64 k = icon.cacheKey();
        NSString* key = [NSString stringWithFormat:@"%qi", k];
        //
        NSObject* obj = [m_nsImages objectForKey:key];
        if (obj)
        {
            if ([obj isKindOfClass: [NSImage class]])
            {
                return (NSImage *)obj;
            }
        }
        //
        NSImage* img = WizToNSImage(icon);
        //
        [m_nsImages setObject:img forKey:key];
        //
        return img;
    }
};


class CWizMacToolBarActionGroupItem : public CWizMacToolBarItem
{
public:
    CWizMacToolBarActionGroupItem(CWizMacToolBarDelegate* delegate, QActionGroup* actionGroup)
        : m_delegate(delegate)
        , m_actionGroup(actionGroup)
        , m_groupItem(nil)
        , m_id(WizGenGUID())
    {
    }
private:
    CWizMacToolBarDelegate* m_delegate;
    QActionGroup* m_actionGroup;
    NSToolbarItemGroup* m_groupItem;
    NSString* m_id;
    QList<CWizMacToolBarItem*> m_subItems;
public:
    virtual NSString* itemIdentifier() const
    {
        return m_id;
    }
    virtual NSToolbarItem* toItem()
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
            CWizMacToolBarActionItem* item = new CWizMacToolBarActionItem(m_delegate, action);
            NSToolbarItem *toolbarItem = item->toItem();
            //
            [groupItems addObject:toolbarItem];
            //
            [groupView setWidth:24.0 forSegment:i];
            //
            if (!action->icon().isNull())
            {
                NSImage* image = [toolbarItem image];
                [groupView setImage:image forSegment:i];
            }
            //
            [groupView setEnabled: (action->isEnabled() ? YES : NO) forSegment:i];

            //
            m_subItems.append(item);
        }
        //
        [group setSubitems:groupItems];
        //
        //
        int groupViewWidth = actionCount * 24 + 8;
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
    virtual CWizMacToolBarItem* childItemFromItemIdentifier(NSString* itemIdentifier)
    {
        foreach (CWizMacToolBarItem* item, m_subItems)
        {
            if ([item->itemIdentifier() isEqualToString:itemIdentifier])
                return item;
        }
        //
        return NULL;
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
    virtual void setChildItemEnabled(CWizMacToolBarItem* itemChild, bool enabled)
    {
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
                [control setEnabled: (enabled ? YES : NO) forSegment:index];
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
    virtual NSToolbarItem* toItem()
    {
        assert(false);
        return nil;
    }
};



class CWizMacToolBarSearchItem : public CWizMacToolBarItem
{
public:
    CWizMacToolBarSearchItem(CWizMacToolBarDelegate* delegate, const QString& label, const QString& tooltip)
        : m_delegate(delegate)
        , m_id(WizGenGUID())
        //, m_searchFieldOutlet(nil)
        , m_searchField(new CWizSearchWidget())
        , m_strLabel(label)
        , m_strTooltip(tooltip)

    {
    }
private:
    CWizMacToolBarDelegate* m_delegate;
    NSString* m_id;
    //NSSearchField* m_searchFieldOutlet;
    CWizSearchWidget* m_searchField;
    QString m_strLabel;
    QString m_strTooltip;
public:
    CWizSearchWidget* widget() const { return m_searchField; }

    virtual NSString* itemIdentifier() const
    {
        return m_id;
    }
    virtual NSToolbarItem* toItem()
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

        //m_searchField = new CWizSearchWidget();
        //m_searchFieldOutlet = [[NSSearchField alloc] initWithFrame:NSRectFromString(@"{0, 0}, {150, 26}")];
        //[m_searchFieldOutlet setDelegate: m_delegate];
        //[m_searchFieldOutlet setTarget: m_delegate];
        //[m_searchFieldOutlet setAction: @selector(searchUsingToolbarSearchField:)];

        // Use a custom view, a text field, for the search item
        [toolbarItem setView: m_searchField->cocoaView()];
        [toolbarItem setMinSize:NSMakeSize(30, NSHeight([m_searchField->cocoaView() frame]))];
        [toolbarItem setMaxSize:NSMakeSize(250,NSHeight([m_searchField->cocoaView() frame]))];

        return toolbarItem;
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

- (BOOL) resignFirstResponder
{
    return YES;
}

- (BOOL) becomeFirstResponder
{
    return NO;
}

- (void)controlTextDidEndEditing:(NSNotification *)aNotification
{
    NSWindow* window1 = [[NSApp windows] objectAtIndex:0];
    NSWindow* window2 = [[NSApp windows] objectAtIndex:1];
    NSWindow* window3 = [NSApp mainWindow];
    //NSWindow* window4 = [self window];

    //[[self window] makeFirstResponder:[self window]];
    //[[NSApp mainWindow] makeFirstResponder:[NSApp mainWindow]];
    return;
}

- (BOOL) control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
//- (BOOL) textShouldEndEditing:(NSText *) textObject;
{
    //Q_UNUSED(control);
    //
    if (!m_toolbar)
        return YES;

    //[[NSApp mainWindow] makeFirstResponder:nil];
    //NSWindow* window1 = [[NSApp windows] objectAtIndex:1];
    //[window1 makeFirstResponder:window1];


    //NSWindow* window1 = [[NSApp windows] objectAtIndex:0];
    //NSWindow* window2 = [[NSApp windows] objectAtIndex:1];
    //NSWindow* window3 = [NSApp mainWindow];
    //BOOL b1 = [window1 canBecomeKeyWindow];
    //BOOL b2 = [window2 canBecomeKeyWindow];

    //QString str = WizToQString([fieldEditor string]);

    //m_qtToolBar->onSearchEndEditing(str);
    //[[NSApp mainWindow] makeKeyWindow];

    //m_qtToolBar->clearFocus();

    return YES;
}

- (void) searchUsingToolbarSearchField:(id) sender
{
    NSToolbarItem *item = reinterpret_cast<NSToolbarItem *>(sender);
    NSString* searchString = [(NSTextField *)item stringValue];
    QString str = WizToQString(searchString);
    m_qtToolBar->onSearchEndEditing(str);
}

//- (void)mouseDown:(NSEvent *) event
//{
//    [[event window] makeFirstResponder: self];
//    [super mouseDown: event];
//}


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
    items->append(new CWizMacToolBarActionGroupItem(self, actionGroup));
}

- (void)addAction:(QAction *)action
{
    items->append(new CWizMacToolBarActionItem(self, action));
}


- (void)addStandardItem:(CWizMacToolBar::StandardItem) standardItem
{
    items->append(new CWizMacToolBarStandardItem(standardItem));
}

- (CWizSearchWidget*)addSearch:(const QString&)label tooltip:(const QString&)tooltip
{
    CWizMacToolBarSearchItem* pItem = new CWizMacToolBarSearchItem(self, label, tooltip);
    items->append(pItem);
    return pItem->widget();
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
    return item->toItem();
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
            CWizMacToolBarItem* subItem = item->childItemFromItemIdentifier(itemIdentifier);
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
        CWizMacToolBarItem* subItem = item->childItemFromItemIdentifier(itemIdentifier);
        if (subItem)
            return item;
    }
    //
    return NULL;
}


@end
