#ifndef INDEX_H
#define INDEX_H

#pragma once

#include "wizmisc.h"
#include "wizobject.h"
#include "cppsqlite3.h"
#include <map>
#include <set>


struct WIZSEARCHDATA
{
	CWizStdStringArray arrayTag;
	CWizStdStringArray arrayFileType;
	CString strTitle;
	CString strURL;
	int nHasAttachment;
	CString strAttachmentName;
	CString strSQL;
	CString strSyntax;
	CString strDateCreated;
	CString strDateModified;
	CString strDateAccessed;
	//
	WIZSEARCHDATA()
		: nHasAttachment(-1)
	{
	}
};



struct WIZDOCUMENTLOCATIONDATA
{
	CString strLocation;
	int nDocumentCount;
	//
	WIZDOCUMENTLOCATIONDATA()
	{
		nDocumentCount = 0;
	}
};


typedef std::deque<WIZDOCUMENTLOCATIONDATA> CDocumentLocationArray;

class CIndex : public QObject
{
    Q_OBJECT
public:
	CIndex(void);
	~CIndex(void);
	//
protected:
	CppSQLite3DB m_db;
    CString m_strDeletedItemsLocation;
	//
	CString m_strFileName;
	CString m_strDatabasePath;
	CString m_strFTSPath;
	//
	BOOL m_bUpdating;
public:
    virtual BOOL Open(const CString& strFileName);
	void Close();
	//
	BOOL IsOpened();
	//
	BOOL GetUserName(CString& strUserName);
    BOOL SetUserName(const CString& strUserName);
	BOOL GetPassword(CString& strPassword);
	CString GetEncryptedPassword();
    BOOL SetPassword(const CString& strOldPassword, const CString& strPassword);
	//
	UINT GetPasswordFalgs();
	BOOL SetPasswordFalgs(UINT nFlags);
	//
    CppSQLite3Query Query(const CString& strSQL);
    int Exec(const CString& strSQL);
	//
	void BeginUpdate() { m_bUpdating = TRUE; }
	void EndUpdate() { m_bUpdating = FALSE; }
	BOOL IsUpdating() const { return m_bUpdating; }
	//
	CString GetDeletedItemsLocation() const { return m_strDeletedItemsLocation; }
	CString GetDatabasePath() const { return m_strDatabasePath; }
	//
    BOOL CheckTable(const CString& strTableName);
    //
    BOOL ExecSQL(const CString& strSQL);
    BOOL HasRecord(const CString& strSQL);
    BOOL GetFirstRowFieldValue(const CString& strSQL, int nFieldIndex, CString& strValue);
	//
    BOOL LogSQLException(const CppSQLite3Exception& e, const CString& strSQL);
	//
	BOOL GetLocations(CWizStdStringArray& arrayLocation);
	BOOL GetLocationsNeedToBeSync(CWizStdStringArray& arrayLocation);
	BOOL GetLocations(CDocumentLocationArray& arrayLocation);
	BOOL GetRootTags(CWizTagDataArray& arrayTag);
	BOOL GetAllTags(CWizTagDataArray& arrayTag);
	BOOL GetAttachments(CWizDocumentAttachmentDataArray& arrayAttachment);
    BOOL GetChildTags(const CString& strParentTagGUID, CWizTagDataArray& arrayTag);
    BOOL GetAllChildTags(const CString& strParentTagGUID, CWizTagDataArray& arrayTag);
    BOOL GetDocumentTags(const CString& strDocumentGUID, CWizTagDataArray& arrayTag);
    CString GetDocumentTagsText(const CString& strDocumentGUID);
	BOOL SetDocumentTags(WIZDOCUMENTDATA& data, const CWizTagDataArray& arrayTag);
    BOOL SetDocumentTagsText(WIZDOCUMENTDATA& data, const CString& strTagsText);
    BOOL GetDocumentAttachments(const CString& strDocumentGUID, CWizDocumentAttachmentDataArray& arrayAttachment);
	BOOL GetStyles(CWizStyleDataArray& arrayStyle);
	BOOL GetMetas(CWizMetaDataArray& arrayMeta);
	BOOL GetDeletedGUIDs(CWizDeletedGUIDDataArray& arrayGUID);
    BOOL GetDeletedGUIDs(WizObjectType eType, CWizDeletedGUIDDataArray& arrayGUID);
    BOOL GetDocumentParams(const CString& strDocumentGUID, CWizDocumentParamDataArray& arrayParam);
    BOOL GetDocumentParam(const CString& strDocumentGUID, CString strParamName, CString& strParamValue, const CString& strDefault = NULL, BOOL* pbParamExists = NULL);
    BOOL SetDocumentParam(const CString& strDocumentGUID, CString strParamName, CString strParamValue, BOOL bUpdateParamMD5);
	BOOL SetDocumentParam(WIZDOCUMENTDATA& data, CString strParamName, CString strParamValue, BOOL bUpdateParamMD5);
    static BOOL StringArrayToDDocumentParams(const CString& strDocumentGUID, const CWizStdStringArray& arrayText, std::deque<WIZDOCUMENTPARAMDATA>& arrayParam);
    BOOL SetDocumentParams(WIZDOCUMENTDATA& data, const std::deque<WIZDOCUMENTPARAMDATA>& arrayParam);
	BOOL SetDocumentParams(WIZDOCUMENTDATA& data, const CWizStdStringArray& arrayParam);
	BOOL GetAllDocuments(CWizDocumentDataArray& arrayDocument);
    BOOL GetAllDocumentsTitle(CWizStdStringArray& arrayDocument);
    BOOL GetDocumentsByParamName(const CString& strLocation, BOOL bIncludeSubFolders, CString strParamName, CWizDocumentDataArray& arrayDocument);
    BOOL GetDocumentsByParam(const CString& strLocation, BOOL bIncludeSubFolders, CString strParamName, const CString& strParamValue, CWizDocumentDataArray& arrayDocument, BOOL bIncludeDeletedItems = TRUE);
    BOOL GetDocumentsByTag(const CString& strLocation, const WIZTAGDATA& data, CWizDocumentDataArray& arrayDocument);
    BOOL GetDocumentsByStyle(const CString& strLocation, const WIZSTYLEDATA& data, CWizDocumentDataArray& arrayDocument);
    BOOL GetDocumentsByTags(BOOL bAnd, const CString& strLocation, const CWizTagDataArray& arrayTag, CWizDocumentDataArray& arrayDocument);
    BOOL GetDocumentsByLocation(const CString& strLocation, CWizDocumentDataArray& arrayDocument);
    BOOL GetDocumentsByLocationIncludeSubFolders(const CString& strLocation, CWizDocumentDataArray& arrayDocument);
    BOOL GetDocumentsBySQLWhere(const CString& strSQLWhere, CWizDocumentDataArray& arrayDocument);
	BOOL GetDocumentsByGUIDs(const CWizStdStringArray& arrayGUID, CWizDocumentDataArray& arrayDocument);
    BOOL GetMetasByName(const CString& lpszMetaName, CWizMetaDataArray& arrayMeta);
    BOOL GetMeta(CString strMetaName, CString strKey, CString& strValue, const CString& strDefault = "", BOOL* pbMetaExists = NULL);
    CString GetMetaDef(const CString& lpszMetaName, const CString& lpszKey, const CString& strDef = "");
    BOOL SetMeta(CString strMetaName, CString strKey, const CString& lpszValue);
    __int64 GetMetaInt64(const CString& strMetaName, const CString& strKey, __int64 nDef);
    BOOL SetMetaInt64(const CString& strMetaName, const CString& strKey, __int64 n);
    //
    int GetDocumentAttachmentCount(const CString& strDocumentGUID);
    void UpdateDocumentAttachmentCount(const CString& strDocumentGUID);
	//

    BOOL LogDeletedGUID(const CString& strGUID, WizObjectType eType);
    BOOL LogDeletedGUIDs(const CWizStdStringArray& arrayGUID, WizObjectType eType);
	//
    BOOL SQLToTagDataArray(const CString& strSQL, CWizTagDataArray& arrayTag);
    BOOL SQLToStyleDataArray(const CString& strSQL, CWizStyleDataArray& arrayStyle);
    BOOL SQLToMetaDataArray(const CString& strSQL, CWizMetaDataArray& arrayMeta);
    BOOL SQLToDeletedGUIDDataArray(const CString& strSQL, CWizDeletedGUIDDataArray& arrayGUID);
    BOOL SQLToStringArray(const CString& strSQL, int nFieldIndex, CWizStdStringArray& arrayString);
    BOOL SQLToDocumentParamDataArray(const CString& strSQL, CWizDocumentParamDataArray& arrayParam);
    BOOL SQLToDocumentAttachmentDataArray(const CString& strSQL, CWizDocumentAttachmentDataArray& arrayAttachment);
    BOOL SQLToDocumentDataArray(const CString& strSQL, CWizDocumentDataArray& arrayDocument);
	//
	void InitDocumentExFields(CWizDocumentDataArray& arrayDocument, const CWizStdStringArray& arrayGUID, const std::map<CString, int>& mapDocumentIndex);
    void InitDocumentShareFlags(CWizDocumentDataArray& arrayDocument, const CString& strDocumentGUIDs, const std::map<CString, int>& mapDocumentIndex, const CString& strTagName, int nShareFlags);
	//
    BOOL TagArrayByName(const CString& strName, CWizTagDataArray& arrayTag);
    BOOL TagByName(const CString& strName, WIZTAGDATA& data, const CString& strExceptGUID = "");
    BOOL StyleByName(const CString& strName, WIZSTYLEDATA& data, const CString& strExceptGUID = "");
	//
    BOOL TagByNameEx(const CString& strName, WIZTAGDATA& data);
	//
    BOOL TagsTextToTagArray(CString strText, CWizTagDataArray& arrayTag);
	//
	BOOL CreateTagEx(const WIZTAGDATA& data);
	BOOL CreateStyleEx(const WIZSTYLEDATA& data);
	BOOL CreateDocumentEx(const WIZDOCUMENTDATA& data);
	BOOL CreateAttachmentEx(const WIZDOCUMENTATTACHMENTDATA& data);
	//
    BOOL CreateTag(const CString& strParentTagGUID, const CString& strName, const CString& strDescription, WIZTAGDATA& data);
    BOOL CreateStyle(const CString& strName, const CString& strDescription, COLORREF crTextColor, COLORREF crBackColor, BOOL bTextBold, int nFlagIndex, WIZSTYLEDATA& data);
    BOOL CreateDocument(const CString& strTitle, const CString& strName, const CString& strLocation, const CString& strURL, const CString& strAuthor, const CString& strKeywords, const CString& strType, const CString& strOwner, const CString& strFileType, const CString& strStyleGUID, int nIconIndex, int nSync, int nProtected, WIZDOCUMENTDATA& data);
    BOOL CreateDocument(const CString& strTitle, const CString& strName, const CString& strLocation, const CString& strURL, WIZDOCUMENTDATA& data);
    BOOL CreateAttachment(const CString& strDocumentGUID, const CString& strName, const CString& strURL, const CString& strDescription, const CString& strDataMD5, WIZDOCUMENTATTACHMENTDATA& data);
	//
	CString CalDocumentInfoMD5(const WIZDOCUMENTDATA& data);
	CString CalDocumentParamInfoMD5(const WIZDOCUMENTDATA& data);
	CString CalDocumentParamInfoMD5(const CWizDocumentParamDataArray& arrayParam);
	CString CalDocumentAttachmentInfoMD5(const WIZDOCUMENTATTACHMENTDATA& data);
	//
    virtual BOOL UpdateDocumentInfoMD5(WIZDOCUMENTDATA& data);
    virtual BOOL UpdateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName);
    BOOL UpdateDocumentsInfoMD5(CWizDocumentDataArray& arrayDocument);
	//
	BOOL ModifyTagEx(const WIZTAGDATA& data);
	BOOL ModifyStyleEx(const WIZSTYLEDATA& data);
	BOOL ModifyDocumentInfoEx(const WIZDOCUMENTDATA& data);
	BOOL ModifyAttachmentInfoEx(const WIZDOCUMENTATTACHMENTDATA& data);
	//
	BOOL ModifyTag(WIZTAGDATA& data);
	BOOL ModifyStyle(WIZSTYLEDATA& data);
	BOOL ModifyDocumentInfo(WIZDOCUMENTDATA& data);
	BOOL ModifyDocumentDateModified(WIZDOCUMENTDATA& data);
	BOOL ModifyDocumentDateAccessed(WIZDOCUMENTDATA& data);
	BOOL ModifyDocumentDataDateModified(WIZDOCUMENTDATA& data);
	BOOL ModifyAttachmentInfo(WIZDOCUMENTATTACHMENTDATA& data);
	BOOL ModifyDocumentReadCount(const WIZDOCUMENTDATA& data);
	//
	BOOL DeleteTag(const WIZTAGDATA& data, BOOL bLog);
	BOOL DeleteStyle(const WIZSTYLEDATA& data, BOOL bLog);
	BOOL DeleteDocument(const WIZDOCUMENTDATA& data, BOOL bLog);
	BOOL DeleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data, BOOL bLog);
    BOOL DeleteDeletedGUID(const CString& strGUID);
	//
	BOOL DeleteDocumentTags(WIZDOCUMENTDATA& data);
	//
    //BOOL DeleteDocumentAttachments(const CString& strDocumentGUID);
	BOOL DeleteTagDocuments(const WIZTAGDATA& data);
	BOOL DeleteStyleDocuments(const WIZSTYLEDATA& data);
    BOOL DeleteDocumentsByLocation(const CString& strLocation);
    BOOL DeleteDocumentsTagsByLocation(const CString& strLocation);
    BOOL GetDocumentsGUIDByLocation(const CString& strLocation, CWizStdStringArray& arrayGUID);
    BOOL DeleteDocumentParams(const CString& strDocumentGUID);
    BOOL DeleteDocumentParam(const CString& strDocumentGUID, CString strParamName, BOOL bUpdateParamMD5);
    BOOL DeleteDocumentParamEx(const CString& strDocumentGUID, CString strParamNamePart);
	//
    BOOL TagFromGUID(const CString& strTagGUID, WIZTAGDATA& data);
    BOOL StyleFromGUID(const CString& strStyleGUID, WIZSTYLEDATA& data);
    BOOL DocumentFromGUID(const CString& strDocumentGUID, WIZDOCUMENTDATA& data);
    BOOL AttachmentFromGUID(const CString& strAttachcmentGUID, WIZDOCUMENTATTACHMENTDATA& data);
    BOOL DocumentFromLocationAndName(const CString& strLocation, const CString& strName, WIZDOCUMENTDATA& data);
	//
	BOOL UpdateDocumentParamMD5(WIZDOCUMENTDATA& data);
    BOOL UpdateDocumentParamMD5(const CString& strDocumentGUID);
	//
    BOOL InsertDocumentTag(WIZDOCUMENTDATA& data, const CString& strTagGUID);
    BOOL DeleteDocumentTag(WIZDOCUMENTDATA& data, const CString& strTagGUID);
	//
	BOOL SetDocumentTags(WIZDOCUMENTDATA& data, const CWizStdStringArray& arrayTagGUID);
    BOOL GetDocumentTags(const CString& strDocumentGUID, CWizStdStringArray& arrayTagGUID);
    CString GetDocumentTagGUIDsString(const CString& strDocumentGUID);
	//
    BOOL GetDocumentTagsNameStringArray(const CString& strDocumentGUID, CWizStdStringArray& arrayTagName);
    CString GetDocumentTagNameText(const CString& strDocumentGUID);
	//
    BOOL ChangeDocumentsLocation(const CString& strOldLocation, const CString& strNewLocation);
	//
    BOOL TitleExists(const CString& strLocation, CString strTitle);
    BOOL GetNextTitle(const CString& strLocation, CString& strTitle);
	//
    static CString FormatQuerySQL(const CString& strTableName, const CString& strFieldList);
    static CString FormatQuerySQL(const CString& strTableName, const CString& strFieldList, const CString& strWhere);
    static CString FormatInsertSQLFormat(const CString& strTableName, const CString& strFieldList, const CString& strParamList);
    static CString FormatUpdateSQLFormat(const CString& strTableName, const CString& strFieldList, const CString& strKey);
    static CString FormatDeleteSQLFormat(const CString& strTableName, const CString& strKey);
    static CString FormatQuerySQLByTime(const CString& strTableName, const CString& strFieldList, const CString& strFieldName, const COleDateTime& t);
    static CString FormatQuerySQLByTime2(const CString& strTableName, const CString& strFieldList, const CString& strInfoFieldName, const CString& strDataFieldName, const COleDateTime& t);
    static CString FormatQuerySQLByTime3(const CString& strTableName, const CString& strFieldList, const CString& strInfoFieldName, const CString& strDataFieldName, const CString& strParamFieldName, const COleDateTime& t);
    static CString FormatModifiedQuerySQL(const CString& strTableName, const CString& strFieldList);
    static CString FormatModifiedQuerySQL2(const CString& strTableName, const CString& strFieldList, int nCount);
	//
	BOOL FilterDocumentsInDeletedItems(CWizDocumentDataArray& arrayDocument);
	//
	BOOL GetTagsByTime(const COleDateTime& t, CWizTagDataArray& arrayData);
	BOOL GetStylesByTime(const COleDateTime& t, CWizStyleDataArray& arrayData);
	BOOL GetDocumentsByTime(const COleDateTime& t, CWizDocumentDataArray& arrayData);
	BOOL GetAttachmentsByTime(const COleDateTime& t, CWizDocumentAttachmentDataArray& arrayData);
	BOOL GetDeletedGUIDsByTime(const COleDateTime& t, CWizDeletedGUIDDataArray& arrayData);
	//
	BOOL IsModified();
	BOOL IsDocumentModified();
	BOOL GetModifiedTags(CWizTagDataArray& arrayData);
	BOOL GetModifiedStyles(CWizStyleDataArray& arrayData);
	BOOL GetModifiedDocuments(CWizDocumentDataArray& arrayData);
	BOOL GetModifiedDocuments(const CWizStdStringArray& arrayLocation, CWizDocumentDataArray& arrayData);
	BOOL GetModifiedAttachments(CWizDocumentAttachmentDataArray& arrayData);
	BOOL GetModifiedAttachments(const CWizStdStringArray& arrayLocation, CWizDocumentAttachmentDataArray& arrayData);
	//
	static CString GetLocationArraySQLWhere(const CWizStdStringArray& arrayLocation);
	//
    BOOL ObjectExists(const CString& strGUID, const CString& strType, BOOL& bExists);
    BOOL DeleteObject(const CString& strGUID, const CString& strType, BOOL bLog);
    BOOL GetObjectTableInfo(const CString& strType, CString& strTableName, CString& strKeyFieldName);
    BOOL ModifyObjectVersion(const CString& strGUID, const CString& strType, __int64 nVersion);
    BOOL ModifyObjectModifiedTime(const CString& strGUID, const CString& strType, const COleDateTime& t);
    BOOL GetObjectModifiedTime(const CString& strGUID, const CString& strType, COleDateTime& t);
	//
    BOOL Search(const CString& strKeywords, const WIZSEARCHDATA& data, const CString& strLocation, BOOL bIncludeSubFolders, size_t nMaxCount, CWizDocumentDataArray& arrayDocument);
    BOOL Search(const CString& strKeywords, const CString& strLocation, BOOL bIncludeSubFolders, CWizDocumentDataArray& arrayDocument);
    BOOL SearchDocumentByTitle(const CString& strTitle, const CString& strLocation, BOOL bIncludeSubFolders, size_t nMaxCount, CWizDocumentDataArray& arrayDocument);
    BOOL GetRecentDocuments(long nFlags, const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    BOOL GetRecentDocumentsCreated(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    BOOL GetRecentDocumentsModified(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument);
    BOOL GetRecentDocumentsByCreatedTime(const COleDateTime& t, CWizDocumentDataArray& arrayDocument);
    //
	BOOL GetCalendarEvents(const COleDateTime& tStart, const COleDateTime& tEnd, CWizDocumentDataArray& arrayDocument);
	BOOL GetAllRecurrenceCalendarEvents(CWizDocumentDataArray& arrayDocument);
	//
    BOOL GetSync(const CString& strLocation);
    BOOL SetSync(const CString& strLocation, BOOL bSync);
	//
	BOOL GetAllLocationsDocumentCount(std::map<CString, int>& mapLocationDocumentCount);
    BOOL GetLocationDocumentCount(CString strLocation, BOOL bIncludeSubFolders, int& nDocumentCount);
	//
	BOOL GetAllTagsDocumentCount(std::map<CString, int>& mapTagDocumentCount);
	//
    BOOL GetAllParentsTagGUID(CString strTagGUID, CWizStdStringArray& arrayGUID);
    BOOL GetAllParentsTagGUID(CString strTagGUID, std::set<CString>& setGUID);
	//
    BOOL SetDocumentVersion(const CString& strDocumentGUID, __int64 nVersion);
	//
    BOOL IsLocationEmpty(const CString& strLocation);
	BOOL GetAllLocations(CWizStdStringArray& arrayLocation);
    BOOL GetChildLocations(const CString& strLocation, CWizStdStringArray& arrayLocation);
    //
    BOOL ObjectInReserved(const CString& strGUID, const CString& strType);
    //
    //
    enum WizObjectReservedInt { reserved1, reserved2, reserved3, reserved4};
    static CString GetReservedIntFieldName(WizObjectReservedInt e);
    BOOL SetObjectReservedInt(const CString& strGUID, const CString& strType, WizObjectReservedInt e, int val);
    BOOL GetObjectReservedInt(const CString& strGUID, const CString& strType, WizObjectReservedInt e, int& val);
    //
    enum WizObjectReservedStr { reserved5, reserved6, reserved7, reserved8};
    static CString GetReservedStrFieldName(WizObjectReservedStr e);
    BOOL SetObjectReservedStr(const CString& strGUID, const CString& strType, WizObjectReservedStr e, const CString& val);
    BOOL GetObjectReservedStr(const CString& strGUID, const CString& strType, WizObjectReservedStr e, CString& val);
    //
    static const WizObjectReservedInt DATA_DOWNLOADED_FIELD = reserved1;
    //
    BOOL SetDocumentDataDownloaded(const CString& strGUID, BOOL bDownloaded);
    BOOL SetAttachmentDataDownloaded(const CString& strGUID, BOOL bDownloaded);
    BOOL SetObjectDataDownloaded(const CString& strGUID, const CString& strType, BOOL bDownloaded);
    BOOL IsObjectDataDownloaded(const CString& strGUID, const CString& strType);
    //
    int GetNeedToBeDownloadedDocumentCount();
    int GetNeedToBeDownloadedAttachmentCount();
    BOOL GetNeedToBeDownloadedDocuments(CWizDocumentDataArray& arrayData);
    BOOL GetNeedToBeDownloadedAttachments(CWizDocumentAttachmentDataArray& arrayData);


    BOOL SetDocumentIndexed(const CString& strDocumentGUID, BOOL b);
	BOOL SetAllDocumentsIndexed(BOOL b);
	CString GetDocumentIndexPath();
	BOOL GetAllDocumentNeedToBeIndexed(CWizDocumentDataArray& arrayDocument);
    BOOL DeleteDocumentIndex(const CString& strDocumentGUID);
	BOOL DeleteDocumentIndexData();
    BOOL SearchDocumentByFullTextSearch(const CString& strKeywords, int nMaxResult, CWizDocumentDataArray& arrayDocument);
	BOOL StartDocumentIndexing(BOOL bPrompt);
	BOOL SetDocumentIndexingEnabled(BOOL b);
	BOOL IsDocumentIndexingEnabled();
	//
    static BOOL IsRootLocation(CString strLocation);
    static CString GetRootLocationName(CString strLocation);
    static CString TagDisplayNameToName(const CString& strDisplayName);
	//
	template <class TData>
    static void RemoveMultiElements(std::deque<TData>& arrayData);
	//
    BOOL Repair(const CString& strDestFileName);
    //
    void GetExtraFolder(CWizStdStringArray& arrayLocation);
    void SetExtraFolder(const CWizStdStringArray& arrayLocation);
    //
    void GetDeletedFolder(CWizStdStringArray& arrayLocation);
    void SetDeletedFolder(const CWizStdStringArray& arrayLocation);
    //
    void AddExtraFolder(const CString& strLocation);
    void LogDeletedFolder(const CString& strLocation);

    void RemoveFromExtraFolder(const CString& strLocation);
    void RemoveFromDeletedFolder(const CString& strLocation);

Q_SIGNALS:
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
    void attachmentCreated(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void attachmentModified(const WIZDOCUMENTATTACHMENTDATA& attachmentOld, const WIZDOCUMENTATTACHMENTDATA& attachmentNew);
    void attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void folderCreated(const CString& strLocation);
    void folderDeleted(const CString& strLocation);
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


#endif
