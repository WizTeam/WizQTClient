#ifndef WIZTHREADS_H
#define WIZTHREADS_H

#include <QObject>
#include <QThread>
#include <QDebug>

#define WIZ_THREAD_MAIN				0
#define WIZ_THREAD_DEFAULT			1
#define WIZ_THREAD_NETWORK          2
#define WIZ_THREAD_DOWNLOAD         3



void WizQueuedThreadsInit();
void WizQueuedThreadsShutdown();
int WizQueuedGetCurrentThreadId();

struct IWizThreadPool;
//
struct IWizRunableEvents
{
    virtual void ReportProgress(int pos) = 0;
    virtual void ReportStatus(const QString& strStatus) = 0;
};
//
struct IWizRunable
{
    virtual void Destroy() {
//        qDebug() << "IWizRunable Destroy";
        delete this; }
    virtual void Run(int threadIndex, IWizThreadPool* pThreadPool, IWizRunableEvents* pEvents) = 0;
    virtual QString GetTaskID() { return QString(); }
    virtual ~IWizRunable() {
//        qDebug() << "IWizRunable desstruct";
    }
};

struct IWizThreadPoolEvents
{
    virtual void BeforeTask(IWizRunable* task) = 0;
    virtual void AfterTask(IWizRunable* task) = 0;
};

struct IWizThreadPool
{
    virtual void AddTask(IWizRunable* task) = 0;
    virtual void ClearTasks() = 0;
    virtual IWizRunable* PeekOne() = 0;
    virtual void Shutdown(int timeout) = 0;
    virtual bool IsShuttingDown() = 0;
    virtual bool IsIdle() = 0;
    virtual void GetTaskCount(int* pnWorking, int* pnWaiting) = 0;
    virtual void SetEventsListener(IWizThreadPoolEvents* pEvents) = 0;
    virtual void Destroy() {
        //qDebug() << "IWizThreadPool Destroy";
        delete this; }
    virtual ~IWizThreadPool() {
        //qDebug() << "IWizThreadPool desstruct";
    }
};

struct IWizDelayedThreadPool : public IWizThreadPool
{
    virtual void AddDelayedTask(IWizRunable* task, int delayedSeconds) = 0;
    virtual void ExecuteAllNow() = 0;
};

IWizThreadPool* WizCreateThreadPool(int threadCount, QThread::Priority priority = QThread::NormalPriority);
IWizDelayedThreadPool* WizCreateDelayedThreadPool(int threadCount, QThread::Priority priority = QThread::NormalPriority);


// ***********************************************************************************
template<class TFun>
class CWizFunctionalAction
    : public IWizRunable
{
public:
    CWizFunctionalAction(TFun f)
        : m_fun(f)
    {
    }
    //
    virtual void Run(int threadIndex, IWizThreadPool* pool, IWizRunableEvents* pEvents)
    {
        Q_UNUSED(threadIndex);
        Q_UNUSED(pool);
        Q_UNUSED(pEvents);
        m_fun();
    }
    //
private:
    TFun m_fun;
};

template <class TFun>
inline IWizRunable* WizCreateRunable(TFun f)
{
    IWizRunable* action = new CWizFunctionalAction<TFun>(f);
    return action;
}

template<class TFun>
class CWizFunctionalActionEx
    : public IWizRunable
{
public:
    CWizFunctionalActionEx(TFun f)
        : m_fun(f)
    {
    }
    //
    virtual void Run(int threadIndex, IWizThreadPool* pool, IWizRunableEvents* pEvents)
    {
        m_fun(threadIndex, pool, pEvents);
    }
    //
private:
    TFun m_fun;
};

template <class TFun>
inline IWizRunable* WizCreateRunableEx(TFun f)
{
    IWizRunable* action = new CWizFunctionalActionEx<TFun>(f);
    return action;
}


void WizQueuedThreadAddAction(int threadID, IWizRunable* action);
void WizQueuedThreadAddAction(int threadID, IWizRunable* action, int milliseconds, int nTimeoutThreadId, IWizRunable* timeoutAction);

template <class TFun>
inline void WizExecuteOnThread(int threadID, TFun f)
{
    IWizRunable* action = new CWizFunctionalAction<TFun>(f);
    WizQueuedThreadAddAction(threadID, action);
}

template <class TFun>
inline void WizExecuteOnThread(int threadID, IWizRunable* pRunable)
{
    WizQueuedThreadAddAction(threadID, pRunable);
}

template <class TFun, class TFunTimeout>
void WizExecuteOnThread(int threadID, TFun f, int milliseconds, int nTimeoutThreadId, TFunTimeout fTimeout)
{
    IWizRunable* action = new CWizFunctionalAction<TFun>(f);
    IWizRunable* actionTimeout = new CWizFunctionalAction<TFunTimeout>(fTimeout);
    //
    WizQueuedThreadAddAction(threadID, action, milliseconds, nTimeoutThreadId, actionTimeout);
}



#endif // WIZTHREADS_H
