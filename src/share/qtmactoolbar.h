#ifndef CWizMacToolBar_H
#define CWizMacToolBar_H

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



class CWizMacToolBarPrivate;
class CWizMacToolBarItem;
//
class CWizMacToolBar
    : public QObject
    , public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)
public:
    CWizMacToolBar(QObject *parent = 0);
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

    void classBegin();
    void componentComplete();

    QDeclarativeListProperty<QObject> buttons();
    QDeclarativeListProperty<QObject> allowedButtons();

    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode displayMode);

    SizeMode sizeMode() const;
    void setSizeMode(SizeMode sizeMode);

    void showInMainWindow();
    void showInWindow(QWidget *window);

    // Add actions to the toolbar
    void addActionGroup(QActionGroup* actionGroup);
    void addAction(QAction* action);
    void addStandardItem(MacToolButton::StandardItem standardItem);

private:
    void showInWindowImpl(QWidget *window);
    void showInTargetWindow();

    bool m_showText;
    QList<CWizMacToolBarItem *> m_items;
    CWizMacToolBarPrivate *d;
};

#endif

#endif

