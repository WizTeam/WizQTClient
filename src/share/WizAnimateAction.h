#ifndef WIZANIMATEACTION_H
#define WIZANIMATEACTION_H

#include "WizQtHelper.h"

#include <QIcon>
#include <QList>

#include "WizDef.h"

class QTimer;
class QAction;
class QToolButton;

class WizAnimateContainerBase : public QObject
{
public:
    WizAnimateContainerBase(QObject* parent);

    virtual QIcon icon() = 0;
    virtual void setIcon(const QIcon &icon) = 0;
    virtual bool setProperty(const char *name, const QVariant &value) = 0;
};

class WizAnimateActionContainer : public WizAnimateContainerBase
{
public:
    explicit WizAnimateActionContainer(QAction* action, QObject* parent);

    virtual QIcon icon();
    virtual void setIcon(const QIcon &icon);
    virtual bool setProperty(const char *name, const QVariant &value);

private:
    QAction* m_action;
};

class WizAnimateButtonContainer : public WizAnimateContainerBase
{
public:
    explicit WizAnimateButtonContainer(QToolButton* button, QObject* parent);

    virtual QIcon icon();
    virtual void setIcon(const QIcon &icon);
    virtual bool setProperty(const char *name, const QVariant &value);

private:
    QToolButton* m_button;
};

class WizAnimateAction : public QObject
{
    Q_OBJECT

public:
    WizAnimateAction(QObject* parent);
    void setAction(QAction* action);
    void setToolButton(QToolButton* button);
    void setSingleIcons(const QString& strIconBaseName, QSize size = QSize());
    void setTogetherIcon(const QString& strIconBaseName);
    void startPlay();
    void stopPlay();
    bool isPlaying();

private:    
    WizAnimateContainerBase* m_target;
    int m_nIconIndex;
    QIcon m_iconDefault;
    QList<QIcon> m_icons;
    QTimer* m_timer;

    void nextIcon();

public slots:
    void on_timer_timeout();

};

#endif // WIZANIMATEACTION_H
