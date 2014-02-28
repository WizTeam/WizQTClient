#ifndef WIZDATABASE_H
#define WIZDATABASE_H

#include <QPointer>
#include <QMap>

#include "wizIndex.h"
#include "wizthumbindex.h"
#include "wizziwreader.h"
#include "wizSyncableDatabase.h"

class CWizDatabase;
class CWizFolder;
class CWizDocument;

class CWizDocument : public QObject
{
    Q_OBJECT

public:
    CWizDocument(CWizDatabase& db, const WIZDOCUMENTDATA& data);

    QString GUID() const { return m_data.strGUID; }

    bool isProtected() const { return m_data.nProtected; }
    bool encryptDocument() { return false; }

    QString GetAttachmentsPath(bool create);
    bool IsInDeletedItemsFolder();
    bool MoveDocument(CWizFolder* pFolder);
    bool AddTag(const WIZTAGDATA& dataTag);
    bool RemoveTag(const WIZTAGDATA& dataTag);
    QString GetMetaText();

private:
    CWizDatabase& m_db;
    WIZDOCUMENTDATA m_data;

public:
    Q_INVOKABLE void Delete();
    Q_INVOKABLE void PermanentlyDelete(void);
    Q_INVOKABLE void MoveTo(QObject* pFolder);
    Q_INVOKABLE bool UpdateDocument4(const QString& strHtml, const QString& strURL, int nFlags);
};


class CWizFolder : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString Location READ Location)

public:
    CWizFolder(CWizDatabase& db, const QString& strLocation);

    bool IsDeletedItems() const;
    bool IsInDeletedItems() const;

    // used for move and rename folder
    void MoveToLocation(const QString& strDestLocation);

    static bool CanMove(const QString& strSource, const QString& strDest);
    bool CanMove(CWizFolder* pSrc, CWizFolder* pDest) const;

    Q_INVOKABLE void Delete();
    Q_INVOKABLE void MoveTo(QObject* dest);
    Q_INVOKABLE QString Location() const { return m_strLocation; }
    //QObject* CreateDocument2(const QString& strTitle, const QString& strURL);

protected:
    CWizDatabase& m_db;
    CString m_strLocation;

Q_SIGNALS:
    void moveDocument(int nTotal, int nProcessed,
                      const QString& strOldLocation,
                      const QString& strNewLocation,
                      const WIZDOCUMENTDATA& data);
};

/*
 * High level operation layer of database
 */
class CWizDatabase
        : public CWizIndex
        , public CThumbIndex
        , public IWizSyncableDatabase
{
    Q_OBJECT

private:
    QString m_strUserId;
    QString m_strPassword;
    WIZDATABASEINFO m_info;
    QPointer<CWizZiwReader> m_ziwReader;

    bool m_bIsPersonal;
    QMap<QString, CWizDatabase*> m_mapGroups;

public:
    CWizDatabase();

    const WIZDATABASEINFO& info() { return m_info; }
    QString name() const { return m_info.name; }
    int permission() const { return m_info.nPermission; }

    // IWizSyncableDatabase interface implementations
    virtual QString GetUserId();
    virtual QString GetUserGUID();
    virtual QString GetPassword();

    virtual bool SaveLastSyncTime();
    virtual COleDateTime GetLastSyncTime();

    // versions
    virtual qint64 GetObjectVersion(const QString& strObjectType);
    virtual bool SetObjectVersion(const QString& strObjectType, qint64 nVersion);

    virtual qint64 GetObjectLocalVersion(const QString& strObjectGUID,
                                         const QString& strObjectType);
    virtual bool SetObjectLocalServerVersion(const QString& strObjectGUID,
                                             const QString& strObjectType,
                                             qint64 nVersion);

    // query
    virtual bool GetModifiedDeletedList(CWizDeletedGUIDDataArray& arrayData);
    virtual bool GetModifiedTagList(CWizTagDataArray& arrayData);
    virtual bool GetModifiedStyleList(CWizStyleDataArray& arrayData);
    virtual bool GetModifiedDocumentList(CWizDocumentDataArray& arrayData);
    virtual bool GetModifiedAttachmentList(CWizDocumentAttachmentDataArray& arrayData);
    virtual bool GetObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayObject);

    virtual bool DocumentFromGUID(const QString& strGUID,
                                  WIZDOCUMENTDATA& dataExists);

    // download
    virtual bool OnDownloadDeletedList(const CWizDeletedGUIDDataArray& arrayData);
    virtual bool OnDownloadTagList(const CWizTagDataArray& arrayData);
    virtual bool OnDownloadStyleList(const CWizStyleDataArray& arrayData);
    virtual bool OnDownloadAttachmentList(const CWizDocumentAttachmentDataArray& arrayData);
    virtual bool OnDownloadMessages(const CWizUserMessageDataArray& arrayData);
    virtual bool OnDownloadDocument(int part, const WIZDOCUMENTDATAEX& data);
    virtual bool UpdateObjectData(const QString& strObjectGUID,
                                  const QString& strObjectType,
                                  const QByteArray& stream);

    virtual bool IsObjectDataDownloaded(const QString& strGUID,
                                        const QString& strType);
    virtual bool SetObjectDataDownloaded(const QString& strGUID,
                                         const QString& strType,
                                         bool downloaded);

    virtual bool SetObjectServerDataInfo(const QString& strGUID,
                                         const QString& strType,
                                         COleDateTime& tServerDataModified,
                                         const QString& strServerMD5);

    // upload
    virtual bool InitDocumentData(const QString& strGUID,
                                  WIZDOCUMENTDATAEX& data,
                                  UINT part);

    virtual bool InitAttachmentData(const QString& strGUID,
                                    WIZDOCUMENTATTACHMENTDATAEX& data,
                                    UINT part);

    virtual bool OnUploadObject(const QString& strGUID,
                                const QString& strObjectType);

    // info and groups
    virtual void SetUserInfo(const WIZUSERINFO& info);
    virtual void SetKbInfo(const QString& strKBGUID, const WIZKBINFO& info);
    virtual bool OnDownloadGroups(const CWizGroupDataArray& arrayGroup);
    virtual bool OnDownloadBizs(const CWizBizDataArray& arrayBiz);
    virtual IWizSyncableDatabase* GetGroupDatabase(const WIZGROUPDATA& group);
    virtual void CloseGroupDatabase(IWizSyncableDatabase* pDatabase);

    virtual bool IsGroup();
    virtual bool IsGroupAdmin();
    virtual bool IsGroupSuper();
    virtual bool IsGroupEditor();
    virtual bool IsGroupAuthor();
    virtual bool IsGroupReader();
    virtual bool IsGroupOwner();

    virtual bool CanEditDocument(const WIZDOCUMENTDATA& data);
    virtual bool CanEditAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);

    // obsolote
    virtual qint64 GetObjectLocalServerVersion(const QString& strObjectGUID,
                                               const QString& strObjectType);

    virtual bool CreateConflictedCopy(const QString& strObjectGUID,
                                      const QString& strObjectType);

    virtual long GetLocalFlags(const QString& strObjectGUID,
                               const QString& strObjectType);
    virtual bool SetLocalFlags(const QString& strObjectGUID,
                               const QString& strObjectType,
                               long flags);

    // key-value api based syncing, user config, folders, biz users list etc.
    virtual void GetAccountKeys(CWizStdStringArray& arrayKey);
    virtual qint64 GetAccountLocalValueVersion(const QString& strKey);
    virtual void SetAccountLocalValue(const QString& strKey,
                                      const QString& strValue,
                                      qint64 nServerVersion,
                                      bool bSaveVersion);

    virtual void GetKBKeys(CWizStdStringArray& arrayKey);
    virtual qint64 GetLocalValueVersion(const QString& strKey);
    virtual QString GetLocalValue(const QString& strKey);
    virtual void SetLocalValueVersion(const QString& strKey,
                                      qint64 nServerVersion);
    virtual void SetLocalValue(const QString& strKey,
                               const QString& strValue,
                               qint64 nServerVersion,
                               bool bSaveVersion);

    virtual bool ProcessValue(const QString& strKey);

    virtual void GetAllBizUserIds(CWizStdStringArray& arrayText);

    virtual void ClearError();
    virtual void OnTrafficLimit(const QString& strErrorMessage);
    virtual void OnStorageLimit(const QString& strErrorMessage);
    virtual bool IsTrafficLimit();
    virtual bool IsStorageLimit();

    virtual bool setMeta(const QString& strSection, const QString& strKey, const QString& strValue);
    virtual QString meta(const QString& strSection, const QString& strKey);
    virtual void setBizGroupUsers(const QString& strkbGUID, const QString& strJson);

    // end interface implementations

    // helper methods for interface
    void SetObjectSyncTimeLine(int nDays);
    int GetObjectSyncTimeline();
    QString GetFolders();
    void SetFoldersPos(const QString& foldersPos, qint64 nVersion);
    void SetFolders(const QString& strFolders, qint64 nVersion, bool bSaveVersion);

    void SetBizUsers(const QString &strBizGUID, const QString& strUsers);
    bool loadBizUsersFromJson(const QString &strBizGUID,
                              const QString& strJsonRaw,
                              CWizBizUserDataArray& arrayUser);

public:
    bool Open(const QString& strUserId, const QString& strKbGUID = NULL);
    bool LoadDatabaseInfo();
    bool SetDatabaseInfo(const WIZDATABASEINFO& dbInfo);
    bool InitDatabaseInfo(const WIZDATABASEINFO& dbInfo);
    bool GetUserInfo(WIZUSERINFO& userInfo);

    // path resolve
    QString GetAccountPath() const;

    QString GetDataPath() const;
    QString GetIndexFileName() const;
    QString GetThumbFileName() const;
    QString GetDocumentsDataPath() const;
    QString GetAttachmentsDataPath() const;
    QString GetDocumentFileName(const QString& strGUID) const;
    QString GetAttachmentFileName(const QString& strGUID) const;
    QString GetAvatarPath() const;

    bool GetUserName(QString& strUserName);
    bool SetUserName(const QString& strUserName);

    QString getUserId() const { return m_strUserId; }
    //QString getPassword() const { return m_strPassword; }

    QString GetEncryptedPassword();
    bool GetPassword(CString& strPassword);
    bool SetPassword(const QString& strPassword, bool bSave = false);
    UINT GetPasswordFalgs();
    bool SetPasswordFalgs(UINT nFlags);

    bool SetUserCert(const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    bool GetUserCert(QString& strN, QString& stre, QString& strd, QString& strHint);

    //bool GetBizGroupInfo(QMap<QString, QString>& bizInfo);
    bool GetUserGroupInfo(CWizGroupDataArray& arrayGroup);
    bool SetUserGroupInfo(const CWizGroupDataArray& arrayGroup);
    bool SetUserBizInfo(const CWizBizDataArray& arrayBiz);
    bool GetUserBizInfo(bool bAllowEmptyBiz, CWizBizDataArray& arrayBiz);
    bool GetUserBizInfo(bool bAllowEmptyBiz, const CWizGroupDataArray& arrayAllGroup, CWizBizDataArray& arrayBiz);
    bool GetBizData(const QString& bizGUID, WIZBIZDATA& biz);
    bool GetGroupData(const QString& groupGUID, WIZGROUPDATA& group);
    //
    static bool IsEmptyBiz(const CWizGroupDataArray& arrayGroup, const QString& bizGUID);
    static bool GetOwnGroups(const CWizGroupDataArray& arrayAllGroup, CWizGroupDataArray& arrayOwnGroup);
    static bool GetJionedGroups(const CWizGroupDataArray& arrayAllGroup, CWizGroupDataArray& arrayJionedGroup);

    bool updateBizUser(const WIZBIZUSER& user);
    bool UpdateBizUsers(const CWizBizUserDataArray& arrayUser);
    bool updateMessage(const WIZMESSAGEDATA& msg);
    bool UpdateMessages(const CWizMessageDataArray& arrayMsg);
    bool UpdateDeletedGUID(const WIZDELETEDGUIDDATA& data);
    bool UpdateDeletedGUIDs(const CWizDeletedGUIDDataArray& arrayDeletedGUID);
    bool UpdateTag(const WIZTAGDATA& data);
    bool UpdateTags(const CWizTagDataArray &arrayTag);
    bool UpdateStyle(const WIZSTYLEDATA& data);
    bool UpdateStyles(const CWizStyleDataArray& arrayStyle);
    bool UpdateDocument(const WIZDOCUMENTDATAEX& data);
    bool UpdateDocuments(const std::deque<WIZDOCUMENTDATAEX>& arrayDocument);
    bool UpdateAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);
    bool UpdateAttachments(const CWizDocumentAttachmentDataArray& arrayAttachment);

    bool UpdateDocumentData(WIZDOCUMENTDATA& data, const QString& strHtml,
                            const QString& strURL, int nFlags);

    bool UpdateDocumentAbstract(const QString& strDocumentGUID);

    virtual bool UpdateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName);

    bool DeleteTagWithChildren(const WIZTAGDATA& data, bool bLog);
    bool DeleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data, bool bLog, bool bReset = true);

    bool IsDocumentDownloaded(const CString& strGUID);
    bool IsAttachmentDownloaded(const CString& strGUID);
    bool GetAllObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayData, int nTimeLine);
    bool UpdateSyncObjectLocalData(const WIZOBJECTDATA& data);

    CString GetObjectFileName(const WIZOBJECTDATA& data);

    bool GetDocumentsByTag(const WIZTAGDATA& tag, CWizDocumentDataArray& arrayDocument);

    bool LoadDocumentData(const QString& strDocumentGUID, QByteArray& arrayData);
    bool LoadAttachmentData(const CString& strDocumentGUID,
                            QByteArray& arrayData);
    bool LoadCompressedAttachmentData(const QString& strDocumentGUID,
                                      QByteArray& arrayData);
    bool SaveCompressedAttachmentData(const CString& strDocumentGUID,
                                      const QByteArray& arrayData);

    static CString GetRootLocation(const CString& strLocation);
    static CString GetLocationName(const CString& strLocation);
    static CString GetLocationDisplayName(const CString& strLocation);
    static bool GetAllRootLocations(const CWizStdStringArray& arrayAllLocation, \
                                    CWizStdStringArray& arrayLocation);
    static bool GetChildLocations(const CWizStdStringArray& arrayAllLocation, \
                                  const QString& strLocation, \
                                  CWizStdStringArray& arrayLocation);

    static bool IsInDeletedItems(const CString& strLocation);

    bool CreateDocumentAndInit(const CString& strHtml, \
                               const CString& strHtmlUrl, \
                               int nFlags, \
                               const CString& strTitle, \
                               const CString& strName, \
                               const CString& strLocation, \
                               const CString& strURL, \
                               WIZDOCUMENTDATA& data);

    bool AddAttachment(const WIZDOCUMENTDATA& document, \
                       const CString& strFileName, \
                       WIZDOCUMENTATTACHMENTDATA& dataRet);


    bool DocumentToTempHtmlFile(const WIZDOCUMENTDATA& document, \
                                QString& strTempHtmlFileName);

    bool IsFileAccessible(const WIZDOCUMENTDATA& document);

    // CWizZiwReader passthrough methods
    bool loadUserCert();
    const QString& userCipher() const { return m_ziwReader->userCipher(); }
    void setUserCipher(const QString& cipher) { m_ziwReader->setUserCipher(cipher); }
    QString userCipherHint() { return m_ziwReader->userCipherHint(); }
    void setSaveUserCipher(bool b) { m_ziwReader->setSaveUserCipher(b); }

public:
    Q_INVOKABLE QObject* GetDeletedItemsFolder();
    Q_INVOKABLE QObject* GetFolderByLocation(const QString& strLocation, bool create);

    //using CWizIndexBase::DocumentFromGUID;
    //Q_INVOKABLE QObject* DocumentFromGUID(const QString& strGUID);

Q_SIGNALS:
    void userInfoChanged();
    void groupsInfoDownloaded(const CWizGroupDataArray& arrayGroup);
    void bizInfoDownloaded(const CWizBizDataArray& arrayBiz);
    void databaseOpened(CWizDatabase* db, const QString& strKbGUID);
    void databaseRename(const QString& strKbGUID);
    void databasePermissionChanged(const QString& strKbGUID);
    void databaseBizChanged(const QString& strKbGUID);
    void updateError(const QString& msg);
    void processLog(const QString& msg);

    void folderPositionChanged();
};



#define WIZNOTE_MIMEFORMAT_TAGS             "wiznote/tags"
#define WIZNOTE_MIMEFORMAT_DOCUMENTS        "wiznote/documents"
#define WIZNOTE_MIMEFORMAT_FOLDER           "wiznote/folder"


#endif // WIZDATABASE_H
