#ifndef CWIZOBJECTOPERATOR_H
#define CWIZOBJECTOPERATOR_H

#include <QObject>
#include "wizobject.h"

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
    void copyPersonalFolderToPersonalDB(const QString& sourceFolder, const QString& targetParentFolder,
                                        bool keepDocTime, bool keepTag, CWizObjectDataDownloaderHost* downloader);
    void copyPersonalFolderToGroupDB(const QString &sourceFolder, const WIZTAGDATA& targetParentTag,
                                     bool keepDocTime, CWizObjectDataDownloaderHost *downloader);
    void copyGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder, const QString& targetParentFolder,
                                     bool keepDocTime, CWizObjectDataDownloaderHost* downloader);
    void copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                  bool keepDocTime, CWizObjectDataDownloaderHost* downloader);
    //
    void moveGroupFolderToPersonalDB(const WIZTAGDATA& groupFolder, const QString& targetParentFolder,
                                         CWizObjectDataDownloaderHost* downloader);
    void moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder,
                                      CWizObjectDataDownloaderHost* downloader);
    void movePersonalFolderToPersonalDB(const QString& sourceFolder, const QString& targetParentFolder);
    void movePersonalFolderToGroupDB(const QString& sourceFolder, const WIZTAGDATA& targetFolder,
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
    void copyPersonalFolderToPersonalDB(const QString& childFolder, const QString& targetParentFolder);
    void copyPersonalFolderToGroupDB(const QString& childFolder, const WIZTAGDATA& targetParentTag);
    void copyGroupFolderToPersonalDB(const WIZTAGDATA& childFolder, const QString& targetParentFolder);
    void copyGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder);
    //
    void moveGroupFolderToPersonalDB(const WIZTAGDATA &childFolder, const QString &targetParentFolder);
    void moveGroupFolderToGroupDB(const WIZTAGDATA& sourceFolder, const WIZTAGDATA& targetFolder);
    void _movePersonalFolderToPersonalDB(const QString& childFolder, const QString& targetParentFolder);
    void movePersonalFolderToGroupDB(const QString& sourceFolder, const WIZTAGDATA& targetFolder);

    //
    int documentCount(CWizDatabase& db, const QString& personalFolder);
    int documentCount(CWizDatabase &db, const WIZTAGDATA &groupFolder);


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

    //
    QThread* m_thread;
    bool m_stop;
    int m_totoalCount;
    int m_counter;
};



#endif // CWIZOBJECTOPERATOR_H
