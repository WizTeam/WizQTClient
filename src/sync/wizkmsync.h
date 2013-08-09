#ifndef WIZKMSYNC_H
#define WIZKMSYNC_H

#include <QThread>

class CWizDatabase;

class CWizKMSyncThread : public QThread
{
    Q_OBJECT

public:
    CWizKMSyncThread(CWizDatabase& db, QObject* parent = 0);
    void startSync();

protected:
    virtual void run();

private:
    CWizDatabase& m_db;
};

#endif // WIZKMSYNC_H
