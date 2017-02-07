#ifndef WIZSERVICE_SYNC_H
#define WIZSERVICE_SYNC_H

#include "../share/WizSyncableDatabase.h"

bool WizSyncDatabase(const WIZUSERINFO& info,
                     IWizKMSyncEvents* pEvents,
                     IWizSyncableDatabase* pDatabase, bool bBackground);

bool WizQuickDownloadMessage(const WIZUSERINFO& info,
                             IWizKMSyncEvents* pEvents,
                             IWizSyncableDatabase* pDatabase);

bool WizDownloadDocumentsByGuids(const WIZUSERINFO& info,
                                 IWizSyncableDatabase* pDatabase,
                                 const QString& kbGuid,
                                 const CWizStdStringArray& guids,
                                 CWizDocumentDataArray& arrayDocument);


#endif // WIZSERVICE_SYNC_H
