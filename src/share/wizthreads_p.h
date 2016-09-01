#ifndef WIZTHREADS_P_H
#define WIZTHREADS_P_H

#include <deque>
#include <QMutex>
#include "wizthreads.h"

class QTimer;

class CWizMainQueuedThread
    : public QObject
    , public IWizThreadPool
{
    Q_OBJECT
public:
    CWizMainQueuedThread();
private:
    QMutex m_cs;
    std::deque<IWizRunable*> m_tasks;
    bool m_bShutingDown;
public:
    virtual void AddTask(IWizRunable* task);
    virtual void ClearTasks();
    virtual bool IsIdle() { return false; }
    virtual IWizRunable* PeekOne() { return NULL; }
    virtual void Shutdown(int timeout)  { Q_UNUSED(timeout);  m_bShutingDown = true; }
    virtual bool IsShuttingDown()  { return m_bShutingDown; }
    virtual void SetEventsListener(IWizThreadPoolEvents* pEvents) { Q_UNUSED(pEvents); }
    //
    virtual void GetTaskCount(int* pnWorking, int* pnWaiting);
    //
    virtual IWizThreadPool* GetThreadPool() { return this; }
    //
    void GetAllTasks(std::deque<IWizRunable*>& tasks);
protected slots:
    void ExecuteAllActions();
Q_SIGNALS:
    void TaskAdded();
};

class CWizTimeoutRunable
        : public QObject
        , public IWizRunable

{
    Q_OBJECT
public:
    CWizTimeoutRunable(IWizRunable* pAction, int milliseconds, int nTimeoutThreadId, IWizRunable* pTimeoutAction);
private:
    IWizRunable* m_pRunable;
    int m_nTimeoutThreadId;
    IWizRunable* m_pTimeoutRunable;
    QTimer* m_timer;
    //
    QMutex m_cs;
public:
    virtual void Destroy();
    virtual void Run(int threadIndex, IWizThreadPool* pThreadPool, IWizRunableEvents* pEvents);
    virtual QString GetTaskID();
private:
    IWizRunable* BeforeTimeout();
protected slots:
    void Timeout();
};


#endif // WIZTHREADS_P_H
