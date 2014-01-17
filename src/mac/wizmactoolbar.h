#ifndef CWizMacToolBar_H
#define CWizMacToolBar_H

#include <QtGlobal>

#ifdef Q_OS_MAC
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QAction>
#include <QtDeclarative/QDeclarativeListProperty>
#include <QtDeclarative/QDeclarativeParserStatus>


class CWizMacToolBarPrivate;
class CWizMacToolBarItem;
class CWizSearchWidget;
class QMacCocoaViewContainer;

class CWizMacToolBar
    : public QWidget
{
    Q_OBJECT
public:
    CWizMacToolBar(QWidget *parent = 0);
    ~CWizMacToolBar();
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

    // Add actions to the Toolbar
    void addAction(QAction* action);
    void addStandardItem(StandardItem standardItem);
    void addSearch(const QString& label, const QString& tooltip);
    void addWidget(QMacCocoaViewContainer* widget, const QString& label, const QString& tooltip);

    void onSearchEndEditing(const QString& str);
    //
    CWizSearchWidget* getSearchWidget();

private:
    void showInWindowImpl(QWidget *window);
protected slots:
    void showInTargetWindow();

Q_SIGNALS:
    void doSearch(const QString& str);

private:
    bool m_showText;
    QList<CWizMacToolBarItem *> m_items;
    CWizMacToolBarPrivate *d;
};

#endif

#endif

