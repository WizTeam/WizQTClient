#ifndef WIZINDEX_H
#define WIZINDEX_H

#include <map>
#include <set>

#include "wizIndexBase.h"

#define WIZ_DATABASE_VERSION "1.0"
#define WIZ_NO_OBSOLETE


/*
 * Lower operations of sqlite database
 */
class CWizIndex
        : public CWizIndexBase
{
    Q_OBJECT

public:
    CWizIndex(void);

protected:
    CString m_strDeletedItemsLocation;

public:
    /* Message related operations */
    bool createMessage(const WIZMESSAGEDATA& data);
    bool getAllMessages(CWizMessageDataArray& arrayMsg);
    bool getAllMessageSenders(CWizStdStringArray& arraySender);
    bool getLastestMessages(CWizMessageDataArray& arrayMsg, int nMax = 200);
    bool setMessageReadStatus(const WIZMESSAGEDATA& msg);
    bool setMessageDeleteStatus(const WIZMESSAGEDATA& msg);
    bool getModifiedMessages(CWizMessageDataArray&  arrayMsg);
    bool getUnreadMessages(CWizMessageDataArray& arrayMsg);
    bool modifyMessageLocalChanged(const WIZMESSAGEDATA& msg);
    int getUnreadMessageCount();

    /* Tags related operations */
    bool CreateTag(const CString& strParentTagGUID, const CString& strName, \
                   const CString& strDescription, WIZTAGDATA& data);
    bool ModifyTag(WIZTAGDATA& data);
    bool DeleteTag(const WIZTAGDATA& data, bool bLog, bool bReset = true);
    bool ModifyTagPosition(const WIZTAGDATA& data);


    bool SetDocumentTags(WIZDOCUMENTDATA& data, const CWizTagDataArray& arrayTag);

    bool InsertDocumentTag(WIZDOCUMENTDATA& data,
                           const CString& strTagGUID,
                           bool bReset = true);

    bool DeleteDocumentTags(WIZDOCUMENTDATA& data, bool bReset = true);

    bool SetDocumentTags(WIZDOCUMENTDATA& data,
                         const CWizStdStringArray& arrayTagGUID,
                         bool bReset = true);

    bool SetDocumentTagsText(WIZDOCUMENTDATA& data, const CString& strTagsText);

    bool DeleteDocumentTag(WIZDOCUMENTDATA& data, const CString& strTagGUID);

    bool DeleteDocumentsTagsByLocation(const CString& strLocation);
    bool DeleteTagDocuments(const WIZTAGDATA& data, bool bReset);

    // Query tags by name
    bool TagByName(const CString& strName, CWizTagDataArray& arrayTag, const CString& strExceptGUID = "");
    bool TagByNameEx(const CString& strName, WIZTAGDATA& data);
    bool TagArrayByName(const CString& strName, CWizTagDataArray& arrayTag);

    // Query tags by document guid
    bool GetDocumentTagsNameStringArray(const CString& strDocumentGUID, CWizStdStringArray& arrayTagName);
    int GetDocumentTagCount(const CString& strDocumentGUID);
    bool GetDocumentTags(const CString& strDocumentGUID, CWizTagDataArray& arrayTag);
    bool GetDocumentTags(const CString& strDocumentGUID, CWizStdStringArray& arrayTagGUID);
    CString GetDocumentTagsText(const CString& strDocumentGUID);
    CString GetDocumentTagNameText(const CString& strDocumentGUID);
    CString GetDocumentTagGUIDsString(const CString& strDocumentGUID);

    bool GetDocumentTagsDisplayNameStringArray(const CString& strDocumentGUID, CWizStdStringArray& arrayTagDisplayName);
    CString GetDocumentTagDisplayNameText(const CString& strDocumentGUID);
    QString GetDocumentTagTreeDisplayString(const QString& strDocumentGUID);

    bool GetAllParentsTagGUID(CString strTagGUID, CWizStdStringArray& arrayGUID);
    bool GetAllParentsTagGUID(CString strTagGUID, std::set<CString>& setGUID);

    // Query tags by time
    bool GetTagsByTime(const COleDateTime& t, CWizTagDataArray& arrayData);

    // Tags extend Query

    bool GetModifiedTags(CWizTagDataArray& arrayData);

    // tag helper method

    // Indicate group document location path
    QString getTagTreeText(const QString& strDocumentGUID);

    // convert user input text to tag data structure
    bool TagsTextToTagArray(CString strText, CWizTagDataArray& arrayTag);

    // deprecated note share tags, still avaiable, but will be removed from future release
    static CString TagDisplayNameToName(const CString& strDisplayName);
    static CString TagNameToDisplayName(const CString& strName);

    /* Style related operations */
    bool CreateStyle(const CString& strName, const CString& strDescription,
                     COLORREF crTextColor, COLORREF crBackColor,
                     bool bTextBold, int nFlagIndex, WIZSTYLEDATA& data);
    bool ModifyStyle(WIZSTYLEDATA& data);
    bool DeleteStyle(const WIZSTYLEDATA& data, bool bLog, bool bReset = true);

    bool DeleteStyleDocuments(const WIZSTYLEDATA& data, bool bReset);

    // Query style by name
    bool StyleByName(const CString& strName, WIZSTYLEDATA& data,
                     const CString& strExceptGUID = "");

    // Query style by time
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

    bool SetDocumentParam(const QString& strDocumentGUID, \
                          const QString& strParamName, \
                          const QString& strParamValue, \
                          bool bUpdateParamMD5);

    bool SetDocumentParam(WIZDOCUMENTDATA& data, \
                          const QString &strParamName, \
                          const QString &strParamValue, \
                          bool bUpdateParamMD5);

    // reset flag inicate need update param md5 or reset nVersion = -1
    bool SetDocumentParams(WIZDOCUMENTDATA& data,
                           const CWizDocumentParamDataArray& arrayParam,
                           bool bReset = true);

    bool SetDocumentParams(WIZDOCUMENTDATA& data,
                           const CWizStdStringArray& arrayParam,
                           bool bReset = true);

    bool DeleteDocumentParams(const QString& strDocumentGUID,
                              bool bReset = true);

    CString CalDocumentParamInfoMD5(const WIZDOCUMENTDATA& data);
    CString CalDocumentParamInfoMD5(const CWizDocumentParamDataArray& arrayParam);
    bool UpdateDocumentParamMD5(WIZDOCUMENTDATA& data);
    bool UpdateDocumentParamMD5(const CString& strDocumentGUID);

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
    void AddExtraFolder(const QString& strLocation);
    void DeleteExtraFolder(const QString& strLocation);

    bool UpdateLocation(const QString& strOldLocation, const QString& strNewLocation);

    bool IsLocationEmpty(const CString& strLocation);
    bool GetAllLocations(CWizStdStringArray& arrayLocation);
    bool GetAllChildLocations(const CString& strLocation, CWizStdStringArray& arrayLocation);
    void GetAllLocationsWithExtra(CWizStdStringArray& arrayLocation);

    // Extend
    bool GetSync(const CString& strLocation);
    bool SetSync(const CString& strLocation, bool bSync);

    /* Document related operations */
    virtual bool UpdateDocumentInfoMD5(WIZDOCUMENTDATA& data);
    bool UpdateDocumentsInfoMD5(CWizDocumentDataArray& arrayDocument);
    virtual bool UpdateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName, bool notifyDataModify = true);

    bool CreateDocument(const CString& strTitle, const CString& strName, \
                        const CString& strLocation, const CString& strURL, \
                        const CString& strAuthor, const CString& strKeywords, \
                        const CString& strType, const CString& strOwner, \
                        const CString& strFileType, const CString& strStyleGUID, \
                        int nIconIndex, int nSync, int nProtected, WIZDOCUMENTDATA& data);

    bool CreateDocument(const CString& strTitle, const CString& strName, \
                        const CString& strLocation, const CString& strURL, WIZDOCUMENTDATA& data);


    bool ModifyDocumentInfo(WIZDOCUMENTDATA& data, bool bReset = true);
    bool ModifyDocumentDateModified(WIZDOCUMENTDATA& data);
    bool ModifyDocumentDateAccessed(WIZDOCUMENTDATA& data);
    bool ModifyDocumentDataDateModified(WIZDOCUMENTDATA& data);
    bool ModifyDocumentReadCount(const WIZDOCUMENTDATA& data);
    bool ModifyDocumentLocation(WIZDOCUMENTDATA& data);

    bool DeleteDocument(const WIZDOCUMENTDATA& data, bool bLog);

    bool getDocumentsNoTag(CWizDocumentDataArray& arrayDocument, bool includeTrash = false);

    bool getLastestDocuments(CWizDocumentDataArray& arrayDocument, int nMax = 1000);

    bool GetDocumentsByParamName(const CString& strLocation, bool bIncludeSubFolders, \
                                 CString strParamName, CWizDocumentDataArray& arrayDocument);
    bool GetDocumentsByParam(const CString& strLocation, bool bIncludeSubFolders, \
                             CString strParamName, const CString& strParamValue, \
                             CWizDocumentDataArray& arrayDocument, bool bIncludeDeletedItems = true);

    bool GetDocumentsByStyle(const CString& strLocation, \
                             const WIZSTYLEDATA& data, CWizDocumentDataArray& arrayDocument);

    bool DeleteDocumentsByLocation(const CString& strLocation);

    bool SetDocumentVersion(const CString& strDocumentGUID, qint64 nVersion);

    void InitDocumentExFields(CWizDocumentDataArray& arrayDocument,
                              const CWizStdStringArray& arrayGUID,
                              const std::map<CString, int>& mapDocumentIndex);

    void InitDocumentShareFlags(CWizDocumentDataArray& arrayDocument,
                                const CString& strDocumentGUIDs,
                                const std::map<CString, int>& mapDocumentIndex,
                                const CString& strTagName,
                                int nShareFlags);

    bool getGroupUnreadDocuments(CWizDocumentDataArray& arrayDocument);
    int getGroupUnreadDocumentCount();
    void setGroupDocumentsReaded();
    CString KbGUIDToSQL();

    // Raw Query
    bool GetAllDocumentsTitle(CWizStdStringArray& arrayDocument);

    // Query by tag
    // includeTrash only take effect when strLocation is empty.
    bool GetDocumentsByTag(const CString& strLocation,
                           const WIZTAGDATA& data,
                           CWizDocumentDataArray& arrayDocument,
                           bool includeTrash);

    bool GetDocumentsSizeByTag(const WIZTAGDATA& data, int& size);
    bool GetAllDocumentsSizeByTag(const WIZTAGDATA& data, int& size);

    bool GetDocumentsByTags(bool bAnd, const CString& strLocation,
                            const CWizTagDataArray& arrayTag,
                            CWizDocumentDataArray& arrayDocument);

    // Query by document guid
    bool GetDocumentsByGUIDs(const CWizStdStringArray& arrayGUID, CWizDocumentDataArray& arrayDocument);

    // Query by location(folder)
    bool GetDocumentsCountByLocation(const CString& strLocation,
                                    int& count,
                                    bool bIncludeSubFolders = false);

    bool GetDocumentsByLocation(const CString& strLocation,
                                CWizDocumentDataArray& arrayDocument,
                                bool bIncludeSubFolders = false);

    //bool GetDocumentsByLocationIncludeSubFolders(const CString& strLocation, CWizDocumentDataArray& arrayDocument);

    bool GetDocumentsGUIDByLocation(const CString& strLocation, CWizStdStringArray& arrayGUID);


    bool DocumentFromLocationAndName(const CString& strLocation, const CString& strName, WIZDOCUMENTDATA& data);

    // Query by time
    bool GetDocumentsByTime(const QDateTime &t, CWizDocumentDataArray& arrayData);
    bool GetRecentDocuments(long nFlags, const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    bool GetRecentDocumentsCreated(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    bool GetRecentDocumentsModified(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    bool GetRecentDocumentsByCreatedTime(const COleDateTime& t, CWizDocumentDataArray& arrayDocument);
    bool GetRecentDocumentsByModifiedTime(const COleDateTime& t, CWizDocumentDataArray& arrayDocument);
    bool GetRecentDocumentsByAccessedTime(const COleDateTime& t, CWizDocumentDataArray& arrayDocument);

    // Extend
    bool GetModifiedDocuments(CWizDocumentDataArray& arrayData);
    bool GetModifiedDocuments(const CWizStdStringArray& arrayLocation, CWizDocumentDataArray& arrayData);

    int GetNeedToBeDownloadedDocumentCount();
    bool GetNeedToBeDownloadedDocuments(CWizDocumentDataArray& arrayData);

    // Helper
    CString CalDocumentInfoMD5(const WIZDOCUMENTDATA& data);

    /* Attachment related operations */
    void UpdateDocumentAttachmentCount(const CString& strDocumentGUID,
                                       bool bResetDocInfo = true);

    bool CreateAttachment(const CString& strDocumentGUID, const CString& strName,
                          const CString& strURL, const CString& strDescription,
                          const CString& strDataMD5, WIZDOCUMENTATTACHMENTDATA& data);

    bool ModifyAttachmentInfo(WIZDOCUMENTATTACHMENTDATA& data);
    virtual bool DeleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data, bool bLog,
                                  bool bResetDocInfo, bool updateAttachList = true);

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
    bool GetNextTitle(const QString& strLocation, QString& strTitle);

    virtual QString getTableStructureVersion();
    virtual bool setTableStructureVersion(const QString& strVersion);

    /* Metas related operations */
    bool GetMetasByName(const QString& lpszMetaName,
                        CWizMetaDataArray& arrayMeta);

    bool GetMeta(CString strMetaName, CString strKey, CString& strValue, \
                 const CString& strDefault = "", bool* pbMetaExists = NULL);
    CString GetMetaDef(const CString& lpszMetaName, const CString& lpszKey, const CString& strDef = "");
    bool SetMeta(CString strMetaName, CString strKey, const CString& lpszValue);
    qint64 GetMetaInt64(const CString& strMetaName, const CString& strKey, qint64 nDef);
    bool SetMetaInt64(const CString& strMetaName, const CString& strKey, qint64 n);
    bool deleteMetasByName(const QString& strMetaName);
    bool deleteMetaByKey(const QString& strMetaName, const QString& strMetaKey);

    /* Deleted related operations */
    bool GetDeletedGUIDs(CWizDeletedGUIDDataArray& arrayGUID);
    bool GetDeletedGUIDs(WizObjectType eType, CWizDeletedGUIDDataArray& arrayGUID);

    bool DeleteDeletedGUID(const CString& strGUID);
    bool IsObjectDeleted(const CString& strGUID);
    bool GetDeletedGUIDsByTime(const COleDateTime& t, CWizDeletedGUIDDataArray& arrayData);

    CString GetDeletedItemsLocation() const { return m_strDeletedItemsLocation; }

    bool LogDeletedGUID(const CString& strGUID, WizObjectType eType);
    bool LogDeletedGUIDs(const CWizStdStringArray& arrayGUID, WizObjectType eType);

	static CString GetLocationArraySQLWhere(const CWizStdStringArray& arrayLocation);

    bool ObjectExists(const QString &strGUID, const QString &strType, bool& bExists);
    bool GetObjectTableInfo(const CString& strType, CString& strTableName, CString& strKeyFieldName);

    qint64 GetObjectLocalVersion(const QString &strGUID, const QString &strType);
    qint64 GetObjectLocalVersionEx(const QString &strGUID, const QString &strType, bool& bObjectExists);
    bool ModifyObjectVersion(const CString& strGUID, const CString& strType, qint64 nVersion);

    bool IsObjectDataModified(const CString& strGUID, const CString& strType);

    bool ModifyObjectModifiedTime(const CString& strGUID, const CString& strType, const COleDateTime& t);
    bool GetObjectModifiedTime(const CString& strGUID, const CString& strType, COleDateTime& t);

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

    /* thumb index related */
    bool setThumbIndexVersion(const QString& strVersion);
    QString getThumIndexVersion();

    /* Search related operations */
    bool setDocumentFTSVersion(const QString& strVersion);
    QString getDocumentFTSVersion();
    bool setDocumentFTSEnabled(bool b);
    bool isDocumentFTSEnabled();
    bool setAllDocumentsSearchIndexed(bool b);
    bool getAllDocumentsNeedToBeSearchIndexed(CWizDocumentDataArray& arrayDocument);
    bool setDocumentSearchIndexed(const QString& strDocumentGUID, bool b);

    bool SearchDocumentByWhere(const QString& strWhere,
                               int nMaxCount,
                               CWizDocumentDataArray& arrayDocument);

    bool SearchDocumentByTitle(const QString& strTitle,
                               const QString& strLocation,
                               bool bIncludeSubFolders,
                               int nMaxCount,
                               CWizDocumentDataArray& arrayDocument);

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

    /* Helper Methods */
    bool GetDocumentsNoTagCount(int& nSize, bool includeTrash = false);
    bool GetAllTagsDocumentCount(std::map<CString, int>& mapTagDocumentCount);
    bool GetAllLocationsDocumentCount(std::map<CString, int>& mapLocationDocumentCount);
    int GetTrashDocumentCount();
    bool GetAllDocumentsOwners(CWizStdStringArray& arrayOwners);

#ifndef WIZ_NO_OBSOLETE
    bool IsModified();
    bool IsDocumentModified();
    bool FilterDocumentsInDeletedItems(CWizDocumentDataArray& arrayDocument);
    bool GetCalendarEvents(const COleDateTime& tStart, const COleDateTime& tEnd, CWizDocumentDataArray& arrayDocument);
    bool GetAllRecurrenceCalendarEvents(CWizDocumentDataArray& arrayDocument);
    template <class TData> static void RemoveMultiElements(std::deque<TData>& arrayData);

    bool GetLocationDocumentCount(CString strLocation, bool bIncludeSubFolders, int& nDocumentCount);
#endif

};

#ifndef WIZ_NO_OBSOLETE
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
#endif

#endif // WIZINDEX_H
