#include "WizThreads.h"
#include "WizThreads_p.h"

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
    virtual void addTask(IWizRunable* task);
    virtual void clearTasks();
    virtual IWizRunable* peekOne();    //execute on worker
    //
    virtual bool isShuttingDown();
    virtual void shutdown(int timeout);
    virtual bool isIdle();
    virtual void setEventsListener(IWizThreadPoolEvents* pEvents);
    virtual void getTaskCount(int* pnWorking, int* pnWaiting);
    //
    virtual void addDelayedTask(IWizRunable* task, int delayedSeconds) { Q_UNUSED(task); Q_UNUSED(delayedSeconds); }
    virtual void executeAllNow() {}
public:
    void beforeTask(IWizRunable* task);
    void afterTask(IWizRunable* task);
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
        while (!m_pool->isShuttingDown())
        {            
            //
            IWizRunable* task = m_pool->peekOne();
            if (task)
            {
                m_pool->beforeTask(task);
                n_bWorking = true;
                task->run(m_nThreadIndex, m_pool, NULL);
                n_bWorking = false;
                m_pool->afterTask(task);
                task->destroy();
            }
        }
    }
    //
public:
    virtual IWizThreadPool* getThreadPool()
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
void CWizThreadPool::addTask(IWizRunable* task)
{
    QMutexLocker lock(&m_cs);
    //
    QString strTaskId = task->getTaskID();
    if (!strTaskId.isEmpty())
    {
        //remove exists tasks
        intptr_t count = m_tasks.size();
        for (intptr_t i = count - 1; i >= 0; i--)
        {
            IWizRunable* t = m_tasks[i];
            if (t->getTaskID() == strTaskId)
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

void CWizThreadPool::clearTasks()
{
    QMutexLocker lock(&m_cs);
    //
    m_tasks.clear();
}

IWizRunable* CWizThreadPool::peekOne()    //execute on worker
{
    if (m_bShuttingDown)
        return nullptr;
    //
    IWizRunable* task = nullptr;
    //
    //
    //
    {
        QMutexLocker lock(&m_cs);
        if (m_bShuttingDown)
            return nullptr;
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
        //
        if (m_bShuttingDown)
            return nullptr;
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
bool CWizThreadPool::isShuttingDown()
{
    return m_bShuttingDown;
}
//
void CWizThreadPool::shutdown(int timeout)
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
    //while ((int)WizGetTickCount() - start >= timeout)
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
                thread->deleteLater();
            }
            else
            {
                thread->quit();
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
        WaitThread::msleep(100);
#else
        QThread::msleep(100);
#endif
    }
}
//
void CWizThreadPool::getTaskCount(int* pnWorking, int* pnWaiting)
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
bool CWizThreadPool::isIdle()
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
void CWizThreadPool::setEventsListener(IWizThreadPoolEvents* pEvents)
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

void CWizThreadPool::beforeTask(IWizRunable* task)
{
    if (!m_pEvents)
        return;
    m_pEvents->beforeTask(task);
}
void CWizThreadPool::afterTask(IWizRunable* task)
{
    if (!m_pEvents)
        return;
    m_pEvents->afterTask(task);
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
    virtual void run()
    {
        while (1)
        {
            if (m_pool->isShuttingDown())
                return;
            //
            IWizRunable* task = m_pool->peekOne();
            if (task)
            {
                m_pool->beforeTask(task);
                n_bWorking = true;
                task->run(m_nThreadIndex, m_pool, NULL);
                n_bWorking = false;
                m_pool->afterTask(task);
                task->destroy();
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
    virtual void addTask(IWizRunable* task)
    {
        addDelayedTask(task, 0);
    }
    //
    virtual void addDelayedTask(IWizRunable* task, int delayedSeconds)
    {
        CWizThreadPool::addTask(task);
        //
        QMutexLocker lock(&m_cs);
        //
        QDateTime now = QDateTime::currentDateTime();
        QDateTime work = now.addSecs(delayedSeconds);
        //
        m_mapTasksTime[task] = work;
    }
    virtual void executeAllNow()
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
    virtual IWizRunable* peekOne()    //execute on worker
    {
        if (m_bShuttingDown)
            return nullptr;
        //
        QDateTime now = QDateTime::currentDateTime();
        //
        QMutexLocker lock(&m_cs);

        if (m_bShuttingDown)
            return nullptr;

        //
        if (m_tasks.empty())
            return nullptr;
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
        return nullptr;
    }
};

IWizDelayedThreadPool* WizCreateDelayedThreadPool(int threadCount, QThread::Priority priority)
{
    CWizDelayedThreadPool* pool = new CWizDelayedThreadPool(threadCount, CWizDelayedWorkThread::Create, priority);
    return pool;
}
///////////////////////////////////////////////////////////////////////////////////

WizMainQueuedThread::WizMainQueuedThread()
    : m_bShutingDown(false)
{
    connect(this, SIGNAL(taskAdded()), this, SLOT(executeAllActions()), Qt::QueuedConnection);
}
void WizMainQueuedThread::addTask(IWizRunable* task)
{
    //
    {
        QMutexLocker lock(&m_cs);
        m_tasks.push_back(task);
    }
    //
    emit taskAdded();
}
//
void WizMainQueuedThread::clearTasks()
{
    QMutexLocker lock(&m_cs);
    m_tasks.clear();
}

//
void WizMainQueuedThread::getTaskCount(int* pnWorking, int* pnWaiting)
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
void WizMainQueuedThread::getAllTasks(std::deque<IWizRunable*>& tasks)
{
    QMutexLocker lock(&m_cs);
    tasks = m_tasks;
    m_tasks.clear();
}
void WizMainQueuedThread::executeAllActions()
{
    std::deque<IWizRunable*> tasks;
    getAllTasks(tasks);
    //
    for (auto it = tasks.begin();
        it != tasks.end();
        it++)
    {
        if (isShuttingDown())
            return;
        IWizRunable* pRunable = *it;
        pRunable->run(WIZ_THREAD_MAIN, this, NULL);
        pRunable->destroy();

    }
}


WizTimeoutRunable::WizTimeoutRunable(IWizRunable* pAction, int milliseconds, int nTimeoutThreadId, IWizRunable* pTimeoutAction)
    : m_pRunable(pAction)
    , m_nTimeoutThreadId(nTimeoutThreadId)
    , m_pTimeoutRunable(pTimeoutAction)
{
    m_timer = new QTimer(NULL);
    m_timer->setSingleShot(true);
    m_timer->setInterval(milliseconds);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}
void WizTimeoutRunable::destroy()
{
    m_pRunable->destroy();
    delete this;
}
void WizTimeoutRunable::run(int threadIndex, IWizThreadPool* pThreadPool, IWizRunableEvents* pEvents)
{
    m_pRunable->run(threadIndex, pThreadPool, pEvents);
}
QString WizTimeoutRunable::getTaskID()
{
    return m_pRunable->getTaskID();
}


IWizRunable* WizTimeoutRunable::beforeTimeout()
{
    QMutexLocker lock(&m_cs);
    //
    IWizRunable* runable = m_pTimeoutRunable;
    m_pTimeoutRunable = NULL;
    //
    return runable;
}


void WizTimeoutRunable::timeout()
{
    if (IWizRunable* pRunable = beforeTimeout())
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
        static IWizThreadPool* pool = new WizMainQueuedThread();
        return pool;
    }
public:
    static IWizThreadPool* getThreadPool(int threadID, int threadCount = 1)
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
        IWizThreadPool* pool = ::WizCreateThreadPool(threadCount);
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
            threadPool->shutdown(5);
        }
        threads.clear();
    }
};

void WizQueuedThreadsInit()
{
    CWizQueuedThreads::getThreadPool(WIZ_THREAD_MAIN);
    CWizQueuedThreads::getThreadPool(WIZ_THREAD_DOWNLOAD_RESOURCES, 5);
    CWizQueuedThreads::getThreadPool(WIZ_THREAD_NETWORK, 2);
}

void WizQueuedThreadAddAction(int threadID, IWizRunable* action)
{
    IWizThreadPool* thread = CWizQueuedThreads::getThreadPool(threadID);
    //
    thread->addTask(action);
}

void WizQueuedThreadAddAction(int threadID, IWizRunable* action, int milliseconds, int nTimeoutThreadId, IWizRunable* timeoutAction)
{
    IWizThreadPool* thread = CWizQueuedThreads::getThreadPool(threadID);
    //
    thread->addTask(new WizTimeoutRunable(action, milliseconds, nTimeoutThreadId, timeoutAction));
}


void WizQueuedThreadsShutdown()
{
//    CWizQueuedThreads::ShutdownAll();

    CWizQueuedThreads::ClearThreadPool();
}
