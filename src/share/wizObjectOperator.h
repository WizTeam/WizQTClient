#ifndef CWIZOBJECTOPERATOR_H
#define CWIZOBJECTOPERATOR_H

#include <QObject>
#include "wizobject.h"

class CWizUserSettings;
class CWizDatabase;
class CWizProgressDialog;
class CWizDatabaseManager;
class CWizObjectDataDownloaderHost;

class CWizDocumentOperator : public QObject
{
    Q_OBJECT
public:
    CWizDocumentOperator(CWizDatabaseManager& dbMgr, QObject* parent = 0);
    ~CWizDocumentOperator();

    /**
      IMPORTANT!!! :   All of these copy or move functions could only call once, and the operator object will destroy automatic
     */
    void copyDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder,
                                     bool keepDocTime, bool keepTag, CWizObjectDataDownloaderHost* downloader);
    void copyDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag,
                                     bool keepDocTime, CWizObjectDataDownloaderHost* downloader);
    void moveDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder,
                                      CWizObjectDataDownloaderHost* downloader);
    void moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag,
                                      CWizObjectDataDownloaderHost* downloader);
    //
    void deleteDocuments(const CWizDocumentDataArray& arrayDocument);
    //
    void copyPersonalFolderToPersonalDB(const QString& sourceFolder, const QString& targetParentFolder,
                                        bool keepDocTime, bool keepTag, bool combineFolders, CWizObjectDataDownloaderHost* downloader);
    void copyPersonalFolderToGroupDB(const QString &sourceFolder, const WIZTAGDATA& targetParentTag,
                                     bool keepDocTime, bool combineFolders, CWizObjectDataDownloaderHost *downloader);
    void copyGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder, const QString& targetParentFolder,
                                     bool keepDocTime, bool combineFolders, CWizObjectDataDownloaderHost* downloader);
    void copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                  bool keepDocTime, bool combineFolders, CWizObjectDataDownloaderHost* downloader);
    //
    void moveGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder, const QString& targetParentFolder, bool combineFolders,
                                         CWizObjectDataDownloaderHost* downloader);
    void moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder, bool combineFolders,
                                      CWizObjectDataDownloaderHost* downloader);
    void movePersonalFolderToPersonalDB(const QString& sourceFolder, const QString& targetParentFolder, bool combineFolder);
    void movePersonalFolderToGroupDB(const QString& sourceFolder, const WIZTAGDATA& targetFolder, bool combineFolders,
                                         CWizObjectDataDownloaderHost* downloader);


    void bindSignalsToProgressDialog(CWizProgressDialog* progress);

public slots:
    void stop();

signals:
    void progress(int max, int value);
    void newAction(const QString& action);
    void finished();

private slots:
    void copyDocumentToPersonalFolder();
    void copyDocumentToGroupFolder();
    void moveDocumentToPersonalFolder();
    void moveDocumentToGroupFolder();
    //
    void deleteDocuments();
    //
    void copyPersonalFolderToPersonalDB();
    void copyPersonalFolderToGroupDB();
    void copyGroupFolderToPersonalDB();
    void copyGroupFolderToGroupDB();
    //
    void moveGroupFolderToPersonalDB();
    void moveGroupFolderToGroupDB();
    void movePersonalFolderToPersonalDB();
    void movePersonalFolderToGroupDB();


private:
    void copyDocumentToPersonalFolder(const WIZDOCUMENTDATA& doc);
    void copyDocumentToGroupFolder(const WIZDOCUMENTDATA& doc);
    void moveDocumentToPersonalFolder(const WIZDOCUMENTDATA& doc);
    void moveDocumentToGroupFolder(const WIZDOCUMENTDATA& doc);    
    //
    void copyPersonalFolderToPersonalDB(const QString& childFolder, const QString& targetParentFolder,
                                        const QString& targetFolderName);
    void copyPersonalFolderToGroupDB(const QString& childFolder, const WIZTAGDATA& targetParentTag,
                                     const QString& targetTagName);
    void copyGroupFolderToPersonalDB(const WIZTAGDATA& childFolder, const QString& targetParentFolder);
    void copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder);
    //
    void moveGroupFolderToPersonalDB(const WIZTAGDATA &childFolder, const QString &targetParentFolder);
    void moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetParentTag);
    void _movePersonalFolderToPersonalDB(const QString& childFolder, const QString& targetParentFolder,
                                         const QString& targetFolderName);
    void movePersonalFolderToGroupDB(const QString& sourceFolder, const WIZTAGDATA& targetParentTag,
                                     const QString& targetTagName);

    //
    int documentCount(CWizDatabase& db, const QString& personalFolder);
    int documentCount(CWizDatabase &db, const WIZTAGDATA &groupFolder);

    //
    void combineSameNameGroupFolder(const WIZTAGDATA& parentTag, const WIZTAGDATA& childTag);
    QString getUniqueTagName(const WIZTAGDATA& parentTag, const WIZTAGDATA& tag);
    QString getUniqueFolderName(const QString& parentLocation, const QString& locationName);
    QString getUniqueFolderName(const WIZTAGDATA& parentTag, const WIZTAGDATA& sourceTag, bool combineFolder, const QString& exceptGUID = "");
    QString getUniqueFolderName(const WIZTAGDATA& parentTag, const QString& sourceFolder, bool combineFolder);

protected:
    CWizDocumentDataArray m_arrayDocument;
    QString m_sourceFolder;
    WIZTAGDATA m_sourceTag;
    QString m_targetFolder;
    WIZTAGDATA m_targetTag;
    bool m_keepDocTime;
    bool m_keepTag;
    CWizDatabaseManager& m_dbMgr;
    CWizObjectDataDownloaderHost* m_downloader;
    bool m_combineFolders;
    //
    QThread* m_thread;
    bool m_stop;
    int m_totoalCount;
    int m_counter;
};


//  ask user cipher to operate encrypted notes, after operation need to clear cipher.
bool WizAskUserCipherToOperateEncryptedNotes(const QString& sourceFolder, CWizDatabase& db);
bool WizAskUserCipherToOperateEncryptedNote(const CWizDocumentDataArray& arrayDocument, CWizDatabase& db);
void WizClearUserCipher(CWizDatabase& db, CWizUserSettings& settings);


#endif // CWIZOBJECTOPERATOR_H
