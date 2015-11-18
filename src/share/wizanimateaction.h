#ifndef WIZANIMATEACTION_H
#define WIZANIMATEACTION_H

#include "wizqthelper.h"

#include <QIcon>
#include <QList>

#include "wizdef.h"

class QTimer;
class QAction;
class QToolButton;

class CWizAnimateContainerBase : public QObject
{
public:
    CWizAnimateContainerBase(QObject* parent);

    virtual QIcon icon() = 0;
    virtual void setIcon(const QIcon &icon) = 0;
    virtual bool setProperty(const char *name, const QVariant &value) = 0;
};

class CWizAnimateActionContainer : public CWizAnimateContainerBase
{
public:
    explicit CWizAnimateActionContainer(QAction* action, QObject* parent);

    virtual QIcon icon();
    virtual void setIcon(const QIcon &icon);
    virtual bool setProperty(const char *name, const QVariant &value);

private:
    QAction* m_action;
};

class CWizAnimateButtonContainer : public CWizAnimateContainerBase
{
public:
    explicit CWizAnimateButtonContainer(QToolButton* button, QObject* parent);

    virtual QIcon icon();
    virtual void setIcon(const QIcon &icon);
    virtual bool setProperty(const char *name, const QVariant &value);

private:
    QToolButton* m_button;
};

class CWizAnimateAction : public QObject
{
    Q_OBJECT

public:
    CWizAnimateAction(QObject* parent);
    void setAction(QAction* action);
    void setToolButton(QToolButton* button);
    void setSingleIcons(const QString& strIconBaseName);
    void setTogetherIcon(const QString& strIconBaseName);
    void startPlay();
    void stopPlay();
    bool isPlaying();

private:    
    CWizAnimateContainerBase* m_target;
    int m_nIconIndex;
    QIcon m_iconDefault;
    QList<QIcon> m_icons;
    QTimer* m_timer;

    void nextIcon();

public slots:
    void on_timer_timeout();

};

#endif // WIZANIMATEACTION_H
