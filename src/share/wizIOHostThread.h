#ifndef WIZIOHOSTTHREAD_H
#define WIZIOHOSTTHREAD_H

#include <iostream>
#include <functional>
#include <vector>

#include <QThread>
#include <QtCore>

typedef std::function<void(void)> WizTaskMethod;

class CWizIOTask
{

public:
    CWizIOTask()
    {
        m_task_method = NULL;
        m_callback_method = NULL;
    }

    CWizIOTask(WizTaskMethod _task_method, WizTaskMethod _callback_method)
    {
        m_task_method = _task_method;
        m_callback_method = _callback_method;
    }

    CWizIOTask(const CWizIOTask& t)
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
class CWizFutureWatcher : public QFutureWatcher<T>
{

public:
    CWizFutureWatcher(const CWizIOTask& ts, QObject* parent = 0)
        : QFutureWatcher<T>(parent)
        , m_task(ts)
    {
    }

    CWizIOTask task() { return m_task; }

private:
    CWizIOTask m_task;
};


class CWizIOHost : public QObject
{
    Q_OBJECT

public:
    explicit CWizIOHost(QObject* parent = 0);

private Q_SLOTS:
    void on_taskBegin(const CWizIOTask& task);
    void on_watcher_finished();

Q_SIGNALS:
    void taskEnd(const CWizIOTask& task);
};


class CWizIOHostThread : public QThread
{
    Q_OBJECT

public:
    explicit CWizIOHostThread(QObject *parent = 0);
    virtual void run();

    void postTask(WizTaskMethod fn, WizTaskMethod callback);

private:
    CWizIOHost* m_ioHost;

private Q_SLOTS:
    void on_taskEnd(const CWizIOTask& task);

Q_SIGNALS:
    void taskBegin(const CWizIOTask& task);
};


#endif // WIZIOHOSTTHREAD_H
