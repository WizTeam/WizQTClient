#include "../wizSearchWidget.h"

#ifdef Q_WS_MAC
#ifdef QT_MAC_USE_COCOA
#import <Cocoa/Cocoa.h>

#include "wizmachelper.h"

// NSSearchField delegate
@interface WizSearchTarget: NSObject
{
    CWizSearchWidget* m_pTarget;
}
- (id)initWithObject:(CWizSearchWidget*)object;
- (IBAction)onEditing:(id)sender;
@end

@implementation WizSearchTarget
- (id)initWithObject:(CWizSearchWidget*)object
{
    self = [super init];
    m_pTarget = object;
    return self;
}
- (IBAction)onEditing:(id)sender
{
    QString text = WizToQString([sender stringValue]);
    m_pTarget->on_search_textChanged(text);
}
@end

// WizSearchField
@interface WizSearchField: NSSearchField
- (BOOL)performKeyEquivalent:(NSEvent *)theEvent;
- (BOOL)becomeFirstResponder;
@end

@implementation WizSearchField
-(BOOL)performKeyEquivalent:(NSEvent *)theEvent
{
    if (([theEvent type] == NSKeyDown) && ([theEvent modifierFlags] & NSCommandKeyMask))
    {
        NSResponder * responder = [[self window] firstResponder];

        if ((responder != nil) && [responder isKindOfClass:[NSTextView class]])
        {
            NSTextView * textView = (NSTextView *)responder;
            NSRange range = [textView selectedRange];
            bool bHasSelectedTexts = (range.length > 0);

            unsigned short keyCode = [theEvent keyCode];

            bool bHandled = false;

            //6 Z, 7 X, 8 C, 9 V
            if (keyCode == 6)
            {
                // Lead crash!
//                if ([[textView undoManager] canUndo])
//                {
//                    [[textView undoManager] undo];
//                    bHandled = true;
//                }
            }
            else if (keyCode == 7 && bHasSelectedTexts)
            {
                [textView cut:self];
                bHandled = true;
            }
            else if (keyCode== 8 && bHasSelectedTexts)
            {
                [textView copy:self];
                bHandled = true;
            }
            else if (keyCode == 9)
            {
                [textView paste:self];
                bHandled = true;
            }

            if (bHandled)
                return YES;
        }
    }

    return NO;
}

- (BOOL)becomeFirstResponder
{
    // FIXME: qt can't switch focus between native and alien widgets, manually do it.
    QWidget* widget = QApplication::focusWidget();
    if (widget) {
        widget->clearFocus();
    }

    return YES;
}
@end

CWizSearchWidget::CWizSearchWidget(CWizExplorerApp& app, QWidget* parent /* = 0 */)
    : QMacCocoaViewContainer(0, parent)
{
    Q_UNUSED(app);

    // Many Cocoa objects create temporary autorelease objects,
    // so create a pool to catch them.
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    // Create the NSSearchField, set it on the QCocoaViewContainer.
    WizSearchField* pSearchField = [[WizSearchField alloc] init];
    setCocoaView(pSearchField);

    WizSearchTarget *bt = [[WizSearchTarget alloc] initWithObject:this];
    [pSearchField setTarget: bt];
    [pSearchField setAction: @selector(onEditing:)];

    //NSString* strPlaceHolder = WizToNSString(QObject::tr("Search documents"));
    //[[pSearchField cell] setPlaceholderString:strPlaceHolder];

    // Use a Qt menu for the search field menu.
    //QMenu *qtMenu = createMenu(this);
    //NSMenu *nsMenu = qtMenu->macMenu(0);
    //[[m_pSearchField cell] setSearchMenuTemplate:nsMenu];

    // Release our reference, since our super class takes ownership and we
    // don't need it anymore.
    [pSearchField release];

    // Clean up our pool as we no longer need it.
    [pool release];
}

void CWizSearchWidget::clear()
{
    NSSearchField* pSearchField = reinterpret_cast<NSSearchField* >(cocoaView());
    QString strText = WizToQString([pSearchField stringValue]);
    if (strText.isEmpty()) {
        return;
    }

    [pSearchField setStringValue:@""];
    [pSearchField.window makeFirstResponder:pSearchField];
    //[pSearchField becomeFirstResponder];
}

void CWizSearchWidget::focus()
{
    NSSearchField* pSearchField = reinterpret_cast<NSSearchField* >(cocoaView());
    [pSearchField.window makeFirstResponder:pSearchField];
}

QSize CWizSearchWidget::sizeHint() const
{
    return QSize(400, height());
}

void CWizSearchWidget::on_search_textChanged(const QString& strText)
{
    emit doSearch(strText);
}

#endif // QT_MAC_USE_COCOA
#endif // Q_WS_MAC
