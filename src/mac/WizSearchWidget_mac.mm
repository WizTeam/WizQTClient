#include "WizSearchWidget_mm.h"

#include <QWidget>
#include <QApplication>
#include <QTimer>

#ifdef USECOCOATOOLBAR

#import <Cocoa/Cocoa.h>
#include "share/WizSettings.h"
#include "WizMacHelper_mm.h"
#include "WizMacToolBar.h"

// WizSearchField

@interface WizSearchField: NSSearchField <NSTextFieldDelegate>
{
    WizSearchView* m_pSearchWidget;
    BOOL m_isEditing;
}

- (void)setSearchView:(WizSearchView*)widget;
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
- (void)setSearchView:(WizSearchView*)widget
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

    m_pSearchWidget->onFocused(true);
    m_pSearchWidget->on_search_textChanged("");

    return YES;
}

- (void)textDidBeginEditing:(NSNotification *)aNotification
{
     self->m_isEditing = YES;
     [super textDidBeginEditing:aNotification];
     m_pSearchWidget->on_search_textChanging();
}

- (void) changePlaceHolderString:(NSString*)text;
{
    [self setPlaceholderString:text];
    NSString* currentSearchStringValue = self.stringValue;
    self.stringValue = @"-";
    self.stringValue = currentSearchStringValue;
}

- (void)textDidEndEditing:(NSNotification *)aNotification
{
    NSDictionary* dict = [aNotification userInfo];
    //
    NSUInteger textMove = [[dict objectForKey: @"NSTextMovement"] unsignedIntegerValue];
    if (textMove == NSReturnTextMovement) {
        m_pSearchWidget->on_search_editFinished(WizToQString([self stringValue]));
    }

    [self hideSearchCompleter];

    self->m_isEditing = NO;
    //
    QString search = QObject::tr("Search");
    [self changePlaceHolderString:WizToNSString(search)];
    //[self setPlaceholderString:WizToNSString(search)];
    //
    QTimer::singleShot(300, [=]{
        if (!m_pSearchWidget->hasFocus()) {
            m_pSearchWidget->onFocused(false);
        }
    });
    //
    [super textDidEndEditing:aNotification];
    //
}
- (void)textDidChange:(NSNotification *)aNotification
{        
    //
    QString text = WizToQString([self stringValue]);
    m_pSearchWidget->on_search_textChanged(text);
}
@end


WizSearchView::WizSearchView()
    : m_sizeHint(QSize(WizIsHighPixel() ? HIGHPIXSEARCHWIDGETWIDTH : NORMALSEARCHWIDGETWIDTH, TOOLBARITEMHEIGHT))
{
    // Many Cocoa objects create temporary autorelease objects,
    // so create a pool to catch them.
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    // Create the NSSearchField, set it on the QCocoaViewContainer.
    WizSearchField* pSearchField = [[WizSearchField alloc] init];
    [pSearchField setAutoresizesSubviews: YES];
    [pSearchField setAutoresizingMask: NSViewMinYMargin | NSViewWidthSizable];
    NSTextField* field = pSearchField;
    [field setDelegate: pSearchField];
    //[pSearchField setDelegate: pSearchField];
    [pSearchField.window makeFirstResponder:nil];
    [pSearchField setSearchView:this];
    setCocoaView(pSearchField);
    //
    // Clean up our pool as we no longer need it.
    [pool release];


    m_completer = new WizSuggestCompletionon(this);
}

void WizSearchView::clear()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    QString strText = WizToQString([pSearchField stringValue]);
    if (strText.isEmpty()) {
        return;
    }

    [pSearchField setStringValue:@""];
}

void WizSearchView::clearFocus()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    [pSearchField.window makeFirstResponder:nil];
}

bool WizSearchView::hasFocus()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    //
    NSResponder *firstResponder = [[NSApp keyWindow] firstResponder];
    if (firstResponder
            && [firstResponder isKindOfClass:[NSText class]]
            && [(id)firstResponder delegate] == pSearchField)
    {
        return true;
    }
    return false;
}

void WizSearchView::focus()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    [pSearchField.window makeFirstResponder:pSearchField];
}

void WizSearchView::onFocused(bool focused)
{
    emit textFocused(focused);
}

void WizSearchView::setText(const QString& text)
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    NSString* nstring = WizToNSString(text);
    [pSearchField setStringValue:nstring];
    focus();
}

QString WizSearchView::currentText()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    NSString* nstring = [pSearchField stringValue];
    return WizToQString(nstring);
}

void WizSearchView::setCurrentKb(const QString &kbGuid)
{
    m_strCurrentKbGuid = kbGuid;
    //
    m_completer->setCurrentKb(kbGuid);
}

void WizSearchView::setSearchPlaceHolder(const QString& placeHolder)
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    [pSearchField changePlaceHolderString:WizToNSString(placeHolder)];
    //[pSearchField setPlaceholderString:WizToNSString(placeHolder)];
}

void WizSearchView::setUserSettings(WizUserSettings* settings)
{
    m_completer->setUserSettings(settings);
}

bool WizSearchView::isEditing()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    return [pSearchField isEditing];
}

void WizSearchView::setCompleterUsable(bool usable)
{
    m_completer->setUsable(usable);
}

bool WizSearchView::isCompleterVisible()
{
    return m_completer->isVisible();
}

void WizSearchView::showCompleter()
{
    m_completer->autoSuggest();
}

void WizSearchView::hideCompleter()
{
    m_completer->hide();
}

void WizSearchView::moveCompleter(bool up)
{
    m_completer->selectSuggestItem(up);
}

QString WizSearchView::getCurrentCompleterText()
{
    return m_completer->getCurrentText();
}

void WizSearchView::setSizeHint(QSize sizeHint)
{
    m_sizeHint = sizeHint;
}

QSize WizSearchView::sizeHint() const
{
    return m_sizeHint;
}

static QString oldSearchText = QString();

void WizSearchView::on_search_editFinished(const QString& strText)
{
    emit doSearch(strText);
    oldSearchText = strText;
    //
    emit textStopEditing();
}

void WizSearchView::on_search_textChanged(const QString& strText)
{
    if (!oldSearchText.isEmpty() && strText.isEmpty())
    {
        emit doSearch("");
        oldSearchText = "";
    }

    emit textEdited(strText);
}
void WizSearchView::on_search_textChanging()
{
    emit textStartEditing();
}

void WizSearchView::setFocus()
{
    focus();
}

void WizSearchView::clearSearchFocus()
{
    WizSearchField* pSearchField = reinterpret_cast<WizSearchField *>(cocoaView());
    [pSearchField.window makeFirstResponder:nil];
}

QRect WizSearchView::globalRect()
{
    NSView* view = cocoaView();
    if (!view) {
        return QRect();
    }
    //
    NSRect rc = view.bounds;
    rc = [view convertRect:rc toView:nil];
    //
    NSWindow* window = view.window;
    rc = [window convertRectToScreen:rc];
    //
    NSScreen* screen = window.screen;
    NSRect rcScreen = screen.frame;

    QRect ret(int(rc.origin.x), int(rc.origin.y), int(rc.size.width), int(rc.size.height));
    //
    ret.moveTop(int(rcScreen.size.height) - ret.y() - ret.height());
    //
    return ret;
}



#endif
