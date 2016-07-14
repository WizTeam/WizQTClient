#ifndef CWIZOBJECTOPERATOR_H
#define CWIZOBJECTOPERATOR_H

#include <QObject>
#include "wizobject.h"

class CWizUserSettings;
class CWizDatabase;
class CWizProgressDialog;
class CWizDatabaseManager;

class CWizDocumentOperator : public QObject
{
    Q_OBJECT
public:
    CWizDocumentOperator(CWizDatabaseManager& dbMgr, QObject* parent = 0);

    void copyDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder,
                                     bool keepDocTime, bool keepTag, bool showProgressDialog = true);
    void copyDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag,
                                     bool keepDocTime, bool showProgressDialog = true);
    void moveDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder,
                                      bool waitForSync);
    void moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag,
                                      bool showProgressDialog = true);
    //
    void deleteDocuments(const CWizDocumentDataArray& arrayDocument);
    //
    void copyPersonalFolderToPersonalDB(const QString& sourceFolder, const QString& targetParentFolder,
                                        bool keepDocTime, bool keepTag, bool combineFolders, bool showProgressDialog = true);
    void copyPersonalFolderToGroupDB(const QString &sourceFolder, const WIZTAGDATA& targetParentTag,
                                     bool keepDocTime, bool combineFolders, bool showProgressDialog = true);
    void copyGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder, const QString& targetParentFolder,
                                     bool keepDocTime, bool combineFolders, bool showProgressDialog = true);
    void copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                  bool keepDocTime, bool combineFolders, bool showProgressDialog = true);
    //
    void moveGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder, const QString& targetParentFolder, bool combineFolders,
                                         bool showProgressDialog = true);
    void moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder, bool combineFolders,
                                      bool showProgressDialog = true);
    void movePersonalFolderToGroupDB(const QString& sourceFolder, const WIZTAGDATA& targetFolder, bool combineFolders,
                                         bool showProgressDialog = true);

private:
    CWizDatabaseManager* m_dbMgr;
};


//  ask user cipher to operate encrypted notes, after operation need to clear cipher.
bool WizAskUserCipherToOperateEncryptedNotes(const QString& sourceFolder, CWizDatabase& db);
bool WizAskUserCipherToOperateEncryptedNote(const CWizDocumentDataArray& arrayDocument, CWizDatabase& db);
void WizClearUserCipher(CWizDatabase& db, CWizUserSettings& settings);


#endif // CWIZOBJECTOPERATOR_H
