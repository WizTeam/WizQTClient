#include "wizthreads.h"
#include "wizthreads_p.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QDateTime>
#include <QTimer>

#include <deque>
#include <vector>
#include <map>

class CWizTaskWorkThread;
class CWizThreadPool;

typedef CWizTaskWorkThread* WizCreateThreadFunction(CWizThreadPool* pool, int threadIndex, QThread::Priority priority);


class CWizThreadPool : public IWizDelayedThreadPool
{
public:
    CWizThreadPool(int poolCount, WizCreateThreadFunction* createThreadFun, QThread::Priority priority);
    ~CWizThreadPool();
protected:
    std::deque<CWizTaskWorkThread*> m_threads;
    std::deque<IWizRunable*> m_tasks;

    QMutex m_cs;
    QMutex m_csEvent;
    QWaitCondition m_event;
    bool m_bShuttingDown;
    IWizThreadPoolEvents* m_pEvents;
public:
    virtual void AddTask(IWizRunable* task);
    virtual void ClearTasks();
    virtual IWizRunable* PeekOne();    //execute on worker
    //
    virtual bool IsShuttingDown();
    virtual void Shutdown(int timeout);
    virtual bool IsIdle();
    virtual void SetEventsListener(IWizThreadPoolEvents* pEvents);
    virtual void GetTaskCount(int* pnWorking, int* pnWaiting);
    //
    virtual void AddDelayedTask(IWizRunable* task, int delayedSeconds) { Q_UNUSED(task); Q_UNUSED(delayedSeconds); }
    virtual void ExecuteAllNow() {}
public:
    void BeforeTask(IWizRunable* task);
    void AfterTask(IWizRunable* task);
protected:
    int GetWorkingTaskCount();
    int GetWaitingTaskCount();
};


class CWizTaskWorkThread : public QThread
{
public:
    CWizTaskWorkThread(CWizThreadPool* pool, int threadIndex)
        : m_pool(pool)
        , m_nThreadIndex(threadIndex)
        , n_bWorking(false)
    {
    }
    virtual ~CWizTaskWorkThread()
    {
//        qDebug() << "CWizTaskWorkThread destruct";
    }
protected:
    CWizThreadPool* m_pool;
    int m_nThreadIndex;
    bool n_bWorking;
protected:
    virtual void run()
    {
        while (1)
        {
            if (m_pool->IsShuttingDown())
                return;
            //
            IWizRunable* task = m_pool->PeekOne();
            if (task)
            {
                m_pool->BeforeTask(task);
                n_bWorking = true;
                task->Run(m_nThreadIndex, m_pool, NULL);
                n_bWorking = false;
                m_pool->AfterTask(task);
                task->Destroy();
            }
        }
    }
    //
public:
    virtual IWizThreadPool* GetThreadPool()
    {
        return m_pool;
    }
    //
    virtual bool IsAlive()
    {
        return isRunning();
    }
    //
    virtual bool IsWorking()
    {
        return n_bWorking;
    }
public:
    static CWizTaskWorkThread* Create(CWizThreadPool* pool, int threadIndex, QThread::Priority priority)
    {
        CWizTaskWorkThread* thread = new CWizTaskWorkThread(pool, threadIndex);
        thread->start(priority);
        return thread;
    }
};

CWizThreadPool::CWizThreadPool(int poolCount, WizCreateThreadFunction* createThreadFun, QThread::Priority priority)
    : m_cs(QMutex::Recursive)
    , m_csEvent(QMutex::NonRecursive)
    , m_bShuttingDown(false)
    , m_pEvents(NULL)
{
    //
    for (int i = 0; i < poolCount; i++)
    {
        CWizTaskWorkThread* thread = createThreadFun(this, i, priority);
        m_threads.push_back(thread);
    }
}

CWizThreadPool::~CWizThreadPool()
{
    //qDebug() << "CWizThreadPool destruct , threads : " << m_threads.size();
    for (CWizTaskWorkThread* thread : m_threads)
    {
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->quit();
    }
    m_threads.clear();
}
//
void CWizThreadPool::AddTask(IWizRunable* task)
{
    QMutexLocker lock(&m_cs);
    //
    QString strTaskId = task->GetTaskID();
    if (!strTaskId.isEmpty())
    {
        //remove exists tasks
        intptr_t count = m_tasks.size();
        for (intptr_t i = count - 1; i >= 0; i--)
        {
            IWizRunable* t = m_tasks[i];
            if (t->GetTaskID() == strTaskId)
            {
                m_tasks.erase(m_tasks.begin() + i);
            }
        }
    }
    //
    m_tasks.push_back(task);
    //
    lock.unlock();
    //
    QMutexLocker lockEvent(&m_csEvent);
    m_event.wakeAll();
}

void CWizThreadPool::ClearTasks()
{
    QMutexLocker lock(&m_cs);
    //
    m_tasks.clear();
}

IWizRunable* CWizThreadPool::PeekOne()    //execute on worker
{
    if (m_bShuttingDown)
        return NULL;
    //
    IWizRunable* task = NULL;
    //
    //
    //
    {
        QMutexLocker lock(&m_cs);
        //
        if (!m_tasks.empty())
        {
            task = *m_tasks.begin();
            m_tasks.pop_front();
        }
    }
    //
    if (task)
        return task;
    //
    {
        QMutexLocker locker(&m_csEvent);
        m_event.wait(&m_csEvent);
    }
    //
    //
    {
        QMutexLocker lock(&m_cs);
        //
        if (!m_tasks.empty())
        {
            task = *m_tasks.begin();
            m_tasks.pop_front();
        }
        //
    }
    //
    return task;
}
//
bool CWizThreadPool::IsShuttingDown()
{
    return m_bShuttingDown;
}
//
void CWizThreadPool::Shutdown(int timeout)
{
    m_bShuttingDown = true;
    //
    //
    {
        QMutexLocker lock(&m_csEvent);
        m_event.wakeAll();
    }
    //
    //
    Q_UNUSED(timeout);
    //while ((int)GetTickCount() - start >= timeout)
    while (true)
    {
        int count = (int)m_threads.size();
        for (int i = count - 1; i >= 0; i--)
        {
            CWizTaskWorkThread* thread = m_threads[i];
            //
            if (!thread->IsAlive())
            {
                m_threads.erase(m_threads.begin() + i);
                delete thread;
            }
        }
        //
        if (m_threads.empty())
        {
            delete this;
            return;
        }
        //
        //WizProcessMessages();
#ifdef Q_OS_LINUX
        class WaitThread : public QThread
        {
         public :
             static void sleep(long iSleepTime)
             {
                  QThread::sleep(iSleepTime);
             }
             static void msleep(long iSleepTime)
             {
                  QThread::msleep(iSleepTime);
             }
        };
        WaitThread::sleep(1);
#else
        QThread::sleep(1);
#endif
    }
}
//
void CWizThreadPool::GetTaskCount(int* pnWorking, int* pnWaiting)
{
    if (pnWorking)
    {
        *pnWorking = GetWorkingTaskCount();
    }
    if (pnWaiting)
    {
        *pnWaiting = GetWaitingTaskCount();
    }
}
//
int CWizThreadPool::GetWorkingTaskCount()
{
    int count = 0;
    //
    for (auto it = m_threads.begin(); it != m_threads.end(); it++)
    {
        CWizTaskWorkThread* thread = *it;
        if (thread->IsWorking())
        {
            count++;
        }
    }
    //
    return count;
}
//
bool CWizThreadPool::IsIdle()
{
    if (GetWaitingTaskCount() > 0)
        return false;
    //
    for (auto it = m_threads.begin(); it != m_threads.end(); it++)
    {
        CWizTaskWorkThread* thread = *it;
        if (thread->IsWorking())
        {
            return false;
        }
    }
    return true;
}
void CWizThreadPool::SetEventsListener(IWizThreadPoolEvents* pEvents)
{
    m_pEvents = pEvents;
}


//
int CWizThreadPool::GetWaitingTaskCount()
{
    QMutexLocker locker(&m_cs);
    int count = (int)m_tasks.size();
    //
    return count;
}

void CWizThreadPool::BeforeTask(IWizRunable* task)
{
    if (!m_pEvents)
        return;
    m_pEvents->BeforeTask(task);
}
void CWizThreadPool::AfterTask(IWizRunable* task)
{
    if (!m_pEvents)
        return;
    m_pEvents->AfterTask(task);
}



IWizThreadPool* WizCreateThreadPool(int threadCount, QThread::Priority priority)
{
    CWizThreadPool* pool = new CWizThreadPool(threadCount, CWizTaskWorkThread::Create, priority);
    return pool;
}

///////////////////////////////////////////////////////////////////////////////////

class CWizDelayedWorkThread : public CWizTaskWorkThread
{
public:
    CWizDelayedWorkThread(CWizThreadPool* pool, int threadIndex)
        : CWizTaskWorkThread(pool, threadIndex)
    {
    }
protected:
    virtual void Run()
    {
        while (1)
        {
            if (m_pool->IsShuttingDown())
                return;
            //
            IWizRunable* task = m_pool->PeekOne();
            if (task)
            {
                m_pool->BeforeTask(task);
                n_bWorking = true;
                task->Run(m_nThreadIndex, m_pool, NULL);
                n_bWorking = false;
                m_pool->AfterTask(task);
                task->Destroy();
            }
            else
            {
                QThread::sleep(1);
            }
        }
    }
public:
    static CWizTaskWorkThread* Create(CWizThreadPool* pool, int threadIndex, QThread::Priority priority)
    {
        CWizDelayedWorkThread* thread = new CWizDelayedWorkThread(pool, threadIndex);
        thread->start(priority);
        return thread;
    }
};


class CWizDelayedThreadPool : public CWizThreadPool
{
public:
    CWizDelayedThreadPool(int poolCount, WizCreateThreadFunction* createThreadFunction, QThread::Priority priority)
        : CWizThreadPool(poolCount, createThreadFunction, priority)
    {
    }
protected:
    std::map<IWizRunable*, QDateTime> m_mapTasksTime;
public:
    //
    virtual void AddTask(IWizRunable* task)
    {
        AddDelayedTask(task, 0);
    }
    //
    virtual void AddDelayedTask(IWizRunable* task, int delayedSeconds)
    {
        CWizThreadPool::AddTask(task);
        //
        QMutexLocker lock(&m_cs);
        //
        QDateTime now = QDateTime::currentDateTime();
        QDateTime work = now;
        work.addSecs(delayedSeconds);
        //
        m_mapTasksTime[task] = work;
    }
    virtual void ExecuteAllNow()
    {
        QMutexLocker lock(&m_cs);
        //
        m_mapTasksTime.clear();
        QDateTime now = QDateTime::currentDateTime();
        //
        for (auto it = m_tasks.begin();
            it != m_tasks.end();
            it++)
        {
            m_mapTasksTime[*it] = now;
        }
    }
    //
    virtual IWizRunable* PeekOne()    //execute on worker
    {
        if (m_bShuttingDown)
            return NULL;
        //
        QDateTime now = QDateTime::currentDateTime();
        //
        QMutexLocker lock(&m_cs);
        //
        if (m_tasks.empty())
            return NULL;
        //
        size_t count = m_tasks.size();
        for (size_t i = 0; i < count; i++)
        {
            IWizRunable* task = m_tasks[i];
            //
            QDateTime t = m_mapTasksTime[task];
            //
            if (now >= t)
            {
                m_tasks.erase(m_tasks.begin() + i);
                return task;
            }
        }
        //
        return NULL;
    }
};

IWizDelayedThreadPool* WizCreateDelayedThreadPool(int threadCount, QThread::Priority priority)
{
    CWizDelayedThreadPool* pool = new CWizDelayedThreadPool(threadCount, CWizDelayedWorkThread::Create, priority);
    return pool;
}
///////////////////////////////////////////////////////////////////////////////////

CWizMainQueuedThread::CWizMainQueuedThread()
    : m_bShutingDown(false)
{
    connect(this, SIGNAL(TaskAdded()), this, SLOT(ExecuteAllActions()), Qt::QueuedConnection);
}
void CWizMainQueuedThread::AddTask(IWizRunable* task)
{
    //
    {
        QMutexLocker lock(&m_cs);
        m_tasks.push_back(task);
    }
    //
    emit TaskAdded();
}
//
void CWizMainQueuedThread::ClearTasks()
{
    QMutexLocker lock(&m_cs);
    m_tasks.clear();
}

//
void CWizMainQueuedThread::GetTaskCount(int* pnWorking, int* pnWaiting)
{
    QMutexLocker lock(&m_cs);
    //
    if (pnWaiting)
    {
        *pnWaiting = (int)m_tasks.size();
    }
    //
    if (pnWorking)
    {
        *pnWorking = 1;
    }
}
//
void CWizMainQueuedThread::GetAllTasks(std::deque<IWizRunable*>& tasks)
{
    QMutexLocker lock(&m_cs);
    tasks = m_tasks;
    m_tasks.clear();
}
void CWizMainQueuedThread::ExecuteAllActions()
{
    std::deque<IWizRunable*> tasks;
    GetAllTasks(tasks);
    //
    for (auto it = tasks.begin();
        it != tasks.end();
        it++)
    {
        if (IsShuttingDown())
            return;
        IWizRunable* pRunable = *it;
        pRunable->Run(WIZ_THREAD_MAIN, this, NULL);
        pRunable->Destroy();

    }
}


CWizTimeoutRunable::CWizTimeoutRunable(IWizRunable* pAction, int milliseconds, int nTimeoutThreadId, IWizRunable* pTimeoutAction)
    : m_pRunable(pAction)
    , m_nTimeoutThreadId(nTimeoutThreadId)
    , m_pTimeoutRunable(pTimeoutAction)
{
    m_timer = new QTimer(NULL);
    m_timer->setSingleShot(true);
    m_timer->setInterval(milliseconds);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(Timeout()));
}
void CWizTimeoutRunable::Destroy()
{
    m_pRunable->Destroy();
    delete this;
}
void CWizTimeoutRunable::Run(int threadIndex, IWizThreadPool* pThreadPool, IWizRunableEvents* pEvents)
{
    m_pRunable->Run(threadIndex, pThreadPool, pEvents);
}
QString CWizTimeoutRunable::GetTaskID()
{
    return m_pRunable->GetTaskID();
}


IWizRunable* CWizTimeoutRunable::BeforeTimeout()
{
    QMutexLocker lock(&m_cs);
    //
    IWizRunable* runable = m_pTimeoutRunable;
    m_pTimeoutRunable = NULL;
    //
    return runable;
}


void CWizTimeoutRunable::Timeout()
{
    if (IWizRunable* pRunable = BeforeTimeout())
    {
        WizExecuteOnThread<IWizRunable*>(m_nTimeoutThreadId, pRunable);
    }
}


class CWizQueuedThreads
{
private:
    static QMutex& GetCriticalSection()
    {
        static QMutex cs;
        return cs;
    }
    static std::map<int, IWizThreadPool*>& GetThreads()
    {
        static std::map<int, IWizThreadPool*> threads;
        return threads;
    }
    //
    static IWizThreadPool* GetMainThreadPool()
    {
        static IWizThreadPool* pool = new CWizMainQueuedThread();
        return pool;
    }
public:
    static IWizThreadPool* GetThreadPool(int threadID)
    {
        QMutex& cs = GetCriticalSection();
        QMutexLocker lock(&cs);
        //
        std::map<int, IWizThreadPool*>& threads = GetThreads();
        //
        std::map<int, IWizThreadPool*>::const_iterator it = threads.find(threadID);
        if (it != threads.end())
            return it->second;
        //
        if (threadID == WIZ_THREAD_MAIN)
        {
            IWizThreadPool* pool = GetMainThreadPool();
            threads[threadID] = pool;
            return pool;
        }
        //
        IWizThreadPool* pool = ::WizCreateThreadPool(1);
        threads[threadID] = pool;
        return pool;
    }
    //

    static void ClearThreadPool()
    {
        QMutex& cs = GetCriticalSection();
        QMutexLocker lock(&cs);
        //
        std::map<int, IWizThreadPool*>& threads = GetThreads();
        qDebug() << "clear thread pool, threads count : " << threads.size();
        std::map<int, IWizThreadPool*>::const_iterator it;
        for (it = threads.begin(); it != threads.end(); it++)
        {
            IWizThreadPool* threadPool = it->second;
            threadPool->Shutdown(5);
        }
        threads.clear();
    }
};

void WizQueuedThreadsInit()
{
    CWizQueuedThreads::GetThreadPool(WIZ_THREAD_MAIN);
}

void WizQueuedThreadAddAction(int threadID, IWizRunable* action)
{
    IWizThreadPool* thread = CWizQueuedThreads::GetThreadPool(threadID);
    //
    thread->AddTask(action);
}

void WizQueuedThreadAddAction(int threadID, IWizRunable* action, int milliseconds, int nTimeoutThreadId, IWizRunable* timeoutAction)
{
    IWizThreadPool* thread = CWizQueuedThreads::GetThreadPool(threadID);
    //
    thread->AddTask(new CWizTimeoutRunable(action, milliseconds, nTimeoutThreadId, timeoutAction));
}


void WizQueuedThreadsShutdown()
{
//    CWizQueuedThreads::ShutdownAll();

    CWizQueuedThreads::ClearThreadPool();
}
