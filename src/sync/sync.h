#ifndef WIZSERVICE_SYNC_H
#define WIZSERVICE_SYNC_H

#include "../share/wizSyncableDatabase.h"

bool WizSyncDatabase(const WIZUSERINFO& info,
                     IWizKMSyncEvents* pEvents,
                     IWizSyncableDatabase* pDatabase,
                     bool bUseWizServer, bool bBackground);

#endif // WIZSERVICE_SYNC_H
