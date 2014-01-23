#include "notify.h"
#import <Cocoa/Cocoa.h>

#include "../mac/wizmachelper_mm.h"

using namespace Utils;

void Notify::sendNotify(const QString& strTile, const QString& strText)
{
    NSUserNotification* userNotification = [[[NSUserNotification alloc] init] autorelease];
    userNotification.title = ::WizToNSString(strTile);
    userNotification.informativeText = ::WizToNSString(strText);
    [userNotification setDeliveryDate:[NSDate dateWithTimeInterval:5 sinceDate:[NSDate date]]];

    [[NSUserNotificationCenter defaultUserNotificationCenter] scheduleNotification:userNotification];
}

void Notify::setDockBadge(int nCount)
{
    if (nCount)
        [[NSApp dockTile] setBadgeLabel:[NSString stringWithFormat:@"%d", nCount]];
    else
        [[NSApp dockTile] setBadgeLabel:nil];
}


