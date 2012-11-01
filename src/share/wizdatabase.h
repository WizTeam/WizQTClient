#ifndef WIZDATABASE_H
#define WIZDATABASE_H

#include "wizindex.h"
#include "wizthumbindex.h"

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
    QString GetAttachmentsPath(bool create);
    bool IsInDeletedItemsFolder();
    bool MoveDocument(CWizFolder* pFolder);
    bool AddTag(const WIZTAGDATA& dataTag);
    bool RemoveTag(const WIZTAGDATA& dataTag);
    QString GetMetaText();

private:
    CWizDatabase& m_db;
    WIZDOCUMENTDATA m_data;

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

    bool IsDeletedItems() const;
    bool IsInDeletedItems() const;
    void MoveToLocation(const CString& strDestLocation);
    bool CanMove(const CString& strSrcLocation, const CString& strDestLocation) const;
    bool CanMove(CWizFolder* pSrc, CWizFolder* pDest) const;

    Q_PROPERTY(QString Location READ Location)

protected:
    CWizDatabase& m_db;
    CString m_strLocation;

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

private:
    QString m_strUserId;
    QString m_strPassword;

public:
    CWizDatabase();

    QString GetUserId() const { return m_strUserId; }
    QString GetPassword2() const { return m_strPassword; }

    QString GetAccountDataPath() const;
    QString GetUserDataDataPath() const;
    QString GetGroupDataDataPath() const;
    QString GetIndexFileName() const;
    QString GetThumbFileName() const;
    QString GetDocumentsDataPath() const;
    QString GetAttachmentsDataPath() const;

    bool Open(const QString& strUserId, const QString& strPassword);

    __int64 GetObjectVersion(const CString& strObjectName);
    bool SetObjectVersion(const CString& strObjectName, __int64 nVersion);

    bool UpdateDeletedGUID(const WIZDELETEDGUIDDATA& data);
    bool UpdateTag(const WIZTAGDATA& data);
    bool UpdateStyle(const WIZSTYLEDATA& data);
    bool UpdateDocument(const WIZDOCUMENTDATAEX& data);
    bool UpdateAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);

    bool UpdateDeletedGUIDs(const std::deque<WIZDELETEDGUIDDATA>& arrayDeletedGUID);
    bool UpdateTags(const std::deque<WIZTAGDATA>& arrayTag);
    bool UpdateStyles(const std::deque<WIZSTYLEDATA>& arrayStyle);
    bool UpdateDocuments(const std::deque<WIZDOCUMENTDATAEX>& arrayDocument);
    bool UpdateAttachments(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayAttachment);

    bool UpdateDocumentData(WIZDOCUMENTDATA& data, const QString& strHtml, const QString& strURL, int nFlags);
    virtual bool UpdateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName);
    bool DeleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data, bool bLog);

    bool IsDocumentDownloaded(const CString& strGUID);
    bool IsAttachmentDownloaded(const CString& strGUID);
    bool GetAllObjectsNeedToBeDownloaded(std::deque<WIZOBJECTDATA>& arrayData);
    bool UpdateSyncObjectLocalData(const WIZOBJECTDATA& data);

    CString GetDocumentFileName(const CString& strGUID);
    CString GetAttachmentFileName(const CString& strGUID);

    CString GetObjectFileName(const WIZOBJECTDATA& data);

    bool GetDocumentsByTag(const WIZTAGDATA& tag, CWizDocumentDataArray& arrayDocument);

    bool GetModifiedDeletedGUIDs(CWizDeletedGUIDDataArray& arrayData) { return GetDeletedGUIDs(arrayData); }
    bool LoadDocumentData(const CString& strDocumentGUID, QByteArray& arrayData);
    bool LoadAttachmentData(const CString& strDocumentGUID, QByteArray& arrayData);
    bool LoadCompressedAttachmentData(const CString& strDocumentGUID, QByteArray& arrayData);
    bool SaveCompressedAttachmentData(const CString& strDocumentGUID, const QByteArray& arrayData);

    bool UpdateDocumentAbstract(const CString& strDocumentGUID);

    static CString GetRootLocation(const CString& strLocation);
    static CString GetLocationName(const CString& strLocation);
    static CString GetLocationDisplayName(const CString& strLocation);
    static bool GetAllRootLocations(const CWizStdStringArray& arrayAllLocation, \
                                    CWizStdStringArray& arrayLocation);
    static bool GetChildLocations(const CWizStdStringArray& arrayAllLocation, \
                                  const QString& strLocation, \
                                  CWizStdStringArray& arrayLocation);

    bool IsInDeletedItems(const CString& strLocation);

    using CIndex::GetDocumentsByTag;
    using CIndex::DocumentFromGUID;

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

    bool DeleteTagWithChildren(const WIZTAGDATA& data, bool bLog);

    bool DocumentToTempHtmlFile(const WIZDOCUMENTDATA& document, \
                                QString& strTempHtmlFileName);

Q_SIGNALS:
    void updateError(const QString& msg);
    void processLog(const QString& msg);

public Q_SLOTS:
    QObject* DocumentFromGUID(const QString& strGUID);
    QObject* GetFolderByLocation(const QString& strLocation, bool create);
    QObject* GetDeletedItemsFolder();
};



#define WIZNOTE_MIMEFORMAT_TAGS             "wiznote/tags"
#define WIZNOTE_MIMEFORMAT_DOCUMENTS        "wiznote/documents"
#define WIZNOTE_MIMEFORMAT_FOLDER           "wiznote/folder"


#endif // WIZDATABASE_H
