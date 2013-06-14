#include "wizIOHostThread.h"

CWizIOHostThread::CWizIOHostThread(QObject *parent)
    : QThread(parent)
    , m_ioHost(NULL)
{
}

void CWizIOHostThread::run()
{
    m_ioHost = new CWizIOHost();

    qRegisterMetaType<CWizIOTask>("CWizIOTask");
    connect(this, SIGNAL(taskBegin(const CWizIOTask&)),
            m_ioHost, SLOT(on_taskBegin(const CWizIOTask&)),
            Qt::QueuedConnection);

    connect(m_ioHost, SIGNAL(taskEnd(const CWizIOTask&)),
            this, SLOT(on_taskEnd(const CWizIOTask&)),
            Qt::QueuedConnection);

    exec();
}

void CWizIOHostThread::on_taskEnd(const CWizIOTask& task)
{
    task.callback()();
}

void CWizIOHostThread::postTask(WizTaskMethod fn, WizTaskMethod callback)
{
    std::cout << "postTask: " << QThread::currentThreadId() << std::endl;

    CWizIOTask task(fn, callback);

    Q_EMIT taskBegin(task);
}


CWizIOHost::CWizIOHost(QObject* parent)
    : QObject(parent)
{

}

void CWizIOHost::on_taskBegin(const CWizIOTask& task)
{
    std::cout << "on_taskBegin: " << QThread::currentThreadId() << std::endl;

    CWizFutureWatcher<void>* watcher = new CWizFutureWatcher<void>(task);
    connect(watcher, SIGNAL(finished()), SLOT(on_watcher_finished()));

    QFuture<void> future = QtConcurrent::run(task.task());
    watcher->setFuture(future);
}

void CWizIOHost::on_watcher_finished()
{
    std::cout << "watcher, finished: " << QThread::currentThreadId() << std::endl;

    CWizFutureWatcher<void>* watcher = dynamic_cast<CWizFutureWatcher<void> *>(sender());

    Q_EMIT taskEnd(watcher->task());

    // delete QFutureWatcher later
    sender()->deleteLater();
}
