#ifndef WIZLOCKEDFILE_H
#define WIZLOCKEDFILE_H

#include <QFile>

class CWizLockedFile : public QFile
{
    Q_OBJECT

public:
    enum LockMode { NoLock = 0, ReadLock, WriteLock };

    CWizLockedFile();
    CWizLockedFile(const QString &name);
    ~CWizLockedFile();

    bool lock(LockMode mode, bool block = true);
    bool unlock();
    bool isLocked() const;
    LockMode lockMode() const;

private:
    LockMode m_lock_mode;

};

#endif // WIZLOCKEDFILE_H
