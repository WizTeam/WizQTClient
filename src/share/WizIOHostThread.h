#ifndef WIZIOHOSTTHREAD_H
#define WIZIOHOSTTHREAD_H

#include <iostream>
#include <functional>
#include <vector>

#include <QThread>
#include <QtCore>

typedef std::function<void(void)> WizTaskMethod;

class WizIOTask
{

public:
    WizIOTask()
    {
        m_task_method = NULL;
        m_callback_method = NULL;
    }

    WizIOTask(WizTaskMethod _task_method, WizTaskMethod _callback_method)
    {
        m_task_method = _task_method;
        m_callback_method = _callback_method;
    }

    WizIOTask(const WizIOTask& t)
    {
        m_task_method = t.task();
        m_callback_method = t.callback();
    }

    WizTaskMethod task() const { return m_task_method; }
    WizTaskMethod callback() const { return m_callback_method; }

private:
    WizTaskMethod m_task_method;
    WizTaskMethod m_callback_method;
};

template <typename T>
class WizFutureWatcher : public QFutureWatcher<T>
{

public:
    WizFutureWatcher(const WizIOTask& ts, QObject* parent = 0)
        : QFutureWatcher<T>(parent)
        , m_task(ts)
    {
    }

    WizIOTask task() { return m_task; }

private:
    WizIOTask m_task;
};


class WizIOHost : public QObject
{
    Q_OBJECT

public:
    explicit WizIOHost(QObject* parent = 0);

private Q_SLOTS:
    void on_taskBegin(const WizIOTask& task);
    void on_watcher_finished();

Q_SIGNALS:
    void taskEnd(const WizIOTask& task);
};


class WizIOHostThread : public QThread
{
    Q_OBJECT

public:
    explicit WizIOHostThread(QObject *parent = 0);
    virtual void run();

    void postTask(WizTaskMethod fn, WizTaskMethod callback);

private:
    WizIOHost* m_ioHost;

private Q_SLOTS:
    void on_taskEnd(const WizIOTask& task);

Q_SIGNALS:
    void taskBegin(const WizIOTask& task);
};


#endif // WIZIOHOSTTHREAD_H
