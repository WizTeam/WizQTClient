#ifndef QTMACTOOLBAR_H
#define QTMACTOOLBAR_H

#include <QtGlobal>

#ifdef Q_OS_MAC
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QtDeclarative/QDeclarativeListProperty>
#include <QtDeclarative/QDeclarativeParserStatus>

class MacToolButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QUrl iconSource READ iconSource WRITE setIconSource)
    Q_PROPERTY(bool selectable READ selectable WRITE setSelectable)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)
    Q_PROPERTY(StandardItem standardItem READ standardItem WRITE setStandardItem)
    Q_ENUMS(StandardItem)
public:
    enum StandardItem
    {
        NoItem,
        Separator,
        Space,
        FlexibleSpace,
        ShowColors,
        ShowFonts,
        CustomizeToolbar,
        PrintItem,
    };

    MacToolButton();
    MacToolButton(QObject *parent);
    virtual ~MacToolButton();

    QString text() const;
    void setText(const QString &text);

    QUrl iconSource() const;
    void setIconSource(const QUrl &iconSource);

    bool selectable() const;
    void setSelectable(bool selectable);

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    void setAction(QAction *action);
    void setActionGroup(QActionGroup *actionGroup);

    StandardItem standardItem() const;
    void setStandardItem(StandardItem standardItem);
    //
    bool isGroup() const { return m_actionGroup != NULL; }
    QActionGroup* actionGroup() const { return m_actionGroup; }
    QAction* action() const { return m_action; }
signals:
    void activated();
private:
    QString m_text;
    QUrl m_iconSource;
    bool m_selectable;
    QString m_toolTip;
    StandardItem m_standardItem;
public: // (not really public)
    QAction *m_action;
    QActionGroup* m_actionGroup;
    void emitActivated() { emit activated(); }
};


class QtMacToolBarPrivate;
class QtMacToolBar : public QObject, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)
    Q_PROPERTY(QDeclarativeListProperty<QObject> buttons READ buttons)
    Q_PROPERTY(QDeclarativeListProperty<QObject> allowedButtons READ allowedButtons)
    Q_CLASSINFO("DefaultProperty", "buttons")
    Q_PROPERTY(DisplayMode displayMode READ displayMode WRITE setDisplayMode)
    Q_PROPERTY(SizeMode sizeMode READ sizeMode WRITE setSizeMode)
    Q_ENUMS(DisplayMode)
    Q_ENUMS(SizeMode)
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

    QtMacToolBar(QObject *parent = 0);
    ~QtMacToolBar();

    void classBegin();
    void componentComplete();

    QDeclarativeListProperty<QObject> buttons();
    QDeclarativeListProperty<QObject> allowedButtons();

    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode displayMode);

    SizeMode sizeMode() const;
    void setSizeMode(SizeMode sizeMode);

    // In addition to the QML bindings QtMacToolBar also has a
    // QAction-based interface for use from C++.
    //
    // Usage example:
    //
    // QWidget *mainWindow = ...;
    // QtMacToolBar * macToolBar = new QtMacToolBar(mainWindow);
    // macToolBar->addAction("FooButton");
    // macToolBar->showInWindow(mainWindow);
    //

    Q_INVOKABLE void showInMainWindow();
    void showInWindow(QWidget *window);

    // Add actions to the toolbar
    Q_INVOKABLE void addActionGroup(QActionGroup* actionGroup);
    Q_INVOKABLE void addAction(QAction* action);
    Q_INVOKABLE QAction *addAction(const QString &text);
    Q_INVOKABLE QAction *addAction(const QIcon &icon, const QString &text);
    Q_INVOKABLE QAction *addStandardItem(MacToolButton::StandardItem standardItem);

    // Add actions to the "Customize Toolbar" menu
    Q_INVOKABLE QAction *addAllowedAction(const QString &text);
    Q_INVOKABLE QAction *addAllowedAction(const QIcon &icon, const QString &text);
    Q_INVOKABLE QAction *addAllowedStandardItem(MacToolButton::StandardItem standardItem);
private:
    void showInWindowImpl(QWidget *window);
    Q_INVOKABLE void showInTargetWindow();

    bool m_showText;
    QList<QObject *> m_buttons;
    QList<QObject *> m_allowedButtons;
    QtMacToolBarPrivate *d;
};

#endif

#endif

