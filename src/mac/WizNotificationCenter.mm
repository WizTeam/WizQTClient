#include "WizNotificationCenter.h"

#import <AppKit/AppKit.h>

#include "share/WizObject.h"
#include "mac/WizMacHelper_mm.h"

#define NOTIFICATION_KEY_VALUE   "key"
#define NOTIFICATION_USER_VALUE   "user"

@interface QNotificationCenterDelegate : NSObject
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_8
    <NSUserNotificationCenterDelegate>
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_8
- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification;
- (void)userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification;
#endif
@end

@implementation QNotificationCenterDelegate

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_8
- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification {
    Q_UNUSED(center);
    Q_UNUSED(notification);
    return YES;
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification {
    Q_UNUSED(center);
    Q_UNUSED(notification);

    NSLog(@"Notification - Clicked");
    NSLog(@"notification %@", [notification title]);

    NSDictionary *myDictionary = [notification userInfo];
    NSString* array1 = [myDictionary objectForKey: @"mobile"];

    NSLog(@"%@", array1);
    [center removeDeliveredNotification:notification];
}
#endif

@end


WizNotificationCenter::WizNotificationCenter(QObject* parent)
    : QObject(parent)
{
}



void WizNotificationCenter::showNofification(WizNotificationCenter::NotificationType type,
                                              const QString& title, const QString& text, const QString& userInfo)
{
    NSUserNotification *notification = [[NSUserNotification alloc] init];
    NSString* nsText = WizToNSString(title);
    [notification setTitle: nsText];
    NSString* nsInfo = WizToNSString(text);
    [notification setInformativeText: nsInfo];
    [notification setHasActionButton: NO];

    NSString* keyValue = [NSString stringWithFormat:@"%d",(int)type];
    NSString* userData = WizToNSString(userInfo);
    NSDictionary *myDictionary = [NSDictionary dictionaryWithObjectsAndKeys:keyValue,@NOTIFICATION_KEY_VALUE,userData,@NOTIFICATION_USER_VALUE,nil];//注意用nil结束
    [notification setUserInfo: myDictionary];

    [notification setSoundName: NSUserNotificationDefaultSoundName];

    [notification setDeliveryDate: [NSDate dateWithTimeIntervalSinceNow: 3]];
    QNotificationCenterDelegate* delegate = [[[QNotificationCenterDelegate alloc] init] autorelease];
    [[NSUserNotificationCenter defaultUserNotificationCenter] scheduleNotification: notification];
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:delegate];
}

void WizNotificationCenter::showNofification(const WIZMESSAGEDATA& message)
{
    NSUserNotification *notification = [[NSUserNotification alloc] init];
    NSString* nsText = WizToNSString(message.title);
    [notification setTitle: nsText];
    NSString* nsInfo = WizToNSString(message.messageBody);
    [notification setInformativeText: nsInfo];
    [notification setHasActionButton: NO];

    NSString* keyValue = [NSString stringWithFormat:@"%d",(int)Notification_Message];
    NSString* userData = [NSString stringWithFormat:@"%lld",message.nId];
    NSDictionary *myDictionary = [NSDictionary dictionaryWithObjectsAndKeys:keyValue,@NOTIFICATION_KEY_VALUE,userData,@NOTIFICATION_USER_VALUE,nil];//注意用nil结束
    [notification setUserInfo: myDictionary];

    [notification setSoundName: NSUserNotificationDefaultSoundName];

    [notification setDeliveryDate: [NSDate dateWithTimeIntervalSinceNow: 3]];
    QNotificationCenterDelegate* delegate = [[[QNotificationCenterDelegate alloc] init] autorelease];
    [[NSUserNotificationCenter defaultUserNotificationCenter] scheduleNotification: notification];
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:delegate];
}
