#ifndef WIZINDEX_H
#define WIZINDEX_H

#include <map>
#include <set>

#include "WizIndexBase.h"

#define WIZ_DATABASE_VERSION "1.0"
#define WIZ_NO_OBSOLETE


/*
 * Lower operations of sqlite database
 */
class WizIndex
        : public WizIndexBase
{
    Q_OBJECT

public:
    WizIndex(void);

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
    bool createTag(const CString& strParentTagGUID, const CString& strName, \
                   const CString& strDescription, WIZTAGDATA& data);
    bool modifyTag(WIZTAGDATA& data);
    bool deleteTag(const WIZTAGDATA& data, bool bLog, bool bReset = true);
    bool modifyTagPosition(const WIZTAGDATA& data);


    bool setDocumentTags(WIZDOCUMENTDATA& data, const CWizTagDataArray& arrayTag);

    bool insertDocumentTag(WIZDOCUMENTDATA& data,
                           const CString& strTagGUID,
                           bool bReset = true);

    bool deleteDocumentTags(WIZDOCUMENTDATA& data, bool bReset = true);

    bool setDocumentTags(WIZDOCUMENTDATA& data,
                         const CWizStdStringArray& arrayTagGUID,
                         bool bReset = true);

    bool setDocumentTags2(WIZDOCUMENTDATA& data,
                         const CWizStdStringArray& arrayTagGUID,
                         bool bReset = true);

    bool setDocumentTagsText(WIZDOCUMENTDATA& data, const CString& strTagsText);

    bool deleteDocumentTag(WIZDOCUMENTDATA& data, const CString& strTagGUID);

    bool deleteDocumentsTagsByLocation(const CString& strLocation);
    bool deleteTagDocuments(const WIZTAGDATA& data, bool bReset);

    // Query tags by name
    bool tagByName(const CString& strName, CWizTagDataArray& arrayTag, const CString& strExceptGUID = "");
    bool tagByNameEx(const CString& strName, WIZTAGDATA& data);
    bool tagArrayByName(const CString& strName, CWizTagDataArray& arrayTag);

    // Query tags by document guid
    bool getDocumentTagsNameStringArray(const CString& strDocumentGUID, CWizStdStringArray& arrayTagName);
    int getDocumentTagCount(const CString& strDocumentGUID);
    bool getDocumentTags(const CString& strDocumentGUID, CWizTagDataArray& arrayTag);
    bool getDocumentTags(const CString& strDocumentGUID, CWizStdStringArray& arrayTagGUID);
    CString getDocumentTagsText(const CString& strDocumentGUID);
    CString getDocumentTagNameText(const CString& strDocumentGUID);
    CString getDocumentTagGuidsString(const CString& strDocumentGUID);

    bool getDocumentTagsDisplayNameStringArray(const CString& strDocumentGUID, CWizStdStringArray& arrayTagDisplayName);
    CString getDocumentTagDisplayNameText(const CString& strDocumentGUID);
    QString getDocumentTagTreeDisplayString(const QString& strDocumentGUID);

    bool getAllParentsTagGuid(CString strTagGUID, CWizStdStringArray& arrayGUID);
    bool getAllParentsTagGuid(CString strTagGUID, std::set<CString>& setGUID);

    // Query tags by time
    bool getTagsByTime(const WizOleDateTime& t, CWizTagDataArray& arrayData);

    // Tags extend Query

    bool getModifiedTags(CWizTagDataArray& arrayData);

    // tag helper method

    // Indicate group document location path
    QString getTagTreeText(const QString& strDocumentGUID);

    // convert user input text to tag data structure
    bool tagsTextToTagArray(CString strText, CWizTagDataArray& arrayTag);

    // deprecated note share tags, still avaiable, but will be removed from future release
    static CString tagDisplayNameToName(const CString& strDisplayName);
    static CString tagNameToDisplayName(const CString& strName);

    /* Style related operations */
    bool createStyle(const CString& strName, const CString& strDescription,
                     COLORREF crTextColor, COLORREF crBackColor,
                     bool bTextBold, int nFlagIndex, WIZSTYLEDATA& data);
    bool modifyStyle(WIZSTYLEDATA& data);
    bool deleteStyle(const WIZSTYLEDATA& data, bool bLog, bool bReset = true);

    bool deleteStyleDocuments(const WIZSTYLEDATA& data, bool bReset);

    // Query style by name
    bool styleByName(const CString& strName, WIZSTYLEDATA& data,
                     const CString& strExceptGUID = "");

    // Query style by time
    bool getStylesByTime(const WizOleDateTime& t, CWizStyleDataArray& arrayData);

    bool getModifiedStyles(CWizStyleDataArray& arrayData);
    bool getModifiedParams(CWizDocumentParamDataArray& arrayData);

    bool deleteDocumentParams(const QString& strDocumentGUID);
    bool setDocumentParam(const QString& strDocumentGUID, const QString& strParamName, const QString& strParamValue);
    bool setDocumentParams(const QString& strDocumentGUID, const CWizDocumentParamDataArray& arrayParam);
    bool modifyDocumentParamVersion(const QString& strDocumentGUID, const QString& strParamName, __int64 version);

    /* Location(Folder) related operations */
    bool changeDocumentsLocation(const CString& strOldLocation, const CString& strNewLocation);
    static bool isRootLocation(CString strLocation);
    static CString getRootLocationName(CString strLocation);

    bool getLocations(CWizStdStringArray& arrayLocation);
    bool getLocationsNeedToBeSync(CWizStdStringArray& arrayLocation);
    bool getLocations(CDocumentLocationArray& arrayLocation);

    void getExtraFolder(CWizStdStringArray& arrayLocation);
    void setExtraFolder(const CWizStdStringArray& arrayLocation);
    void addExtraFolder(const QString& strLocation);
    void deleteExtraFolder(const QString& strLocation);

    bool updateLocation(const QString& strOldLocation, const QString& strNewLocation);

    bool isLocationEmpty(const CString& strLocation);
    bool getAllLocations(CWizStdStringArray& arrayLocation);
    bool getAllChildLocations(const CString& strLocation, CWizStdStringArray& arrayLocation);
    void getAllLocationsWithExtra(CWizStdStringArray& arrayLocation);

    // Extend
    bool getSync(const CString& strLocation);
    bool setSync(const CString& strLocation, bool bSync);

    /* Document related operations */
    virtual bool updateDocumentInfoMD5(WIZDOCUMENTDATA& data);
    bool updateDocumentsInfoMD5(CWizDocumentDataArray& arrayDocument);
    virtual bool updateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName, bool notifyDataModify = true);

    bool createDocument(const CString& strTitle, const CString& strName, \
                        const CString& strLocation, const CString& strURL, \
                        const CString& strAuthor, const CString& strKeywords, \
                        const CString& strType, const CString& strOwner, \
                        const CString& strFileType, const CString& strStyleGUID, \
                        int nIconIndex, int nSync, int nProtected, WIZDOCUMENTDATA& data);

    bool createDocument(const CString& strTitle, const CString& strName, \
                        const CString& strLocation, const CString& strURL, int nProtected, WIZDOCUMENTDATA& data);


    bool modifyDocumentInfo(WIZDOCUMENTDATA& data, bool bReset = true);
    bool modifyDocumentDateModified(WIZDOCUMENTDATA& data);
    bool modifyDocumentDateAccessed(WIZDOCUMENTDATA& data);
    bool modifyDocumentDataDateModified(WIZDOCUMENTDATA& data);
    bool modifyDocumentReadCount(const WIZDOCUMENTDATA& data);
    bool modifyDocumentLocation(WIZDOCUMENTDATA& data);

    bool deleteDocument(const WIZDOCUMENTDATA& data, bool bLog);

    bool getDocumentsNoTag(CWizDocumentDataArray& arrayDocument, bool includeTrash = false);

    bool getLastestDocuments(CWizDocumentDataArray& arrayDocument, int nMax = 1000);

    bool getDocumentsByParamName(const CString& strLocation, bool bIncludeSubFolders, \
                                 CString strParamName, CWizDocumentDataArray& arrayDocument);
    bool getDocumentsByParam(const CString& strLocation, bool bIncludeSubFolders, \
                             CString strParamName, const CString& strParamValue, \
                             CWizDocumentDataArray& arrayDocument, bool bIncludeDeletedItems = true);

    bool getDocumentsByStyle(const CString& strLocation, \
                             const WIZSTYLEDATA& data, CWizDocumentDataArray& arrayDocument);

    bool deleteDocumentsByLocation(const CString& strLocation);

    bool setDocumentVersion(const CString& strDocumentGUID, qint64 nVersion);

    bool getGroupUnreadDocuments(CWizDocumentDataArray& arrayDocument);
    int getGroupUnreadDocumentCount();
    void setGroupDocumentsReaded();
    CString kbGuidToSQL();

    // Raw Query
    bool getAllDocumentsTitle(CWizStdStringArray& arrayDocument);

    // Query by tag
    // includeTrash only take effect when strLocation is empty.
    bool getDocumentsByTag(const CString& strLocation,
                           const WIZTAGDATA& data,
                           CWizDocumentDataArray& arrayDocument,
                           bool includeTrash);

    bool getDocumentsSizeByTag(const WIZTAGDATA& data, int& size);
    bool getAllDocumentsSizeByTag(const WIZTAGDATA& data, int& size);

    bool getDocumentsByTags(bool bAnd, const CString& strLocation,
                            const CWizTagDataArray& arrayTag,
                            CWizDocumentDataArray& arrayDocument);

    // Query by document guid
    bool getDocumentsByGuids(const CWizStdStringArray& arrayGUID, CWizDocumentDataArray& arrayDocument);

    // Query by location(folder)
    bool getDocumentsCountByLocation(const CString& strLocation,
                                    int& count,
                                    bool bIncludeSubFolders = false);

    bool getDocumentsByLocation(const CString& strLocation,
                                CWizDocumentDataArray& arrayDocument,
                                bool bIncludeSubFolders = false);

    //bool GetDocumentsByLocationIncludeSubFolders(const CString& strLocation, CWizDocumentDataArray& arrayDocument);

    bool getDocumentsGuidByLocation(const CString& strLocation, CWizStdStringArray& arrayGUID);
    bool getDocumentsByTitle(const QString& title, CWizDocumentDataArray& arrayDocument);

    bool documentFromLocationAndName(const CString& strLocation, const CString& strName, WIZDOCUMENTDATA& data);

    // Query by time
    bool getDocumentsByTime(const QDateTime &t, CWizDocumentDataArray& arrayData);
    bool getRecentDocuments(long nFlags, const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    bool getRecentDocumentsCreated(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    bool getRecentDocumentsModified(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    bool getRecentDocumentsByCreatedTime(const WizOleDateTime& t, CWizDocumentDataArray& arrayDocument);
    bool getRecentDocumentsByModifiedTime(const WizOleDateTime& t, CWizDocumentDataArray& arrayDocument);
    bool getRecentDocumentsByAccessedTime(const WizOleDateTime& t, CWizDocumentDataArray& arrayDocument);

    // Extend
    bool getModifiedDocuments(CWizDocumentDataArray& arrayData);
    bool getModifiedDocuments(const CWizStdStringArray& arrayLocation, CWizDocumentDataArray& arrayData);

    int getNeedToBeDownloadedDocumentCount();
    bool getNeedToBeDownloadedDocuments(CWizDocumentDataArray& arrayData);

    /* Attachment related operations */
    void updateDocumentAttachmentCount(const CString& strDocumentGUID,
                                       bool bResetDocInfo = true);

    bool createAttachment(const CString& strDocumentGUID, const CString& strName,
                          const CString& strURL, const CString& strDescription,
                          const CString& strDataMD5, WIZDOCUMENTATTACHMENTDATA& data);

    bool modifyAttachmentInfo(WIZDOCUMENTATTACHMENTDATA& data);
    virtual bool deleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data, bool bLog,
                                  bool bResetDocInfo, bool updateAttachList = true);

    // Raw Query
    int getDocumentAttachmentCount(const CString& strDocumentGUID);

    // Query by document guid
    bool getDocumentAttachments(const CString& strDocumentGUID,
                                CWizDocumentAttachmentDataArray& arrayAttachment);

    // Query by time
    bool getAttachmentsByTime(const WizOleDateTime& t,
                              CWizDocumentAttachmentDataArray& arrayData);

    // Extend
    bool getModifiedAttachments(CWizDocumentAttachmentDataArray& arrayData);
    bool getModifiedAttachments(const CWizStdStringArray& arrayLocation,
                                CWizDocumentAttachmentDataArray& arrayData);

    int getNeedToBeDownloadedAttachmentCount();
    bool getNeedToBeDownloadedAttachments(CWizDocumentAttachmentDataArray& arrayData);

    bool setAttachmentDataDownloaded(const CString& strGUID, bool bDownloaded);

    // helper
    CString calDocumentAttachmentInfoMD5(const WIZDOCUMENTATTACHMENTDATA& data);
    bool titleExists(const CString& strLocation, CString strTitle);
    bool getNextTitle(const QString& strLocation, QString& strTitle);

    virtual QString getTableStructureVersion();
    virtual bool setTableStructureVersion(const QString& strVersion);

    /* Metas related operations */
    bool getMetasByName(const QString& lpszMetaName,
                        CWizMetaDataArray& arrayMeta);

    bool getMeta(CString strMetaName, CString strKey, CString& strValue, \
                 const CString& strDefault = "", bool* pbMetaExists = NULL);
    CString getMetaDef(const CString& lpszMetaName, const CString& lpszKey, const CString& strDef = "");
    bool setMeta(CString strMetaName, CString strKey, const CString& lpszValue);
    qint64 getMetaInt64(const CString& strMetaName, const CString& strKey, qint64 nDef);
    bool setMetaInt64(const CString& strMetaName, const CString& strKey, qint64 n);
    bool deleteMetasByName(const QString& strMetaName);
    bool deleteMetaByKey(const QString& strMetaName, const QString& strMetaKey);

    /* Deleted related operations */
    bool getDeletedGuids(CWizDeletedGUIDDataArray& arrayGUID);
    bool getDeletedGuids(WizObjectType eType, CWizDeletedGUIDDataArray& arrayGUID);

    bool deleteDeletedGuid(const CString& strGUID);
    bool isObjectDeleted(const CString& strGUID);
    bool getDeletedGuidsByTime(const WizOleDateTime& t, CWizDeletedGUIDDataArray& arrayData);

    CString getDeletedItemsLocation() const { return m_strDeletedItemsLocation; }

    bool logDeletedGuid(const CString& strGUID, WizObjectType eType);
    bool logDeletedGuids(const CWizStdStringArray& arrayGUID, WizObjectType eType);

	static CString getLocationArraySQLWhere(const CWizStdStringArray& arrayLocation);

    bool objectExists(const QString &strGUID, const QString &strType, bool& bExists);
    bool getObjectTableInfo(const CString& strType, CString& strTableName, CString& strKeyFieldName);

    qint64 getObjectLocalVersion(const QString &strGUID, const QString &strType);
    qint64 getObjectLocalVersionEx(const QString &strGUID, const QString &strType, bool& bObjectExists);
    bool modifyObjectVersion(const CString& strGUID, const CString& strType, qint64 nVersion);

    bool isObjectDataModified(const CString& strGUID, const CString& strType);

    bool modifyObjectModifiedTime(const CString& strGUID, const CString& strType, const WizOleDateTime& t);
    bool getObjectModifiedTime(const CString& strGUID, const CString& strType, WizOleDateTime& t);

    bool objectInReserved(const CString& strGUID, const CString& strType);

    enum WizObjectReservedInt { reserved1, reserved2, reserved3, reserved4};
    static CString getReservedIntFieldName(WizObjectReservedInt e);
    bool setObjectReservedInt(const CString& strGUID, const CString& strType, WizObjectReservedInt e, int val);
    bool getObjectReservedInt(const CString& strGUID, const CString& strType, WizObjectReservedInt e, int& val);

    enum WizObjectReservedStr { reserved5, reserved6, reserved7, reserved8};
    static CString getReservedStrFieldName(WizObjectReservedStr e);
    bool setObjectReservedStr(const CString& strGUID, const CString& strType, WizObjectReservedStr e, const CString& val);
    bool getObjectReservedStr(const CString& strGUID, const CString& strType, WizObjectReservedStr e, CString& val);

    static const WizObjectReservedInt DATA_DOWNLOADED_FIELD = reserved1;

    bool setDocumentDataDownloaded(const CString& strGUID, bool bDownloaded);
    bool setObjectDataDownloaded(const CString& strGUID, const CString& strType, bool bDownloaded);
    bool isObjectDataDownloaded(const CString& strGUID, const CString& strType);

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

    bool searchDocumentByWhere(const QString& strWhere,
                               int nMaxCount,
                               CWizDocumentDataArray& arrayDocument);

    bool searchDocumentByTitle(const QString& strTitle,
                               const QString& strLocation,
                               bool bIncludeSubFolders,
                               int nMaxCount,
                               CWizDocumentDataArray& arrayDocument);

    bool search(const CString& strKeywords,
                const WIZSEARCHDATA& data,
                const CString& strLocation,
                bool bIncludeSubFolders,
                size_t nMaxCount,
                CWizDocumentDataArray& arrayDocument);

    bool search(const CString& strKeywords,
                const CString& strLocation,
                bool bIncludeSubFolders,
                CWizDocumentDataArray& arrayDocument);

    /* Helper Methods */
    bool getDocumentsNoTagCount(int& nSize, bool includeTrash = false);
    bool getAllTagsDocumentCount(std::map<CString, int>& mapTagDocumentCount);
    bool getAllLocationsDocumentCount(std::map<CString, int>& mapLocationDocumentCount);
    int getTrashDocumentCount();
    bool getAllDocumentsOwners(CWizStdStringArray& arrayOwners);

#ifndef WIZ_NO_OBSOLETE
    bool isModified();
    bool isDocumentModified();
    bool filterDocumentsInDeletedItems(CWizDocumentDataArray& arrayDocument);
    bool getCalendarEvents(const WizOleDateTime& tStart, const WizOleDateTime& tEnd, CWizDocumentDataArray& arrayDocument);
    bool getAllRecurrenceCalendarEvents(CWizDocumentDataArray& arrayDocument);
    template <class TData> static void RemoveMultiElements(std::deque<TData>& arrayData);

    bool getLocationDocumentCount(CString strLocation, bool bIncludeSubFolders, int& nDocumentCount);
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
