#ifndef WIZDATABASEMANAGER_H
#define WIZDATABASEMANAGER_H

#include <QObject>
#include <QPointer>
#include <QMap>
#include <QMutex>
#include <deque>

class QString;

struct WIZTAGDATA;
struct WIZSTYLEDATA;
struct WIZDOCUMENTDATA;
struct WIZDOCUMENTATTACHMENTDATA;
struct WIZGROUPDATA;
class CWizDatabase;
struct WIZDATABASEINFO;

typedef std::deque<WIZGROUPDATA> CWizGroupDataArray;

class CWizDatabaseManager : public QObject
{
    Q_OBJECT

public:
    CWizDatabaseManager(const QString& strUserId);
    ~CWizDatabaseManager();

    static CWizDatabaseManager* instance();

    // open private db if strKbGUID is empty, otherwise open groups db
    bool open(const QString& strKbGUID = "");
    bool openWithInfo(const QString& strKbGUID, const WIZDATABASEINFO* pInfo);
    bool openAll();
    bool isOpened(const QString& strKbGUID = "");

    // get db reference by strKbGUID (include private), or null to get private
    CWizDatabase& db(const QString& strKbGUID = "");
    CWizDatabase& addDb(const QString& strKbGUID, const WIZDATABASEINFO& info);

    // get all group guid list, exclude private
    //void Guids(QStringList& strings);
    // get group db count, exclude private
    int count();
    // get group db reference by index
    CWizDatabase& at(int i);

    //bool removeKb(const QString& strKbGUID);

    bool close(const QString& strKbGUID = NULL, bool bNotify = true);
    void closeAll();

private:
    QMutex m_mutex;
    QString m_strUserId;
    QPointer<CWizDatabase> m_dbPrivate;
    QMap<QString, CWizDatabase*> m_mapGroups;

    void initSignals(CWizDatabase* db);

private Q_SLOTS:
    void on_groupDatabaseOpened(CWizDatabase* db, const QString& strKbGUID);
    void on_groupsInfoDownloaded(const CWizGroupDataArray& arrayGroups);

Q_SIGNALS:
    void databaseOpened(const QString& strKbGUID);
    void databaseClosed(const QString& strKbGUID);
    void databaseRename(const QString& strKbGUID);
    void databasePermissionChanged(const QString& strKbGUID);
    void databaseBizchanged(const QString&);

    // CWizDatabase passthrough signals
    void tagCreated(const WIZTAGDATA& tag);
    void tagModified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    void tagDeleted(const WIZTAGDATA& tag);
    void styleCreated(const WIZSTYLEDATA& style);
    void styleModified(const WIZSTYLEDATA& styleOld, const WIZSTYLEDATA& styleNew);
    void styleDeleted(const WIZSTYLEDATA& style);
    void documentCreated(const WIZDOCUMENTDATA& document);
    void documentModified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew);
    void documentDeleted(const WIZDOCUMENTDATA& document);
    void documentDataModified(const WIZDOCUMENTDATA& document);
    void documentAbstractModified(const WIZDOCUMENTDATA& document);
    void documentTagModified(const WIZDOCUMENTDATA& document);
    void attachmentCreated(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void attachmentModified(const WIZDOCUMENTATTACHMENTDATA& attachmentOld, const WIZDOCUMENTATTACHMENTDATA& attachmentNew);
    void attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void folderCreated(const QString& strLocation);
    void folderDeleted(const QString& strLocation);
    void folderPositionChanged();

};

#endif // WIZDATABASEMANAGER_H
