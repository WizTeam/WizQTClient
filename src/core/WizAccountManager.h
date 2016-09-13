#ifndef CWIZACCOUNTMANAGER_H
#define CWIZACCOUNTMANAGER_H

#include "share/WizDatabaseManager.h"

class WizAccountManager
{
public:
    WizAccountManager(WizDatabaseManager& dbMgr);

    bool isVip();
    bool isPaidUser();

    bool isPaidGroup(const QString& kbGUID);
    bool isBizGroup(const QString& kbGUID);

private:
    WizDatabaseManager& m_dbMgr;
};

#endif // CWIZACCOUNTMANAGER_H
