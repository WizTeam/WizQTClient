#include "dialog.h"
#include "ui_dialog.h"

#import "INAppStoreWindow/INAppStoreWindow.h"

void Dialog::onButtonClicked()
{
  INAppStoreWindow* aWindow = [[INAppStoreWindow alloc] initWithContentRect:CGRectMake(300, 300, 300, 300)
      styleMask:NSTitledWindowMask | NSClosableWindowMask
      backing:NSBackingStoreBuffered
      defer:YES
    ];

  //aWindow.titleBarHeight = 40.0;
  //aWindow.trafficLightButtonsLeftMargin = 13.0;
  aWindow.titleBarDrawingBlock = ^(BOOL drawsAsMainWindow, CGRect drawingRect, CGPathRef clippingPath) {
      CGContextRef ctx = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
      CGContextAddPath(ctx, clippingPath);
      CGContextClip(ctx);
      //
      Q_UNUSED(drawsAsMainWindow);

      [[NSColor windowBackgroundColor] setFill];
      NSRectFill(drawingRect);
  };


  NSWindowController *controller = [[NSWindowController alloc] initWithWindow:aWindow];
  [controller showWindow:nil];

}

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    //
    connect(ui->pushButton, SIGNAL(clicked()), SLOT(onButtonClicked()));
}

Dialog::~Dialog()
{
    delete ui;
}
