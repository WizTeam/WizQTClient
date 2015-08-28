#ifndef WIZSERVICE_SYNC_H
#define WIZSERVICE_SYNC_H

#include "../share/wizSyncableDatabase.h"

bool WizSyncDatabase(const WIZUSERINFO& info,
                     IWizKMSyncEvents* pEvents,
                     IWizSyncableDatabase* pDatabase, bool bBackground);

bool WizQuickDownloadMessage(const WIZUSERINFO& info,
                             IWizKMSyncEvents* pEvents,
                             IWizSyncableDatabase* pDatabase);

#endif // WIZSERVICE_SYNC_H
