#include "WizIOHostThread.h"

WizIOHostThread::WizIOHostThread(QObject *parent)
    : QThread(parent)
    , m_ioHost(NULL)
{
}

void WizIOHostThread::run()
{
    m_ioHost = new WizIOHost();

    qRegisterMetaType<WizIOTask>("CWizIOTask");
    connect(this, SIGNAL(taskBegin(const WizIOTask&)),
            m_ioHost, SLOT(on_taskBegin(const WizIOTask&)),
            Qt::QueuedConnection);

    connect(m_ioHost, SIGNAL(taskEnd(const WizIOTask&)),
            this, SLOT(on_taskEnd(const WizIOTask&)),
            Qt::QueuedConnection);

    exec();
}

void WizIOHostThread::on_taskEnd(const WizIOTask& task)
{
    task.callback()();
}

void WizIOHostThread::postTask(WizTaskMethod fn, WizTaskMethod callback)
{
    std::cout << "postTask: " << QThread::currentThreadId() << std::endl;

    WizIOTask task(fn, callback);

    Q_EMIT taskBegin(task);
}


WizIOHost::WizIOHost(QObject* parent)
    : QObject(parent)
{

}

void WizIOHost::on_taskBegin(const WizIOTask& task)
{
    std::cout << "on_taskBegin: " << QThread::currentThreadId() << std::endl;

    WizFutureWatcher<void>* watcher = new WizFutureWatcher<void>(task);
    connect(watcher, SIGNAL(finished()), SLOT(on_watcher_finished()));

    QFuture<void> future = QtConcurrent::run(task.task());
    watcher->setFuture(future);
}

void WizIOHost::on_watcher_finished()
{
    std::cout << "watcher, finished: " << QThread::currentThreadId() << std::endl;

    WizFutureWatcher<void>* watcher = dynamic_cast<WizFutureWatcher<void> *>(sender());

    Q_EMIT taskEnd(watcher->task());

    // delete QFutureWatcher later
    sender()->deleteLater();
}
