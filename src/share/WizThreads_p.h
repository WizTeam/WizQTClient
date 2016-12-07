#ifndef WIZTHREADS_P_H
#define WIZTHREADS_P_H

#include <deque>
#include <QMutex>
#include "WizThreads.h"

class QTimer;

class WizMainQueuedThread
    : public QObject
    , public IWizThreadPool
{
    Q_OBJECT
public:
    WizMainQueuedThread();
private:
    QMutex m_cs;
    std::deque<IWizRunable*> m_tasks;
    bool m_bShutingDown;
public:
    virtual void addTask(IWizRunable* task);
    virtual void clearTasks();
    virtual bool isIdle() { return false; }
    virtual IWizRunable* peekOne() { return NULL; }
    virtual void shutdown(int timeout)  { Q_UNUSED(timeout);  m_bShutingDown = true; }
    virtual bool isShuttingDown()  { return m_bShutingDown; }
    virtual void setEventsListener(IWizThreadPoolEvents* pEvents) { Q_UNUSED(pEvents); }
    //
    virtual void getTaskCount(int* pnWorking, int* pnWaiting);
    //
    virtual IWizThreadPool* getThreadPool() { return this; }
    //
    void getAllTasks(std::deque<IWizRunable*>& tasks);
protected slots:
    void executeAllActions();
Q_SIGNALS:
    void taskAdded();
};

class WizTimeoutRunable
        : public QObject
        , public IWizRunable

{
    Q_OBJECT
public:
    WizTimeoutRunable(IWizRunable* pAction, int milliseconds, int nTimeoutThreadId, IWizRunable* pTimeoutAction);
private:
    IWizRunable* m_pRunable;
    int m_nTimeoutThreadId;
    IWizRunable* m_pTimeoutRunable;
    QTimer* m_timer;
    //
    QMutex m_cs;
public:
    virtual void destroy();
    virtual void run(int threadIndex, IWizThreadPool* pThreadPool, IWizRunableEvents* pEvents);
    virtual QString getTaskID();
private:
    IWizRunable* beforeTimeout();
protected slots:
    void timeout();
};


#endif // WIZTHREADS_P_H
