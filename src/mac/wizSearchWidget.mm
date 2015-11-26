#include "wizSearchWidget_mm.h"

#include <QWidget>
#include <QApplication>

#ifdef USECOCOATOOLBAR

#import <Cocoa/Cocoa.h>
#include "share/wizsettings.h"
#include "wizmachelper_mm.h"
#include "wizmactoolbar.h"


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
    //QString text = WizToQString([sender stringValue]);
    //m_pTarget->on_search_editFinished(text);
}
@end

// WizSearchField
@interface WizSearchField: NSSearchField <NSSearchFieldDelegate>
{
    CWizSearchWidget* m_pSearchWidget;
    BOOL m_isEditing;
}

- (void)setSearchWidget:(CWizSearchWidget*)widget;
- (BOOL)performKeyEquivalent:(NSEvent *)theEvent;
- (BOOL)becomeFirstResponder;
- (void)textDidBeginEditing:(NSNotification *)aNotification;
- (void)textDidEndEditing:(NSNotification *)aNotification;
- (void)textDidChange:(NSNotification *)aNotification;
- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector;
- (void)enterKeyPressed;
- (void)hideSearchCompleter;
- (BOOL)isEditing;
- (void)mouseDown:(NSEvent *)theEvent;
@end

@implementation WizSearchField
- (void)setSearchWidget:(CWizSearchWidget*)widget
{
    m_pSearchWidget = widget;
}

- (BOOL)isEditing
{
     return self->m_isEditing;
}

- (void)enterKeyPressed
{
    if (m_pSearchWidget->isCompleterVisible())
    {
        QString strText = m_pSearchWidget->getCurrentCompleterText();
        m_pSearchWidget->hideCompleter();
        if (strText.isEmpty())
            return;

        NSString* nsText = WizToNSString(strText);
        [self setStringValue: nsText];
    }
}

- (void)hideSearchCompleter
{
    if (m_pSearchWidget->isCompleterVisible())
    {
        m_pSearchWidget->hideCompleter();
    }
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector
{    
    if (commandSelector == @selector(insertNewline:))
    {
        [self enterKeyPressed];
        return NO;
    }
    else if (commandSelector == @selector(cancelOperation:))
    {
        // just hide suggestion list and pass command to system
        [self hideSearchCompleter];
        return NO;
    }
    return NO;
}

-(BOOL)performKeyEquivalent:(NSEvent *)theEvent
{
    if (([theEvent type] == NSKeyDown))
    {
        NSResponder * responder = [[self window] firstResponder];

        if ((responder != nil) && [responder isKindOfClass:[NSTextView class]])
        {
            NSTextView * textView = (NSTextView *)responder;
            NSRange range = [textView selectedRange];
            bool bHasSelectedTexts = (range.length > 0);

            bool bHandled = false;
            unsigned short keyCode = [theEvent keyCode];
            if (([theEvent modifierFlags] & NSCommandKeyMask))
            {
                //0 A, 6 Z, 7 X, 8 C, 9 V
                if (keyCode == 0)
                {
                    [textView selectAll:self];
                    bHandled = true;
                }
                else if (keyCode == 6)
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

            }
            else if (m_pSearchWidget->isCompleterVisible())
            {
                // up 126   down 125
                if (keyCode == 126)
                {
                    m_pSearchWidget->moveCompleter(true);
                    bHandled = true;
                }
                else if (keyCode == 125)
                {
                    m_pSearchWidget->moveCompleter(false);
                    bHandled = true;
                }
            }
            if (bHandled)
                return YES;
        }
    }


    return NO;
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [super mouseDown:theEvent];
    m_pSearchWidget->setCompleterUsable(true);
    m_pSearchWidget->showCompleter();
}

- (BOOL)becomeFirstResponder
{       

    // FIXME: qt can't switch focus between native and alien widgets, manually do it.
    QWidget* widget = QApplication::focusWidget();
        // CWizMacToolBar
    if (widget) {
        widget->clearFocus();
    }

    [self selectText:self];
    [[self currentEditor] setSelectedRange:NSMakeRange([[self stringValue] length], 0)];

    //
    self->m_isEditing = YES;    

    m_pSearchWidget->on_search_textChanged("");

    return YES;
}

- (void)textDidBeginEditing:(NSNotification *)aNotification
{
     self->m_isEditing = YES;
     [super textDidBeginEditing:aNotification];
}

- (void)textDidEndEditing:(NSNotification *)aNotification
{
    NSDictionary* dict = [aNotification userInfo];
    NSUInteger textMove = [[dict objectForKey: @"NSTextMovement"] unsignedIntegerValue];
    if (textMove == NSReturnTextMovement) {
        m_pSearchWidget->on_search_editFinished(WizToQString([self stringValue]));
    }

    [self hideSearchCompleter];

    self->m_isEditing = NO;
    //
    [super textDidEndEditing:aNotification];
}
- (void)textDidChange:(NSNotification *)aNotification
{        
    //
    QString text = WizToQString([self stringValue]);
    m_pSearchWidget->on_search_textChanged(text);
}
@end

CWizSearchWidget::CWizSearchWidget(QWidget* parent /* = 0 */)
    : QMacCocoaViewContainer(0, parent)
    , m_sizeHint(QSize(WizIsHighPixel() ? HIGHPIXSEARCHWIDGETWIDTH : NORMALSEARCHWIDGETWIDTH, TOOLBARITEMHEIGHT))
{
    // Many Cocoa objects create temporary autorelease objects,
    // so create a pool to catch them.
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    // Create the NSSearchField, set it on the QCocoaViewContainer.
    WizSearchField* pSearchField = [[WizSearchField alloc] init];
    [pSearchField setSearchWidget: this];
    [pSearchField setAutoresizesSubviews: YES];
    [pSearchField setAutoresizingMask: NSViewMinYMargin | NSViewWidthSizable];
    [pSearchField setDelegate: pSearchField];
    [pSearchField.window makeFirstResponder:nil];
//    NSRect f = pSearchField.frame;
//    pSearchField.frame = f;
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


    m_completer = new WizSuggestCompletionon(this);
}

void CWizSearchWidget::clear()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    QString strText = WizToQString([pSearchField stringValue]);
    if (strText.isEmpty()) {
        return;
    }

    [pSearchField setStringValue:@""];
}

void CWizSearchWidget::clearFocus()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    [pSearchField.window makeFirstResponder:nil];
    QMacCocoaViewContainer::clearFocus();
}

void CWizSearchWidget::focus()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
//    [pSearchField.window makeFirstResponder:pSearchField];
    [pSearchField selectText:pSearchField];
}

void CWizSearchWidget::setText(const QString& text)
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    NSString* nstring = WizToNSString(text);
    [pSearchField setStringValue:nstring];
    focus();
}

QString CWizSearchWidget::currentText()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    NSString* nstring = [pSearchField stringValue];
    return WizToQString(nstring);
}

void CWizSearchWidget::setUserSettings(CWizUserSettings* settings)
{
    m_completer->setUserSettings(settings);
}

bool CWizSearchWidget::isEditing()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    return [pSearchField isEditing];
}

void CWizSearchWidget::setCompleterUsable(bool usable)
{
    m_completer->setUsable(usable);
}

bool CWizSearchWidget::isCompleterVisible()
{
    return m_completer->isVisible();
}

void CWizSearchWidget::showCompleter()
{
    m_completer->autoSuggest();
}

void CWizSearchWidget::hideCompleter()
{
    m_completer->hide();
}

void CWizSearchWidget::moveCompleter(bool up)
{
    m_completer->selectSuggestItem(up);
}

QString CWizSearchWidget::getCurrentCompleterText()
{
    return m_completer->getCurrentText();
}

void CWizSearchWidget::setPopupWgtOffset(int popupWgtWidth, const QSize& offset)
{
    if (m_completer)
    {
        m_completer->setPopupOffset(popupWgtWidth, offset);
    }
}

void CWizSearchWidget::setSizeHint(QSize sizeHint)
{
    m_sizeHint = sizeHint;
}

QSize CWizSearchWidget::sizeHint() const
{
    return m_sizeHint;
}

void CWizSearchWidget::processEvent(QEvent* ev)
{
    event(ev);
}


static QString oldSearchText = QString();

void CWizSearchWidget::on_search_editFinished(const QString& strText)
{
    emit doSearch(strText);
    oldSearchText = strText;
}

void CWizSearchWidget::on_search_textChanged(const QString& strText)
{
    if (!oldSearchText.isEmpty() && strText.isEmpty())
    {
        emit doSearch("");
        oldSearchText = "";
    }

    emit textEdited(strText);
}

void CWizSearchWidget::setFocus()
{
    focus();
}

void CWizSearchWidget::clearSearchFocus()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    [pSearchField.window makeFirstResponder:nil];
    QMacCocoaViewContainer::clearFocus();
}

void CWizSearchWidget::on_advanced_buttonClicked()
{
    clearSearchFocus();

    emit advancedSearchRequest();
    m_completer->hide();
}

#endif
