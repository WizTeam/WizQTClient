#ifndef CWIZACCOUNTMANAGER_H
#define CWIZACCOUNTMANAGER_H

#include "share/wizDatabaseManager.h"

class CWizAccountManager
{
public:
    CWizAccountManager(CWizDatabaseManager& dbMgr);

    bool isVip();
    bool isPaidUser();

private:
    CWizDatabaseManager& m_dbMgr;
};

#endif // CWIZACCOUNTMANAGER_H
