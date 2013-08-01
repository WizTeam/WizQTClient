#ifndef WIZDATABASE_H
#define WIZDATABASE_H

#include <QPointer>

#include "wizindex.h"
#include "wizthumbindex.h"
#include "wizziwreader.h"

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
    void MoveToLocation(const QString& strDestLocation);

    static bool CanMove(const QString& strSrcLocation,
                        const QString& strDestLocation);

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


class CWizDatabase
        : public CWizIndex
        , public CThumbIndex
        , public IWizSyncableDatabase
{
    Q_OBJECT

public:

    enum OpenMode {
        notOpened = 0,
        OpenPrivate,
        OpenGroup
    };

private:
    int m_nOpenMode;
    QString m_strUserId;
    QString m_strPassword;
    QPointer<CWizZiwReader> m_ziwReader;

public:
    CWizDatabase();

    // IWizSyncableDatabase implementation.
    virtual QString GetUserId() const;
    virtual QString GetPasssword() const;

    virtual WIZDATABASEINFO GetInfo() const;
    virtual bool SetInfo(const WIZDATABASEINFO& dbInfo);

    virtual qint64 GetObjectVersion(const QString& strObjectName);
    virtual bool SetObjectVersion(const QString& strObjectName,
                                  qint64 nVersion);

    virtual bool ModifyObjectVersion(const QString& strGUID,
                                     const QString& strType,
                                     qint64 nVersion);

    virtual bool GetBizGroupInfo(QMap<QString, QString>& bizInfo);  // kb_guid:name map
    virtual bool UpdateBizUsers(const CWizBizUserDataArray& arrayUser);

    virtual bool GetUserGroupInfo(CWizGroupDataArray& arrayGroup);
    virtual bool SetUserGroupInfo(const CWizGroupDataArray& arrayGroup);

    virtual bool GetModifiedMessages(CWizMessageDataArray& arrayMsg);
    virtual bool UpdateMessages(const CWizMessageDataArray& arrayMsg);

    virtual bool GetModifiedDeletedGUIDs(CWizDeletedGUIDDataArray& arrayData);
    virtual bool UpdateDeletedGUIDs(const CWizDeletedGUIDDataArray& arrayDeletedGUID);
    virtual bool DeleteDeletedGUID(const QString& strGUID);

    virtual bool GetModifiedTags(CWizTagDataArray& arrayData);
    virtual bool UpdateTags(const CWizTagDataArray &arrayTag);

    virtual bool GetModifiedStyles(CWizStyleDataArray& arrayData);
    virtual bool UpdateStyles(const CWizStyleDataArray& arrayStyle);

    virtual bool GetModifiedDocuments(CWizDocumentDataArray& arrayData);
    virtual bool UpdateDocument(const WIZDOCUMENTDATAEX& data);

    virtual bool GetModifiedAttachments(CWizDocumentAttachmentDataArray& arrayData);
    virtual bool UpdateAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);
    virtual bool UpdateAttachments(const CWizDocumentAttachmentDataArray& arrayAttachment);

    virtual bool SetObjectDataDownloaded(const QString& strGUID,
                                         const QString& strType,
                                         bool bDownloaded);

    virtual bool LoadDocumentData(const QString& strDocumentGUID,
                                  QByteArray& arrayData);

    virtual bool LoadCompressedAttachmentData(const QString& strDocumentGUID,
                                              QByteArray& arrayData);

public:
    // CWizZiwReader passthrough methods
    bool loadUserCert();
    const QString& userCipher() const { return m_ziwReader->userCipher(); }
    void setUserCipher(const QString& cipher) { m_ziwReader->setUserCipher(cipher); }
    QString userCipherHint() { return m_ziwReader->userCipherHint(); }
    void setSaveUserCipher(bool b) { m_ziwReader->setSaveUserCipher(b); }

    // obsolete
//    bool setDatabaseInfo(const QString& strKbGUID, const QString& strDatabaseServer,
//                         const QString& strName, int nPermission);

    bool setDatabaseInfo(const WIZDATABASEINFO& dbInfo);
    bool loadDatabaseInfo();

    bool openPrivate(const QString& strUserId, const QString& strPassword = QString());
    bool openGroup(const QString& strUserId, const QString& strGroupGUID);

    int openMode() const { return m_nOpenMode; }

    // path resolve
    QString GetAccountPath() const;
    QString GetUserDataPath() const;
    QString GetUserIndexFileName() const;
    QString GetUserThumbFileName() const;
    QString GetUserDocumentsDataPath() const;
    QString GetUserAttachmentsDataPath() const;
    QString GetGroupDataPath() const;
    QString GetGroupIndexFileName() const;
    QString GetGroupThumbFileName() const;
    QString GetGroupDocumentsDataPath() const;
    QString GetGroupAttachmentsDataPath() const;
    QString GetDocumentFileName(const CString& strGUID) const;
    QString GetAttachmentFileName(const CString& strGUID) const;
    QString GetAvatarPath() const;

    bool GetUserName(QString& strUserName);
    bool SetUserName(const QString& strUserName);

    QString getUserId() const { return m_strUserId; }
    QString getPassword() const { return m_strPassword; }

    QString GetEncryptedPassword();
    bool GetPassword(CString& strPassword);
    bool SetPassword(const QString& strOldPassword, const QString& strPassword);
    UINT GetPasswordFalgs();
    bool SetPasswordFalgs(UINT nFlags);

    bool SetUserCert(const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    bool GetUserCert(QString& strN, QString& stre, QString& strd, QString& strHint);

    bool setUserInfo(const WIZUSERINFO& userInfo);
    bool getUserInfo(WIZUSERINFO& userInfo);

    bool updateBizUser(const WIZBIZUSER& user);
    //bool updateBizUsers(const CWizBizUserDataArray& arrayUser);

    bool updateMessage(const WIZMESSAGEDATA& msg);

    bool UpdateDeletedGUID(const WIZDELETEDGUIDDATA& data);
    bool UpdateTag(const WIZTAGDATA& data);
    bool UpdateStyle(const WIZSTYLEDATA& data);

    bool UpdateDocuments(const std::deque<WIZDOCUMENTDATAEX>& arrayDocument);

    bool UpdateDocumentData(WIZDOCUMENTDATA& data, const QString& strHtml, const QString& strURL, int nFlags);
    bool UpdateDocumentAbstract(const QString& strDocumentGUID);

    virtual bool UpdateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName);

    bool DeleteTagWithChildren(const WIZTAGDATA& data, bool bLog);
    bool DeleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data, bool bLog, bool bReset = true);

    bool IsDocumentDownloaded(const CString& strGUID);
    bool IsAttachmentDownloaded(const CString& strGUID);
    bool GetAllObjectsNeedToBeDownloaded(std::deque<WIZOBJECTDATA>& arrayData);
    bool UpdateSyncObjectLocalData(const WIZOBJECTDATA& data);

    CString GetObjectFileName(const WIZOBJECTDATA& data);

    bool GetDocumentsByTag(const WIZTAGDATA& tag, CWizDocumentDataArray& arrayDocument);

    bool LoadAttachmentData(const CString& strDocumentGUID, QByteArray& arrayData);
    bool SaveCompressedAttachmentData(const CString& strDocumentGUID, const QByteArray& arrayData);

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

public:
    Q_INVOKABLE QObject* GetDeletedItemsFolder();
    Q_INVOKABLE QObject* GetFolderByLocation(const QString& strLocation, bool create);

    using CWizIndexBase::DocumentFromGUID;
    Q_INVOKABLE QObject* DocumentFromGUID(const QString& strGUID);

Q_SIGNALS:
    void databaseRename(const QString& strKbGUID);
    void databasePermissionChanged(const QString& strKbGUID);
    void updateError(const QString& msg);
    void processLog(const QString& msg);
};



#define WIZNOTE_MIMEFORMAT_TAGS             "wiznote/tags"
#define WIZNOTE_MIMEFORMAT_DOCUMENTS        "wiznote/documents"
#define WIZNOTE_MIMEFORMAT_FOLDER           "wiznote/folder"


#endif // WIZDATABASE_H
