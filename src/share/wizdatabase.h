#ifndef WIZDATABASE_H
#define WIZDATABASE_H

#include "index.h"
#include "thumbindex.h"

class CWizDatabase;
class CWizFolder;
class CWizDocument;

class CWizDocument : public QObject
{
    Q_OBJECT
protected:
    CWizDatabase& m_db;
    WIZDOCUMENTDATA m_data;
public:
    CWizDocument(CWizDatabase& db, const WIZDOCUMENTDATA& data);
public:
    QString GUID() const { return m_data.strGUID; }
    CString GetAttachmentsPath(BOOL create);
    BOOL IsInDeletedItemsFolder();
    BOOL MoveDocument(CWizFolder* pFolder);
    BOOL AddTag(const WIZTAGDATA& dataTag);
    BOOL RemoveTag(const WIZTAGDATA& dataTag);
    CString GetMetaText();
public:
    Q_PROPERTY(QString GUID READ GUID)
public slots:
    bool UpdateDocument4(const QString& strHtml, const QString& strURL, int nFlags);
    void Delete();
    void PermanentlyDelete(void);
    void MoveTo(QObject* pFolder);
};

class CWizFolder : public QObject
{
    Q_OBJECT
public:
    CWizFolder(CWizDatabase& db, const CString& strLocation);
protected:
    CWizDatabase& m_db;
    CString m_strLocation;
public:
    BOOL IsDeletedItems() const;
    BOOL IsInDeletedItems() const;
    void MoveToLocation(const CString& strDestLocation);
    BOOL CanMove(const CString& strSrcLocation, const CString& strDestLocation) const;
    BOOL CanMove(CWizFolder* pSrc, CWizFolder* pDest) const;
public:
public:
    Q_PROPERTY(QString Location READ Location)
public slots:
    QString Location() const { return m_strLocation; }
    QObject* CreateDocument2(const QString& strTitle, const QString& strURL);
    void Delete();
    void MoveTo(QObject* dest);
};




class CWizDatabase
    : public CIndex
    , public CThumbIndex
{
    Q_OBJECT

public:
    CWizDatabase();

    QString GetUserId() const { return m_strUserId; }
    QString GetPassword() const { return m_strPassword; }

    QString GetAccountDataPath() const;
    QString GetUserDataDataPath() const;
    QString GetGroupDataDataPath() const;
    QString GetIndexFileName() const;
    QString GetThumbFileName() const;
    QString GetDocumentsDataPath() const;
    QString GetAttachmentsDataPath() const;

    BOOL Open(const QString& strUserId, const QString& strPassword);

private:
    QString m_strUserId;
    QString m_strPassword;

public:
    __int64 GetObjectVersion(const CString& strObjectName);
    BOOL SetObjectVersion(const CString& strObjectName, __int64 nVersion);

    BOOL UpdateDeletedGUID(const WIZDELETEDGUIDDATA& data, CWizSyncEvents* pEvents);
    BOOL UpdateTag(const WIZTAGDATA& data, CWizSyncEvents* pEvents);
    BOOL UpdateStyle(const WIZSTYLEDATA& data, CWizSyncEvents* pEvents);
    BOOL UpdateDocument(const WIZDOCUMENTDATAEX& data, CWizSyncEvents* pEvents);
    BOOL UpdateAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data, CWizSyncEvents* pEvents);
    //
    BOOL UpdateDeletedGUIDs(const std::deque<WIZDELETEDGUIDDATA>& arrayDeletedGUID, CWizSyncEvents* pEvents);
    BOOL UpdateTags(const std::deque<WIZTAGDATA>& arrayTag, CWizSyncEvents* pEvents);
    BOOL UpdateStyles(const std::deque<WIZSTYLEDATA>& arrayStyle, CWizSyncEvents* pEvents);
    BOOL UpdateDocuments(const std::deque<WIZDOCUMENTDATAEX>& arrayDocument, CWizSyncEvents* pEvents);
    BOOL UpdateAttachments(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayAttachment, CWizSyncEvents* pEvents);
    //
    bool UpdateDocumentData(WIZDOCUMENTDATA& data, const QString& strHtml, const QString& strURL, int nFlags);
    virtual BOOL UpdateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName);
    BOOL DeleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data, BOOL bLog);
    //
    BOOL IsDocumentDownloaded(const CString& strGUID);
    BOOL IsAttachmentDownloaded(const CString& strGUID);
    BOOL GetAllObjectsNeedToBeDownloaded(std::deque<WIZOBJECTDATA>& arrayData);
    BOOL UpdateSyncObjectLocalData(const WIZOBJECTDATA& data);
    //
    CString GetDocumentFileName(const CString& strGUID);
    CString GetAttachmentFileName(const CString& strGUID);
    //
    CString GetObjectFileName(const WIZOBJECTDATA& data);
    //
    BOOL GetAllRootLocations(CWizStdStringArray& arrayLocation);
    BOOL GetChildLocations(const CString& strLocation, CWizStdStringArray& arrayLocation);
    //
    BOOL GetDocumentsByTag(const WIZTAGDATA& tag, CWizDocumentDataArray& arrayDocument);
    //
    BOOL GetModifiedDeletedGUIDs(CWizDeletedGUIDDataArray& arrayData) { return GetDeletedGUIDs(arrayData); }
    BOOL LoadDocumentData(const CString& strDocumentGUID, QByteArray& arrayData);
    BOOL LoadAttachmentData(const CString& strDocumentGUID, QByteArray& arrayData);
    BOOL LoadCompressedAttachmentData(const CString& strDocumentGUID, QByteArray& arrayData);
    BOOL SaveCompressedAttachmentData(const CString& strDocumentGUID, const QByteArray& arrayData);
    //
    BOOL UpdateDocumentAbstract(const CString& strDocumentGUID);
    //
    static CString GetRootLocation(const CString& strLocation);
    static CString GetLocationName(const CString& strLocation);
    static CString GetLocationDisplayName(const CString& strLocation);
    static BOOL GetAllRootLocations(const CWizStdStringArray& arrayAllLocation, CWizStdStringArray& arrayLocation);
    static BOOL GetChildLocations(const CWizStdStringArray& arrayAllLocation, const CString& strLocation, CWizStdStringArray& arrayLocation);
    //
    BOOL IsInDeletedItems(const CString& strLocation);
    //
    using CIndex::GetDocumentsByTag;
    using CIndex::DocumentFromGUID;
    //
    BOOL CreateDocumentAndInit(const CString& strHtml, const CString& strHtmlUrl, int nFlags, const CString& strTitle, const CString& strName, const CString& strLocation, const CString& strURL, WIZDOCUMENTDATA& data);
    BOOL AddAttachment(const WIZDOCUMENTDATA& document, const CString& strFileName, WIZDOCUMENTATTACHMENTDATA& dataRet);
    //
    BOOL DeleteTagWithChildren(const WIZTAGDATA& data, BOOL bLog);

public:
    BOOL DocumentToTempHtmlFile(const WIZDOCUMENTDATA& document, CString& strTempHtmlFileName);

public slots:
    QObject* DocumentFromGUID(const QString& strGUID);
    QObject* GetFolderByLocation(const QString& strLocation, bool create);
    QObject* GetDeletedItemsFolder();
};



#define WIZNOTE_MIMEFORMAT_TAGS             "wiznote/tags"
#define WIZNOTE_MIMEFORMAT_DOCUMENTS        "wiznote/documents"
#define WIZNOTE_MIMEFORMAT_FOLDER           "wiznote/folder"


#endif // WIZDATABASE_H
