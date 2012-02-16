#ifndef WIZMACACTIONHELPER_H
#define WIZMACACTIONHELPER_H

#include <QtGlobal>
#include <QtCore/QObject>

#ifdef Q_OS_MAC

class CWizMacToolBarItem;
class QAction;

class CWizMacActionHelper : public QObject
{
    Q_OBJECT
public:
    CWizMacActionHelper(CWizMacToolBarItem* item, QAction* action, QObject* parent);
private:
    CWizMacToolBarItem* m_item;
public slots:
    void on_action_changed();
};

#else

//
//temp class for avoid moc tool error
//
class CWizMacActionHelper : public QObject
{
    Q_OBJECT
};

#endif

#endif // WIZMACACTIONHELPER_H
