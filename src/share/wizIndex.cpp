#include "wizIndex.h"

#include <QDebug>

#include <algorithm>
#include "wizkmcore.h"
#include "utils/logger.h"
#include "utils/misc.h"
#include "wizmisc.h"


CWizIndex::CWizIndex(void)
    : m_strDeletedItemsLocation(LOCATION_DELETED_ITEMS)
{
}

bool CWizIndex::GetDocumentTags(const CString& strDocumentGUID, CWizTagDataArray& arrayTag)
{
    CString strWhere = WizFormatString1(_T("TAG_GUID in (select TAG_GUID from WIZ_DOCUMENT_TAG where DOCUMENT_GUID='%1')"), strDocumentGUID);
	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, strWhere); 
	return SQLToTagDataArray(strSQL, arrayTag);
}

bool CWizIndex::GetDocumentAttachments(const CString& strDocumentGUID, CWizDocumentAttachmentDataArray& arrayAttachment)
{
    CString strWhere = WizFormatString1(_T("DOCUMENT_GUID='%1'"), strDocumentGUID);
	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT, strWhere); 
	return SQLToDocumentAttachmentDataArray(strSQL, arrayAttachment);
}

bool CWizIndex::GetMetasByName(const QString& strMetaName, CWizMetaDataArray& arrayMeta)
{
    if (strMetaName.isEmpty()) {
		TOLOG(_T("Meta name is empty!"));
        return false;
	}

    CString strWhere = WizFormatString1(_T("META_NAME=%1"), STR2SQL(strMetaName));
	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_META, FIELD_LIST_WIZ_META, strWhere); 
	return SQLToMetaDataArray(strSQL, arrayMeta);
}

bool CWizIndex::GetDeletedGUIDs(CWizDeletedGUIDDataArray& arrayGUID)
{
	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID); 
	return SQLToDeletedGUIDDataArray(strSQL, arrayGUID);
}

bool CWizIndex::GetDeletedGUIDs(WizObjectType eType, CWizDeletedGUIDDataArray& arrayGUID)
{
	CString strWhere = WizFormatString1(_T("GUID_TYPE=%1"), WizIntToStr(eType));
	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID, strWhere);
	return SQLToDeletedGUIDDataArray(strSQL, arrayGUID);
}

bool CWizIndex::LogDeletedGUID(const CString& strGUID, WizObjectType eType)
{
	CString strFormat = FormatInsertSQLFormat(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID, PARAM_LIST_WIZ_DELETED_GUID);

	CString strSQL;
    strSQL.Format(strFormat,
        STR2SQL(strGUID).utf16(),
		int(eType),
        TIME2SQL(WizGetCurrentTime()).utf16()
		);

	return ExecSQL(strSQL);
}

bool CWizIndex::LogDeletedGUIDs(const CWizStdStringArray& arrayGUID, WizObjectType eType)
{
    CWizStdStringArray::const_iterator it;
    for (it = arrayGUID.begin(); it != arrayGUID.end(); it++) {
        if (!LogDeletedGUID(*it, eType)) {
			TOLOG1(_T("Warning: Failed to log deleted guid %1"), *it);
		}
    }

    return true;
}

bool CWizIndex::TagByName(const CString& strName, CWizTagDataArray& arrayTag, const CString& strExceptGUID /*= ""*/)
{
    CWizTagDataArray arrayAllTag;
    if (!GetAllTags(arrayAllTag)) {
		TOLOG(_T("Failed to get tags!"));
        return false;
	}

    CString strFindName = TagDisplayNameToName(strName);

    CWizTagDataArray::const_iterator it;
    for (it = arrayAllTag.begin(); it != arrayAllTag.end(); it++) {
        if (0 == it->strName.CompareNoCase(strFindName)) {
			if (0 == strExceptGUID.CompareNoCase(it->strGUID))
				continue;

            arrayTag.push_back(*it);
		}
	}

    return !arrayTag.empty();
}

bool CWizIndex::TagByNameEx(const CString& strName, WIZTAGDATA& data)
{
    CWizTagDataArray arrayTag;
    if (!TagByName(strName, arrayTag, ""))
        return false;

    for (WIZTAGDATA tag : arrayTag)
    {
        if (tag.strParentGUID.IsEmpty())
            return true;
    }

    return CreateTag(_T(""), strName, _T(""), data);
}

bool CWizIndex::TagArrayByName(const CString& strName, CWizTagDataArray& arrayTagRet)
{
    CWizTagDataArray arrayTag;
    if (!GetAllTags(arrayTag)) {
        TOLOG(_T("Failed to get tags!"));
        return false;
    }

    CString strFindName = TagDisplayNameToName(strName);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        if (-1 != ::WizStrStrI_Pos(it->strName, strFindName)) {
            arrayTagRet.push_back(*it);
        }
    }

    return true;
}

bool CWizIndex::TagsTextToTagArray(CString strText, CWizTagDataArray& arrayTag)
{
	CWizStdStringArray arrayText;
	WizSplitTextToArray(strText, ';', arrayText);
    WizStringArrayRemoveMultiElementNoCase(arrayText);

    CWizStdStringArray::const_iterator it;
    for (it = arrayText.begin(); it != arrayText.end(); it++) {
		CString strName = *it;
		strName.Trim();
		if (strName.IsEmpty())
			continue;

		WIZTAGDATA data;
        if (!TagByNameEx(strName, data)) {
			TOLOG1(_T("Failed to create tag %1"), strName);
            return false;
		}

		arrayTag.push_back(data);
	}

    return true;
}

bool CWizIndex::StyleByName(const CString& strName, WIZSTYLEDATA& data, const CString& strExceptGUID /*= ""*/)
{
	CWizStyleDataArray arrayStyle;
    if (!GetStyles(arrayStyle)) {
		TOLOG(_T("Failed to get Styles!"));
        return false;
	}

    CWizStyleDataArray::const_iterator it;
    for (it = arrayStyle.begin(); it != arrayStyle.end(); it++) {
        if (0 == it->strName.CompareNoCase(strName)) {
            if (0 == strExceptGUID.CompareNoCase(it->strGUID))
				continue;

			data = *it;
            return true;
		}
	}

    return false;
}

bool CWizIndex::createMessage(const WIZMESSAGEDATA& data)
{
    return createMessageEx(data);
}

bool CWizIndex::getAllMessages(CWizMessageDataArray& arrayMsg)
{
    QString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_MESSAGE,
                                    FIELD_LIST_WIZ_MESSAGE);
    return SQLToMessageDataArray(strSQL, arrayMsg);
}

bool CWizIndex::getAllMessageSenders(CWizStdStringArray& arraySender)
{
    QString strSQL = WizFormatString1("SELECT distinct SENDER_GUID from %1",
                                      TABLE_NAME_WIZ_MESSAGE);
    return SQLToStringArray(strSQL, 0, arraySender);
}

bool CWizIndex::getLastestMessages(CWizMessageDataArray& arrayMsg, int nMax)
{
    CString strExt;
    strExt.Format("order by DT_CREATED desc limit 0, %s",
                  WizIntToStr(nMax).utf16());
    QString strSQL = FormatCanonicSQL(TABLE_NAME_WIZ_MESSAGE,
                                      FIELD_LIST_WIZ_MESSAGE,
                                      strExt);

    return SQLToMessageDataArray(strSQL, arrayMsg);
}

bool CWizIndex::setMessageReadStatus(const WIZMESSAGEDATA& msg)
{
    WIZMESSAGEDATA readMsg;
    if (!messageFromId(msg.nId, readMsg))
        return false;

    if (readMsg.nReadStatus != 1) {
        readMsg.nReadStatus = 1;
        readMsg.nLocalChanged = readMsg.nLocalChanged | WIZMESSAGEDATA::localChanged_Read;
        return modifyMessageEx(readMsg);
    }
    return true;
}

bool CWizIndex::setMessageDeleteStatus(const WIZMESSAGEDATA& msg)
{
    WIZMESSAGEDATA delMsg;
    if (!messageFromId(msg.nId, delMsg))
        return false;

    if (delMsg.nDeleteStatus != 1) {
        delMsg.nDeleteStatus = 1;
        delMsg.nLocalChanged = delMsg.nLocalChanged | WIZMESSAGEDATA::localChanged_Delete;
        return modifyMessageEx(delMsg);
    }
    return true;
}


bool CWizIndex::getModifiedMessages(CWizMessageDataArray& arrayMsg)
{
    CString strExt;
    strExt.Format("where LOCAL_CHANGED>0");
    QString strSQL = FormatCanonicSQL(TABLE_NAME_WIZ_MESSAGE,
                                      FIELD_LIST_WIZ_MESSAGE,
                                      strExt);

    return SQLToMessageDataArray(strSQL, arrayMsg);
}

bool CWizIndex::getUnreadMessages(CWizMessageDataArray& arrayMsg)
{
    CString strExt;
    strExt.Format("where READ_STATUS=0 order by DT_CREATED desc");
    QString strSQL = FormatCanonicSQL(TABLE_NAME_WIZ_MESSAGE,
                                      FIELD_LIST_WIZ_MESSAGE,
                                      strExt);

    return SQLToMessageDataArray(strSQL, arrayMsg);
}

bool CWizIndex::modifyMessageLocalChanged(const WIZMESSAGEDATA& msg)
{
    CString strSQL = WizFormatString4("update %1 set LOCAL_CHANGED=%2 where %3=%4",
        TABLE_NAME_WIZ_MESSAGE,
        WizIntToStr(msg.nLocalChanged),
        TABLE_KEY_WIZ_MESSAGE,
        WizInt64ToStr(msg.nId));

    return ExecSQL(strSQL);
}

int CWizIndex::getUnreadMessageCount()
{
    CString strSQL;
    strSQL.Format("select count(*) from WIZ_MESSAGE where READ_STATUS=0 and DELETE_STATUS=0");

    CppSQLite3Query query = m_db.execQuery(strSQL);

    if (!query.eof()) {
        int nCount = query.getIntField(0);
        return nCount;
    }

    return 0;
}


bool CWizIndex::CreateTag(const CString& strParentTagGUID,
                          const CString& strName,
                          const CString& strDescription,
                          WIZTAGDATA& data)
{
    if (strName.isEmpty()) {
        TOLOG("Tag name is empty");
        return false;
    }

    CWizTagDataArray arrayTag;
    if (TagByName(strName, arrayTag)) {
        for (WIZTAGDATA tag : arrayTag) {
            if (tag.strParentGUID == strParentTagGUID) {
                TOLOG1("[WARNING]Tag already exists: %1", tag.strName);
                data = tag;
                return false;
            }
        }
    }

    data.strKbGUID = kbGUID();
	data.strGUID = WizGenGUIDLowerCaseLetterOnly();
    data.strParentGUID = strParentTagGUID;
    data.strName = strName;
    data.strDescription = strDescription;
	data.tModified = WizGetCurrentTime();
    data.nVersion = -1;
    data.nPostion = 0;

	return CreateTagEx(data);
}

bool CWizIndex::CreateStyle(const CString& strName, const CString& strDescription,
                            COLORREF crTextColor, COLORREF crBackColor, bool bTextBold,
                            int nFlagIndex, WIZSTYLEDATA& data)
{
    if (!strName) {
		TOLOG(_T("NULL Pointer: CreateStyle:Name!"));
        return false;
	}

    if (!*strName) {
		TOLOG(_T("Style name is empty"));
        return false;
	}

    if (StyleByName(strName, data)) {
		TOLOG1(_T("Style already exists: %1!"), data.strName);
        return false;
	}

	data.strGUID = WizGenGUIDLowerCaseLetterOnly();
    data.strName = strName;
    data.strDescription = strDescription;
	data.crTextColor = crTextColor;
	data.crBackColor = crBackColor;
	data.bTextBold = bTextBold;
	data.nFlagIndex = nFlagIndex;
	data.tModified = WizGetCurrentTime();

	return CreateStyleEx(data);
}

bool CWizIndex::CreateDocument(const CString& strTitle, const CString& strName, \
                            const CString& strLocation, const CString& strURL, \
                            const CString& strAuthor, const CString& strKeywords, \
                            const CString& strType, const CString& strOwner, \
                            const CString& strFileType, const CString& strStyleGUID, \
                            int nIconIndex, int nSync, int nProtected, WIZDOCUMENTDATA& data)
{
    Q_UNUSED(strOwner);

    if (strTitle.IsEmpty()) {
		TOLOG(_T("NULL Pointer or Document title is empty: CreateDocument:Title!"));
        return false;
	}

    data.strKbGUID = kbGUID();
	data.strGUID = WizGenGUIDLowerCaseLetterOnly();
    data.strTitle = strTitle;
    data.strName = strName;
	data.strSEO = WizMd5StringNoSpace(data.strGUID);
    data.strLocation = CString(strLocation);
    data.strURL = strURL;
    data.strAuthor = strAuthor;
    data.strKeywords = strKeywords;
    data.strType = strType;
    data.strOwner = data.strOwner;
    data.strFileType = strFileType;
    data.strStyleGUID = strStyleGUID;

	data.tCreated = WizGetCurrentTime();
	data.tModified = data.tCreated;
	data.tAccessed = data.tCreated;

	data.nIconIndex = nIconIndex;
	data.nSync = nSync;
	data.nProtected = nProtected;
	data.nReadCount = 0;
	data.nAttachmentCount = 0;
	data.nIndexed = 0;

	GetNextTitle(data.strLocation, data.strTitle);

	data.tInfoModified = data.tCreated;
	data.strInfoMD5 = CalDocumentInfoMD5(data);
	data.tDataModified = data.tCreated;
	data.strDataMD5 = _T("");
	data.tParamModified = data.tCreated;
	data.strParamMD5 = _T("");

	return CreateDocumentEx(data);

}

bool CWizIndex::CreateDocument(const CString& strTitle, const CString& strName, \
                            const CString& strLocation, const CString& strURL, \
                            WIZDOCUMENTDATA& data)
{
    return CreateDocument(strTitle, strName, \
                          strLocation, strURL, \
                          "", "", "", "", "", "", 0, 0, 0, data);
}

bool CWizIndex::CreateAttachment(const CString& strDocumentGUID, const CString& strName,
                                 const CString& strURL, const CString& strDescription,
                                 const CString& strDataMD5, WIZDOCUMENTATTACHMENTDATA& data)
{
    if (strDocumentGUID.IsEmpty()) {
		TOLOG(_T("NULL Pointer or Document guid is empty: CreateAttachment!"));
        return false;
    }

    if (strName.IsEmpty()) {
		TOLOG(_T("NULL Pointer or name is empty: CreateAttachment!"));
        return false;
    }

	data.strGUID = WizGenGUIDLowerCaseLetterOnly();
    data.strDocumentGUID = CString(strDocumentGUID).MakeLower();
    data.strName = strName;
    data.strDescription = strDescription;
    data.strURL = strURL;
	data.tInfoModified = WizGetCurrentTime();
	data.strInfoMD5 = CalDocumentAttachmentInfoMD5(data);
	data.tDataModified = data.tInfoModified;
    data.strDataMD5 = strDataMD5;

	return CreateAttachmentEx(data);
}

bool CWizIndex::TitleExists(const CString& strLocation, CString strTitle)
{
	strTitle.MakeLower();

	CString strSQL;
	strSQL.Format(_T("select DOCUMENT_GUID from WIZ_DOCUMENT where DOCUMENT_LOCATION like %s and lower(DOCUMENT_TITLE)=%s"),
        STR2SQL(strLocation).utf16(),
        STR2SQL(strTitle).utf16()
        );

	try
	{
		CppSQLite3Query query = m_db.execQuery(strSQL);
        if (!query.eof()) {
            return true;
		}

        return false;
	}
	catch (const CppSQLite3Exception& e)
	{
		LogSQLException(e, strSQL);
	}

    return true;
}

bool CWizIndex::GetNextTitle(const QString& strLocation, QString& strTitle)
{
	CString strTemplate(strTitle);
    Utils::Misc::extractTitleTemplate(strTemplate);
	strTitle = strTemplate;

    if (!TitleExists(strLocation, strTitle))
        return true;

	int nIndex = 2;
    while (TitleExists(strLocation, strTitle)) {
		strTitle = WizFormatString2(_T("%1 (%2)"), strTemplate, WizIntToStr(nIndex));
		nIndex++;
	}

    return true;
}

QString CWizIndex::getTableStructureVersion()
{
    return GetMetaDef("TableStructure", "Version");
}

bool CWizIndex::setTableStructureVersion(const QString& strVersion)
{
    return SetMeta("TableStructure", "Version", strVersion);
}


CString CWizIndex::CalDocumentInfoMD5(const WIZDOCUMENTDATA& data)
{
	return WizMd5StringNoSpace(data.strTitle + data.strAuthor 
		+ data.strSEO + data.strKeywords 
		+ data.strType + data.strOwner + data.strFileType
		+ data.strURL + data.strLocation + WizIntToStr(data.nIconIndex)
		+ WizIntToStr(data.nProtected)
		+ WizIntToStr(data.nAttachmentCount)
		+ data.strStyleGUID
        + GetDocumentTagGUIDsString(data.strGUID) + data.strDataMD5);
}

CString CWizIndex::CalDocumentParamInfoMD5(const CWizDocumentParamDataArray& arrayParam)
{
    CString strFeed;

    CWizDocumentParamDataArray::const_iterator it;
    for (it = arrayParam.begin(); it != arrayParam.end(); it++) {
		strFeed += (it->strName  + it->strValue);
	}

	return WizMd5StringNoSpace(strFeed);
}

CString CWizIndex::CalDocumentParamInfoMD5(const WIZDOCUMENTDATA& data)
{
	CWizDocumentParamDataArray arrayParam;
    if (!GetDocumentParams(data.strGUID, arrayParam)) {
		TOLOG1(_T("Failed to get document params: %1"), data.strTitle);
		return CString();
	}

	return CalDocumentParamInfoMD5(arrayParam);
}

CString CWizIndex::CalDocumentAttachmentInfoMD5(const WIZDOCUMENTATTACHMENTDATA& data)
{
    return WizMd5StringNoSpace(data.strDocumentGUID \
                               + data.strName \
                               + data.strURL \
                               + data.strDescription);
}

bool CWizIndex::ModifyTag(WIZTAGDATA& data)
{
    if (data.strKbGUID.isEmpty()) {
        TOLOG("ModifyTag: Failed to modify tag: kbguid is empty");
        return false;
    }

    if (data.strGUID.IsEmpty()) {
        TOLOG("ModifyTag: Failed to modify tag: guid is empty");
        return false;
	}

    if (data.strName.IsEmpty()) {
        TOLOG("ModifyTag: Failed to modify tag: name is empty");
        return false;
    }

    CWizTagDataArray arrayTag;
    if (TagByName(data.strName, arrayTag, data.strGUID)) {
        for (WIZTAGDATA tagItem : arrayTag) {
            if (tagItem.strParentGUID == data.strParentGUID) {
                TOLOG1("ModifyTag: Tag already exists with the same parent: %1", data.strName);
                return false;
            }
        }
    }

	data.tModified = WizGetCurrentTime();
    data.nVersion = -1;

	return ModifyTagEx(data);
}

bool CWizIndex::DeleteTag(const WIZTAGDATA& data, bool bLog, bool bReset /* = true */)
{
    if (!DeleteTagDocuments(data, bReset)) {
        TOLOG1("Failed to delete documents of tag: %1", data.strName);
        return false;
    }

    if (bLog) {
        if (!LogDeletedGUID(data.strGUID, wizobjectTag)) {
            TOLOG("Warning: Failed to log deleted tag guid!");
        }
    }

    return DeleteTagEx(data);
}

bool CWizIndex::ModifyTagPosition(const WIZTAGDATA& data)
{
    CString strSQL = WizFormatString2(_T("update WIZ_TAG set TAG_POS=%1 where TAG_GUID=%2"),
        WizInt64ToStr(data.nPostion),
        STR2SQL(data.strGUID));

    //
    if (!ExecSQL(strSQL))
        return false;

    return true;
}

QString CWizIndex::getTagTreeText(const QString& strTagGUID)
{
    WIZTAGDATA tag;
    if (!TagFromGUID(strTagGUID, tag)) {
        return NULL;
    }

    QString strText = "/" + tag.strName;

    QString strTagParentGUID = tag.strParentGUID;
    while(!strTagParentGUID.isEmpty()) {
        if (!TagFromGUID(strTagParentGUID, tag)) {
            return strText;
        }

        strTagParentGUID = tag.strParentGUID;

        QString strCurrent = "/" + tag.strName;
        strText.prepend(strCurrent);
    }

    return strText;
}

bool CWizIndex::ModifyStyle(WIZSTYLEDATA& data)
{
    if (data.strGUID.IsEmpty()) {
		TOLOG(_T("Failed to modify style: guid is empty!"));
        return false;
    }

    if (data.strName.IsEmpty()) {
		TOLOG(_T("Failed to modify style: name is empty!"));
        return false;
	}

	WIZSTYLEDATA dataTemp;
    if (StyleByName(data.strName, dataTemp, data.strGUID)) {
		TOLOG1(_T("Failed to modify style: Style already exists: %1!"), data.strName);
        return false;
	}

	data.tModified = WizGetCurrentTime();
    data.nVersion = -1;

	return ModifyStyleEx(data);
}

bool CWizIndex::DeleteStyle(const WIZSTYLEDATA& data, bool bLog, bool bReset /* = true */)
{
    if(!DeleteStyleDocuments(data, bReset)) {
        TOLOG1("Failed to delete documents of style: %1", data.strName);
        return false;
    }

    if (bLog) {
        if (!LogDeletedGUID(data.strGUID, wizobjectStyle)) {
            TOLOG("Warning: Failed to log deleted style guid!");
        }
    }

    return DeleteStyleEx(data);
}

bool CWizIndex::ModifyDocumentInfo(WIZDOCUMENTDATA& data, bool bReset /* = true */)
{
    if (data.strGUID.isEmpty()) {
        TOLOG("Failed to modify document: guid is empty!");
        return false;
    }

    if (data.strTitle.isEmpty()) {
        TOLOG("Failed to modify document: title is empty!");
        return false;
	}
    //

    if (bReset) {
        data.tInfoModified = WizGetCurrentTime();
        data.strInfoMD5 = CalDocumentInfoMD5(data);
        data.tModified = WizGetCurrentTime();
        data.nVersion = -1;
        //
        qDebug() << "Reset document info & version, guid: " << data.strGUID << ", title: " << data.strTitle;
    }

	return ModifyDocumentInfoEx(data);
}

bool CWizIndex::ModifyDocumentDateModified(WIZDOCUMENTDATA& data)
{
    if (data.strGUID.isEmpty()) {
		TOLOG(_T("Failed to modify document: guid is empty!"));
        return false;
	}

	data.tInfoModified = WizGetCurrentTime();
	data.strInfoMD5 = CalDocumentInfoMD5(data);
    data.nVersion = -1;

	return ModifyDocumentInfoEx(data);
}

bool CWizIndex::ModifyDocumentDataDateModified(WIZDOCUMENTDATA& data)
{
    if (data.strGUID.isEmpty()) {
		TOLOG(_T("Failed to modify document: guid is empty!"));
        return false;
    }

	data.strInfoMD5 = CalDocumentInfoMD5(data);
    data.nVersion = -1;

	return ModifyDocumentInfoEx(data);
}

bool CWizIndex::ModifyDocumentDateAccessed(WIZDOCUMENTDATA& data)
{
    Q_ASSERT(data.strKbGUID == kbGUID());

	CString strSQL;
	strSQL.Format(_T("update WIZ_DOCUMENT set DT_ACCESSED=%s where DOCUMENT_GUID=%s"),
        TIME2SQL(data.tAccessed).utf16(),
        STR2SQL(data.strGUID).utf16()
        );

	if (!ExecSQL(strSQL))
        return false;

    emit documentAccessDateModified(data);
    return true;
}

bool CWizIndex::ModifyDocumentReadCount(const WIZDOCUMENTDATA& data)
{
    WIZDOCUMENTDATA dataOld;
    DocumentFromGUID(data.strGUID, dataOld);

	CString strSQL;
	strSQL.Format(_T("update WIZ_DOCUMENT set DOCUMENT_READ_COUNT=%d where DOCUMENT_GUID=%s"),
		data.nReadCount,
        STR2SQL(data.strGUID).utf16()
        );

    bool ret = ExecSQL(strSQL);
    WIZDOCUMENTDATA dataNew;
    DocumentFromGUID(data.strGUID, dataNew);

    emit documentReadCountChanged(dataNew);
    return ret;
}

bool CWizIndex::ModifyDocumentLocation(WIZDOCUMENTDATA& data)
{
    if (data.strGUID.isEmpty()) {
        TOLOG(_T("Failed to modify document: guid is empty!"));
        return false;
    }

    data.strInfoMD5 = CalDocumentInfoMD5(data);
    data.tInfoModified = WizGetCurrentTime();
    data.nVersion = -1;

    return ModifyDocumentInfoEx(data);
}

bool CWizIndex::DeleteDocument(const WIZDOCUMENTDATA& data, bool bLog)
{
    WIZDOCUMENTDATA dataTemp = data;

    // don't have to reset md5 and version
    if (!DeleteDocumentTags(dataTemp, false)) {
        TOLOG1("Failed to delete document tags: %1", data.strTitle);
        return false;
    }

    // as above
    if (!DeleteDocumentParams(data.strGUID, false)) {
        TOLOG1("Failed to delete document params: %1", data.strTitle);
        return false;
    }

    if (bLog) {
        if (!LogDeletedGUID(data.strGUID, wizobjectDocument)) {
            TOLOG("Warning: Failed to log deleted document guid!");
        }
    }

    return DeleteDocumentEx(data);
}

void CWizIndex::InitDocumentExFields(CWizDocumentDataArray& arrayDocument,
                                     const CWizStdStringArray& arrayGUID,
                                     const std::map<CString, int>& mapDocumentIndex)
{
    CString strDocumentGUIDs;
    ::WizStringArrayToText(arrayGUID, strDocumentGUIDs, _T("\',\'"));

    CString strParamSQL = WizFormatString1(_T("select DOCUMENT_GUID, PARAM_NAME, PARAM_VALUE from WIZ_DOCUMENT_PARAM where (PARAM_NAME='DOCUMENT_FLAGS' or PARAM_NAME='RATE' or PARAM_NAME='SYSTEM_TAGS') and DOCUMENT_GUID in('%1')"), strDocumentGUIDs);

    CppSQLite3Query queryParam = m_db.execQuery(strParamSQL);
    while (!queryParam.eof())
    {
        CString strGUID = queryParam.getStringField(0);
        CString strParamName = queryParam.getStringField(1);

        std::map<CString, int>::const_iterator it = mapDocumentIndex.find(strGUID);
        Q_ASSERT(it != mapDocumentIndex.end());
        if (it != mapDocumentIndex.end())
        {
            int index = it->second;
            Q_ASSERT(index >= 0 && index < int(arrayDocument.size()));
            if (index >= 0 && index < int(arrayDocument.size()))
            {
                WIZDOCUMENTDATA& data = arrayDocument[index];
                Q_ASSERT(strGUID == data.strGUID);
                if (strGUID == data.strGUID)
                {
                    if (strParamName == _T(TABLE_KEY_WIZ_DOCUMENT_PARAM_FLAGS))
                    {
                        int nFlags = queryParam.getIntField(2);
                        data.nFlags = nFlags;
                    }
                    else if (strParamName == _T("RATE"))
                    {
                        int nRate = queryParam.getIntField(2);
                        data.nRate = nRate;
                    }
                    else if (strParamName == _T("SYSTEM_TAGS"))
                    {
                        CString strSystemTags = queryParam.getStringField(2);
                        data.strSystemTags = strSystemTags;
                    }
                }
            }
        }

        queryParam.nextRow();
    }

    InitDocumentShareFlags(arrayDocument, strDocumentGUIDs, mapDocumentIndex, TAG_NAME_SHARE_WITH_FRIENDS, WIZDOCUMENT_SHARE_FRIENDS);
    InitDocumentShareFlags(arrayDocument, strDocumentGUIDs, mapDocumentIndex, TAG_NAME_PUBLIC, WIZDOCUMENT_SHARE_PUBLIC);
}

void CWizIndex::InitDocumentShareFlags(CWizDocumentDataArray& arrayDocument,
                                           const CString& strDocumentGUIDs,
                                           const std::map<CString, int>& mapDocumentIndex,
                                           const CString& strTagName,
                                           int nShareFlags)
{        
    CWizTagDataArray arrayTag;
    TagByName(strTagName, arrayTag);

    for (WIZTAGDATA dataShare : arrayTag) {
        GetAllChildTags(dataShare.strGUID, arrayTag);
    }


    CWizStdStringArray arrayTagGUID;
    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        arrayTagGUID.push_back(it->strGUID);
    }

    CString strTagGUIDs;
    ::WizStringArrayToText(arrayTagGUID, strTagGUIDs, _T("\',\'"));

    CString strTagSQL = WizFormatString2(_T("select distinct DOCUMENT_GUID from WIZ_DOCUMENT_TAG where DOCUMENT_GUID in('%1') and TAG_GUID in ('%2')"), strDocumentGUIDs, strTagGUIDs);

    CppSQLite3Query queryTag = m_db.execQuery(strTagSQL);
    while (!queryTag.eof())
    {
        CString strGUID = queryTag.getStringField(0);

        std::map<CString, int>::const_iterator it = mapDocumentIndex.find(strGUID);
        ATLASSERT(it != mapDocumentIndex.end());
        if (it != mapDocumentIndex.end())
        {
            int index = it->second;
            ATLASSERT(index >= 0 && index < int(arrayDocument.size()));
            if (index >= 0 && index < int(arrayDocument.size()))
            {
                WIZDOCUMENTDATA& data = arrayDocument[index];
                ATLASSERT(strGUID == data.strGUID);
                if (strGUID == data.strGUID)
                {
                    data.nShareFlags = nShareFlags;
                }
            }
        }

        queryTag.nextRow();
    }
}

bool CWizIndex::getGroupUnreadDocuments(CWizDocumentDataArray& arrayDocument)
{

    CString strExt;
    strExt.Format("where DOCUMENT_READ_COUNT=0 and DOCUMENT_LOCATION not like '/Deleted Items/%'");
    QString strSQL = FormatCanonicSQL(TABLE_NAME_WIZ_DOCUMENT,
                                      FIELD_LIST_WIZ_DOCUMENT,
                                      strExt);

    return SQLToDocumentDataArray(strSQL, arrayDocument);
}

int CWizIndex::getGroupUnreadDocumentCount()
{
    CString strSQL;
    strSQL.Format("select count(*) from WIZ_DOCUMENT where DOCUMENT_READ_COUNT=0 and DOCUMENT_LOCATION not like '/Deleted Items/%'");

    CppSQLite3Query query = m_db.execQuery(strSQL);

    if (!query.eof()) {
        int nCount = query.getIntField(0);
        return nCount;
    }


    return 0;
}

void CWizIndex::setGroupDocumentsReaded()
{
    CString strSQL = WizFormatString0("update WIZ_DOCUMENT set DOCUMENT_READ_COUNT=1 where DOCUMENT_READ_COUNT=0");
    ExecSQL(strSQL);
    emit groupDocumentUnreadCountModified(kbGUID());
}

CString CWizIndex::KbGUIDToSQL()
{
    CString strKbGUIDSQL;
    if (kbGUID().isEmpty())
    {
        strKbGUIDSQL = _T(" (KB_GUID is null or KB_GUID = '') ");
    }
    else
    {
        strKbGUIDSQL = WizFormatString1(_T("KB_GUID = '%1'"), kbGUID());
    }
    //
    return strKbGUIDSQL;
}

bool CWizIndex::ModifyAttachmentInfo(WIZDOCUMENTATTACHMENTDATA& data)
{
    if (data.strGUID.isEmpty()) {
        TOLOG("Failed to modify attachment: guid is empty!");
        return false;
    }

    if (data.strName.isEmpty()) {
        TOLOG("Failed to modify attachment: name is empty!");
        return false;
	}

	data.tInfoModified = WizGetCurrentTime();
	data.strInfoMD5 = CalDocumentAttachmentInfoMD5(data);
    data.nVersion = -1;

	return ModifyAttachmentInfoEx(data);
}

bool CWizIndex::DeleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data,
                                 bool bLog, bool bResetDocInfo, bool /*update*/)
{
    if (!DeleteAttachmentEx(data)) {
        TOLOG1("Failed to delete attachment: %1", data.strName);
        return false;
    }

    if (bLog) {
        if (!LogDeletedGUID(data.strGUID, wizobjectDocumentAttachment)) {
            TOLOG("Warning: Failed to log deleted attachment guid!");
        }
    }

    UpdateDocumentAttachmentCount(data.strDocumentGUID, bResetDocInfo);

    return true;
}

bool CWizIndex::GetDocumentsGUIDByLocation(const CString& strLocation, CWizStdStringArray& arrayGUID)
{
    CString strSQL;
	strSQL.Format(_T("select DOCUMENT_GUID from WIZ_DOCUMENT where DOCUMENT_LOCATION like '%s%%'"),
        strLocation.utf16()
        );

	return SQLToStringArray(strSQL, 0, arrayGUID);
}

bool CWizIndex::DeleteDocumentsTagsByLocation(const CString& strLocation)
{
	CString strSQL;
	strSQL.Format(_T("delete from WIZ_DOCUMENT_TAG where DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT where DOCUMENT_LOCATION like '%s%%')"),
        strLocation.utf16()
        );
    bool bRet = ExecSQL(strSQL);

    if (!bRet)
        return false;

    return true;
}

bool CWizIndex::DeleteDocumentsByLocation(const CString& strLocation)
{
    if (!DeleteDocumentsTagsByLocation(strLocation)) {
        TOLOG1(_T("Failed to delete document tags by location: %1"), strLocation);
        return false;
	}

    CWizStdStringArray arrayGUID;
    if (GetDocumentsGUIDByLocation(strLocation, arrayGUID)) {
        if (!LogDeletedGUIDs(arrayGUID, wizobjectDocument)) {
            TOLOG1(_T("Warning: Failed to log document guids by location: %1"), strLocation);
		}
	}

	CString strSQL;
	strSQL.Format(_T("delete from WIZ_DOCUMENT where DOCUMENT_LOCATION like '%s%%'"),
        strLocation.utf16()
        );

    return ExecSQL(strSQL);
}

bool CWizIndex::UpdateDocumentInfoMD5(WIZDOCUMENTDATA& data)
{
//	data.tModified = WizGetCurrentTime();
	data.tInfoModified = WizGetCurrentTime();
	data.strInfoMD5 = CalDocumentInfoMD5(data);
	data.nVersion = -1;

	return ModifyDocumentInfoEx(data);
}

bool CWizIndex::UpdateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName, bool notifyDataModify /*= true*/)
{
    data.tModified = WizGetCurrentTime();

    data.strDataMD5 = ::WizMd5FileString(strZipFileName);
    data.tDataModified = WizGetCurrentTime();

    // modify note data lead modify time change, recount info md5 needed
    data.strInfoMD5 = CalDocumentInfoMD5(data);
    data.tInfoModified = WizGetCurrentTime();

	data.nVersion = -1;

    bool bRet = ModifyDocumentInfoEx(data);

    if (notifyDataModify)
    {
        Q_EMIT documentDataModified(data);
    }
    //
    return bRet;
}

bool CWizIndex::DeleteDocumentTags(WIZDOCUMENTDATA& data, bool bReset /* = true */)
{
    CString strFormat = FormatDeleteSQLFormat(TABLE_NAME_WIZ_DOCUMENT_TAG,
                                              "DOCUMENT_GUID");

	CString strSQL;
	strSQL.Format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

	if (!ExecSQL(strSQL))
        return false;

    bool bRet = true;

    if (bReset) {
        bRet = UpdateDocumentInfoMD5(data);
    }

    Q_EMIT documentTagModified(data);

    return bRet;
}

bool CWizIndex::DeleteDocumentParams(const QString& strDocumentGUID,
                                     bool bReset /* = true */)
{
    CString strFormat = FormatDeleteSQLFormat(TABLE_NAME_WIZ_DOCUMENT_PARAM,
                                              "DOCUMENT_GUID");

	CString strSQL;
	strSQL.Format(strFormat,
        STR2SQL(strDocumentGUID).utf16()
        );

	if (!ExecSQL(strSQL))
        return false;

    if (bReset && !UpdateDocumentParamMD5(strDocumentGUID)) {
        TOLOG("Failed to update document param md5, guid: " + strDocumentGUID);
        return false;
    }

    return true;
}

bool CWizIndex::DeleteDocumentParam(const CString& strDocumentGUID, CString strParamName, bool bUpdateParamMD5)
{
	if (strParamName.IsEmpty())
        return false;

	strParamName.MakeUpper();

	CString strSQL;
	strSQL.Format(_T("delete from %s where DOCUMENT_GUID=%s and PARAM_NAME=%s"),
        QString(TABLE_NAME_WIZ_DOCUMENT_PARAM).utf16(),
        STR2SQL(strDocumentGUID).utf16(),
        STR2SQL(strParamName).utf16()
		);

	if (!ExecSQL(strSQL))
        return false;

    if (bUpdateParamMD5) {
        if (!UpdateDocumentParamMD5(strDocumentGUID)) {
			TOLOG(_T("Failed to update document param md5!"));
		}
	}

    return true;
}

bool CWizIndex::DeleteDocumentParamEx(const CString& strDocumentGUID, CString strParamNamePart)
{
	if (strParamNamePart.IsEmpty())
        return false;

    strParamNamePart.MakeUpper();

	CString strSQL;
	strSQL.Format(_T("delete from %s where DOCUMENT_GUID=%s and PARAM_NAME like '%s%%'"),
        QString(TABLE_NAME_WIZ_DOCUMENT_PARAM).utf16(),
        STR2SQL(strDocumentGUID).utf16(),
        strParamNamePart.utf16()
		);

	if (!ExecSQL(strSQL))
        return false;

    if (!UpdateDocumentParamMD5(strDocumentGUID)) {
		TOLOG(_T("Failed to update document param md5!"));
    }

    return true;
}

bool CWizIndex::DeleteDeletedGUID(const CString& strGUID)
{
	CString strFormat = FormatDeleteSQLFormat(TABLE_NAME_WIZ_DELETED_GUID, _T("DELETED_GUID"));

	CString strSQL;
	strSQL.Format(strFormat,
        STR2SQL(strGUID).utf16()
        );

    return ExecSQL(strSQL);
}

bool CWizIndex::IsObjectDeleted(const CString& strGUID)
{
    CString strWhere = CString("DELETED_GUID = '%1'").arg(strGUID);
    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID, strWhere);
    CWizDeletedGUIDDataArray arrayGUID;
    SQLToDeletedGUIDDataArray(strSQL, arrayGUID);
    return arrayGUID.size() > 0;
}

bool CWizIndex::UpdateDocumentsInfoMD5(CWizDocumentDataArray& arrayDocument)
{
    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        if (!UpdateDocumentInfoMD5(*it)) {
			TOLOG(_T("Failed to update document info!"));
		}
	}

    return true;
}

bool CWizIndex::DeleteTagDocuments(const WIZTAGDATA& data, bool bReset)
{
	CWizDocumentDataArray arrayDocument;
    GetDocumentsByTag("", data, arrayDocument, true);

    CString strFormat = FormatDeleteSQLFormat(TABLE_NAME_WIZ_DOCUMENT_TAG,
                                              "TAG_GUID");

	CString strSQL;
	strSQL.Format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

    if (!ExecSQL(strSQL)) {
        TOLOG1("Failed to delete documents of tag: %1", data.strName);
        return false;
	}

    if (bReset) {
        UpdateDocumentsInfoMD5(arrayDocument);
    }

    return true;
}

bool CWizIndex::DeleteStyleDocuments(const WIZSTYLEDATA& data, bool bReset)
{
	CWizDocumentDataArray arrayDocument;
    GetDocumentsByStyle("", data, arrayDocument);

	CString strSQL;
    strSQL.Format("update WIZ_DOCUMENT set STYLE_GUID=null where STYLE_GUID=%s",
        STR2SQL(data.strGUID).utf16()
        );

    if (!ExecSQL(strSQL)) {
        TOLOG1("Failed to delete documents of style: %1", data.strName);
        return false;
	}

    if (bReset) {
        UpdateDocumentsInfoMD5(arrayDocument);
    }

    return true;
}

bool CWizIndex::SetDocumentTags(WIZDOCUMENTDATA& data,
                                const CWizStdStringArray& arrayTagGUID,
                                bool bReset /* = true */)
{
    CWizStdStringArray arrayTagOld;
    if (GetDocumentTags(data.strGUID, arrayTagOld)) {
        if (WizKMStringArrayIsEqual<CString>(arrayTagOld, arrayTagGUID))
            return true;
    }

    if (!DeleteDocumentTags(data, bReset)) {
        TOLOG("Failed to delete document tags");
        return false;
	}

    CWizStdStringArray::const_iterator it;
    for (it = arrayTagGUID.begin(); it != arrayTagGUID.end(); it++) {
        if (!InsertDocumentTag(data, *it, bReset)) {
            TOLOG("Failed to insert document-tag record!");
            return false;
		}
	}

    return true;
}

bool CWizIndex::GetDocumentTags(const CString& strDocumentGUID, CWizStdStringArray& arrayTagGUID)
{
    CString strWhere = WizFormatString1(_T("DOCUMENT_GUID='%1'"), strDocumentGUID);
	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT_TAG, FIELD_LIST_WIZ_DOCUMENT_TAG, strWhere); 

    if (!SQLToStringArray(strSQL, 1, arrayTagGUID))
        return false;

    size_t tagCount = arrayTagGUID.size();
    if (tagCount == 0)
        return true;

    WIZDOCUMENTDATA dataDocument;
    if (!DocumentFromGUID(strDocumentGUID, dataDocument))
        return false;

    for (intptr_t i = tagCount - 1; i >= 0; i--)
    {
        WIZTAGDATA temp;
        if (!TagFromGUID(arrayTagGUID[i], temp))
        {
            DeleteDocumentTag(dataDocument, arrayTagGUID[i]);
            arrayTagGUID.erase(arrayTagGUID.begin() + i);
            continue;
        }
    }

    return true;
}

CString CWizIndex::GetDocumentTagsText(const CString& strDocumentGUID)
{
    CWizStdStringArray arrayTagName;
    GetDocumentTagsNameStringArray(strDocumentGUID, arrayTagName);

    CString strText;
    ::WizStringArrayToText(arrayTagName, strText, "; ");

    return strText;
}

bool CWizIndex::GetDocumentTagsNameStringArray(const CString& strDocumentGUID, CWizStdStringArray& arrayTagName)
{
	CWizTagDataArray arrayTag;
    if (!GetDocumentTags(strDocumentGUID, arrayTag)) {
        TOLOG1(_T("Failed to get document tags: %1"), strDocumentGUID);
        return false;
	}

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
		arrayTagName.push_back(it->strName);
	}

    return true;
}

int CWizIndex::GetDocumentTagCount(const CString& strDocumentGUID)
{
    CString strSQL = CString("select count(TAG_GUID) from WIZ_DOCUMENT_TAG where DOCUMENT_GUID='%1'").arg(strDocumentGUID);

    CppSQLite3Query query = m_db.execQuery(strSQL);

    if (!query.eof()) {
        int nCount = query.getIntField(0);
        return nCount;
    }

    return 0;
}

CString CWizIndex::GetDocumentTagNameText(const CString& strDocumentGUID)
{
	CString strText;

	CWizStdStringArray arrayTagName;
    if (!GetDocumentTagsNameStringArray(strDocumentGUID, arrayTagName))
		return strText;

	WizStringArrayToText(arrayTagName, strText, _T("; "));

	return strText;
}

bool CWizIndex::GetDocumentTagsDisplayNameStringArray(const CString& strDocumentGUID, CWizStdStringArray& arrayTagDisplayName)
{
    CWizTagDataArray arrayTag;
    if (!GetDocumentTags(strDocumentGUID, arrayTag)) {
        TOLOG1(_T("Failed to get document tags: %1"), strDocumentGUID);
        return false;
    }

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        arrayTagDisplayName.push_back(TagNameToDisplayName(it->strName));
    }

    return true;
}

QString CWizIndex::GetDocumentTagTreeDisplayString(const QString& strDocumentGUID)
{
    CWizTagDataArray arrayTag;
    if (!GetDocumentTags(strDocumentGUID, arrayTag)) {
        return NULL;
    }

    if (arrayTag.size() == 0)
        return "/";

    //Q_ASSERT(arrayTag.size() == 1);

    QString strTree;
    const WIZTAGDATA& tag = arrayTag.at(0);
    QString strTagGUID = tag.strGUID;
    while (1) {
        WIZTAGDATA data;
        if (!TagFromGUID(strTagGUID, data))
            return "/";

        strTree += ("/" + data.strName);

        if (data.strParentGUID.isEmpty())
            break;

        strTagGUID = data.strParentGUID;
    }

    return strTree + "/";
}

CString CWizIndex::GetDocumentTagDisplayNameText(const CString& strDocumentGUID)
{
    CString strText;

    CWizStdStringArray arrayTagDisplayName;
    if (!GetDocumentTagsDisplayNameStringArray(strDocumentGUID, arrayTagDisplayName))
        return strText;

    WizStringArrayToText(arrayTagDisplayName, strText, _T("; "));

    return strText;
}

CString CWizIndex::GetDocumentTagGUIDsString(const CString& strDocumentGUID)
{
	CWizStdStringArray arrayTagGUID;
    if (!GetDocumentTags(strDocumentGUID, arrayTagGUID))
		return CString();

	std::sort(arrayTagGUID.begin(), arrayTagGUID.end());

	CString strText;
	WizStringArrayToText(arrayTagGUID, strText, _T(";"));
	return strText;
}

bool CWizIndex::SetDocumentTags(WIZDOCUMENTDATA& data, const CWizTagDataArray& arrayTag)
{
	CWizTagDataArray arrayTagOld;
    if (GetDocumentTags(data.strGUID, arrayTagOld)) {        
        if (WizKMObjectArrayIsEqual<WIZTAGDATA>(arrayTagOld, arrayTag))
            return true;
	}

    if (!DeleteDocumentTags(data)) {
		TOLOG(_T("Failed to delete document tags!"));
        return false;
    }

    CWizTagDataArray::const_iterator it;
    for (it  = arrayTag.begin(); it != arrayTag.end(); it++) {
        if (!InsertDocumentTag(data, it->strGUID)) {
			TOLOG(_T("Failed to insert document tag record!"));
            return false;
		}
	}

    return true;
}

bool CWizIndex::SetDocumentTagsText(WIZDOCUMENTDATA& data, const CString& strTagsText)
{
	CWizTagDataArray arrayTag;
    if (!TagsTextToTagArray(strTagsText, arrayTag)) {
		TOLOG1(_T("Failed to convert tags text to tag array, Tags text: %1"), strTagsText);
        return false;
	}

    if (!SetDocumentTags(data, arrayTag)) {
		TOLOG2(_T("Failed to set document tags, Document: %1, Tags text: %2"), data.strTitle, strTagsText);
        return false;
	}

    return true;
}

bool CWizIndex::InsertDocumentTag(WIZDOCUMENTDATA& data,
                                  const CString& strTagGUID,
                                  bool bReset /* = true */)
{
    if (strTagGUID.IsEmpty()) {
        TOLOG("NULL Pointer or empty tag guid: insert into wiz_document_tag,!");
        return false;
	}

    CString strFormat = FormatInsertSQLFormat(TABLE_NAME_WIZ_DOCUMENT_TAG,
                                              FIELD_LIST_WIZ_DOCUMENT_TAG,
                                              PARAM_LIST_WIZ_DOCUMENT_TAG);

	CString strSQL;
	strSQL.Format(strFormat,
        STR2SQL(data.strGUID).utf16(),
        STR2SQL(strTagGUID).utf16()
        );

    if (!ExecSQL(strSQL))
        return false;

    bool bRet = true;
    if (bReset)
        bRet = UpdateDocumentInfoMD5(data);

    Q_EMIT documentTagModified(data);

    return bRet;
}

bool CWizIndex::DeleteDocumentTag(WIZDOCUMENTDATA& data, const CString& strTagGUID)
{
    if (strTagGUID.IsEmpty()) {
		TOLOG(_T("NULL Pointer or empty tag guid: delete from wiz_document_tag,!"));
        return false;
	}

	CString strWhere;
	strWhere.Format(_T("DOCUMENT_GUID=%s and TAG_GUID=%s"),
        STR2SQL(data.strGUID).utf16(),
        STR2SQL(strTagGUID).utf16()
		);

	CString strSQL = WizFormatString1(_T("delete from WIZ_DOCUMENT_TAG where %1"), strWhere);

    bool bRet = ExecSQL(strSQL)
		&& UpdateDocumentInfoMD5(data);
	if (!bRet)
        return false;

    Q_EMIT documentTagModified(data);

    return true;
}

bool CWizIndex::GetDocumentParams(const CString& strDocumentGUID, CWizDocumentParamDataArray& arrayParam)
{
    CString strWhere = WizFormatString1(_T("DOCUMENT_GUID='%1'"), strDocumentGUID);
	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT_PARAM, FIELD_LIST_WIZ_DOCUMENT_PARAM, strWhere); 
	return SQLToDocumentParamDataArray(strSQL, arrayParam);
}

bool CWizIndex::GetDocumentParam(const CString& strDocumentGUID, \
                              CString strParamName, \
                              CString& strParamValue, \
                              const CString& strDefault /*= NULL*/, \
                              bool* pbParamExists /*= NULL*/)
{
    if (pbParamExists) {
        *pbParamExists = false;
	}

	strParamName.Trim();
	strParamName.MakeUpper();

	CString strWhere = WizFormatString2(_T("DOCUMENT_GUID=%1 and PARAM_NAME=%2"), 
        STR2SQL(strDocumentGUID),
		STR2SQL(strParamName)
		);

	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT_PARAM, FIELD_LIST_WIZ_DOCUMENT_PARAM, strWhere); 

	CWizDocumentParamDataArray arrayParam;
    if (!SQLToDocumentParamDataArray(strSQL, arrayParam)) {
		TOLOG1(_T("Failed tog get document param: %1"), strParamName);
        return false;
	}

    if (arrayParam.empty()) {
        strParamValue = CString(strDefault);
    } else {
		strParamValue = arrayParam[0].strValue;

        if (arrayParam.size() > 1) {
			TOLOG1(_T("Warning: too more param: %1"), strParamName);
		}

        if (pbParamExists) {
            *pbParamExists = true;
		}
	}

    return true;
}

CString CWizIndex::GetMetaDef(const CString& strMetaName, const CString& strKey, const CString& strDef /*= NULL*/)
{
	CString str;
    GetMeta(strMetaName, strKey, str, strDef);
	return str;
}

bool CWizIndex::GetMeta(CString strMetaName, CString strKey, CString& strValue, \
                     const CString& strDefault /*= NULL*/, bool* pbMetaExists /*= NULL*/)
{
    if (strMetaName.IsEmpty()) {
		TOLOG(_T("Meta name is empty!"));
        return false;
    }

    if (strKey.IsEmpty()) {
		TOLOG(_T("Meta key is empty!"));
        return false;
	}

    if (pbMetaExists) {
        *pbMetaExists = false;
	}

	strMetaName.MakeUpper();
	strKey.MakeUpper();

	CString strWhere = WizFormatString2(_T("META_NAME=%1 and META_KEY=%2"), 
		STR2SQL(strMetaName),
		STR2SQL(strKey)
		);

	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_META, FIELD_LIST_WIZ_META, strWhere); 

	CWizMetaDataArray arrayMeta;
    if (!SQLToMetaDataArray(strSQL, arrayMeta)) {
        TOLOG1(_T("Failed tog get meta: %1"), strMetaName);
        return false;
	}

    if (arrayMeta.empty()) {
        strValue = CString(strDefault);
    } else {
        strValue = arrayMeta[0].strValue;

        if (arrayMeta.size() > 1) {
            TOLOG1(_T("Warning: too more meta: %1"), strMetaName);
		}

        if (pbMetaExists) {
            *pbMetaExists = true;
		}
	}

    return true;
}

bool CWizIndex::SetDocumentParam(const QString& strDocumentGUID,
                                 const QString &strParamName,
                                 const QString &strParamValue,
                                 bool bUpdateParamMD5)
{
	WIZDOCUMENTDATA data;
    if (!DocumentFromGUID(strDocumentGUID, data)) {
        return false;
    }

	return SetDocumentParam(data, strParamName, strParamValue, bUpdateParamMD5);
}

bool CWizIndex::SetDocumentParam(WIZDOCUMENTDATA& data,
                                 const QString& strName,
                                 const QString& strValue,
                                 bool bUpdateParamMD5)
{
    QString strParamName = strName.trimmed();
    QString strParamValue = strValue.trimmed();

    if (data.strGUID.isEmpty()) {
        TOLOG1("Failed to modify document param %1: document guid is empty!", strParamName);
        return false;
    }

    if (strParamName.isEmpty()) {
        TOLOG("Failed to modify document param %1: param name is empty!");
        return false;
	}

	CString strOldParamValue;
    bool bParamExists = false;
    GetDocumentParam(data.strGUID, strParamName, strOldParamValue, "", &bParamExists);

	if (strOldParamValue == strParamValue)
        return true;

    strParamName = strParamName.toUpper();

	CString strSQL;
    if (bParamExists) {
        strSQL.Format("update WIZ_DOCUMENT_PARAM set PARAM_VALUE=%s where DOCUMENT_GUID=%s and PARAM_NAME=%s",
            STR2SQL(strParamValue).utf16(),
            STR2SQL(data.strGUID).utf16(),
            STR2SQL(strParamName).utf16()
			);
    } else {
        CString strFormat = FormatInsertSQLFormat(TABLE_NAME_WIZ_DOCUMENT_PARAM,
                                                  FIELD_LIST_WIZ_DOCUMENT_PARAM,
                                                  PARAM_LIST_WIZ_DOCUMENT_PARAM);

		strSQL.Format(strFormat,
            STR2SQL(data.strGUID).utf16(),
            STR2SQL(strParamName).utf16(),
            STR2SQL(strParamValue).utf16()
			);
	}

    if (!ExecSQL(strSQL)) {
        TOLOG2("Failed to update document %1 param: %2", data.strTitle, strParamName);
        return false;
	}

    if (bUpdateParamMD5) {
		return UpdateDocumentParamMD5(data);
    } else {
        return true;
	}
}

bool CWizIndex::SetDocumentParams(WIZDOCUMENTDATA& data,
                                  const CWizDocumentParamDataArray& arrayParam,
                                  bool bReset /* = true */)
{
    DeleteDocumentParams(data.strGUID, bReset);

    CWizDocumentParamDataArray::const_iterator it;
    for (it = arrayParam.begin(); it != arrayParam.end(); it++) {
        if (!SetDocumentParam(data, it->strName, it->strValue, bReset)) {
            TOLOG2("Failed to set document param: %1=%2", it->strName, it->strValue);
		}
	}

    if (bReset) {
        UpdateDocumentParamMD5(data);
    }

    return true;
}

bool CWizIndex::StringArrayToDocumentParams(const CString& strDocumentGUID,
                                             const CWizStdStringArray& arrayText,
                                             std::deque<WIZDOCUMENTPARAMDATA>& arrayParam)
{
    CWizStdStringArray::const_iterator it;
    for (it = arrayText.begin(); it != arrayText.end(); it++) {
        CString str = *it;

		int nPos = str.Find('=');
        if (-1 == nPos) {
			TOLOG1(_T("String %1 is not a valid param line"), str);
			continue;
        }

		CString strName = str.Left(nPos);
		str.Delete(0, nPos + 1);
        CString strValue = str;

		WIZDOCUMENTPARAMDATA data;
        data.strDocumentGUID = strDocumentGUID;
		data.strName = strName;
		data.strValue = strValue;
		arrayParam.push_back(data);
    }

    return true;
}

bool CWizIndex::SetDocumentParams(WIZDOCUMENTDATA& data,
                                  const CWizStdStringArray& arrayParam,
                                  bool bReset /* = true */)
{
    CWizDocumentParamDataArray arrayRet;
    StringArrayToDocumentParams(data.strGUID, arrayParam, arrayRet);

    return SetDocumentParams(data, arrayRet, bReset);
}

bool CWizIndex::SetMeta(CString strMetaName, CString strKey, const CString& strValue)
{
    if (strMetaName.IsEmpty()) {
		TOLOG(_T("Meta name is empty!"));
        return false;
    }

    if (strKey.IsEmpty()) {
		TOLOG(_T("Meta key is empty!"));
        return false;
	}

	CString strOldValue;
    bool bMetaExists = false;
    GetMeta(strMetaName, strKey, strOldValue, "", &bMetaExists);

    if (strOldValue == strValue)
        return true;

	strMetaName.MakeUpper();
	strKey.MakeUpper();

	CString strSQL;
    if (bMetaExists) {
		strSQL.Format(_T("update WIZ_META set META_VALUE=%s where META_NAME=%s and META_KEY=%s"),
            STR2SQL(strValue).utf16(),
            STR2SQL(strMetaName).utf16(),
            STR2SQL(strKey).utf16()
			);
    } else {
		CString strFormat = FormatInsertSQLFormat(TABLE_NAME_WIZ_META, FIELD_LIST_WIZ_META, PARAM_LIST_WIZ_META);

		strSQL.Format(strFormat,
            STR2SQL(strMetaName).utf16(),
            STR2SQL(strKey).utf16(),
            STR2SQL(strValue).utf16(),
            TIME2SQL(WizGetCurrentTime()).utf16()
			);
	}

    if (!ExecSQL(strSQL)) {
        TOLOG3(_T("Failed to update meta, meta name= %1, meta key = %2, meta value = %3"), strMetaName, strKey, strValue);
        return false;
	}

	WIZMETADATA data;
	data.strName = strMetaName;
	data.strKey = strKey;
    data.strValue = strValue;
	data.tModified = WizGetCurrentTime();

    return true;
}

bool CWizIndex::GetAllDocumentsTitle(CWizStdStringArray& arrayDocument)
{
	CString strSQL(_T("select DOCUMENT_TITLE from WIZ_DOCUMENT"));

	try
	{
		CppSQLite3Query query = m_db.execQuery(strSQL);
		while (!query.eof())
		{
            CString strTitle = query.getStringField(	0);
			arrayDocument.push_back(strTitle);
			query.nextRow();
		}
        return true;
	}
	catch (const CppSQLite3Exception& e)
	{
		return LogSQLException(e, strSQL);
	}
}

bool CWizIndex::getDocumentsNoTag(CWizDocumentDataArray& arrayDocument, bool includeTrash /* = false */)
{
    CString strWhere;
    if (includeTrash) {
        strWhere = "DOCUMENT_GUID not in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_TAG)";
    } else {
        strWhere.Format("DOCUMENT_GUID not in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_TAG) and DOCUMENT_LOCATION not like %s",
                        STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16()
                        );
    }

    QString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere);

    return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::getLastestDocuments(CWizDocumentDataArray& arrayDocument, int nMax)
{
    CString strExt;
    strExt.Format("where DOCUMENT_LOCATION not like %s order by DT_MODIFIED desc limit 0, %s",
                  STR2SQL(m_strDeletedItemsLocation + "%").utf16(),
                  WizIntToStr(nMax).utf16());
    QString strSQL = FormatCanonicSQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strExt);
    return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetDocumentsByParamName(const CString& strLocation,
                                        bool bIncludeSubFolders,
                                        CString strParamName,
                                        CWizDocumentDataArray& arrayDocument)
{
	strParamName.MakeUpper();

	CString strWhere;
	strWhere.Format(_T("DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_PARAM where PARAM_NAME=%s)"),
        STR2SQL(strParamName).utf16()
		);

    if (strLocation && *strLocation) {
        CString strLocation2(strLocation);
        if (bIncludeSubFolders) {
            strLocation2.AppendChar('%');
		}

        strWhere += WizFormatString1(_T(" and DOCUMENT_LOCATION like %1"), STR2SQL(strLocation2));
	}

	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 

	return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetDocumentsByParam(const CString& strLocation,
                                    bool bIncludeSubFolders,
                                    CString strParamName,
                                    const CString& strParamValue,
                                    CWizDocumentDataArray& arrayDocument,
                                    bool bIncludeDeletedItems /*= true*/)
{
	strParamName.MakeUpper();

	CString strWhere;
	strWhere.Format(_T("DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_PARAM where PARAM_NAME=%s and PARAM_VALUE=%s)"),
        STR2SQL(strParamName).utf16(),
        STR2SQL(strParamValue).utf16()
		);

    if (!strLocation.IsEmpty()) {
        CString strLocation2(strLocation);
        if (bIncludeSubFolders) {
            strLocation2.AppendChar('%');
		}

        strWhere += WizFormatString1(_T(" and DOCUMENT_LOCATION like %1"), STR2SQL(strLocation2));
	}

    if (!bIncludeDeletedItems) {
		strWhere += WizFormatString1(_T(" and DOCUMENT_LOCATION not like %1"),
            STR2SQL(m_strDeletedItemsLocation + _T("%"))
			);
	}

	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 

	return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetDocumentsByTag(const CString& strLocation,
                                  const WIZTAGDATA& data,
                                  CWizDocumentDataArray& arrayDocument,
                                  bool includeTrash)
{
	CString strWhere;
    if (!strLocation.IsEmpty()) {
        strWhere.Format(_T("DOCUMENT_LOCATION like %s and DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID=%s)"),
                        STR2SQL(CString(strLocation) + '%').utf16(),
                        STR2SQL(data.strGUID).utf16()
                        );
    } else {
        if (includeTrash) {
            strWhere.Format(_T("DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID=%s)"),
                            STR2SQL(data.strGUID).utf16()
                            );
        } else {
            strWhere.Format(_T("DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID=%s) and DOCUMENT_LOCATION not like %s"),
                            STR2SQL(data.strGUID).utf16(),
                            STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16()
                            );
        }
	}

	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 

	return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetDocumentsSizeByTag(const WIZTAGDATA& data, int& size)
{
    CString strWhere;
    strWhere.Format(_T("DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID=%s) and DOCUMENT_LOCATION not like %s"),
                    STR2SQL(data.strGUID).utf16(),
                    STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16()
                    );

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, "COUNT(*)", strWhere);

    return SQLToSize(strSQL, size);
}

bool CWizIndex::GetAllDocumentsSizeByTag(const WIZTAGDATA& data, int& size)
{
    GetDocumentsSizeByTag(data, size);

    CWizTagDataArray arrayTag;
    GetAllChildTags(data.strGUID, arrayTag);

    int nSizeCurrent;
    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        const WIZTAGDATA& tag = *it;
        GetDocumentsSizeByTag(tag, nSizeCurrent);
        size += nSizeCurrent;
    }

    return true;
}

bool CWizIndex::GetDocumentsByStyle(const CString& strLocation, const WIZSTYLEDATA& data, CWizDocumentDataArray& arrayDocument)
{
	CString strWhere;
    if (strLocation && *strLocation) {
		strWhere.Format(_T("DOCUMENT_LOCATION like %s and STYLE_GUID=%s"),
            STR2SQL(CString(strLocation) + '%').utf16(),
            STR2SQL(data.strGUID).utf16()
			);
    } else {
		strWhere.Format(_T("STYLE_GUID=%s"),
            STR2SQL(data.strGUID).utf16()
			);
	}

	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 

	return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetDocumentsByTags(bool bAnd, const CString& strLocation,
                                const CWizTagDataArray& arrayTag,
                                CWizDocumentDataArray& arrayDocument)
{
	if (arrayTag.empty())
        return true;

	if (arrayTag.size() == 1)
        return GetDocumentsByTag(strLocation, arrayTag[0], arrayDocument, true);

    CString strWhere;
    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
		CString strLine;

		strLine.Format(_T("DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID=%s)"),
            STR2SQL(it->strGUID).utf16()
			);

        if (strWhere.IsEmpty()) {
			strWhere = strLine;
        } else {
            if (bAnd) {
				strWhere = strWhere + _T(" and ") + strLine;
            } else {
				strWhere = strWhere + _T(" or ") + strLine;
			}
		}
	}

    if (strLocation && *strLocation) {
        CString strLocation2(strLocation);
        strLocation2.AppendChar('%');

        strWhere = WizFormatString2(_T(" (%1) and DOCUMENT_LOCATION like %2"), strWhere, STR2SQL(strLocation2));
	}

	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 

	return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetDocumentsCountByLocation(const CString& strLocation,
                                           int& count,
                                           bool bIncludeSubFolders /* = false */)
{
    CString strWhere;
    if (bIncludeSubFolders) {
        strWhere.Format(_T("DOCUMENT_LOCATION like %s"),
                        STR2SQL(strLocation + _T("%")).utf16());
    } else {
        strWhere.Format(_T("DOCUMENT_LOCATION like %s"),
                        STR2SQL(strLocation).utf16());
    }

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, "COUNT(*)", strWhere);
    return SQLToSize(strSQL, count);
}

bool CWizIndex::GetDocumentsByLocation(const CString& strLocation,
                                       CWizDocumentDataArray& arrayDocument,
                                       bool bIncludeSubFolders /* = false */)
{
    CString strWhere;
    if (bIncludeSubFolders) {
        strWhere.Format(_T("DOCUMENT_LOCATION like %s"),
                        STR2SQL(strLocation + _T("%")).utf16());
    } else {
        strWhere.Format(_T("DOCUMENT_LOCATION like %s"),
                        STR2SQL(strLocation).utf16());
    }

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere);
	return SQLToDocumentDataArray(strSQL, arrayDocument);
}

//bool CWizIndex::GetDocumentsByLocationIncludeSubFolders(const CString& strLocation, CWizDocumentDataArray& arrayDocument)
//{
//	CString strWhere;
//	strWhere.Format(_T("DOCUMENT_LOCATION like %s"),
//        STR2SQL(strLocation + _T("%")).utf16()
//		);

//	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere);

//	return SQLToDocumentDataArray(strSQL, arrayDocument);
//}

bool CWizIndex::GetDocumentsByGUIDs(const CWizStdStringArray& arrayGUID, CWizDocumentDataArray& arrayDocument)
{
    CWizStdStringArray arrayGUID2;
    CWizStdStringArray::const_iterator it;
    for (it = arrayGUID.begin(); it != arrayGUID.end(); it++) {
		arrayGUID2.push_back(STR2SQL(*it));
	}

	CString strText;
	WizStringArrayToText(arrayGUID2, strText, _T(","));

	CString strWhere = WizFormatString1(_T("DOCUMENT_GUID in (%1)"), strText);

	return GetDocumentsBySQLWhere(strWhere, arrayDocument);
}

bool CWizIndex::UpdateDocumentParamMD5(const CString& strDocumentGUID)
{
	WIZDOCUMENTDATA data;
    if (!DocumentFromGUID(strDocumentGUID, data))
        return false;

	return UpdateDocumentParamMD5(data);
}

bool CWizIndex::UpdateDocumentParamMD5(WIZDOCUMENTDATA& data)
{
	CString strMD5 = CalDocumentParamInfoMD5(data);
	if (strMD5 == data.strParamMD5)
        return true;

	data.tModified = WizGetCurrentTime();
	data.tParamModified = WizGetCurrentTime();
	data.strParamMD5 = strMD5;

	CString strSQL;
	strSQL.Format(_T("update WIZ_DOCUMENT set DT_PARAM_MODIFIED=%s, DOCUMENT_PARAM_MD5=%s, WIZ_VERSION=-1 where DOCUMENT_GUID=%s"),
        TIME2SQL(data.tParamModified).utf16(),
        STR2SQL(data.strParamMD5).utf16(),
        STR2SQL(data.strGUID).utf16()
		);

    if (!ExecSQL(strSQL)) {
        TOLOG1(_T("Failed to update document params: %1"), data.strTitle);
        return false;
	}

    return true;
}

bool CWizIndex::DocumentFromLocationAndName(const CString& strLocation, const CString& strName, WIZDOCUMENTDATA& data)
{
    if (!strLocation || !*strLocation) {
		TOLOG(_T("Location is empty"));
        return false;
    }

    if (!strName || !*strName) {
		TOLOG(_T("Name is empty"));
        return false;
	}

	CString strWhere;
	strWhere.Format(_T("DOCUMENT_LOCATION like %s AND DOCUMENT_NAME like %s"),
        STR2SQL(strLocation).utf16(),
        STR2SQL(strName).utf16()
		);

	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 

	CWizDocumentDataArray arrayDocument;
    if (!SQLToDocumentDataArray(strSQL, arrayDocument)) {
		TOLOG(_T("Failed to get document by guid"));
        return false;
	}

    if (arrayDocument.empty()) {
		//TOLOG(_T("Failed to get document by guid, result is empty"));
        return false;
    } else if (arrayDocument.size() > 1) {
        CWizDocumentDataArray::const_iterator it;
        for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
            TOLOG(WizFormatString5(_T("Warning: Too more file name %1 in %2, Title=%3, FileName=%4, GUID=%5"), strName, strLocation, it->strTitle, it->strName, it->strGUID));
		}
	}

	data = arrayDocument[0];
    return true;
}

bool CWizIndex::ChangeDocumentsLocation(const CString& strOldLocation, const CString& strNewLocation)
{
	CString strSQL;
	strSQL.Format(_T("update %s set DOCUMENT_LOCATION=%s where DOCUMENT_LOCATION=%s"),
        STR2SQL(TABLE_NAME_WIZ_DOCUMENT).utf16(),
        STR2SQL(strNewLocation).utf16(),
        STR2SQL(strOldLocation).utf16()
		);

	return ExecSQL(strSQL);
}

#ifndef WIZ_NO_OBSOLETE
bool CWizIndex::FilterDocumentsInDeletedItems(CWizDocumentDataArray& arrayDocument)
{
	if (m_strDeletedItemsLocation.IsEmpty())
        return false;

	size_t nCount = arrayDocument.size();
	for (intptr_t i = nCount - 1; i >= 0; i--)
	{
		if (0 == _tcsnicmp(arrayDocument[i].strLocation, m_strDeletedItemsLocation, m_strDeletedItemsLocation.GetLength()))
		{
			arrayDocument.erase(arrayDocument.begin() + i);
		}
	}

    return true;
}
#endif

bool CWizIndex::GetTagsByTime(const COleDateTime& t, CWizTagDataArray& arrayData)
{
	CString strSQL = FormatQuerySQLByTime(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, FIELD_MODIFIED_WIZ_TAG, t); 
	return SQLToTagDataArray(strSQL, arrayData);
}

bool CWizIndex::GetStylesByTime(const COleDateTime& t, CWizStyleDataArray& arrayData)
{
	CString strSQL = FormatQuerySQLByTime(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE, FIELD_MODIFIED_WIZ_STYLE, t); 
	return SQLToStyleDataArray(strSQL, arrayData);
}

bool CWizIndex::GetDocumentsByTime(const QDateTime& t, CWizDocumentDataArray& arrayData)
{
	CString strSQL = FormatQuerySQLByTime3(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, 
		FIELD_INFO_MODIFIED_WIZ_DOCUMENT, 
		FIELD_DATA_MODIFIED_WIZ_DOCUMENT, 
		FIELD_PARAM_MODIFIED_WIZ_DOCUMENT, 
		t); 
	return SQLToDocumentDataArray(strSQL, arrayData);
}

bool CWizIndex::GetAttachmentsByTime(const COleDateTime& t, CWizDocumentAttachmentDataArray& arrayData)
{
	CString strSQL = FormatQuerySQLByTime2(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT, 
		FIELD_INFO_MODIFIED_WIZ_DOCUMENT_ATTACHMENT, 
		FIELD_DATA_MODIFIED_WIZ_DOCUMENT_ATTACHMENT, 
		t); 
	return SQLToDocumentAttachmentDataArray(strSQL, arrayData);
}

bool CWizIndex::GetDeletedGUIDsByTime(const COleDateTime& t, CWizDeletedGUIDDataArray& arrayData)
{
	CString strSQL = FormatQuerySQLByTime(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID, FIELD_MODIFIED_DELETED_GUID, t); 
	return SQLToDeletedGUIDDataArray(strSQL, arrayData);
}

#ifndef WIZ_NO_OBSOLETE

bool CWizIndex::IsDocumentModified()
{
	CWizStdStringArray arrayLocation;
	GetLocationsNeedToBeSync(arrayLocation);
	if (arrayLocation.empty())
        return false;

	CString strWhere2 = GetLocationArraySQLWhere(arrayLocation);

	CString strSQLDocument = WizFormatString3(_T("select %1 from %2 where WIZ_VERSION = -1 and %3 limit 0, 1"), 
		FIELD_LIST_WIZ_DOCUMENT, 
		TABLE_NAME_WIZ_DOCUMENT,
		strWhere2);

	DEBUG_TOLOG(strSQLDocument);
	if (HasRecord(strSQLDocument))
        return true;

    return false;
}

bool CWizIndex::IsModified()
{
	if (IsDocumentModified())
        return true;
	//
	//CString strSQLDeletedGUID = FormatModifiedQuerySQL2(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID, 1); 
	//if (HasRecord(strSQLDeletedGUID))
    //	return true;
	//
	CString strSQLTag = FormatModifiedQuerySQL2(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, 1); 
	if (HasRecord(strSQLTag))
        return true;
	//
	CString strSQLStyle = FormatModifiedQuerySQL2(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE, 1); 
	if (HasRecord(strSQLStyle))
        return true;
	//
        //do not check attachment
	//CString strSQLAttachment = FormatModifiedQuerySQL2(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT, 1); 
	//if (HasRecord(strSQLAttachment))
    //	return true;
	//
    return false;
}

#endif

bool CWizIndex::GetModifiedTags(CWizTagDataArray& arrayData)
{
	CString strSQL = FormatModifiedQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG); 
	return SQLToTagDataArray(strSQL, arrayData);
}

bool CWizIndex::GetModifiedStyles(CWizStyleDataArray& arrayData)
{
	CString strSQL = FormatModifiedQuerySQL(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE); 
	return SQLToStyleDataArray(strSQL, arrayData);
}

bool CWizIndex::GetModifiedDocuments(CWizDocumentDataArray& arrayData)
{
	CString strSQL = FormatModifiedQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT);
	return SQLToDocumentDataArray(strSQL, arrayData);
}

CString CWizIndex::GetLocationArraySQLWhere(const CWizStdStringArray& arrayLocation)
{
    CString strWhere;
    CWizStdStringArray::const_iterator it;
    for (it = arrayLocation.begin(); it != arrayLocation.end(); it++) {
        if (strWhere.IsEmpty()) {
			strWhere = WizFormatString1(_T(" DOCUMENT_LOCATION like %1 "), STR2SQL(*it + _T("%")));
        } else {
			strWhere = strWhere + WizFormatString1(_T(" or DOCUMENT_LOCATION like %1 "), STR2SQL(*it + _T("%")));
		}
	}

	return CString(_T(" (")) + strWhere + _T(")");
}

bool CWizIndex::GetModifiedDocuments(const CWizStdStringArray& arrayLocation, CWizDocumentDataArray& arrayData)
{
	if (arrayLocation.empty())
        return true;

	CString strSQL = FormatModifiedQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT);

	CString strWhere2 = GetLocationArraySQLWhere(arrayLocation);

	strSQL = strSQL + _T(" and ") + strWhere2;

	return SQLToDocumentDataArray(strSQL, arrayData);
}

bool CWizIndex::GetModifiedAttachments(CWizDocumentAttachmentDataArray& arrayData)
{
	CString strSQL = FormatModifiedQuerySQL(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT);
	return SQLToDocumentAttachmentDataArray(strSQL, arrayData);
}

bool CWizIndex::GetModifiedAttachments(const CWizStdStringArray& arrayLocation, CWizDocumentAttachmentDataArray& arrayData)
{
	CString strSQL = FormatModifiedQuerySQL(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT);

	CString strWhere2 = GetLocationArraySQLWhere(arrayLocation);

	strSQL = strSQL + WizFormatString1(_T(" and DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT where %1)"), strWhere2);

	return SQLToDocumentAttachmentDataArray(strSQL, arrayData);
}

bool CWizIndex::ObjectExists(const QString& strGUID, const QString& strType, bool& bExists)
{
	CString strTableName;
	CString strKeyFieldName;

    if (0 == strType.compare("tag", Qt::CaseInsensitive))
	{
		strTableName = TABLE_NAME_WIZ_TAG;
		strKeyFieldName = TABLE_KEY_WIZ_TAG;
	}
    else if (0 == strType.compare("style", Qt::CaseInsensitive))
	{
		strTableName = TABLE_NAME_WIZ_STYLE;
		strKeyFieldName = TABLE_KEY_WIZ_STYLE;
	}
    else if (0 == strType.compare("document", Qt::CaseInsensitive))
	{
		strTableName = TABLE_NAME_WIZ_DOCUMENT;
		strKeyFieldName = TABLE_KEY_WIZ_DOCUMENT;
	}
    else if (0 == strType.compare("attachment", Qt::CaseInsensitive))
	{
		strTableName = TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT;
		strKeyFieldName = TABLE_KEY_WIZ_DOCUMENT_ATTACHMENT;;
	}
	else
	{
        Q_ASSERT(0);
        TOLOG1("Unknown object type: %1", strType);
        return false;
	}

    CString strSQL = WizFormatString3("select %1 from %2 where %1=%3",
        strKeyFieldName, strTableName, STR2SQL(strGUID));

	CWizStdStringArray arrayGUID;
    if (!SQLToStringArray(strSQL, 0, arrayGUID)) {
        TOLOG("Failed to check objects!");
        return false;
	}

    if (arrayGUID.empty()) {
        bExists = false;
        return true;
    } else if (arrayGUID.size() == 1) {
        bExists = true;
        return true;
    } else {
        Q_ASSERT(0);
        return false;
	}
}

bool CWizIndex::GetObjectTableInfo(const CString& strType, CString& strTableName, CString& strKeyFieldName)
{
    if (0 == strType.CompareNoCase("tag")) {
		strTableName = TABLE_NAME_WIZ_TAG;
		strKeyFieldName = TABLE_KEY_WIZ_TAG;

    } else if (0 == strType.CompareNoCase("style")) {
		strTableName = TABLE_NAME_WIZ_STYLE;
		strKeyFieldName = TABLE_KEY_WIZ_STYLE;

    } else if (0 == strType.CompareNoCase("document")) {
		strTableName = TABLE_NAME_WIZ_DOCUMENT;
		strKeyFieldName = TABLE_KEY_WIZ_DOCUMENT;

    } else if (0 == strType.CompareNoCase("attachment")) {
		strTableName = TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT;
        strKeyFieldName = TABLE_KEY_WIZ_DOCUMENT_ATTACHMENT;

    } else if (0 == strType.CompareNoCase("message")) {
        strTableName = TABLE_NAME_WIZ_MESSAGE;
        strKeyFieldName = TABLE_KEY_WIZ_MESSAGE;

    } else {
        Q_ASSERT(0);
        TOLOG1("Unknown object type: %1", strType);
        return false;
	}

    return true;
}

qint64 CWizIndex::GetObjectLocalVersion(const QString& strGUID, const QString& strType)
{
    bool objectExists = false;
    return GetObjectLocalVersionEx(strGUID, strType, objectExists);
}

qint64 CWizIndex::GetObjectLocalVersionEx(const QString& strGUID, const QString& strType, bool& bObjectExists)
{
    CString strTableName;
    CString strKeyFieldName;

    if (!GetObjectTableInfo(strType, strTableName, strKeyFieldName))
        return false;

    CString strSQL;
    strSQL = WizFormatString3("select WIZ_VERSION from %1 where %2=%3",
                            strTableName,
                            strKeyFieldName,
                            STR2SQL(strGUID));

    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);

        if (!query.eof())
        {
            bObjectExists = true;
            return query.getInt64Field(0);
        }
        else
        {
            return -1;
        }
    }
    catch (const CppSQLite3Exception& e)
    {
        return -1;
        //return LogSQLException(e, strSQL);
    }
    return -1;
}

bool CWizIndex::ModifyObjectVersion(const CString& strGUID, const CString& strType, qint64 nVersion)
{
    if (-1 == nVersion)
    {
        qDebug() << "modify object version (-1), type: " << strType << " guid: " << strGUID;
    }

	CString strTableName;
	CString strKeyFieldName;

    if (!GetObjectTableInfo(strType, strTableName, strKeyFieldName))
        return false;
    //
    CString strSQL = WizFormatString4("update %1 set WIZ_VERSION=%2 where %3=%4",
		strTableName,
		WizInt64ToStr(nVersion),
		strKeyFieldName, 
        STR2SQL(strGUID));

    return ExecSQL(strSQL);
}

bool CWizIndex::IsObjectDataModified(const CString& strGUID, const CString& strType)
{
    qint64 nVersion = GetObjectLocalVersion(strGUID, strType);
    return  -1 == nVersion;
}

bool CWizIndex::ModifyObjectModifiedTime(const CString& strGUID, const CString& strType, const COleDateTime& t)
{
	CString strTableName;
	CString strKeyFieldName;

    if (!GetObjectTableInfo(strType, strTableName, strKeyFieldName))
        return false;

	CString strTimeFieldName;
	if (0 == strType.CompareNoCase(_T("document"))
		|| 0 == strType.CompareNoCase(_T("attachment")))
	{
		strTimeFieldName = _T("DT_INFO_MODIFIED");
	}
	else
	{
		strTimeFieldName = _T("DT_MODIFIED");
	}

	CString strSQL = WizFormatString5(_T("update %1 set %2=%3 where %4=%5"),
		strTableName,
		strTimeFieldName,
		TIME2SQL(t),
		strKeyFieldName, 
        STR2SQL(strGUID));

	return ExecSQL(strSQL);
}

bool CWizIndex::GetObjectModifiedTime(const CString& strGUID, const CString& strType, COleDateTime& t)
{
	CString strTableName;
	CString strKeyFieldName;

    if (!GetObjectTableInfo(strType, strTableName, strKeyFieldName))
        return false;

	CString strTimeFieldName;
	if (0 == strType.CompareNoCase(_T("document"))
		|| 0 == strType.CompareNoCase(_T("attachment")))
	{
		strTimeFieldName = _T("DT_INFO_MODIFIED");
	}
	else
	{
		strTimeFieldName = _T("DT_MODIFIED");
	}

	CString strSQL = WizFormatString4(_T("select %1 from %2 where %3=%4"),
		strTimeFieldName,
		strTableName,
		strKeyFieldName, 
        STR2SQL(strGUID));

	CString strTime;
	if (!GetFirstRowFieldValue(strSQL, 0, strTime))
        return false;

	t = WizStringToDateTime(strTime);

    return true;
}

bool CWizIndex::SearchDocumentByTitle(const QString& strTitle,
                                      const QString& strLocation,
                                      bool bIncludeSubFolders,
                                      int nMaxCount,
                                      CWizDocumentDataArray& arrayDocument)
{
    QStringList listTitle = strTitle.split(getWizSearchSplitChar());
    if (listTitle.isEmpty())
        return false;

    CString strWhere = WizFormatString1(" DOCUMENT_TITLE like '%%1%'", listTitle.first());
    for (int i = 1; i < listTitle.count(); i++) {
        strWhere += WizFormatString1(" AND DOCUMENT_TITLE like '%%1%'", listTitle.at(i));
    }

    if (!strLocation.isEmpty()) {
        CString strWhereLocation;
        if (bIncludeSubFolders) {
            strWhereLocation = " AND DOCUMENT_LOCATION like " + STR2SQL(WizFormatString1("%%1%", strLocation));
        } else {
            strWhereLocation = " AND DOCUMENT_LOCATION like " + STR2SQL(WizFormatString1("%%1", strLocation));
        }

        strWhere += strWhereLocation;
    }

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere);

    if (!SQLToDocumentDataArray(strSQL, arrayDocument))
        return false;

    if (arrayDocument.size() > nMaxCount) {
        arrayDocument.resize(nMaxCount);
    }

    return true;
}

CString URLToSQL(const CString& strURL)
{
	if (strURL.IsEmpty())
		return strURL;

	CString strWhere = WizFormatString1(_T("DOCUMENT_URL like %1"), STR2SQL_LIKE_BOTH(strURL));
	return strWhere;
}

CString AttachmentNameToSQL(const CString& strName)
{
	if (strName.IsEmpty())
		return strName;

	CString strWhere = WizFormatString1(_T("DOCUMENT_GUID in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_ATTACHMENT where ATTACHMENT_NAME like %1)"), STR2SQL_LIKE_BOTH(strName));
	return strWhere;
}

CString HasAttachmentToSQL(int nAttachment)
{
	if (-1 == nAttachment)
		return CString();
	//
	if (nAttachment)
	{
		CString strWhere(_T("DOCUMENT_GUID in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_ATTACHMENT)"));
		return strWhere;
	}
	else
	{
		CString strWhere(_T("DOCUMENT_GUID not in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_ATTACHMENT)"));
		return strWhere;
	}
}

CString TagArrayToSQL(CWizIndex& index, const CWizStdStringArray& arrayTagName)
{
	CWizStdStringArray arrayWhere;
	//
	CWizTagDataArray arrayAllTag;
	std::set<CString> setAllTag;
	//
	for (CWizStdStringArray::const_iterator it = arrayTagName.begin();
		it != arrayTagName.end();
		it++)
	{
		CString strTagName = *it;
		if (strTagName.IsEmpty())
			continue;
		//
		CWizTagDataArray arrayTag;
		index.TagArrayByName(strTagName, arrayTag);
		if (arrayTag.empty())
			continue;
		//
		for (CWizTagDataArray::const_iterator itTag = arrayTag.begin();
			itTag != arrayTag.end();
			itTag++)
		{
			if (setAllTag.find(itTag->strGUID) == setAllTag.end())
			{
				setAllTag.insert(itTag->strGUID);
				arrayAllTag.push_back(*itTag);
			}
		}
	}
	//
	for (CWizTagDataArray::const_iterator it = arrayAllTag.begin();
		it != arrayAllTag.end();
		it++)
	{
		CString strWhere = WizFormatString1(_T("DOCUMENT_GUID in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID ='%1')"), it->strGUID);
		arrayWhere.push_back(strWhere);
	}
	//
	if (arrayWhere.empty())
		return CString();
	//
	if (arrayWhere.size() == 1)
	{
		return arrayWhere[0];
	}
	else
	{
		CString strWhere;
		::WizStringArrayToText(arrayWhere, strWhere, _T(" or "));
		strWhere = WizFormatString1(_T("( %1 )"), strWhere);
		return strWhere;
	}
}

CString FileTypeArrayToSQL(const CWizStdStringArray& arrayFileType)
{
	CWizStdStringArray arrayWhere;
	//
	for (CWizStdStringArray::const_iterator it = arrayFileType.begin();
		it != arrayFileType.end();
		it++)
	{
		CString strWhere = WizFormatString1(_T("DOCUMENT_FILE_TYPE like %1"), STR2SQL_LIKE_BOTH(*it));
		arrayWhere.push_back(strWhere);
	}
	//
	if (arrayWhere.empty())
		return CString();
	//
	if (arrayWhere.size() == 1)
	{
		return arrayWhere[0];
	}
	else
	{
		CString strWhere;
		::WizStringArrayToText(arrayWhere, strWhere, _T(" or "));
		strWhere = WizFormatString1(_T("( %1 )"), strWhere);
		return strWhere;
	}
}

CString LocationToSQL(const CString& strLocation, bool bIncludeSubFolders)
{
	if (strLocation.IsEmpty())
		return strLocation;
	//
	CString strWhereLocation;
	if (bIncludeSubFolders)
	{
		strWhereLocation = _T(" DOCUMENT_LOCATION like ") + STR2SQL_LIKE_RIGHT(strLocation);
	}
	else
	{
		strWhereLocation = _T(" DOCUMENT_LOCATION like ") + STR2SQL(strLocation);
	}
	//
	return strWhereLocation;
}

CString WizFormatDateInt(int n)
{
	if (n < 10)
		return CString(_T("0")) + WizIntToStr(n);
	return WizIntToStr(n);
}

bool WizDateRegularDateString(CString& str)
{
	CWizStdStringArray arrayText;
	::WizSplitTextToArray(str, '-', arrayText);
	if (arrayText.size() != 3)
        return false;
	//
	arrayText[1] = ::WizFormatDateInt(_ttoi(arrayText[1]));
	arrayText[2] = ::WizFormatDateInt(_ttoi(arrayText[2]));
	//
	str = arrayText[0] + _T("-") + arrayText[1] + _T("-") + arrayText[2];
    return true;
}

bool WizDateStringToDateTimeOfBegin(CString str, COleDateTime& t)
{
	WizDateRegularDateString(str);
	str += _T(" 00:00:00");
	//
	CString strError;
	if (WizStringToDateTime(str, t, strError))
        return true;
	//
    //WizMessageBox(strError);
	//
    return false;
}

bool WizDateStringToDateTimeOfEnd(CString str, COleDateTime& t)
{
	WizDateRegularDateString(str);
	str += _T(" 23:59:59");
	//
	CString strError;
	if (WizStringToDateTime(str, t, strError))
        return true;
	//
    //WizMessageBox(strError);
	//
    return false;
}

CString DateToSQL(const CString& strFieldName, CString strValue)
{
	if (strValue.IsEmpty())
		return CString();
	//
    if (!strFieldName || !*strFieldName)
		return CString();
	//
    QChar charOpera = strValue[0];
	if (charOpera == '>')
	{
		strValue.Delete(0, 1);
		//
		COleDateTime t;
		if (!WizDateStringToDateTimeOfBegin(strValue, t))
			return CString();
		//
        return WizFormatString2(_T("%1 >= '%2'"), strFieldName, ::WizDateTimeToString(t));
	}
	else if (charOpera == '<')
	{
		strValue.Delete(0, 1);
		//
		COleDateTime t;
		if (!WizDateStringToDateTimeOfEnd(strValue, t))
			return CString();
		//
        return WizFormatString2(_T("%1 <= '%2'"), strFieldName, ::WizDateTimeToString(t));
	}
	else if (charOpera == '=')
	{
		strValue.Delete(0, 1);
	}
	else
	{
		CString strLeft;
		CString strRight;
		if (::WizStringSimpleSplit(strValue, '&', strLeft, strRight))
		{
			COleDateTime t1;
			if (!WizDateStringToDateTimeOfBegin(strLeft, t1))
				return CString();
			//
			COleDateTime t2;
			if (!WizDateStringToDateTimeOfEnd(strRight, t2))
				return CString();
			//
            return WizFormatString4(_T("(%1 >= '%2' and %3 <= '%4')"), strFieldName, ::WizDateTimeToString(t1), strFieldName, ::WizDateTimeToString(t2));
		}
	}
	//
	COleDateTime t1;
	if (!WizDateStringToDateTimeOfBegin(strValue, t1))
		return CString();
	//
	COleDateTime t2;
	if (!WizDateStringToDateTimeOfEnd(strValue, t2))
		return CString();
	//
    return WizFormatString4(_T("(%1 >= '%2' and %3 <= '%4')"), strFieldName, ::WizDateTimeToString(t1), strFieldName, ::WizDateTimeToString(t2));
}

CString TitleToSQL(CWizIndex& index, const CString& strKeywords, const WIZSEARCHDATA& data)
{
    bool bAddExtra = false;
	CString strTitle(data.strTitle);
	if (strTitle.IsEmpty())
	{
        strTitle = strKeywords;
        bAddExtra = true;
	}
	//
	if (strTitle.IsEmpty())
		return CString();
	//
	//
	CWizStdStringArray arrayWhere;
	
	if (bAddExtra)
	{
		arrayWhere.push_back(WizFormatString1(_T("(DOCUMENT_TITLE like %1)"), STR2SQL_LIKE_BOTH(strTitle)));
		arrayWhere.push_back(WizFormatString1(_T("(DOCUMENT_NAME like %1)"), STR2SQL_LIKE_BOTH(strTitle)));
		arrayWhere.push_back(WizFormatString1(_T("(DOCUMENT_SEO like %1)"), STR2SQL_LIKE_BOTH(strTitle)));
		arrayWhere.push_back(WizFormatString1(_T("(DOCUMENT_URL like %1)"), STR2SQL_LIKE_BOTH(strTitle)));
		arrayWhere.push_back(WizFormatString1(_T("(DOCUMENT_AUTHOR like %1)"), STR2SQL_LIKE_BOTH(strTitle)));
		arrayWhere.push_back(WizFormatString1(_T("(DOCUMENT_KEYWORDS like %1)"), STR2SQL_LIKE_BOTH(strTitle)));
		if (data.strAttachmentName.IsEmpty())
		{
			arrayWhere.push_back(WizFormatString1(_T("(%1)"), AttachmentNameToSQL(strTitle)));
		}
		if (data.arrayTag.empty())
		{
			CString strText = strTitle;
            //WizKMRegularTagsText(strText);
			//
			CWizStdStringArray arrayTag;
			::WizSplitTextToArray(strText, ';', arrayTag);
			//
			CString strTagSQL = TagArrayToSQL(index, arrayTag);
			if (!strTagSQL.IsEmpty())
			{
				arrayWhere.push_back(WizFormatString1(_T("(%1)"), strTagSQL));
			}
		}
		if (data.arrayFileType.empty())
		{
			CString strText = strTitle;
            //WizKMRegularTagsText(strText);
			//
			CWizStdStringArray arrayFileType;
			::WizSplitTextToArray(strText, ';', arrayFileType);
			//
			CString strFileTypeSQL = FileTypeArrayToSQL(arrayFileType);
			if (!strFileTypeSQL.IsEmpty())
			{
				arrayWhere.push_back(WizFormatString1(_T("(%1)"), strFileTypeSQL));
			}
		}
		CString strWhere;
		::WizStringArrayToText(arrayWhere, strWhere, _T(" or "));
		//
		return strWhere;
	}
	else
	{
		CWizStdStringArray arrayText;
		::WizSplitTextToArray(strTitle, ' ', arrayText);
		for (CWizStdStringArray::const_iterator it = arrayText.begin();
			it != arrayText.end();
			it++)
		{
			CString str = *it;
			str.Trim();
			if (str.IsEmpty())
				continue;
			arrayWhere.push_back(WizFormatString1(_T("(DOCUMENT_TITLE like %1)"), STR2SQL_LIKE_BOTH(str)));
		}
		//
		CString strWhere;
		::WizStringArrayToText(arrayWhere, strWhere, _T(" and "));
		//
		return strWhere;

	}
}

bool CWizIndex::Search(const CString& strKeywords,
                       const WIZSEARCHDATA& data,
                       const CString& strLocation,
                       bool bIncludeSubFolders,
                       size_t nMaxCount,
                       CWizDocumentDataArray& arrayDocument)
{
	if (!data.strSyntax.IsEmpty())
        return true;
	//
	CWizStdStringArray arrayWhere;
	//
	if (!data.strSQL.IsEmpty())
	{
		arrayWhere.push_back(data.strSQL);
	}
	else
	{
        arrayWhere.push_back(TitleToSQL(*this, strKeywords, data));
		arrayWhere.push_back(URLToSQL(data.strURL));
		arrayWhere.push_back(AttachmentNameToSQL(data.strAttachmentName));
		arrayWhere.push_back(HasAttachmentToSQL(data.nHasAttachment));
		arrayWhere.push_back(TagArrayToSQL(*this, data.arrayTag));
		arrayWhere.push_back(FileTypeArrayToSQL(data.arrayFileType));
		arrayWhere.push_back(DateToSQL(_T("DT_CREATED"), data.strDateCreated));
		arrayWhere.push_back(DateToSQL(_T("DT_DATA_MODIFIED"), data.strDateModified));
		arrayWhere.push_back(DateToSQL(_T("DT_ACCESSED"), data.strDateAccessed));
	}
	//
    arrayWhere.push_back(LocationToSQL(CString(strLocation), bIncludeSubFolders));
	//
	::WizStringArrayEraseEmptyLine(arrayWhere);
	if (arrayWhere.empty())
        return true;
	//
	CString strWhere;
	::WizStringArrayToText(arrayWhere, strWhere, _T(") and ("));
	strWhere = WizFormatString1(_T("( %1 )"), strWhere);
	//
	strWhere += WizFormatString1(_T(" order by DT_CREATED desc limit 0, %1"), WizIntToStr(int(nMaxCount)));
	//
	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 
	//
	DEBUG_TOLOG(strSQL);
	//
	return SQLToDocumentDataArray(strSQL, arrayDocument);

}

bool CWizIndex::Search(const CString& strKeywords,
                       const CString& strLocation,
                       bool bIncludeSubFolders,
                       CWizDocumentDataArray& arrayDocument)
{
	CWizStdStringArray arrayKeywords;
    WizSplitTextToArray(strKeywords, ' ', arrayKeywords);
	//
	WizStringArrayEraseEmptyLine(arrayKeywords);
	//
	CWizStdStringArray arrayField;
	arrayField.push_back(_T("DOCUMENT_TITLE"));
	arrayField.push_back(_T("DOCUMENT_NAME"));
	arrayField.push_back(_T("DOCUMENT_SEO"));
	arrayField.push_back(_T("DOCUMENT_URL"));
	arrayField.push_back(_T("DOCUMENT_AUTHOR"));
	arrayField.push_back(_T("DOCUMENT_KEYWORDS"));
	arrayField.push_back(_T("DOCUMENT_OWNER"));
	arrayField.push_back(_T("DOCUMENT_AUTHOR"));
	//
	CString strFormat;
	//
	for (CWizStdStringArray::const_iterator it =  arrayField.begin();
		it != arrayField.end();
		it++)
	{
		if (strFormat.IsEmpty())
		{
			strFormat = CString("(") + *it + _T(" like '%%1%'") + _T(")");
		}
		else
		{
			strFormat = strFormat + _T(" OR ") + CString("(") + *it + _T(" like '%%1%'") + _T(")");
		}
	}
	//
	strFormat = _T("(") + strFormat + _T(")");
	//
	CString strWhere;
	//
	for (CWizStdStringArray::const_iterator it = arrayKeywords.begin();
		it != arrayKeywords.end();
		it++)
	{
		CString strLine = WizFormatString1(strFormat, *it);
		if (strWhere.IsEmpty())
		{
			strWhere = strLine;
		}
		else
		{
			strWhere = strWhere + _T(" AND ") + strLine;
		}
	}
	//
	if (!strLocation.IsEmpty())
	{
		CString strWhereLocation;
		if (bIncludeSubFolders)
		{
			strWhereLocation = _T(" AND DOCUMENT_LOCATION like ") + STR2SQL(WizFormatString1(_T("%%1%"), strLocation));
		}
		else
		{
			strWhereLocation = _T(" AND DOCUMENT_LOCATION like ") + STR2SQL(WizFormatString1(_T("%%1"), strLocation));
		}
		//
		strWhere += strWhereLocation;
	}
	//
	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 
	//
	return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetRecentDocuments(long nFlags, const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument)
{
	CString strSQL;
	//
	CString strTimeField = (0 == nFlags) ? _T("DT_CREATED, DT_DATA_MODIFIED") : _T("DT_CREATED, DT_MODIFIED, DT_INFO_MODIFIED, DT_DATA_MODIFIED");
	//
	if (strDocumentType.IsEmpty())
	{
		strSQL.Format(_T("select %s, MAX(%s) AS WIZ_TEMP_MODIFIED from %s where DOCUMENT_LOCATION not like %s order by WIZ_TEMP_MODIFIED desc limit 0, %d"),
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            strTimeField.utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16(),
			nCount
			);
	}
	else
	{
		strSQL.Format(_T("select %s, MAX(%s) AS WIZ_TEMP_MODIFIED from %s where DOCUMENT_TYPE=%s and DOCUMENT_LOCATION not like %s order by WIZ_TEMP_MODIFIED desc limit 0,%d"),
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            strTimeField.utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(strDocumentType).utf16(),
            STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16(),
			nCount
			);
	}
	//
	return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetRecentDocumentsCreated(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument)
{
	CString strSQL;
	//
	if (strDocumentType.IsEmpty())
	{
		strSQL.Format(_T("select %s from %s where DOCUMENT_LOCATION not like %s order by DT_CREATED desc limit 0, %d"),
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16(),
			nCount
			);
	}
	else
	{
		strSQL.Format(_T("select %s from %s where DOCUMENT_TYPE=%s and DOCUMENT_LOCATION not like %s order by DT_CREATED desc limit 0,%d"),
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(strDocumentType).utf16(),
            STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16(),
			nCount
			);
	}
	//
	return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetRecentDocumentsModified(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument)
{
    CString strSQL;
	//
	if (strDocumentType.IsEmpty())
	{
		strSQL.Format(_T("select %s from %s where DOCUMENT_LOCATION not like %s and datetime(DT_CREATED, '+1 minute') < DT_DATA_MODIFIED order by DT_DATA_MODIFIED desc limit 0, %d"),
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16(),
			nCount
			);
	}
	else
	{
		strSQL.Format(_T("select %s from %s where DOCUMENT_TYPE=%s and DOCUMENT_LOCATION not like %s and datetime(DT_CREATED, '+1 minute') < DT_DATA_MODIFIED order by DT_DATA_MODIFIED desc limit 0,%d"),
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(strDocumentType).utf16(),
            STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16(),
			nCount
			);
	}
	//
	return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetRecentDocumentsByCreatedTime(const COleDateTime& t, CWizDocumentDataArray& arrayDocument)
{
    CString strTime = TIME2SQL(t);

    CString strSQL;
    strSQL.Format(_T("select %s from %s where DOCUMENT_LOCATION not like '/Deleted Items/%%' and DT_CREATED>=%s order by DT_CREATED desc"),
        QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
        QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
        strTime.utf16()
        );

    return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetRecentDocumentsByModifiedTime(const COleDateTime& t, CWizDocumentDataArray& arrayDocument)
{
    CString strTime = TIME2SQL(t);

    CString strSQL;
    strSQL.Format(_T("select %s from %s where DOCUMENT_LOCATION not like '/Deleted Items/%%' and DT_MODIFIED>=%s order by DT_MODIFIED desc"),
        QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
        QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
        strTime.utf16()
        );

    return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndex::GetRecentDocumentsByAccessedTime(const COleDateTime& t, CWizDocumentDataArray& arrayDocument)
{
    CString strTime = TIME2SQL(t);

    CString strSQL;
    strSQL.Format(_T("select %s from %s where DOCUMENT_LOCATION not like '/Deleted Items/%%' and DT_ACCESSED>=%s order by DT_ACCESSED desc"),
        QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
        QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
        strTime.utf16()
        );

    return SQLToDocumentDataArray(strSQL, arrayDocument);
}

#ifndef WIZ_NO_OBSOLETE
bool CWizIndex::GetCalendarEvents(const COleDateTime& tStart, const COleDateTime& tEnd, CWizDocumentDataArray& arrayDocument)
{
	CString strParamNameStart(DOCUMENT_PARAM_NAME_CALENDAR_EVENT_START);
	CString strParamNameEnd(DOCUMENT_PARAM_NAME_CALENDAR_EVENT_END);
	//
	strParamNameStart.MakeUpper();
	strParamNameEnd.MakeUpper();
	//
	CString strWhere;

	strWhere.Format(_T("DOCUMENT_LOCATION not like %s and\
		((DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE>=%s) \
		and DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE<=%s)) \
		or \
		(DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE>=%s) \
		and DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE<=%s)) \
		or \
		(DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE<%s) \
		and DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE>%s))) \
		"),
        STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16(),
        TABLE_NAME_WIZ_DOCUMENT_PARAM.utf16(),
        strParamNameStart.utf16(),
        TIME2SQL(tStart).utf16(),
        TABLE_NAME_WIZ_DOCUMENT_PARAM.utf16(),
        strParamNameStart.utf16(),
        TIME2SQL(tEnd).utf16(),

        TABLE_NAME_WIZ_DOCUMENT_PARAM.utf16(),
        strParamNameEnd.utf16(),
        TIME2SQL(tStart).utf16(),
        TABLE_NAME_WIZ_DOCUMENT_PARAM.utf16(),
        strParamNameEnd.utf16(),
        TIME2SQL(tEnd).utf16(),

        TABLE_NAME_WIZ_DOCUMENT_PARAM.utf16(),
        strParamNameStart.utf16(),
        TIME2SQL(tStart).utf16(),
        TABLE_NAME_WIZ_DOCUMENT_PARAM.utf16(),
        strParamNameEnd.utf16(),
        TIME2SQL(tEnd).utf16()

		);
	//
	return GetDocumentsBySQLWhere(strWhere, arrayDocument);
}

bool CWizIndex::GetAllRecurrenceCalendarEvents(CWizDocumentDataArray& arrayDocument)
{
	CString strParamName(DOCUMENT_PARAM_NAME_CALENDAR_EVENT_RECURRENCE);
	strParamName.MakeUpper();

	CString strWhere;
	strWhere.Format(_T("DOCUMENT_LOCATION not like %s and DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_PARAM where PARAM_NAME=%s)"),
        STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16(),
        STR2SQL(strParamName).utf16()
		);
	//
	//
	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 
	//
	return SQLToDocumentDataArray(strSQL, arrayDocument);
}
#endif

bool CWizIndex::GetLocations(CWizStdStringArray& arrayLocation)
{
	CString strSQL(_T("select distinct DOCUMENT_LOCATION from WIZ_DOCUMENT"));
	return SQLToStringArray(strSQL, 0, arrayLocation);
}

CString CWizIndex::GetRootLocationName(CString strLocation)
{
    strLocation.Trim('/');
	int nIndex = strLocation.Find('/');
	if (-1 == nIndex)
		return strLocation;
	else
		return strLocation.Left(nIndex);
}

bool CWizIndex::IsRootLocation(CString strLocation)
{
	strLocation.Trim('/');
	return -1 == strLocation.Find('/');
}

bool CWizIndex::GetLocationsNeedToBeSync(CWizStdStringArray& arrayLocation)
{
	if (!GetLocations(arrayLocation))
        return false;
	//
	for (intptr_t i = arrayLocation.size() - 1; i >= 0; i--)
	{
		CString strLocation = arrayLocation[i];
		if (!IsRootLocation(strLocation)
			|| !GetSync(strLocation))
		{
			arrayLocation.erase(arrayLocation.begin() + i);
			continue;
		}
	}
	//
    return true;
}

bool CWizIndex::GetLocations(CDocumentLocationArray& arrayLocation)
{
	CString strSQL(_T("select DOCUMENT_LOCATION, count(*) as DOCUMENT_COUNT from WIZ_DOCUMENT group by DOCUMENT_LOCATION order by DOCUMENT_LOCATION"));
	try
	{
		CppSQLite3Query query = m_db.execQuery(strSQL);
		//
		while (!query.eof())
		{
			WIZDOCUMENTLOCATIONDATA data;
            data.strLocation = query.getStringField(0);
			data.nDocumentCount = query.getIntField(1);
			//
			arrayLocation.push_back(data);
			//
			query.nextRow();
		}
        return true;
	}
	catch (const CppSQLite3Exception& e)
	{
		return LogSQLException(e, strSQL);
	}
    return true;
}

qint64 CWizIndex::GetMetaInt64(const CString& strMetaName, const CString& strKey, qint64 nDef)
{
	CString str;
    GetMeta(strMetaName, strKey, str);
	if (str.IsEmpty())
		return nDef;

	return _ttoi64(str);
}

bool CWizIndex::SetMetaInt64(const CString& strMetaName, const CString& strKey, qint64 n)
{
    return SetMeta(strMetaName, strKey, WizInt64ToStr(n));
}

bool CWizIndex::deleteMetasByName(const QString& strMetaName)
{
    CString strFormat = FormatDeleteSQLFormat(TABLE_NAME_WIZ_META, "META_NAME");

    CString strSQL;
    strSQL.Format(strFormat,
        STR2SQL(strMetaName).utf16()
        );

    if (!ExecSQL(strSQL))
        return false;

    return true;
}

bool CWizIndex::deleteMetaByKey(const QString& strMetaName, const QString& strMetaKey)
{
    CString strWhere = QString("META_NAME='%1' AND META_KEY='%2'").arg(strMetaName).arg(strMetaKey);
    CString strSQL = FormatDeleteSQLByWhere(TABLE_NAME_WIZ_META, strWhere);

    if (!ExecSQL(strSQL))
        return false;

    return true;
}

int CWizIndex::GetDocumentAttachmentCount(const CString& strDocumentGUID)
{
    CString strSQL;
    strSQL.Format("select count(*) from WIZ_DOCUMENT_ATTACHMENT where DOCUMENT_GUID=%s",
                  STR2SQL(strDocumentGUID).utf16());

    CppSQLite3Query query = m_db.execQuery(strSQL);

    if (!query.eof()) {
        int nCount = query.getIntField(0);
        return nCount;
    }

    return 0;
}

void CWizIndex::UpdateDocumentAttachmentCount(const CString& strDocumentGUID,
                                              bool bResetDocInfo /* = true */)
{
    WIZDOCUMENTDATA data;
    if (!DocumentFromGUID(strDocumentGUID, data))
        return;

    data.nAttachmentCount = GetDocumentAttachmentCount(strDocumentGUID);

    ModifyDocumentInfo(data, bResetDocInfo);
}

bool CWizIndex::GetSync(const CString& strLocation)
{
    CString strRootLocationName = GetRootLocationName(strLocation);
	
    return GetMetaDef(_T("Sync"), strRootLocationName, _T("1")) == _T("1");
}

bool CWizIndex::SetSync(const CString& strLocation, bool bSync)
{
    CString strRootLocationName = GetRootLocationName(strLocation);

	return SetMeta(_T("Sync"), strRootLocationName, bSync ? _T("1") : _T("0"));
}

bool CWizIndex::GetDocumentsNoTagCount(int& nSize, bool includeTrash /* = false */)
{
    CString strWhere;
    if (includeTrash) {
        strWhere = "DOCUMENT_GUID not in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_TAG)";
    } else {
        strWhere.Format("DOCUMENT_GUID not in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_TAG) and DOCUMENT_LOCATION not like %s",
                        STR2SQL(m_strDeletedItemsLocation + _T("%")).utf16()
                        );
    }

    QString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, "COUNT(*)", strWhere);

    return SQLToSize(strSQL, nSize);
}


bool CWizIndex::GetAllTagsDocumentCount(std::map<CString, int>& mapTagDocumentCount)
{
	CString strSQL;
    strSQL.Format(_T("select TAG_GUID, count(*) from WIZ_DOCUMENT_TAG where DOCUMENT_GUID in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT where DOCUMENT_LOCATION not like '/Deleted Items/%%') group by TAG_GUID")
		);
	try
	{
		CppSQLite3Query query = m_db.execQuery(strSQL);

		while (!query.eof())
		{
            CString strTagGUID = query.getStringField(0);
			int nDocumentCount = query.getIntField(1);

			mapTagDocumentCount[strTagGUID] = nDocumentCount;

			query.nextRow();
		}
        return true;
	}
	catch (const CppSQLite3Exception& e)
	{
		return LogSQLException(e, strSQL);
	}
    return true;
}

bool CWizIndex::GetAllLocationsDocumentCount(std::map<CString, int>& mapLocationDocumentCount)
{
	CString strSQL;
    strSQL.Format("select DOCUMENT_LOCATION, count(*) as DOCUMENT_COUNT from WIZ_DOCUMENT group by DOCUMENT_LOCATION");
	try
	{
		CppSQLite3Query query = m_db.execQuery(strSQL);
		while (!query.eof())
		{
            CString strLocation = query.getStringField(0);
			int nDocumentCount = query.getIntField(1);
			mapLocationDocumentCount[strLocation] = nDocumentCount;
			query.nextRow();
		}
        return true;
	}
	catch (const CppSQLite3Exception& e)
	{
        TOLOG(strSQL);
		return LogSQLException(e, strSQL);
	}

    return true;
}

int CWizIndex::GetTrashDocumentCount()
{
    int nTotal = 0;

    CString strSQL;
    strSQL.Format("select count(*) as DOCUMENT_COUNT from WIZ_DOCUMENT where DOCUMENT_LOCATION like '/Deleted Items/%'");
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            nTotal += query.getIntField(0);
            query.nextRow();
        }
    }
    catch (const CppSQLite3Exception& e)
    {
        TOLOG(strSQL);
        return LogSQLException(e, strSQL);
    }

    return nTotal;
}

bool CWizIndex::GetAllDocumentsOwners(CWizStdStringArray& arrayOwners)
{
    CString strSQL;
    strSQL.Format(_T("select distinct DOCUMENT_OWNER from WIZ_DOCUMENT"));

    return SQLToStringArray(strSQL, 0, arrayOwners);
}


#ifndef WIZ_NO_OBSOLETE
bool CWizIndex::GetLocationDocumentCount(CString strLocation, bool bIncludeSubFolders, int& nDocumentCount)
{
	nDocumentCount = 0;
	//
	if (bIncludeSubFolders)
		strLocation.Append(_T("%"));
	//
	CString strSQL;
	//
	strSQL.Format(_T("SELECT count(*) as DOCUMENT_COUNT from WIZ_DOCUMENT where DOCUMENT_LOCATION like %s"),
        STR2SQL(strLocation).utf16()
        );
	//
	try
	{
		CppSQLite3Query query = m_db.execQuery(strSQL);
		//
		if (!query.eof())
		{
			nDocumentCount = query.getIntField(0);
			//
            return true;
		}
	}
	catch (const CppSQLite3Exception& e)
	{
		return LogSQLException(e, strSQL);
	}
	//
    return true;
}
#endif

bool CWizIndex::SetDocumentVersion(const CString& strDocumentGUID, qint64 nVersion)
{
    if (nVersion == -1)
    {
        qDebug() << "modify document version (-1), guid: " << strDocumentGUID;
    }
    //
	CString strSQL = WizFormatString2(_T("update WIZ_DOCUMENT set WIZ_VERSION=%1 where DOCUMENT_GUID=%2"),
		WizInt64ToStr(nVersion),
        STR2SQL(strDocumentGUID));

	if (!ExecSQL(strSQL))
        return false;

    return true;
}

bool CWizIndex::setThumbIndexVersion(const QString& strVersion)
{
    return SetMeta("ThumbIndex", "Version", strVersion);
}

QString CWizIndex::getThumIndexVersion()
{
    return GetMetaDef("ThumbIndex", "Version");
}

bool CWizIndex::setDocumentFTSVersion(const QString& strVersion)
{
    return SetMeta("FTS", "Version", strVersion);
}

QString CWizIndex::getDocumentFTSVersion()
{
    return GetMetaDef("FTS", "Version");
}

bool CWizIndex::setDocumentFTSEnabled(bool b)
{
    return SetMeta(_T("FTS"), _T("Enabled"), b ? _T("1") : _T("0"));
}

bool CWizIndex::isDocumentFTSEnabled()
{
    QString str = GetMetaDef(_T("FTS"), _T("Enabled"));
    if (str == _T("1") || str.isEmpty())
        return true;

    return false;
}

bool CWizIndex::setAllDocumentsSearchIndexed(bool b)
{
	CString strSQL = WizFormatString1(_T("update WIZ_DOCUMENT set DOCUMENT_INDEXED=%1"),
        b ? _T("1") : _T("0"));

	if (!ExecSQL(strSQL))
        return false;

    return true;
}

bool CWizIndex::setDocumentSearchIndexed(const QString& strDocumentGUID, bool b)
{
	CString strSQL = WizFormatString2(_T("update WIZ_DOCUMENT set DOCUMENT_INDEXED=%1 where DOCUMENT_GUID=%2"),
		b ? _T("1") : _T("0"),
        STR2SQL(strDocumentGUID));

	if (!ExecSQL(strSQL))
        return false;

    return true;
}

bool CWizIndex::SearchDocumentByWhere(const QString& strWhere, int nMaxCount, CWizDocumentDataArray& arrayDocument)
{
    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere);

    if (!SQLToDocumentDataArray(strSQL, arrayDocument))
        return false;

    if (arrayDocument.size() > nMaxCount) {
        arrayDocument.resize(nMaxCount);
    }

    return true;
}

bool CWizIndex::getAllDocumentsNeedToBeSearchIndexed(CWizDocumentDataArray& arrayDocument)
{
    CString strWhere = QString("DOCUMENT_INDEXED=0 and DOCUMENT_GUID in \
                               (select DISTINCT OBJECT_GUID from WIZ_OBJECT_EX where %1=1)").arg(GetReservedIntFieldName(DATA_DOWNLOADED_FIELD));

    if (!GetDocumentsBySQLWhere(strWhere, arrayDocument)) {
		TOLOG(_T("Failed to get documents by DOCUMENT_INDEXED=0"));
        return false;
    }

    return true;
}

bool CWizIndex::GetAllParentsTagGUID(CString strTagGUID, CWizStdStringArray& arrayGUID)
{
	int nIndex = 0;
	while (1)
	{
		WIZTAGDATA data;
		if (!TagFromGUID(strTagGUID, data))
            return false;

		if (data.strParentGUID.IsEmpty())
            return true;

		arrayGUID.push_back(data.strParentGUID);

		strTagGUID = data.strParentGUID;

		nIndex++;

        if (nIndex > 100) {
			TOLOG(_T("Tags data error!"));
            return false;
		}
	}
}

bool CWizIndex::GetAllParentsTagGUID(CString strTagGUID, std::set<CString>& setGUID)
{
	CWizStdStringArray arrayGUID;
    bool bRet = GetAllParentsTagGUID(strTagGUID, arrayGUID);

    CWizStdStringArray::const_iterator it;
    for (it = arrayGUID.begin(); it != arrayGUID.end(); it++) {
		setGUID.insert(*it);
	}

	return bRet;
}

bool CWizIndex::IsLocationEmpty(const CString& strLocation)
{
	CString strSQL = WizFormatString1(_T("select DOCUMENT_LOCATION from WIZ_DOCUMENT where DOCUMENT_LOCATION  like %1 limit 0, 1"), 
        STR2SQL_LIKE_RIGHT(strLocation));

	return !HasRecord(strSQL);
}

bool CWizIndex::GetAllLocations(CWizStdStringArray& arrayLocation)
{
    QString strSQL = "select distinct DOCUMENT_LOCATION from WIZ_DOCUMENT";
	return SQLToStringArray(strSQL, 0, arrayLocation);
}

void CWizIndex::GetAllLocationsWithExtra(CWizStdStringArray& arrayLocation)
{
    GetAllLocations(arrayLocation);

    CWizStdStringArray arrayExtra;
    GetExtraFolder(arrayExtra);
    //
    for (CWizStdStringArray::const_iterator it = arrayExtra.begin();
         it != arrayExtra.end();
         it++)
    {
        if (::WizFindInArray(arrayLocation, *it) == -1)
        {
            arrayLocation.push_back(*it);
        }
    }
}

bool CWizIndex::GetAllChildLocations(const CString& strLocation, CWizStdStringArray& arrayLocation)
{
    CString strSQL = WizFormatString1(_T("select distinct DOCUMENT_LOCATION from WIZ_DOCUMENT where DOCUMENT_LOCATION like %1"),
                                      STR2SQL_LIKE_RIGHT(strLocation));

    return SQLToStringArray(strSQL, 0, arrayLocation);
}

bool CWizIndex::ObjectInReserved(const CString& strGUID, const CString& strType)
{
    CString strSQL;
    strSQL = WizFormatString2("select OBJECT_GUID from WIZ_OBJECT_EX where OBJECT_GUID=%1 and OBJECT_TYPE=%2",
                  STR2SQL(strGUID),
                  STR2SQL(strType));

    return HasRecord(strSQL);
}

CString CWizIndex::GetReservedIntFieldName(CWizIndex::WizObjectReservedInt e)
{
    switch (e)
    {
    case reserved1: return "OBJECT_RESERVED1";
    case reserved2: return "OBJECT_RESERVED2";
    case reserved3: return "OBJECT_RESERVED3";
    case reserved4: return "OBJECT_RESERVED4";
    default:
        ATLASSERT(false);
        return CString();
    }
}

CString CWizIndex::GetReservedStrFieldName(WizObjectReservedStr e)
{
    switch (e)
    {
    case reserved5: return "OBJECT_RESERVED5";
    case reserved6: return "OBJECT_RESERVED6";
    case reserved7: return "OBJECT_RESERVED7";
    case reserved8: return "OBJECT_RESERVED8";
    default:
        ATLASSERT(false);
        return CString();
    }
}

bool CWizIndex::SetObjectReservedInt(const CString& strGUID, const CString& strType, CWizIndex::WizObjectReservedInt e, int val)
{
    CString strSQL;
    if (ObjectInReserved(strGUID, strType))
    {
        strSQL = WizFormatString4("update WIZ_OBJECT_EX set %1=%2 where OBJECT_GUID=%3 and OBJECT_TYPE=%4",
                                  GetReservedIntFieldName(e),
                                  WizIntToStr(val),
                                  STR2SQL(strGUID),
                                  STR2SQL(strType));

    }
    else
    {
        strSQL = WizFormatString4("insert into WIZ_OBJECT_EX (OBJECT_GUID, OBJECT_TYPE, %1) values(%2, %3, %4)",
                                  GetReservedIntFieldName(e),
                                  STR2SQL(strGUID),
                                  STR2SQL(strType),
                                  WizIntToStr(val));

    }
    //
    return ExecSQL(strSQL);
}

bool CWizIndex::GetObjectReservedInt(const CString& strGUID, const CString& strType, WizObjectReservedInt e, int& val)
{
    if (!ObjectInReserved(strGUID, strType))
        return false;
    //
    CString strSQL;
    strSQL = WizFormatString3("select %1 from WIZ_OBJECT_EX where OBJECT_GUID=%2 and OBJECT_TYPE=%3",
                            GetReservedIntFieldName(e),
                            STR2SQL(strGUID),
                            STR2SQL(strType));
    //
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        //
        if (!query.eof())
        {
            val = query.getIntField(val);
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (const CppSQLite3Exception& e)
    {
        return LogSQLException(e, strSQL);
    }
}

//
bool CWizIndex::SetObjectReservedStr(const CString& strGUID, const CString& strType, WizObjectReservedStr e, const CString& val)
{
    CString strSQL;
    if (ObjectInReserved(strGUID, strType))
    {
        strSQL = WizFormatString4("update WIZ_OBJECT_EX set %1=%2 where OBJECT_GUID=%3 and OBJECT_TYPE=%4",
                                  GetReservedStrFieldName(e),
                                  STR2SQL(val),
                                  STR2SQL(strGUID),
                                  STR2SQL(strType));

    }
    else
    {
        strSQL = WizFormatString4("insert into WIZ_OBJECT_EX (OBJECT_GUID, OBJECT_TYPE, %1) values(%2, %3, %4)",
                                  GetReservedStrFieldName(e),
                                  STR2SQL(strGUID),
                                  STR2SQL(strType),
                                  STR2SQL(val));

    }
    //
    return ExecSQL(strSQL);
}

bool CWizIndex::GetObjectReservedStr(const CString& strGUID, const CString& strType, WizObjectReservedStr e, CString& val)
{
    if (!ObjectInReserved(strGUID, strType))
        return false;
    //
    CString strSQL;
    strSQL = WizFormatString3("select %1 from WIZ_OBJECT_EX where OBJECT_GUID=%2 and OBJECT_TYPE=%3",
                            GetReservedStrFieldName(e),
                            STR2SQL(strGUID),
                            STR2SQL(strType));
    //
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        //
        if (!query.eof())
        {
            val = query.getStringField(0);
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (const CppSQLite3Exception& e)
    {
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndex::SetDocumentDataDownloaded(const CString& strGUID, bool bDownloaded)
{
    return SetObjectDataDownloaded(strGUID, "document", bDownloaded);
}

bool CWizIndex::SetAttachmentDataDownloaded(const CString& strGUID, bool bDownloaded)
{
    return SetObjectDataDownloaded(strGUID, "attachment", bDownloaded);
}

bool CWizIndex::SetObjectDataDownloaded(const CString& strGUID, const CString& strType, bool bDownloaded)
{
    return SetObjectReservedInt(strGUID, strType, DATA_DOWNLOADED_FIELD, bDownloaded ? 1 : 0);
}

bool CWizIndex::IsObjectDataDownloaded(const CString& strGUID, const CString& strType)
{
    int nDownloaded = 0;
    GetObjectReservedInt(strGUID, strType, DATA_DOWNLOADED_FIELD, nDownloaded);
    return nDownloaded ? true : false;
}

int CWizIndex::GetNeedToBeDownloadedDocumentCount()
{
    CString strSQL = WizFormatString1("select count(*) from WIZ_DOCUMENT where DOCUMENT_GUID not in (select OBJECT_GUID from WIZ_OBJECT_EX where OBJECT_TYPE='document' and %1=1)",
                                      GetReservedIntFieldName(DATA_DOWNLOADED_FIELD));

    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);

        if (!query.eof()) {
            return query.getIntField(0);
        } else {
            return 0;
        }
    }
    catch (const CppSQLite3Exception& e)
    {
        LogSQLException(e, strSQL);
        return 0;
    }
}

int CWizIndex::GetNeedToBeDownloadedAttachmentCount()
{
    CString strSQL = WizFormatString1("select count(*) from WIZ_DOCUMENT_ATTACHMENT where ATTACHMENT_GUID not in (select OBJECT_GUID from WIZ_OBJECT_EX where OBJECT_TYPE='attachment' and %1=1)",
                                      GetReservedIntFieldName(DATA_DOWNLOADED_FIELD));
    //
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        //
        if (!query.eof())
        {
            return query.getIntField(0);
        }
        else
        {
            return 0;
        }
    }
    catch (const CppSQLite3Exception& e)
    {
        LogSQLException(e, strSQL);
        return 0;
    }
}

bool CWizIndex::GetNeedToBeDownloadedDocuments(CWizDocumentDataArray& arrayData)
{
    CString strSQL = WizFormatString2("select %1 from WIZ_DOCUMENT where DOCUMENT_GUID not in (select OBJECT_GUID from WIZ_OBJECT_EX where OBJECT_TYPE='document' and %2=1) order by DT_DATA_MODIFIED desc",
                                      FIELD_LIST_WIZ_DOCUMENT,
                                      GetReservedIntFieldName(DATA_DOWNLOADED_FIELD));
    //
    CWizDocumentDataArray arrayDocument;
    SQLToDocumentDataArray(strSQL, arrayDocument);
    if (arrayDocument.empty())
        return false;
    //
    arrayData.assign(arrayDocument.begin(), arrayDocument.end());
    return true;
}
bool CWizIndex::GetNeedToBeDownloadedAttachments(CWizDocumentAttachmentDataArray& arrayData)
{
    CString strSQL = WizFormatString2("select %1 from WIZ_DOCUMENT_ATTACHMENT where ATTACHMENT_GUID not in (select OBJECT_GUID from WIZ_OBJECT_EX where OBJECT_TYPE='attachment' and %2=1) order by DT_DATA_MODIFIED desc",
                                      FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT,
                                      GetReservedIntFieldName(DATA_DOWNLOADED_FIELD));
    //
    CWizDocumentAttachmentDataArray arrayAttachment;
    SQLToDocumentAttachmentDataArray(strSQL, arrayAttachment);
    if (arrayAttachment.empty())
        return false;
    //
    arrayData.assign(arrayAttachment.begin(), arrayAttachment.end());
    return true;
}

CString CWizIndex::TagDisplayNameToName(const CString& strDisplayName)
{
    CString strName(strDisplayName);
    if (strName == TAG_DISPLAY_NAME_PUBLIC())
		strName = TAG_NAME_PUBLIC;
    else if (strName == TAG_DISPLAY_NAME_SHARE_WITH_FRIENDS())
		strName = TAG_NAME_SHARE_WITH_FRIENDS;

	return strName;
}

CString CWizIndex::TagNameToDisplayName(const CString& strName)
{
    CString strDisplayName(strName);
    if (strDisplayName == TAG_NAME_PUBLIC)
        strDisplayName = TAG_DISPLAY_NAME_PUBLIC();
    else if (strDisplayName == TAG_NAME_SHARE_WITH_FRIENDS)
        strDisplayName = TAG_DISPLAY_NAME_SHARE_WITH_FRIENDS();

    return strDisplayName;
}

void CWizIndex::GetExtraFolder(CWizStdStringArray& arrayLocation)
{
    CString str = GetMetaDef("FOLDERS", "EXTRA");
    ::WizSplitTextToArray(str, '\\', arrayLocation);
}

void CWizIndex::SetExtraFolder(const CWizStdStringArray& arrayLocation)
{
    CString strText;
    ::WizStringArrayToText(arrayLocation, strText, "\\");
    SetMeta("FOLDERS", "EXTRA", strText);
}

void CWizIndex::AddExtraFolder(const QString& strLocation)
{
    CWizStdStringArray arrayLocation;
    GetExtraFolder(arrayLocation);
    if (-1 != ::WizFindInArray(arrayLocation, strLocation)) {
        return;
    }

    arrayLocation.push_back(strLocation);
    Q_EMIT folderCreated(strLocation);

    // add all of it's parents
    QString strParent = strLocation;
    int idx = strParent.lastIndexOf("/", -2);
    while (idx) {
        // 如果文件夹格式错误，直接退出，防止死循环
        if (strParent.left(1) != "/" || strParent.right(1) != "/")
        {
            qCritical() << "try to add a error location : " << strParent;
            return;
        }

        strParent = strParent.left(idx + 1);
        idx = strParent.lastIndexOf("/", -2);

        if (-1 == ::WizFindInArray(arrayLocation, strParent)) {
            arrayLocation.push_back(strParent);
            Q_EMIT folderCreated(strParent);
        }
    }

    SetExtraFolder(arrayLocation);
}

void CWizIndex::DeleteExtraFolder(const QString& strLocation)
{
    CWizStdStringArray arrayLocation;
    GetExtraFolder(arrayLocation);

    // delete folder and all it's subfolders
    int n = arrayLocation.size();
    for (intptr_t i = n - 1; i >= 0; i--) {
        QString str = arrayLocation.at(i);
        if (str.right(1) != "/") {
            str.append("/");
        }
        //int idx = str.lastIndexOf("/", -2);
        if (str.startsWith(strLocation)) {
            arrayLocation.erase(arrayLocation.begin() + i);
            Q_EMIT folderDeleted(str);
        }
    }

    SetExtraFolder(arrayLocation);
}

/**
 * @brief CWizIndex::UpdateLocation     just modify document location, do not change document modify date
 * @param strOldLocation
 * @param strNewLocation
 * @return
 */
bool CWizIndex::UpdateLocation(const QString& strOldLocation, const QString& strNewLocation)
{
//    QString sql = QString("update %1 set DOCUMENT_LOCATION='%2' where "
//                          "DOCUMENT_LOCATION='%3'").arg(TABLE_NAME_WIZ_DOCUMENT)
//                          .arg(strNewLocation).arg(strOldLocation);
//    bool result = ExecSQL(sql);
    qDebug() << "update location from : " << strOldLocation << " to : " << strNewLocation;

    CWizDocumentDataArray docArray;
    if (!GetDocumentsByLocation(strOldLocation, docArray, true))
        return false;

    //update all include document location
    for (CWizDocumentDataArray::const_iterator it = docArray.begin();
         it != docArray.end();
         it++)
    {
        WIZDOCUMENTDATA doc = *it;
        doc.strLocation.replace(strOldLocation, strNewLocation);
        doc.nVersion = -1;
        ModifyDocumentInfoEx(doc);
    }

    CWizStdStringArray arrayExtra;
    GetExtraFolder(arrayExtra);
    CWizStdStringArray newArray;
    //
    for (CWizStdStringArray::const_iterator it = arrayExtra.begin();
         it != arrayExtra.end();
         it++)
    {
        QString strLocation = *it;
        if (strLocation.contains(strOldLocation))
        {
            strLocation.replace(strOldLocation, strNewLocation);
            newArray.push_back(strLocation);
        }
        else
        {
            newArray.push_back(*it);
        }
    }
    SetExtraFolder(newArray);

    return true;
}
