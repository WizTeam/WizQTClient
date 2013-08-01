#ifndef WIZKMXMLRPC_H
#define WIZKMXMLRPC_H

struct IWizKMSyncEvents;
struct  IWizSyncableDatabase;

bool WizSyncDatabase(IWizKMSyncEvents* pEvents, IWizSyncableDatabase* pDatabase, bool bUseWizServer, bool bBackground);

#endif // WIZKMXMLRPC_H
