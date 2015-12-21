#ifndef CWIZOBJECTOPERATORPRIVATE_H
#define CWIZOBJECTOPERATORPRIVATE_H

#include <QObject>
#include "wizobject.h"

class CWizUserSettings;
class CWizDatabase;
class CWizProgressDialog;
class CWizDatabaseManager;
class CWizObjectDownloaderHost;


struct OperatorData
{
    CWizDocumentDataArray arrayDocument;
    QString sourceFolder;
    WIZTAGDATA sourceTag;
    QString targetFolder;
    WIZTAGDATA targetTag;
    bool keepDocTime;
    bool keepTag;
    bool combineFolders;
};

class CWizDocumentOperatorPrivate : public QObject
{
    Q_OBJECT
public:
    CWizDocumentOperatorPrivate(OperatorData* data, QObject* parent = 0);
    ~CWizDocumentOperatorPrivate();

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

    void bindSignalsToProgressDialog(CWizProgressDialog* progress);

public slots:
    void stop();

signals:
    void progress(int max, int value);
    void newAction(const QString& action);
    void finished();    


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
    void combineSameNameGroupFolder(const WIZTAGDATA& parentTag, const WIZTAGDATA& childTag);
    QString getUniqueTagName(const WIZTAGDATA& parentTag, const WIZTAGDATA& tag);
    QString getUniqueFolderName(const QString& parentLocation, const QString& locationName);
    QString getUniqueFolderName(const WIZTAGDATA& parentTag, const WIZTAGDATA& sourceTag, bool combineFolder, const QString& exceptGUID = "");
    QString getUniqueFolderName(const WIZTAGDATA& parentTag, const QString& sourceFolder, bool combineFolder);

protected:
    OperatorData* m_data;
    //
    bool m_stop;
    int m_totoalCount;
    int m_counter;
};

//
int documentCount(CWizDatabase& db, const QString& personalFolder);
int documentCount(CWizDatabase &db, const WIZTAGDATA &groupFolder);

#endif // CWIZOBJECTOPERATORPRIVATE_H
