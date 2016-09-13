#ifndef CWizMacToolBar_H
#define CWizMacToolBar_H

#ifdef USECOCOATOOLBAR
#include <QtGlobal>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QAction>
#include "WizMacHelper.h"

//#include <QtDeclarative/QDeclarativeListProperty>
//#include <QtDeclarative/QDeclarativeParserStatus>


class CWizMacToolBarPrivate;
class WizMacToolBarItem;
class WizSearchView;


class WizMacToolBarButtonItem : public WizCocoaViewContainer
{
    Q_OBJECT
public:
    WizMacToolBarButtonItem(const QString& title, const QPixmap& extraMenuIcon, int width, QWidget* parent = 0);

    QSize sizeHint() const;

    void buttonClicked();
    void extraMenuClicked();
    //
    QRect geometry();
signals:
    void triggered(bool);
    void showExtraMenuRequest();

private:
    int m_width;
};

class WizMacFixedSpacer : public WizCocoaViewContainer
{
    QSize m_sz;
public:
    WizMacFixedSpacer(QSize sz, QWidget* parent = 0);

    void adjustWidth(int width);

    QSize sizeHint() const { return m_sz; }
};

class WizMacToolBar
    : public QWidget
{
    Q_OBJECT
public:
    WizMacToolBar(QWidget *parent = 0);
    ~WizMacToolBar();
public:
    enum DisplayMode
    {
        DefaultDisplay,
        IconAndLabelDisplay,
        IconOnlyDisplay,
        LabelOnlyDisplay
    };

    enum SizeMode {
       DefaultSize,
       RegularSize,
       SmallSize
    };

    enum StandardItem
    {
        NoItem,
        Separator,
        Space,
        FlexibleSpace,
        ShowColors,
        ShowFonts,
        CustomizeToolbar,
        PrintItem
    };


    void classBegin();
    void componentComplete();

    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode displayMode);

    SizeMode sizeMode() const;
    void setSizeMode(SizeMode sizeMode);

    void showInWindow(QWidget *window);
    void setToolBarVisible(bool bVisible);

    // Add actions to the Toolbar
    void addAction(QAction* action);
    void addStandardItem(StandardItem standardItem);
    void addSearch(const QString& label, const QString& tooltip,int width = 250);
    void addWidget(WizCocoaViewContainer* widget, const QString& label, const QString& tooltip);

    void deleteAllToolBarItems();

    void onSearchEndEditing(const QString& str);
    //
    WizSearchView* getSearchWidget();    
    void adjustSearchWidgetWidth(int nWidth);
    //void adjustWidgetToolBarItemWidth(QWidget* widget, int nWidth);

private:
    void showInWindowImpl(QWidget *window);
protected slots:
    void showInTargetWindow();

Q_SIGNALS:
    void doSearch(const QString& str);

private:
    bool m_showText;
    QList<WizMacToolBarItem *> m_items;
    CWizMacToolBarPrivate *d;
};

#endif

#endif

