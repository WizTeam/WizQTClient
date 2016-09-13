#ifndef WIZLOCKEDFILE_H
#define WIZLOCKEDFILE_H

#include <QFile>

class WizLockedFile : public QFile
{
    Q_OBJECT

public:
    enum LockMode { NoLock = 0, ReadLock, WriteLock };

    WizLockedFile();
    WizLockedFile(const QString &name);
    ~WizLockedFile();

    bool lock(LockMode mode, bool block = true);
    bool unlock();
    bool isLocked() const;
    LockMode lockMode() const;

private:
#ifdef Q_OS_WIN
    Qt::HANDLE m_semaphore_hnd;
    Qt::HANDLE m_mutex_hnd;
#endif
    LockMode m_lock_mode;

};

#endif // WIZLOCKEDFILE_H
