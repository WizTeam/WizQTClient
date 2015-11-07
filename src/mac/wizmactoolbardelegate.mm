#ifdef USECOCOATOOLBAR

#include <QDebug>
#include <QPixmap>
#include <QMacCocoaViewContainer>

#include "wizmactoolbardelegate.h"
#include "wizmachelper_mm.h"
#include "wizmacactionhelper.h"
#include "wizSearchWidget_mm.h"




//@interface CWizToolBarActionItemView: NSView {
//NSImage* m_image;
//}
//- (void)setImage:(NSImage*)image;
//@end
//
//@implementation CWizToolBarActionItemView
//- (void)drawRect:(NSRect)dirtyRect
//{
//    CGRect rect = [self frame];
//    //[[NSColor windowBackgroundColor] set];
//    //[NSBezierPath fillRect:rect];
//    [m_image drawRepresentation:[m_image bestRepresentationForRect:rect context:nil hints:nil] inRect:rect];
//}
//- (void)setImage:(NSImage*)image
//{
//    m_image = image;
//}
//@end



CWizMacActionHelper::CWizMacActionHelper(CWizMacToolBarItem* item, QAction* action, QObject* parent)
    : QObject(parent)
    , m_item(item)
{
    connect(action, SIGNAL(changed()), SLOT(on_action_changed()));
}


void CWizMacActionHelper::on_action_changed()
{
    m_item->onActionChanged();
}



class CWizMacToolBarActionItem : public CWizMacToolBarItem
{
public:
    CWizMacToolBarActionItem(CWizMacToolBarDelegate* delegate, QAction* action)
        : m_helper(this, action, NULL)
        , m_delegate(delegate)
        , m_action(action)
        , m_id(WizGenGUID())
        , m_item(nil)
    {
        m_nsImages = [[NSMutableDictionary alloc] init];
        //connect(action, SIGNAL(changed()), SLOT(on_action_changed()));
    }
    virtual ~CWizMacToolBarActionItem()
    {
        [m_nsImages release];
    }

private:
    CWizMacActionHelper m_helper;
    CWizMacToolBarDelegate* m_delegate;
    QAction* m_action;
    NSString* m_id;
    NSToolbarItem* m_item;
    NSMutableDictionary* m_nsImages;
    //CWizToolBarActionItemView* m_view;

public:
    virtual NSString* itemIdentifier() const
    {
        return m_id;
    }
    virtual NSToolbarItem* toItem()
    {
        if (m_item != nil)
            return m_item;
        //
        NSString* itemId = itemIdentifier();
        //
        NSToolbarItem *item = [[[NSToolbarItem alloc] initWithItemIdentifier: itemId] autorelease];

        [item setLabel: WizToNSString(m_action->text())];
        [item setPaletteLabel:[item label]];
        [item setToolTip: WizToNSString(m_action->toolTip())];

        //m_view = [[CWizToolBarActionItemView alloc] init];
        //[m_view setAutoresizesSubviews: YES];
        //[m_view setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];
        //[item setView:m_view];

        // a reference to and not a copy of the pixmap data.
        QIcon icon = m_action->icon();
        if (!icon.isNull())
        {
            NSImage* image = WizToNSImage(icon, QSize(32, 32));
            //[m_view setImage:image];
            [item setImage:image];
        }

        [item setTarget : m_delegate];
        [item setAction : @selector(itemClicked:)];
        [item setEnabled: (m_action->isEnabled() ? YES : NO)];
        //
        [item setMinSize:NSMakeSize(16, 16)];
        [item setMaxSize:NSMakeSize(64, 64)];

        m_item = item;
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
            bool oldEnabled = [m_item isEnabled] ? true : false;
            bool newEnabled = m_action->isEnabled() ? true : false;
            if (oldEnabled != newEnabled)
            {
                [m_item setEnabled: (newEnabled ? YES : NO)];
            }
            //
            QIcon icon = m_action->icon();
            //qDebug() << icon.cacheKey();
            //
            NSImage* img = iconToNSImage(icon);
            if (img)
            {
                [m_item setImage:img];
                //[m_view setImage:img];
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
        NSImage* img = WizToNSImage(icon, QSize(32, 32));
        //
        [m_nsImages setObject:img forKey:key];
        //
        return img;
    }

private Q_SLOTS:
    void on_action_changed()
    {
        onActionChanged();
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

class CWizMacToolBarWidgetItem : public CWizMacToolBarItem
{
public:
    CWizMacToolBarWidgetItem(CWizMacToolBarDelegate* delegate, QMacCocoaViewContainer* widget, const QString& label, const QString& tooltip)
        : m_delegate(delegate)
        , m_id(WizGenGUID())
        , m_widget(widget)
        , m_strLabel(label)
        , m_strTooltip(tooltip)
    {
    }

    QMacCocoaViewContainer* widget() const { return m_widget; }

    virtual NSString* itemIdentifier() const { return m_id; }

    virtual NSToolbarItem* toItem()
    {
        NSString* itemId = itemIdentifier();
        NSToolbarItem* pItem = [[[NSToolbarItem alloc] initWithItemIdentifier: itemId] autorelease];
        NSString* labelString = WizToNSString(m_strLabel);
        NSString* tooltipString = WizToNSString(m_strTooltip);
        [pItem setLabel: labelString];
        [pItem setPaletteLabel: labelString];
        [pItem setToolTip: tooltipString];
#if QT_VERSION >= 0x050200
        NSView *nsview = m_widget->cocoaView();
#else
        NSView* nsview = (NSView *)m_widget->cocoaView();
#endif
        [pItem setView: nsview];
        [pItem setMinSize:NSMakeSize(m_widget->sizeHint().width(), m_widget->sizeHint().height())];
        [pItem setMaxSize:NSMakeSize(m_widget->sizeHint().width(), m_widget->sizeHint().height())];

        return pItem;
    }

private:
    CWizMacToolBarDelegate* m_delegate;
    NSString* m_id;
    QMacCocoaViewContainer* m_widget;
    QString m_strLabel;
    QString m_strTooltip;
};

class CWizMacToolBarSearchItem : public CWizMacToolBarItem
{
public:
    CWizMacToolBarSearchItem(CWizMacToolBarDelegate* delegate, const QString& label, const QString& tooltip, int width)
        : m_delegate(delegate)
        , m_id(WizGenGUID())
        , m_searchField(new CWizSearchWidget())
        , m_strLabel(label)
        , m_strTooltip(tooltip)
        , m_width(width)
    {
    }
private:
    CWizMacToolBarDelegate* m_delegate;
    NSString* m_id;
    CWizSearchWidget* m_searchField;
    QString m_strLabel;
    QString m_strTooltip;
    int m_width;
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

        // Use a custom view, a text field, for the search item
#if QT_VERSION >= 0x050200
        NSView* nsview = m_searchField->cocoaView();
#else
        NSView* nsview = (NSView *)m_searchField->cocoaView();
#endif
        [toolbarItem setView: nsview];
        [toolbarItem setMinSize:NSMakeSize(m_width, NSHeight([nsview frame]))];
        [toolbarItem setMaxSize:NSMakeSize(m_width, NSHeight([nsview frame]))];

        m_searchField->setSizeHint(QSize(m_width, m_searchField->sizeHint().height()));

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

//-(BOOL)validateToolbarItem:(NSToolbarItem *)toolbarItem
//{
//    return [toolbarItem isEnabled];
//}
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

//    [[self window] makeFirstResponder:[self window]];
//    [[NSApp mainWindow] makeFirstResponder:[NSApp mainWindow]];
    return;
}

- (BOOL) control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
//- (BOOL) textShouldEndEditing:(NSText *) textObject;
{
    //Q_UNUSED(control);
    //
    if (!m_toolbar)
        return YES;

//    [[NSApp mainWindow] makeFirstResponder:nil];
//    NSWindow* window1 = [[NSApp windows] objectAtIndex:1];
//    [window1 makeFirstResponder:window1];


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
    CWizMacToolBarItem* barItem = [self itemFromItemIdentifier: itemId];
    if (!barItem)
        return;
    //
    barItem->trigger();
}

- (void) viewSizeChanged : (NSNotification*)notification
{
    Q_UNUSED(notification);
    // Noop for now.
}

- (NSToolbarItem*) getSearchToolBarItem
{
    foreach (CWizMacToolBarItem* item, *items)
    {
        if (CWizMacToolBarSearchItem* search = dynamic_cast<CWizMacToolBarSearchItem*> (item))
        {
            NSString* nsId = search->itemIdentifier();
            return [self itemIdentifierToItem: nsId];
        }
    }
    return NULL;
}

- (CWizSearchWidget*) getSearchWidget
{
    foreach (CWizMacToolBarItem* item, *items)
    {
        if (CWizMacToolBarSearchItem* search = dynamic_cast<CWizMacToolBarSearchItem*> (item))
        {
            return search->widget();
        }
    }
    //
    return NULL;
}

- (NSToolbarItem*) getWidgetToolBarItemByWidget:(QWidget*) widget
{
    foreach (CWizMacToolBarItem* item, *items)
    {
        if (CWizMacToolBarWidgetItem* widgetItem = dynamic_cast<CWizMacToolBarWidgetItem*> (item))
        {
            if (widgetItem->widget() == widget)
            {
                NSString* nsId = widgetItem->itemIdentifier();
                return [self itemIdentifierToItem: nsId];
            }
        }
    }
    return NULL;
}

- (void)addAction:(QAction *)action
{
    items->append(new CWizMacToolBarActionItem(self, action));
}


- (void)addStandardItem:(CWizMacToolBar::StandardItem) standardItem
{
    items->append(new CWizMacToolBarStandardItem(standardItem));
}

- (void)addSearch:(const QString&)label tooltip:(const QString&)tooltip width:(int)width
{
    CWizMacToolBarSearchItem* pItem = new CWizMacToolBarSearchItem(self, label, tooltip, width);
    items->append(pItem);
}

- (void)addWidget:(QMacCocoaViewContainer *)widget label:(const QString&)label tooltip:(const QString&)tooltip
{
    items->append(new CWizMacToolBarWidgetItem(self, widget, label, tooltip));
}

- (void)deleteAllToolBarItem
{
//    foreach (CWizMacToolBarItem* item, *items)
//    {
//        item->deleteLater();
//    }
//    items->clear();
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


@end

#endif
