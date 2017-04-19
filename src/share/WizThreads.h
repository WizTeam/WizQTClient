#ifndef WIZTHREADS_H
#define WIZTHREADS_H

#include <QObject>
#include <QThread>
#include <QDebug>

#define WIZ_THREAD_MAIN				0
#define WIZ_THREAD_DEFAULT			1
#define WIZ_THREAD_NETWORK          2
#define WIZ_THREAD_DOWNLOAD         3
#define WIZ_THREAD_SEARCH         4
#define WIZ_THREAD_DOWNLOAD_RESOURCES         5


void WizQueuedThreadsInit();
void WizQueuedThreadsShutdown();
int WizQueuedGetCurrentThreadId();

struct IWizThreadPool;
//
struct IWizRunableEvents
{
    virtual void reportProgress(int pos) = 0;
    virtual void reportStatus(const QString& strStatus) = 0;
};
//
struct IWizRunable
{
    virtual void destroy() {
//        qDebug() << "IWizRunable Destroy";
        delete this; }
    virtual void run(int threadIndex, IWizThreadPool* pThreadPool, IWizRunableEvents* pEvents) = 0;
    virtual QString getTaskID() { return QString(); }
    virtual ~IWizRunable() {
//        qDebug() << "IWizRunable desstruct";
    }
};

struct IWizThreadPoolEvents
{
    virtual void beforeTask(IWizRunable* task) = 0;
    virtual void afterTask(IWizRunable* task) = 0;
};

struct IWizThreadPool
{
    virtual void addTask(IWizRunable* task) = 0;
    virtual void clearTasks() = 0;
    virtual IWizRunable* peekOne() = 0;
    virtual void shutdown(int timeout) = 0;
    virtual bool isShuttingDown() = 0;
    virtual bool isIdle() = 0;
    virtual void getTaskCount(int* pnWorking, int* pnWaiting) = 0;
    virtual void setEventsListener(IWizThreadPoolEvents* pEvents) = 0;
    virtual void destroy() {
        //qDebug() << "IWizThreadPool Destroy";
        delete this; }
    virtual ~IWizThreadPool() {
        //qDebug() << "IWizThreadPool desstruct";
    }
};

struct IWizDelayedThreadPool : public IWizThreadPool
{
    virtual void addDelayedTask(IWizRunable* task, int delayedSeconds) = 0;
    virtual void executeAllNow() = 0;
};

IWizThreadPool* WizCreateThreadPool(int threadCount, QThread::Priority priority = QThread::NormalPriority);
IWizDelayedThreadPool* WizCreateDelayedThreadPool(int threadCount, QThread::Priority priority = QThread::NormalPriority);


// ***********************************************************************************
template<class TFun>
class WizFunctionalAction
    : public IWizRunable
{
public:
    WizFunctionalAction(TFun f)
        : m_fun(f)
    {
    }
    //
    virtual void run(int threadIndex, IWizThreadPool* pool, IWizRunableEvents* pEvents)
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
    IWizRunable* action = new WizFunctionalAction<TFun>(f);
    return action;
}

template<class TFun>
class WizFunctionalActionEx
    : public IWizRunable
{
public:
    WizFunctionalActionEx(TFun f)
        : m_fun(f)
    {
    }
    //
    virtual void run(int threadIndex, IWizThreadPool* pool, IWizRunableEvents* pEvents)
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
    IWizRunable* action = new WizFunctionalActionEx<TFun>(f);
    return action;
}


void WizQueuedThreadAddAction(int threadID, IWizRunable* action);
void WizQueuedThreadAddAction(int threadID, IWizRunable* action, int milliseconds, int nTimeoutThreadId, IWizRunable* timeoutAction);

template <class TFun>
inline void WizExecuteOnThread(int threadID, TFun f)
{
    IWizRunable* action = new WizFunctionalAction<TFun>(f);
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
    IWizRunable* action = new WizFunctionalAction<TFun>(f);
    IWizRunable* actionTimeout = new WizFunctionalAction<TFunTimeout>(fTimeout);
    //
    WizQueuedThreadAddAction(threadID, action, milliseconds, nTimeoutThreadId, actionTimeout);
}



#endif // WIZTHREADS_H
