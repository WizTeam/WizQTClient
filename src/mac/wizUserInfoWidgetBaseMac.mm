#include "wizUserInfoWidgetBaseMac_mm.h"
#include "wizmachelper_mm.h"

#ifdef USECOCOATOOLBAR
#include <QWidget>
#include <QMenu>
#include <QToolButton>
#include <QApplication>


#if QT_VERSION >= 0x050200
#import <Cocoa/Cocoa.h>
#endif

bool isHighPixel()
{
    return qApp->devicePixelRatio() >= 2;
}

@interface NSScreen (PointConversion)
+ (NSScreen *)currentScreenForMouseLocation;
- (NSPoint)convertPointToScreenCoordinates:(NSPoint)aPoint;
- (NSPoint)flipPoint:(NSPoint)aPoint;
@end

@implementation NSScreen (PointConversion)

+ (NSScreen *)currentScreenForMouseLocation
{
    NSPoint mouseLocation = [NSEvent mouseLocation];

    NSEnumerator *screenEnumerator = [[NSScreen screens] objectEnumerator];
    NSScreen *screen;
    while ((screen = [screenEnumerator nextObject]) && !NSMouseInRect(mouseLocation, screen.frame, NO))
    {
    }

    return screen;
}

- (NSPoint)convertPointToScreenCoordinates:(NSPoint)aPoint
{
    float normalizedX = fabs(fabs(self.frame.origin.x) - fabs(aPoint.x));
    float normalizedY = aPoint.y - self.frame.origin.y;

    return NSMakePoint(normalizedX, normalizedY);
}

- (NSPoint)flipPoint:(NSPoint)aPoint
{
    return NSMakePoint(aPoint.x, self.frame.size.height - aPoint.y);
}
@end


// WizSearchField
@interface WizUserInfoView: NSView
{
    CWizUserInfoWidgetBaseMac *m_widget;
    NSFont* m_font;
    BOOL m_mouseIn;
    NSTrackingRectTag m_oldTracking;
    NSPoint m_menuPos;
}
- (id)initWithWidget:(CWizUserInfoWidgetBaseMac*)object;
- (NSFont*) font;
@end

@implementation WizUserInfoView
- (id)initWithWidget:(CWizUserInfoWidgetBaseMac*)object;
{
    self = [super init];
    m_widget = object;
    m_font = [NSFont systemFontOfSize:12];
    m_mouseIn = NO;
    m_oldTracking = 0;
    return self;
}
- (NSFont*) font
{
    return m_font;
}

- (void)viewDidMoveToWindow
{
    if (m_oldTracking > 0)
    {
        [self removeTrackingRect:m_oldTracking];
    }
    m_oldTracking = [self addTrackingRect:self.frame owner:self userData:NULL assumeInside:NO];
}
- (NSPoint)convertToScreenFromLocalPoint:(NSPoint)point relativeToView:(NSView *)view
{
    NSScreen *currentScreen = [NSScreen currentScreenForMouseLocation];
    if(currentScreen)
    {
        NSPoint windowPoint = [view convertPoint:point toView:nil];
        NSPoint screenPoint = [[view window] convertBaseToScreen:windowPoint];
        NSPoint flippedScreenPoint = [currentScreen flipPoint:screenPoint];
        flippedScreenPoint.y += [currentScreen frame].origin.y;

        return flippedScreenPoint;
    }

    return NSZeroPoint;
}

    - (NSRect)drawImageAtPoint:(NSPoint)point image:(NSImage*)img
    {
        NSSize imageSize = [img size];
        CGRect imageRect;
        imageRect.origin = NSZeroPoint;
        imageRect.size = imageSize;

        NSRect destRect;
        destRect.origin = point;
        destRect.size = imageSize;

        if (isHighPixel())
        {
            destRect.size.width = imageSize.width / 2;
            destRect.size.height = imageSize.height / 2;
            point.y += destRect.size.height / 2;
            destRect.origin = point;
        }
        //
    //            [img drawAtPoint:(NSPoint)pt fromRect:(NSRect)imageRect operation:NSCompositeSourceOver fraction:(CGFloat)1];
        [img drawInRect:destRect fromRect:(NSRect)imageRect operation:NSCompositeSourceOver fraction:(CGFloat)1];

        return destRect;
    }

- (void)drawRect:(NSRect)dirtyRect
{
    const int nAvatarWidth = 26;
    //
    CGRect rect = [self frame];
    //
    //[[NSColor whiteColor] set];
    //[NSBezierPath fillRect:rect];
    //
    CGRect avatarRect = rect;
    avatarRect.size.width = nAvatarWidth;
    avatarRect.size.height = nAvatarWidth;    
    //
    int pixScale = isHighPixel() ? 2 : 1;
    QPixmap pixmap = m_widget->getAvatar(avatarRect.size.width * pixScale, avatarRect.size.height * pixScale);
    if (!pixmap.isNull())
    {
        NSImage* img = ::WizToNSImage(pixmap);
        if (img)
        {
            NSSize imageSize = [img size];
            CGRect imageRect;
            imageRect.origin = NSZeroPoint;
            imageRect.size = imageSize;
            //
            [img drawInRect: avatarRect fromRect:imageRect operation:NSCompositeSourceOver fraction:1];
            [img release];
        }
    }
    //
    CGRect textRect = rect;
    textRect.origin.x = avatarRect.origin.x + avatarRect.size.width + 8;
    textRect.size.width = m_widget->textWidth();
    //
    QString text = m_widget->text();
    if (!text.isEmpty())
    {
        int fontHeight = m_widget->textHeight();
        //
        CGFloat yOffset = (textRect.size.height - fontHeight) / 2.0 + textRect.origin.y - 6;
        textRect = CGRectMake(textRect.origin.x, yOffset, textRect.size.width, fontHeight);
        //
        NSNumber* underLine = [[NSNumber alloc] initWithInteger: (m_mouseIn ? 1 :0 )];
        NSColor* textColor = [NSColor colorWithDeviceRed:0x78/255.0 green:0x78/255.0 blue:0x78/255.0 alpha:1.0];


        NSString* nsText = ::WizToNSString(text);
        //
        NSMutableParagraphStyle * paragraphStyle =
            [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
        [paragraphStyle setAlignment:NSLeftTextAlignment];
        NSDictionary * attributes = [NSDictionary dictionaryWithObjectsAndKeys:
                paragraphStyle, NSParagraphStyleAttributeName,
                m_font, NSFontAttributeName,
                underLine, NSUnderlineStyleAttributeName,
                textColor, NSForegroundColorAttributeName,
                nil];

        [nsText drawInRect:textRect withAttributes:attributes];
    }


    QIcon vipIcon = m_widget->getVipIcon();
    if (!vipIcon.isNull())
    {
        NSImage* img = ::WizToNSImage(vipIcon);
        if (img)
        {
            NSSize imageSize = [img size];
            int x = textRect.origin.x + textRect.size.width;
            int y = rect.origin.y + (rect.size.height - imageSize.height) / 2;
            NSPoint pt;
            pt.x = x;
            pt.y = y - 6;
            //
            NSRect destRect = [self drawImageAtPoint:pt image:img];

            [img release];
            textRect.origin.x = x + 4;
            textRect.size.width = destRect.size.width;
        }
    }

    //
    m_menuPos.x = textRect.origin.x - self.frame.origin.x;
    m_menuPos.y = textRect.origin.y - self.frame.origin.y;
#if QT_VERSION >= 0x050200
#else
    m_menuPos.y += textRect.size.height;
#endif
    //
    //
    QIcon iconArrow = m_widget->getArrow();
    if (!iconArrow.isNull())
    {
        NSImage* img = ::WizToNSImage(iconArrow);
        if (img)
        {
            NSSize imageSize = [img size];
            int x = textRect.origin.x + textRect.size.width;
            int y = rect.origin.y + (rect.size.height - imageSize.height) / 2;
            NSPoint pt;
            pt.x = x;
            pt.y = y - 6;
            //
            [self drawImageAtPoint:pt image:img];
            [img release];
        }
    }
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [super mouseUp:theEvent];        

    NSPoint event_location = theEvent.locationInWindow;
    NSPoint pos = [self convertPoint:event_location fromView:nil];
    NSRect rect = [self frame];
    if (!NSPointInRect(pos, rect))
        return;
   //
    NSPoint pt = [self convertPoint:m_menuPos toView:nil];
    //
#if QT_VERSION >= 0x050200
    NSMenu* menu = m_widget->getNSMewnu();
    //
    NSEvent* newEvent = [NSEvent mouseEventWithType:[theEvent type]
            location:pt
            modifierFlags:[theEvent modifierFlags]
            timestamp:[theEvent timestamp ]
            windowNumber:[theEvent windowNumber ]
            context:[theEvent context]
            eventNumber:[theEvent eventNumber ]
            clickCount:[theEvent clickCount ]
            pressure:[theEvent pressure]];
    //
    [NSMenu popUpContextMenu:menu withEvent:newEvent forView:self];
#else
    if (QMenu* menu = m_widget->getMenu())
    {
        int x = m_menuPos.x;
        int y = m_widget->height() - m_menuPos.y;
        QPoint ptScreen = m_widget->mapToGlobal(QPoint(x, y));
        menu->popup(ptScreen);
    }
#endif
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    [super mouseEntered:theEvent];
    //
    m_mouseIn = YES;
    [self setNeedsDisplay: YES];
}
- (void)mouseExited:(NSEvent *)theEvent
{
    [super mouseExited:theEvent];
    //
    m_mouseIn = NO;
    [self setNeedsDisplay: YES];
}
@end


CWizUserInfoWidgetBaseMac::CWizUserInfoWidgetBaseMac(QWidget* parent)
    : QMacCocoaViewContainer(nil, parent)
    , m_menuPopup(NULL)
    , m_textWidth(0)
    , m_textHeight(0)
{
    WizUserInfoView* view = [[WizUserInfoView alloc] initWithWidget:this];

    [view setAutoresizesSubviews: YES];
    [view setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];
    setCocoaView(view);
    [view release];
    //
    calTextSize();
}

void CWizUserInfoWidgetBaseMac::calTextSize()
{
    if (m_textWidth <= 0 || m_textHeight <= 0)
    {
        WizUserInfoView* view = (WizUserInfoView *)cocoaView();
        if (!view)
            return;
        //
        NSString* nsText = ::WizToNSString(text());
        //
        NSMutableParagraphStyle * paragraphStyle =
            [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
        [paragraphStyle setAlignment:NSCenterTextAlignment];
        NSDictionary * attributes = [NSDictionary dictionaryWithObjectsAndKeys:
                paragraphStyle, NSParagraphStyleAttributeName,
                [view font], NSFontAttributeName,
                nil];

        NSSize size = [nsText sizeWithAttributes:attributes];
        //
        m_textWidth = size.width + 4;
        m_textHeight = size.height;
    }
}

int CWizUserInfoWidgetBaseMac::textWidth() const
{
    return m_textWidth;
}
int CWizUserInfoWidgetBaseMac::textHeight() const
{
    return m_textHeight;
}

#if QT_VERSION >= 0x050200
NSMenu* CWizUserInfoWidgetBaseMac::getNSMewnu()
{
    if (!m_menuPopup)
        return nil;
    //
    return m_menuPopup->toNSMenu();
}
#endif

void CWizUserInfoWidgetBaseMac::updateUI()
{
    WizUserInfoView* view = (WizUserInfoView *)cocoaView();
    if (!view)
        return;
    //
    [view setNeedsDisplay: YES];
}
#endif
