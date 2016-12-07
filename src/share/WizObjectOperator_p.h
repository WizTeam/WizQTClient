#ifndef CWIZOBJECTOPERATORPRIVATE_H
#define CWIZOBJECTOPERATORPRIVATE_H

#include <QObject>
#include "WizObject.h"

class WizUserSettings;
class WizDatabase;
class WizProgressDialog;
class WizDatabaseManager;
class WizObjectDownloaderHost;


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

class WizDocumentOperatorPrivate : public QObject
{
    Q_OBJECT
public:
    WizDocumentOperatorPrivate(OperatorData* data, QObject* parent = 0);
    ~WizDocumentOperatorPrivate();

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
    void movePersonalFolderToGroupDB();

    void bindSignalsToProgressDialog(WizProgressDialog* progress);

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
int documentCount(WizDatabase& db, const QString& personalFolder);
int documentCount(WizDatabase &db, const WIZTAGDATA &groupFolder);

#endif // CWIZOBJECTOPERATORPRIVATE_H
