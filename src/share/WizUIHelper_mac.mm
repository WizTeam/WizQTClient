#include "WizUIHelper.h"

#ifdef Q_OS_MAC

#import <Cocoa/Cocoa.h>
#import <qmaccocoaviewcontainer_mac.h>
#include <Carbon/Carbon.h>

class CWizMacSearchWidget;


@interface WizMacSearchWidgetEventsObject : NSObject<NSTextFieldDelegate> {
    CWizMacSearchWidget* m_searchWidget;
}
- (id) init;
- (void) setSearchWidget:(CWizMacSearchWidget*)searchWidget;
- (BOOL) control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor;
@end


class CWizMacSearchWidget : public QMacCocoaViewContainer
{
public:
    CWizMacSearchWidget(CWizSearchBox* searchBox);
    //QSize sizeHint() const;

private:
    CWizSearchBox* m_searchBox;

public:
    void onEndEditing(const char* utf8);
};


/////////////////////////////////////////////////////////////////////

@implementation WizMacSearchWidgetEventsObject

- (id)init
{
    m_searchWidget = NULL;
    //
    self = [super init];
    //
    return self;
}

- (BOOL)control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
{
    Q_UNUSED(control);
    Q_UNUSED(fieldEditor);
    //
    if (m_searchWidget)
    {
        const char* utf8 = [[fieldEditor string] UTF8String];
        m_searchWidget->onEndEditing(utf8);
    }
    //
    return YES;
}

- (void) setSearchWidget:(CWizMacSearchWidget*)searchWidget
{
    m_searchWidget = searchWidget;
}

@end

/////////////////////////////////////////////////////////////////////


CWizMacSearchWidget::CWizMacSearchWidget(CWizSearchBox* searchBox)
    : QMacCocoaViewContainer(0, searchBox)
    , m_searchBox(searchBox)
{
    // Many Cocoa objects create temporary autorelease objects,
    // so create a pool to catch them.
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    // Create the NSSearchField, set it on the QCocoaViewContainer.
    NSSearchField *search = [[NSSearchField alloc] init];
    setCocoaView(search);

    WizMacSearchWidgetEventsObject* events = [[WizMacSearchWidgetEventsObject alloc] init];
    [events setSearchWidget:this];
    [search setDelegate:events];

    // Use a Qt menu for the search field menu.
    //QMenu *qtMenu = createMenu(this);
    //NSMenu *nsMenu = qtMenu->macMenu(0);
    //[[search cell] setSearchMenuTemplate:nsMenu];

    // Release our reference, since our super class takes ownership and we
    // don't need it anymore.
    [search release];

    // Clean up our pool as we no longer need it.
    [pool release];
}
//![0]

//QSize CWizMacSearchWidget::sizeHint() const
//{
//    return QSize(250, 40);
//}

void CWizMacSearchWidget::onEndEditing(const char* utf8)
{
    QString str = QString::fromUtf8(utf8);
    m_searchBox->on_search_edited(str);
    m_searchBox->on_search_editingFinished();
}


/////////////////////////////////////////////////////////////////////

CWizSearchBox::CWizSearchBox(WizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
{
    m_search = new CWizMacSearchWidget(this);
    m_search->move(2,2);
    //setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}

/////////////////////////////////////////////////////////////////////
class CWizMacSplitterHandle : public QSplitterHandle
{
public:
    CWizMacSplitterHandle(Qt::Orientation orientation, QSplitter *parent);
    void paintEvent(QPaintEvent *);
    QSize sizeHint() const;
};



CWizMacSplitterHandle::CWizMacSplitterHandle(Qt::Orientation orientation, QSplitter *parent)
: QSplitterHandle(orientation, parent)
{
}

// Paint the horizontal handle as a gradient, paint
// the vertical handle as a line.
void CWizMacSplitterHandle::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QColor topColor(145, 145, 145);
    QColor bottomColor(142, 142, 142);
    QColor gradientStart(252, 252, 252);
    QColor gradientStop(223, 223, 223);

    if (orientation() == Qt::Vertical) {
        painter.setPen(topColor);
        painter.drawLine(0, 0, width(), 0);
        painter.setPen(bottomColor);
        painter.drawLine(0, height() - 1, width(), height() - 1);

        QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, height() -3));
        linearGrad.setColorAt(0, gradientStart);
        linearGrad.setColorAt(1, gradientStop);
        painter.fillRect(QRect(QPoint(0,1), size() - QSize(0, 2)), QBrush(linearGrad));
    } else {
        painter.setPen(topColor);
        painter.drawLine(0, 0, 0, height());
    }
}

QSize CWizMacSplitterHandle::sizeHint() const
{
    QSize parent = QSplitterHandle::sizeHint();
    if (orientation() == Qt::Vertical) {
        return parent + QSize(0, 3);
    } else {
        return QSize(1, parent.height());
    }
}


QSplitterHandle *WizSplitter::createHandle()
{
    return new CWizMacSplitterHandle(orientation(), this);
}


#endif


