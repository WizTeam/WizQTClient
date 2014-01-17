#include "wizUserInfoWidget_mm.h"

#include <QWidget>

#include "wizdef.h"
#include "share/wizDatabaseManager.h"

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


CWizUserInfoWidget::CWizUserInfoWidget(CWizExplorerApp& app, QWidget* parent)
    : QMacCocoaViewContainer(nil, parent)
    , m_app(app)
    , m_db(app.databaseManager().db())
{
    //
    WizUserInfoView* view = [[WizUserInfoView alloc] init];
    [view setAutoresizesSubviews: YES];
    [view setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];
    setCocoaView(view);
    [view release];

    resetUserInfo();
}

QSize CWizUserInfoWidget::sizeHint() const
{
    return QSize(36 + fontMetrics().width(text()) + 18, 36);
}
QString CWizUserInfoWidget::text() const
{
    return m_text;
}
void CWizUserInfoWidget::setText(QString val)
{
    m_text = val;
}

void CWizUserInfoWidget::resetUserInfo()
{
    /*
    WIZUSERINFO info;
    if (!m_db.GetUserInfo(info))
        return;

    if (info.strDisplayName.isEmpty()) {
        setText(::WizGetEmailPrefix(m_db.GetUserId()));
    } else {
        QString strName = fontMetrics().elidedText(info.strDisplayName, Qt::ElideRight, 150);
        setText(strName);
    }

    QString iconName;
    if (info.strUserType == "vip") {
        iconName = "vip1.png";
    } else if (info.strUserType == "vip2") {
        iconName = "vip2.png";
    } else if (info.strUserType == "vip3") {
        iconName = "vip3.png";
    } else {
        iconName = "vip0.png";
    }

    QString strIconPath = ::WizGetSkinResourcePath(m_app.userSettings().skin()) + iconName;
    //m_iconVipIndicator.addFile(strIconPath);
    */
}
