#include "wizUserInfoWidgetBaseMac_mm.h"

#include <QWidget>

#import <Cocoa/Cocoa.h>


// WizSearchField
@interface WizUserInfoView: NSView
@end

@implementation WizUserInfoView
- (void)drawRect:(NSRect)dirtyRect
{
    CGRect rect = [self frame];
    // erase the background by drawing white
    [[NSColor whiteColor] set];
    [NSBezierPath fillRect:rect];

}
@end


CWizUserInfoWidgetBaseMac::CWizUserInfoWidgetBaseMac(QWidget* parent)
    : QMacCocoaViewContainer(nil, parent)
    , m_menuPopup(NULL)
{
    //
    WizUserInfoView* view = [[WizUserInfoView alloc] init];
    [view setAutoresizesSubviews: YES];
    [view setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];
    setCocoaView(view);
    [view release];
}
