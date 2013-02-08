#ifndef WIZINDEX_H
#define WIZINDEX_H

#include "wizmisc.h"
#include "wizobject.h"
#include "cppsqlite3.h"
#include <map>
#include <set>

#define WIZ_DATABASE_VERSION "1.0"

class CWizIndexBase : public QObject
{
    Q_OBJECT

public:
    CWizIndexBase(void);
    ~CWizIndexBase(void);

    virtual bool Open(const CString& strFileName);
    bool IsOpened();
    void Close();
    bool CheckTable(const QString& strTableName);
    bool ExecSQL(const CString& strSQL);
    int Exec(const CString& strSQL);
    CppSQLite3Query Query(const CString& strSQL);
    bool HasRecord(const CString& strSQL);
    bool GetFirstRowFieldValue(const CString& strSQL, int nFieldIndex, CString& strValue);
    bool Repair(const QString& strDestFileName);

    QString kbGUID() const { return m_strKbGUID; }
    QString server() const { return m_strDatabaseServer; }
    QString name() const { return m_strName; }
    int permission() const { return m_nPermission; }
    QString GetDatabasePath() const { return m_strDatabasePath; }

    /* Raw query*/

    // tags
    bool GetAllTags(CWizTagDataArray& arrayTag);
    bool GetRootTags(CWizTagDataArray& arrayTag);
    bool GetChildTags(const CString& strParentTagGUID, CWizTagDataArray& arrayTag);
    bool GetAllChildTags(const CString& strParentTagGUID, CWizTagDataArray& arrayTag);

    bool TagFromGUID(const CString& strTagGUID, WIZTAGDATA& data);

    // styles
    bool GetStyles(CWizStyleDataArray& arrayStyle);
    bool StyleFromGUID(const CString& strStyleGUID, WIZSTYLEDATA& data);

    // metas
    bool GetMetas(CWizMetaDataArray& arrayMeta);

    // documents
    bool GetAllDocuments(CWizDocumentDataArray& arrayDocument);
    bool GetDocumentsBySQLWhere(const CString& strSQLWhere, CWizDocumentDataArray& arrayDocument);
    bool DocumentFromGUID(const CString& strDocumentGUID, WIZDOCUMENTDATA& data);

    // attachments
    bool GetAttachments(CWizDocumentAttachmentDataArray& arrayAttachment);
    bool AttachmentFromGUID(const CString& strAttachcmentGUID, WIZDOCUMENTATTACHMENTDATA& data);

protected:
    bool LogSQLException(const CppSQLite3Exception& e, const CString& strSQL);

    void BeginUpdate() { m_bUpdating = true; }
    void EndUpdate() { m_bUpdating = false; }
    bool IsUpdating() const { return m_bUpdating; }

    static CString FormatQuerySQL(const CString& strTableName,
                                  const CString& strFieldList);

    static CString FormatQuerySQL(const CString& strTableName,
                                  const CString& strFieldList,
                                  const CString& strWhere);

    static CString FormatInsertSQLFormat(const CString& strTableName,
                                         const CString& strFieldList,
                                         const CString& strParamList);

    static CString FormatUpdateSQLFormat(const CString& strTableName,
                                         const CString& strFieldList,
                                         const CString& strKey);

    static CString FormatDeleteSQLFormat(const CString& strTableName,
                                         const CString& strKey);

    static CString FormatQuerySQLByTime(const CString& strTableName,
                                        const CString& strFieldList,
                                        const CString& strFieldName,
                                        const COleDateTime& t);

    static CString FormatQuerySQLByTime2(const CString& strTableName,
                                         const CString& strFieldList,
                                         const CString& strInfoFieldName,
                                         const CString& strDataFieldName,
                                         const COleDateTime& t);

    static CString FormatQuerySQLByTime3(const CString& strTableName,
                                         const CString& strFieldList,
                                         const CString& strInfoFieldName,
                                         const CString& strDataFieldName,
                                         const CString& strParamFieldName,
                                         const COleDateTime& t);

    static CString FormatModifiedQuerySQL(const CString& strTableName,
                                          const CString& strFieldList);

    static CString FormatModifiedQuerySQL2(const CString& strTableName,
                                           const CString& strFieldList,
                                           int nCount);

    /* Basic operations */
    bool SQLToTagDataArray(const CString& strSQL,
                           CWizTagDataArray& arrayTag);

    bool SQLToStyleDataArray(const CString& strSQL,
                             CWizStyleDataArray& arrayStyle);

    bool SQLToMetaDataArray(const CString& strSQL,
                            CWizMetaDataArray& arrayMeta);

    bool SQLToDeletedGUIDDataArray(const CString& strSQL,
                                   CWizDeletedGUIDDataArray& arrayGUID);

    bool SQLToStringArray(const CString& strSQL, int nFieldIndex,
                          CWizStdStringArray& arrayString);

    bool SQLToDocumentParamDataArray(const CString& strSQL,
                                     CWizDocumentParamDataArray& arrayParam);

    bool SQLToDocumentDataArray(const CString& strSQL,
                                CWizDocumentDataArray& arrayDocument);

    virtual void InitDocumentExFields(CWizDocumentDataArray& arrayDocument,
                                      const CWizStdStringArray& arrayGUID,
                                      const std::map<CString, int>& mapDocumentIndex) = 0;

    void InitDocumentShareFlags(CWizDocumentDataArray& arrayDocument,
                                const CString& strDocumentGUIDs,
                                const std::map<CString, int>& mapDocumentIndex,
                                const CString& strTagName,
                                int nShareFlags);

    bool SQLToDocumentAttachmentDataArray(const CString& strSQL,
                                          CWizDocumentAttachmentDataArray& arrayAttachment);

public:
    bool CreateTagEx(const WIZTAGDATA& data);
    bool ModifyTagEx(const WIZTAGDATA& data);
    bool DeleteTagEx(const WIZTAGDATA& data);

    bool CreateStyleEx(const WIZSTYLEDATA& data);
    bool ModifyStyleEx(const WIZSTYLEDATA& data);
    bool DeleteStyleEx(const WIZSTYLEDATA& data);

    bool CreateDocumentEx(const WIZDOCUMENTDATA& data);
    bool ModifyDocumentInfoEx(const WIZDOCUMENTDATA& data);
    bool DeleteDocumentEx(const WIZDOCUMENTDATA& data);

    bool CreateAttachmentEx(const WIZDOCUMENTATTACHMENTDATA& data);
    bool ModifyAttachmentInfoEx(const WIZDOCUMENTATTACHMENTDATA& data);
    bool DeleteAttachmentEx(const WIZDOCUMENTATTACHMENTDATA& data);

protected:
    CppSQLite3DB m_db;
    QString m_strFileName;
    QString m_strDatabasePath;

    // all data query from database will fill this field to indicate which database query from.
    QString m_strKbGUID;
    QString m_strDatabaseServer;
    QString m_strName;
    int m_nPermission;

    bool m_bUpdating;

Q_SIGNALS:
    void tagCreated(const WIZTAGDATA& tag);
    void tagModified(const WIZTAGDATA& tagOld,
                     const WIZTAGDATA& tagNew);
    void tagDeleted(const WIZTAGDATA& tag);

    void styleCreated(const WIZSTYLEDATA& style);
    void styleModified(const WIZSTYLEDATA& styleOld,
                       const WIZSTYLEDATA& styleNew);
    void styleDeleted(const WIZSTYLEDATA& style);

    void documentCreated(const WIZDOCUMENTDATA& document);
    void documentModified(const WIZDOCUMENTDATA& documentOld,
                          const WIZDOCUMENTDATA& documentNew);
    void documentDeleted(const WIZDOCUMENTDATA& document);
    void documentDataModified(const WIZDOCUMENTDATA& document);
    void documentAbstractModified(const WIZDOCUMENTDATA& document);

    void attachmentCreated(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void attachmentModified(const WIZDOCUMENTATTACHMENTDATA& attachmentOld,
                            const WIZDOCUMENTATTACHMENTDATA& attachmentNew);
    void attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA& attachment);

    void folderCreated(const CString& strLocation);
    void folderDeleted(const CString& strLocation);
};


/*
Base class for database operation of sqlite layer
*/
class CWizIndex : public CWizIndexBase
{
    Q_OBJECT

public:
    CWizIndex(void);
    ~CWizIndex(void);

protected:
    CString m_strDeletedItemsLocation;

public:
    /* Tags related operations */
    bool CreateTag(const CString& strParentTagGUID, const CString& strName, \
                   const CString& strDescription, WIZTAGDATA& data);
    bool ModifyTag(WIZTAGDATA& data);
    bool DeleteTag(const WIZTAGDATA& data, bool bLog);
    QString getTagTreeText(const QString& strDocumentGUID);

    bool InsertDocumentTag(WIZDOCUMENTDATA& data, const CString& strTagGUID);
    bool SetDocumentTags(WIZDOCUMENTDATA& data, const CWizTagDataArray& arrayTag);
    bool SetDocumentTags(WIZDOCUMENTDATA& data, const CWizStdStringArray& arrayTagGUID);
    bool SetDocumentTagsText(WIZDOCUMENTDATA& data, const CString& strTagsText);

    bool DeleteDocumentTag(WIZDOCUMENTDATA& data, const CString& strTagGUID);
    bool DeleteDocumentTags(WIZDOCUMENTDATA& data);
    bool DeleteDocumentsTagsByLocation(const CString& strLocation);
    bool DeleteTagDocuments(const WIZTAGDATA& data);

    // Query by name
    bool TagByName(const CString& strName, WIZTAGDATA& data, const CString& strExceptGUID = "");
    bool TagByNameEx(const CString& strName, WIZTAGDATA& data);
    bool TagArrayByName(const CString& strName, CWizTagDataArray& arrayTag);

    // Query by document guid
    bool GetDocumentTagsNameStringArray(const CString& strDocumentGUID, CWizStdStringArray& arrayTagName);
    bool GetDocumentTags(const CString& strDocumentGUID, CWizTagDataArray& arrayTag);
    bool GetDocumentTags(const CString& strDocumentGUID, CWizStdStringArray& arrayTagGUID);
    CString GetDocumentTagsText(const CString& strDocumentGUID);
    CString GetDocumentTagNameText(const CString& strDocumentGUID);
    CString GetDocumentTagGUIDsString(const CString& strDocumentGUID);

    bool GetDocumentTagsDisplayNameStringArray(const CString& strDocumentGUID, CWizStdStringArray& arrayTagDisplayName);
    CString GetDocumentTagDisplayNameText(const CString& strDocumentGUID);

    bool GetAllParentsTagGUID(CString strTagGUID, CWizStdStringArray& arrayGUID);
    bool GetAllParentsTagGUID(CString strTagGUID, std::set<CString>& setGUID);

    // Query by time
    bool GetTagsByTime(const COleDateTime& t, CWizTagDataArray& arrayData);

    // Extend Query
    bool GetAllTagsDocumentCount(std::map<CString, int>& mapTagDocumentCount);
    bool GetModifiedTags(CWizTagDataArray& arrayData);

    // obsolete note share method
    static CString TagDisplayNameToName(const CString& strDisplayName);
    static CString TagNameToDisplayName(const CString& strName);

    // helper method
    bool TagsTextToTagArray(CString strText, CWizTagDataArray& arrayTag);

    /* Style related operations */
    bool CreateStyle(const CString& strName, const CString& strDescription,
                     COLORREF crTextColor, COLORREF crBackColor,
                     bool bTextBold, int nFlagIndex, WIZSTYLEDATA& data);
    bool ModifyStyle(WIZSTYLEDATA& data);
    bool DeleteStyle(const WIZSTYLEDATA& data, bool bLog);

    bool DeleteStyleDocuments(const WIZSTYLEDATA& data);

    // Query
    bool StyleByName(const CString& strName, WIZSTYLEDATA& data,
                     const CString& strExceptGUID = "");


    bool GetStylesByTime(const COleDateTime& t, CWizStyleDataArray& arrayData);

    bool GetModifiedStyles(CWizStyleDataArray& arrayData);

    /* Params related operations */
    bool GetDocumentParams(const CString& strDocumentGUID, \
                           CWizDocumentParamDataArray& arrayParam);

    bool GetDocumentParam(const CString& strDocumentGUID, \
                          CString strParamName, \
                          CString& strParamValue, \
                          const CString& strDefault = QString(), \
                          bool* pbParamExists = NULL);

    bool SetDocumentParam(const CString& strDocumentGUID, \
                          CString strParamName, \
                          CString strParamValue, \
                          bool bUpdateParamMD5);

    bool SetDocumentParam(WIZDOCUMENTDATA& data, \
                          CString strParamName, \
                          CString strParamValue, \
                          bool bUpdateParamMD5);

    bool SetDocumentParams(WIZDOCUMENTDATA& data,
                           const std::deque<WIZDOCUMENTPARAMDATA>& arrayParam);

    bool SetDocumentParams(WIZDOCUMENTDATA& data,
                           const CWizStdStringArray& arrayParam);

    CString CalDocumentParamInfoMD5(const WIZDOCUMENTDATA& data);
    CString CalDocumentParamInfoMD5(const CWizDocumentParamDataArray& arrayParam);
    bool UpdateDocumentParamMD5(WIZDOCUMENTDATA& data);
    bool UpdateDocumentParamMD5(const CString& strDocumentGUID);

    bool DeleteDocumentParams(const CString& strDocumentGUID);
    bool DeleteDocumentParam(const CString& strDocumentGUID, CString strParamName, bool bUpdateParamMD5);
    bool DeleteDocumentParamEx(const CString& strDocumentGUID, CString strParamNamePart);

    static bool StringArrayToDocumentParams(const CString& strDocumentGUID, \
                                             const CWizStdStringArray& arrayText, \
                                             std::deque<WIZDOCUMENTPARAMDATA>& arrayParam);

    /* Location(Folder) related operations */
    bool ChangeDocumentsLocation(const CString& strOldLocation, const CString& strNewLocation);
    static bool IsRootLocation(CString strLocation);
    static CString GetRootLocationName(CString strLocation);

    bool GetLocations(CWizStdStringArray& arrayLocation);
    bool GetLocationsNeedToBeSync(CWizStdStringArray& arrayLocation);
    bool GetLocations(CDocumentLocationArray& arrayLocation);

    void GetExtraFolder(CWizStdStringArray& arrayLocation);
    void SetExtraFolder(const CWizStdStringArray& arrayLocation);

    void GetDeletedFolder(CWizStdStringArray& arrayLocation);
    void SetDeletedFolder(const CWizStdStringArray& arrayLocation);

    void AddExtraFolder(const CString& strLocation);
    void LogDeletedFolder(const CString& strLocation);

    void RemoveFromExtraFolder(const CString& strLocation);
    void RemoveFromDeletedFolder(const CString& strLocation);

    bool IsLocationEmpty(const CString& strLocation);
    bool GetAllLocations(CWizStdStringArray& arrayLocation);
    bool GetChildLocations(const CString& strLocation, CWizStdStringArray& arrayLocation);

    // Extend
    bool GetSync(const CString& strLocation);
    bool SetSync(const CString& strLocation, bool bSync);

    /* Document related operations */
    virtual bool UpdateDocumentInfoMD5(WIZDOCUMENTDATA& data);
    bool UpdateDocumentsInfoMD5(CWizDocumentDataArray& arrayDocument);
    virtual bool UpdateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName);

    bool CreateDocument(const CString& strTitle, const CString& strName, \
                        const CString& strLocation, const CString& strURL, \
                        const CString& strAuthor, const CString& strKeywords, \
                        const CString& strType, const CString& strOwner, \
                        const CString& strFileType, const CString& strStyleGUID, \
                        int nIconIndex, int nSync, int nProtected, WIZDOCUMENTDATA& data);

    bool CreateDocument(const CString& strTitle, const CString& strName, \
                        const CString& strLocation, const CString& strURL, WIZDOCUMENTDATA& data);


    bool ModifyDocumentInfo(WIZDOCUMENTDATA& data);
    bool ModifyDocumentDateModified(WIZDOCUMENTDATA& data);
    bool ModifyDocumentDateAccessed(WIZDOCUMENTDATA& data);
    bool ModifyDocumentDataDateModified(WIZDOCUMENTDATA& data);
    bool ModifyDocumentReadCount(const WIZDOCUMENTDATA& data);

    bool DeleteDocument(const WIZDOCUMENTDATA& data, bool bLog);

    bool getDocumentsNoTag(CWizDocumentDataArray& arrayDocument, bool includeTrash);

    bool GetDocumentsByParamName(const CString& strLocation, bool bIncludeSubFolders, \
                                 CString strParamName, CWizDocumentDataArray& arrayDocument);
    bool GetDocumentsByParam(const CString& strLocation, bool bIncludeSubFolders, \
                             CString strParamName, const CString& strParamValue, \
                             CWizDocumentDataArray& arrayDocument, bool bIncludeDeletedItems = true);

    bool GetDocumentsByStyle(const CString& strLocation, \
                             const WIZSTYLEDATA& data, CWizDocumentDataArray& arrayDocument);

    bool DeleteDocumentsByLocation(const CString& strLocation);

    bool SetDocumentVersion(const CString& strDocumentGUID, __int64 nVersion);

    void InitDocumentExFields(CWizDocumentDataArray& arrayDocument,
                              const CWizStdStringArray& arrayGUID,
                              const std::map<CString, int>& mapDocumentIndex);

    void InitDocumentShareFlags(CWizDocumentDataArray& arrayDocument,
                                const CString& strDocumentGUIDs,
                                const std::map<CString, int>& mapDocumentIndex,
                                const CString& strTagName,
                                int nShareFlags);

    // Raw Query
    bool GetAllDocumentsTitle(CWizStdStringArray& arrayDocument);

    // Query by tag
    // includeTrash only take effect when strLocation is empty.
    bool GetDocumentsByTag(const CString& strLocation,
                           const WIZTAGDATA& data,
                           CWizDocumentDataArray& arrayDocument,
                           bool includeTrash);

    bool GetDocumentsByTags(bool bAnd, const CString& strLocation,
                            const CWizTagDataArray& arrayTag,
                            CWizDocumentDataArray& arrayDocument);

    // Query by document guid
    bool GetDocumentsByGUIDs(const CWizStdStringArray& arrayGUID, CWizDocumentDataArray& arrayDocument);

    // Query by location(folder)
    bool GetDocumentsByLocation(const CString& strLocation, CWizDocumentDataArray& arrayDocument);
    bool GetDocumentsByLocationIncludeSubFolders(const CString& strLocation, CWizDocumentDataArray& arrayDocument);

    bool GetDocumentsGUIDByLocation(const CString& strLocation, CWizStdStringArray& arrayGUID);

    bool GetAllLocationsDocumentCount(std::map<CString, int>& mapLocationDocumentCount);
    bool GetLocationDocumentCount(CString strLocation, bool bIncludeSubFolders, int& nDocumentCount);
    bool DocumentFromLocationAndName(const CString& strLocation, const CString& strName, WIZDOCUMENTDATA& data);

    // Query by time
    bool GetDocumentsByTime(const COleDateTime& t, CWizDocumentDataArray& arrayData);
    bool GetRecentDocuments(long nFlags, const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    bool GetRecentDocumentsCreated(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    bool GetRecentDocumentsModified(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    bool GetRecentDocumentsByCreatedTime(const COleDateTime& t, CWizDocumentDataArray& arrayDocument);

    // Extend
    bool GetModifiedDocuments(CWizDocumentDataArray& arrayData);
    bool GetModifiedDocuments(const CWizStdStringArray& arrayLocation, CWizDocumentDataArray& arrayData);

    int GetNeedToBeDownloadedDocumentCount();
    bool GetNeedToBeDownloadedDocuments(CWizDocumentDataArray& arrayData);

    // Helper
    CString CalDocumentInfoMD5(const WIZDOCUMENTDATA& data);

    /* Attachment related operations */
    void UpdateDocumentAttachmentCount(const CString& strDocumentGUID);

    bool CreateAttachment(const CString& strDocumentGUID, const CString& strName,
                          const CString& strURL, const CString& strDescription,
                          const CString& strDataMD5, WIZDOCUMENTATTACHMENTDATA& data);

    bool ModifyAttachmentInfo(WIZDOCUMENTATTACHMENTDATA& data);
    bool DeleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data, bool bLog);

    // Raw Query
    int GetDocumentAttachmentCount(const CString& strDocumentGUID);

    // Query by document guid
    bool GetDocumentAttachments(const CString& strDocumentGUID,
                                CWizDocumentAttachmentDataArray& arrayAttachment);

    // Query by time
    bool GetAttachmentsByTime(const COleDateTime& t,
                              CWizDocumentAttachmentDataArray& arrayData);

    // Extend
    bool GetModifiedAttachments(CWizDocumentAttachmentDataArray& arrayData);
    bool GetModifiedAttachments(const CWizStdStringArray& arrayLocation,
                                CWizDocumentAttachmentDataArray& arrayData);

    int GetNeedToBeDownloadedAttachmentCount();
    bool GetNeedToBeDownloadedAttachments(CWizDocumentAttachmentDataArray& arrayData);

    bool SetAttachmentDataDownloaded(const CString& strGUID, bool bDownloaded);

    // helper
    CString CalDocumentAttachmentInfoMD5(const WIZDOCUMENTATTACHMENTDATA& data);
    bool TitleExists(const CString& strLocation, CString strTitle);
    bool GetNextTitle(const CString& strLocation, CString& strTitle);

    /* Metas related operations */
    bool GetMetasByName(const CString& lpszMetaName, CWizMetaDataArray& arrayMeta);
    bool GetMeta(CString strMetaName, CString strKey, CString& strValue, \
                 const CString& strDefault = "", bool* pbMetaExists = NULL);
    CString GetMetaDef(const CString& lpszMetaName, const CString& lpszKey, const CString& strDef = "");
    bool SetMeta(CString strMetaName, CString strKey, const CString& lpszValue);
    __int64 GetMetaInt64(const CString& strMetaName, const CString& strKey, __int64 nDef);
    bool SetMetaInt64(const CString& strMetaName, const CString& strKey, __int64 n);

    bool deleteMetasByName(const QString& strMetaName);

    /* Deleted related operations */
    bool GetDeletedGUIDs(CWizDeletedGUIDDataArray& arrayGUID);
    bool GetDeletedGUIDs(WizObjectType eType, CWizDeletedGUIDDataArray& arrayGUID);

    bool DeleteDeletedGUID(const CString& strGUID);
    bool GetDeletedGUIDsByTime(const COleDateTime& t, CWizDeletedGUIDDataArray& arrayData);

    CString GetDeletedItemsLocation() const { return m_strDeletedItemsLocation; }

    bool LogDeletedGUID(const CString& strGUID, WizObjectType eType);
    bool LogDeletedGUIDs(const CWizStdStringArray& arrayGUID, WizObjectType eType);


	static CString GetLocationArraySQLWhere(const CWizStdStringArray& arrayLocation);

    bool ObjectExists(const CString& strGUID, const CString& strType, bool& bExists);
    bool DeleteObject(const CString& strGUID, const CString& strType, bool bLog);
    bool GetObjectTableInfo(const CString& strType, CString& strTableName, CString& strKeyFieldName);
    bool ModifyObjectVersion(const CString& strGUID, const CString& strType, __int64 nVersion);
    bool ModifyObjectModifiedTime(const CString& strGUID, const CString& strType, const COleDateTime& t);
    bool GetObjectModifiedTime(const CString& strGUID, const CString& strType, COleDateTime& t);

    // obsolete methods
    bool IsModified();
    bool IsDocumentModified();
    bool FilterDocumentsInDeletedItems(CWizDocumentDataArray& arrayDocument);
    bool GetCalendarEvents(const COleDateTime& tStart, const COleDateTime& tEnd, CWizDocumentDataArray& arrayDocument);
    bool GetAllRecurrenceCalendarEvents(CWizDocumentDataArray& arrayDocument);
    template <class TData> static void RemoveMultiElements(std::deque<TData>& arrayData);

    bool ObjectInReserved(const CString& strGUID, const CString& strType);

    enum WizObjectReservedInt { reserved1, reserved2, reserved3, reserved4};
    static CString GetReservedIntFieldName(WizObjectReservedInt e);
    bool SetObjectReservedInt(const CString& strGUID, const CString& strType, WizObjectReservedInt e, int val);
    bool GetObjectReservedInt(const CString& strGUID, const CString& strType, WizObjectReservedInt e, int& val);

    enum WizObjectReservedStr { reserved5, reserved6, reserved7, reserved8};
    static CString GetReservedStrFieldName(WizObjectReservedStr e);
    bool SetObjectReservedStr(const CString& strGUID, const CString& strType, WizObjectReservedStr e, const CString& val);
    bool GetObjectReservedStr(const CString& strGUID, const CString& strType, WizObjectReservedStr e, CString& val);

    static const WizObjectReservedInt DATA_DOWNLOADED_FIELD = reserved1;

    bool SetDocumentDataDownloaded(const CString& strGUID, bool bDownloaded);
    bool SetObjectDataDownloaded(const CString& strGUID, const CString& strType, bool bDownloaded);
    bool IsObjectDataDownloaded(const CString& strGUID, const CString& strType);

    /* Search related operations */
    bool setDocumentFTSEnabled(bool b);
    bool isDocumentFTSEnabled();
    bool setAllDocumentsSearchIndexed(bool b);
    bool getAllDocumentsNeedToBeSearchIndexed(CWizDocumentDataArray& arrayDocument);
    bool setDocumentSearchIndexed(const QString& strDocumentGUID, bool b);

    bool Search(const CString& strKeywords,
                const WIZSEARCHDATA& data,
                const CString& strLocation,
                bool bIncludeSubFolders,
                size_t nMaxCount,
                CWizDocumentDataArray& arrayDocument);

    bool Search(const CString& strKeywords,
                const CString& strLocation,
                bool bIncludeSubFolders,
                CWizDocumentDataArray& arrayDocument);

    bool SearchDocumentByTitle(const CString& strTitle,
                               const CString& strLocation,
                               bool bIncludeSubFolders,
                               size_t nMaxCount,
                               CWizDocumentDataArray& arrayDocument);


};


template <class TData>
inline void RemoveMultiElements(std::deque<TData>& arrayData)
{
    std::set<CString> setGUID;
    //
    for (size_t i = 0; i < arrayData.size(); )
    {
        const TData& data = arrayData[i];
        std::set<CString>::const_iterator it = setGUID.find(data.strGUID);
        if (it == setGUID.end())
        {
            setGUID.insert(data.strGUID);
            i++;
        }
        else
        {
            arrayData.erase(arrayData.begin() + i);
        }
    }
}


#endif // WIZINDEX_H
