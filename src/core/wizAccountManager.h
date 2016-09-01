#ifndef CWIZACCOUNTMANAGER_H
#define CWIZACCOUNTMANAGER_H

#include "share/wizDatabaseManager.h"

class CWizAccountManager
{
public:
    CWizAccountManager(CWizDatabaseManager& dbMgr);

    bool isVip();
    bool isPaidUser();

    bool isPaidGroup(const QString& kbGUID);
    bool isBizGroup(const QString& kbGUID);

private:
    CWizDatabaseManager& m_dbMgr;
};

#endif // CWIZACCOUNTMANAGER_H
