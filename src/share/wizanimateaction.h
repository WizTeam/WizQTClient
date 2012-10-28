#ifndef WIZANIMATEACTION_H
#define WIZANIMATEACTION_H

#include "wizqthelper.h"

#include <QIcon>
#include <QList>

#include "wizdef.h"

class QTimer;
class QAction;

class CWizAnimateAction : public QObject
{
    Q_OBJECT

public:
    CWizAnimateAction(CWizExplorerApp& app, QObject* parent);
    void setAction(QAction* action);
    void setIcons(const QString& strIconBaseName);
    void startPlay();
    void stopPlay();

private:
    CWizExplorerApp& m_app;
    QAction* m_action;
    int m_nIconIndex;
    QIcon m_iconDefault;
    QList<QIcon> m_icons;
    QTimer* m_timer;

    void nextIcon();

public slots:
    void on_timer_timeout();

};

#endif // WIZANIMATEACTION_H
