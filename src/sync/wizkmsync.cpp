#include "wizkmxmlrpc.h"
#include "wizkmsync.h"
#include <QThread>

//#include "../share/wizSyncableDatabase.h"
#include "../share/wizDatabase.h"

class CWizKMSyncEvents : public IWizKMSyncEvents
{
    virtual HRESULT OnText(WizKMSyncProgressStatusType type, const QString& strStatus) {}
};

class CWizKMSyncThread : public QThread
{
    CWizDatabase& m_db;
public:
    CWizKMSyncThread(CWizDatabase& db, QObject* parent)
        : QThread(parent)
        , m_db(db)
    {

    }

public:
    virtual void run()
    {
        CWizKMSyncEvents events;
        ::WizSyncDatabase(&events, &m_db, true, true);
    }

};


bool WizKMSync(CWizDatabase& db)
{
    (new CWizKMSyncThread(db, NULL))->start();
    //
    return true;
}
