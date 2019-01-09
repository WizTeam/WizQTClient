#include "WizIndex.h"

#include <QDebug>

#include <algorithm>
#include "WizKMCore.h"
#include "utils/WizLogger.h"
#include "utils/WizMisc.h"
#include "WizMisc.h"


WizIndex::WizIndex(void)
    : m_strDeletedItemsLocation(LOCATION_DELETED_ITEMS)
{
}

bool WizIndex::getDocumentTags(const CString& strDocumentGUID, CWizTagDataArray& arrayTag)
{
    CString strWhere = WizFormatString1("TAG_GUID in (select TAG_GUID from WIZ_DOCUMENT_TAG where DOCUMENT_GUID='%1')", strDocumentGUID);
	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, strWhere); 
	return sqlToTagDataArray(strSQL, arrayTag);
}

bool WizIndex::getDocumentAttachments(const CString& strDocumentGUID, CWizDocumentAttachmentDataArray& arrayAttachment)
{
    CString strWhere = WizFormatString1("DOCUMENT_GUID='%1'", strDocumentGUID);
	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT, strWhere); 
	return sqlToDocumentAttachmentDataArray(strSQL, arrayAttachment);
}

bool WizIndex::getMetasByName(const QString& strMetaName, CWizMetaDataArray& arrayMeta)
{
    if (strMetaName.isEmpty()) {
        TOLOG("Meta name is empty!");
        return false;
	}

    CString strWhere = WizFormatString1("META_NAME=%1", STR2SQL(strMetaName));
	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_META, FIELD_LIST_WIZ_META, strWhere); 
	return sqlToMetaDataArray(strSQL, arrayMeta);
}

bool WizIndex::getDeletedGuids(CWizDeletedGUIDDataArray& arrayGUID)
{
	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID); 
	return sqlToDeletedGuidDataArray(strSQL, arrayGUID);
}

bool WizIndex::getDeletedGuids(WizObjectType eType, CWizDeletedGUIDDataArray& arrayGUID)
{
    CString strWhere = WizFormatString1("GUID_TYPE=%1", WizIntToStr(eType));
	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID, strWhere);
	return sqlToDeletedGuidDataArray(strSQL, arrayGUID);
}

bool WizIndex::logDeletedGuid(const CString& strGUID, WizObjectType eType)
{
	CString strFormat = formatInsertSQLFormat(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID, PARAM_LIST_WIZ_DELETED_GUID);

	CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(strGUID).utf16(),
		int(eType),
        TIME2SQL(WizGetCurrentTime()).utf16()
		);

	return execSQL(strSQL);
}

bool WizIndex::logDeletedGuids(const CWizStdStringArray& arrayGUID, WizObjectType eType)
{
    CWizStdStringArray::const_iterator it;
    for (it = arrayGUID.begin(); it != arrayGUID.end(); it++) {
        if (!logDeletedGuid(*it, eType)) {
            TOLOG1("Warning: Failed to log deleted guid %1", *it);
		}
    }

    return true;
}

bool WizIndex::tagByName(const CString& strName, CWizTagDataArray& arrayTag, const CString& strExceptGUID /*= ""*/)
{
    CWizTagDataArray arrayAllTag;
    if (!getAllTags(arrayAllTag)) {
        TOLOG("Failed to get tags!");
        return false;
	}

    CString strFindName = tagDisplayNameToName(strName);

    CWizTagDataArray::const_iterator it;
    for (it = arrayAllTag.begin(); it != arrayAllTag.end(); it++) {
        if (0 == it->strName.compareNoCase(strFindName)) {
			if (0 == strExceptGUID.compareNoCase(it->strGUID))
				continue;

            arrayTag.push_back(*it);
		}
	}

    return !arrayTag.empty();
}

bool WizIndex::tagByNameEx(const CString& strName, WIZTAGDATA& data)
{
    CWizTagDataArray arrayTag;
    if (!tagByName(strName, arrayTag, ""))
        return false;

    for (WIZTAGDATA tag : arrayTag)
    {
        if (tag.strParentGUID.isEmpty())
            return true;
    }

    return createTag("", strName, "", data);
}

bool WizIndex::tagArrayByName(const CString& strName, CWizTagDataArray& arrayTagRet)
{
    CWizTagDataArray arrayTag;
    if (!getAllTags(arrayTag)) {
        TOLOG("Failed to get tags!");
        return false;
    }

    CString strFindName = tagDisplayNameToName(strName);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        if (-1 != ::WizStrStrI_Pos(it->strName, strFindName)) {
            arrayTagRet.push_back(*it);
        }
    }

    return true;
}

bool WizIndex::tagsTextToTagArray(CString strText, CWizTagDataArray& arrayTag)
{
	CWizStdStringArray arrayText;
	WizSplitTextToArray(strText, ';', arrayText);
    WizStringArrayRemoveMultiElementNoCase(arrayText);

    CWizStdStringArray::const_iterator it;
    for (it = arrayText.begin(); it != arrayText.end(); it++) {
		CString strName = *it;
		strName.trim();
		if (strName.isEmpty())
			continue;

		WIZTAGDATA data;
        if (!tagByNameEx(strName, data)) {
            TOLOG1("Failed to create tag %1", strName);
            return false;
		}

		arrayTag.push_back(data);
	}

    return true;
}

bool WizIndex::styleByName(const CString& strName, WIZSTYLEDATA& data, const CString& strExceptGUID /*= ""*/)
{
	CWizStyleDataArray arrayStyle;
    if (!getStyles(arrayStyle)) {
        TOLOG("Failed to get Styles!");
        return false;
	}

    CWizStyleDataArray::const_iterator it;
    for (it = arrayStyle.begin(); it != arrayStyle.end(); it++) {
        if (0 == it->strName.compareNoCase(strName)) {
            if (0 == strExceptGUID.compareNoCase(it->strGUID))
				continue;

			data = *it;
            return true;
		}
	}

    return false;
}

bool WizIndex::createMessage(const WIZMESSAGEDATA& data)
{
    return createMessageEx(data);
}

bool WizIndex::getAllMessages(CWizMessageDataArray& arrayMsg)
{
    QString strSQL = formatQuerySQL(TABLE_NAME_WIZ_MESSAGE,
                                    FIELD_LIST_WIZ_MESSAGE);
    return sqlToMessageDataArray(strSQL, arrayMsg);
}

bool WizIndex::getAllMessageSenders(CWizStdStringArray& arraySender)
{
    QString strSQL = WizFormatString2("SELECT distinct SENDER_GUID from %1 where MESSAGE_TYPE<%2",
                                      TABLE_NAME_WIZ_MESSAGE, QString::number(WIZ_USER_MSG_TYPE_REQUEST_JOIN_GROUP));
    return sqlToStringArray(strSQL, 0, arraySender);
}

bool WizIndex::getLastestMessages(CWizMessageDataArray& arrayMsg, int nMax)
{
    CString strExt;
    strExt.format("order by DT_CREATED desc limit 0, %s",
                  WizIntToStr(nMax).utf16());
    QString strSQL = formatCanonicSQL(TABLE_NAME_WIZ_MESSAGE,
                                      FIELD_LIST_WIZ_MESSAGE,
                                      strExt);

    return sqlToMessageDataArray(strSQL, arrayMsg);
}

bool WizIndex::setMessageReadStatus(const WIZMESSAGEDATA& msg)
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

bool WizIndex::setMessageDeleteStatus(const WIZMESSAGEDATA& msg)
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


bool WizIndex::getModifiedMessages(CWizMessageDataArray& arrayMsg)
{
    CString strExt;
    strExt.format("where LOCAL_CHANGED>0");
    QString strSQL = formatCanonicSQL(TABLE_NAME_WIZ_MESSAGE,
                                      FIELD_LIST_WIZ_MESSAGE,
                                      strExt);

    return sqlToMessageDataArray(strSQL, arrayMsg);
}

bool WizIndex::getUnreadMessages(CWizMessageDataArray& arrayMsg)
{
    CString strExt;
    strExt.format("where READ_STATUS=0 order by DT_CREATED desc");
    QString strSQL = formatCanonicSQL(TABLE_NAME_WIZ_MESSAGE,
                                      FIELD_LIST_WIZ_MESSAGE,
                                      strExt);

    return sqlToMessageDataArray(strSQL, arrayMsg);
}

bool WizIndex::modifyMessageLocalChanged(const WIZMESSAGEDATA& msg)
{
    CString strSQL = WizFormatString4("update %1 set LOCAL_CHANGED=%2 where %3=%4",
        TABLE_NAME_WIZ_MESSAGE,
        WizIntToStr(msg.nLocalChanged),
        TABLE_KEY_WIZ_MESSAGE,
        WizInt64ToStr(msg.nId));

    return execSQL(strSQL);
}

int WizIndex::getUnreadMessageCount()
{
    CString strSQL;
    strSQL.format("select count(*) from WIZ_MESSAGE where READ_STATUS=0 and DELETE_STATUS=0");

    CppSQLite3Query query = m_db.execQuery(strSQL);

    if (!query.eof()) {
        int nCount = query.getIntField(0);
        return nCount;
    }

    return 0;
}


bool WizIndex::createTag(const CString& strParentTagGUID,
                          const CString& strName,
                          const CString& strDescription,
                          WIZTAGDATA& data)
{
    if (strName.isEmpty()) {
        TOLOG("Tag name is empty");
        return false;
    }

    CWizTagDataArray arrayTag;
    if (tagByName(strName, arrayTag)) {
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

	return createTagEx(data);
}

bool WizIndex::createStyle(const CString& strName, const CString& strDescription,
                            COLORREF crTextColor, COLORREF crBackColor, bool bTextBold,
                            int nFlagIndex, WIZSTYLEDATA& data)
{
    if (!strName) {
        TOLOG("NULL Pointer: CreateStyle:Name!");
        return false;
	}

    if (!*strName) {
        TOLOG("Style name is empty");
        return false;
	}

    if (styleByName(strName, data)) {
        TOLOG1("Style already exists: %1!", data.strName);
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

	return createStyleEx(data);
}

bool WizIndex::createDocument(const CString& strTitle, const CString& strName, \
                            const CString& strLocation, const CString& strURL, \
                            const CString& strAuthor, const CString& strKeywords, \
                            const CString& strType, const CString& strOwner, \
                            const CString& strFileType, const CString& strStyleGUID, \
                            int nIconIndex, int nSync, int nProtected, WIZDOCUMENTDATA& data)
{
    Q_UNUSED(strOwner);

    if (strTitle.isEmpty()) {
        TOLOG("NULL Pointer or Document title is empty: CreateDocument:Title!");
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

    //data.nIconIndex = nIconIndex;
    //data.nSync = nSync;
	data.nProtected = nProtected;
	data.nReadCount = 0;
	data.nAttachmentCount = 0;
	data.nIndexed = 0;
    //
    data.nVersion = -1;
    data.nInfoChanged = 1;
    data.nDataChanged = 1;

	getNextTitle(data.strLocation, data.strTitle);

    //data.tInfoModified = data.tCreated;
    //data.strInfoMD5 = CalDocumentInfoMD5(data);
	data.tDataModified = data.tCreated;
    data.strDataMD5 = "";
    //data.tParamModified = data.tCreated;
    //data.strParamMD5 = "";

	return createDocumentEx(data);

}

bool WizIndex::createDocument(const CString& strTitle, const CString& strName, \
                            const CString& strLocation, const CString& strURL, \
                            int nProtected, WIZDOCUMENTDATA& data)
{
    return createDocument(strTitle, strName, \
                          strLocation, strURL, \
                          "", "", data.strType, "", "", "", 0, 0, nProtected, data);
}

#define ATTACHMENT_MAX_NAME     50

bool WizIndex::createAttachment(const CString& strDocumentGUID, const CString& strName,
                                 const CString& strURL, const CString& strDescription,
                                 const CString& strDataMD5, WIZDOCUMENTATTACHMENTDATA& data)
{
    if (strDocumentGUID.isEmpty()) {
        TOLOG("NULL Pointer or Document guid is empty: CreateAttachment!");
        return false;
    }

    if (strName.isEmpty()) {
        TOLOG("NULL Pointer or name is empty: CreateAttachment!");
        return false;
    }
    //
    QString name = strName;
    if (name.length() > ATTACHMENT_MAX_NAME) {
        QString ext = Utils::WizMisc::extractFileExt(name);
        QString title = Utils::WizMisc::extractFileTitle(name);
        if (ext.length() < ATTACHMENT_MAX_NAME) {
            name = title.left(ATTACHMENT_MAX_NAME - ext.length()) + ext;
        } else {
            name = title.left(ATTACHMENT_MAX_NAME);
        }
    }

	data.strGUID = WizGenGUIDLowerCaseLetterOnly();
    data.strDocumentGUID = CString(strDocumentGUID).makeLower();
    data.strName = name;
    data.strDescription = strDescription;
    data.strURL = strURL;
	data.tInfoModified = WizGetCurrentTime();
	data.strInfoMD5 = calDocumentAttachmentInfoMD5(data);
	data.tDataModified = data.tInfoModified;
    data.strDataMD5 = strDataMD5;

	return createAttachmentEx(data);
}

bool WizIndex::titleExists(const CString& strLocation, CString strTitle)
{
	strTitle.makeLower();

	CString strSQL;
    strSQL.format("select DOCUMENT_GUID from WIZ_DOCUMENT where DOCUMENT_LOCATION like %s and lower(DOCUMENT_TITLE)=%s",
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
		logSQLException(e, strSQL);
	}

    return true;
}

bool WizIndex::getNextTitle(const QString& strLocation, QString& strTitle)
{
	CString strTemplate(strTitle);
    Utils::WizMisc::extractTitleTemplate(strTemplate);
	strTitle = strTemplate;

    if (!titleExists(strLocation, strTitle))
        return true;

	int nIndex = 2;
    while (titleExists(strLocation, strTitle)) {
        strTitle = WizFormatString2("%1 (%2)", strTemplate, WizIntToStr(nIndex));
		nIndex++;
	}

    return true;
}

QString WizIndex::getTableStructureVersion()
{
    return getMetaDef("TableStructure", "Version");
}

bool WizIndex::setTableStructureVersion(const QString& strVersion)
{
    return setMeta("TableStructure", "Version", strVersion);
}


CString WizIndex::calDocumentAttachmentInfoMD5(const WIZDOCUMENTATTACHMENTDATA& data)
{
    return WizMd5StringNoSpace(data.strDocumentGUID \
                               + data.strName \
                               + data.strURL \
                               + data.strDescription);
}

bool WizIndex::modifyTag(WIZTAGDATA& data)
{
    if (data.strKbGUID.isEmpty()) {
        TOLOG("ModifyTag: Failed to modify tag: kbguid is empty");
        return false;
    }

    if (data.strGUID.isEmpty()) {
        TOLOG("ModifyTag: Failed to modify tag: guid is empty");
        return false;
	}

    if (data.strName.isEmpty()) {
        TOLOG("ModifyTag: Failed to modify tag: name is empty");
        return false;
    }

    CWizTagDataArray arrayTag;
    if (tagByName(data.strName, arrayTag, data.strGUID)) {
        for (WIZTAGDATA tagItem : arrayTag) {
            if (tagItem.strParentGUID == data.strParentGUID) {
                TOLOG1("ModifyTag: Tag already exists with the same parent: %1", data.strName);
                return false;
            }
        }
    }

	data.tModified = WizGetCurrentTime();
    data.nVersion = -1;

	return modifyTagEx(data);
}

bool WizIndex::deleteTag(const WIZTAGDATA& data, bool bLog, bool bReset /* = true */)
{
    if (!deleteTagDocuments(data, bReset)) {
        TOLOG1("Failed to delete documents of tag: %1", data.strName);
        return false;
    }

    if (bLog) {
        if (!logDeletedGuid(data.strGUID, wizobjectTag)) {
            TOLOG("Warning: Failed to log deleted tag guid!");
        }
    }

    return deleteTagEx(data);
}

bool WizIndex::modifyTagPosition(const WIZTAGDATA& data)
{
    CString strSQL = WizFormatString2("update WIZ_TAG set TAG_POS=%1 where TAG_GUID=%2",
        WizInt64ToStr(data.nPostion),
        STR2SQL(data.strGUID));

    //
    if (!execSQL(strSQL))
        return false;

    return true;
}

QString WizIndex::getTagTreeText(const QString& strTagGUID)
{
    WIZTAGDATA tag;
    if (!tagFromGuid(strTagGUID, tag)) {
        return QString();
    }

    QString strText = "/" + tag.strName;

    QString strTagParentGUID = tag.strParentGUID;
    while(!strTagParentGUID.isEmpty()) {
        if (!tagFromGuid(strTagParentGUID, tag)) {
            return strText;
        }

        strTagParentGUID = tag.strParentGUID;

        QString strCurrent = "/" + tag.strName;
        strText.prepend(strCurrent);
    }

    return strText;
}

bool WizIndex::modifyStyle(WIZSTYLEDATA& data)
{
    if (data.strGUID.isEmpty()) {
        TOLOG("Failed to modify style: guid is empty!");
        return false;
    }

    if (data.strName.isEmpty()) {
        TOLOG("Failed to modify style: name is empty!");
        return false;
	}

	WIZSTYLEDATA dataTemp;
    if (styleByName(data.strName, dataTemp, data.strGUID)) {
        TOLOG1("Failed to modify style: Style already exists: %1!", data.strName);
        return false;
	}

	data.tModified = WizGetCurrentTime();
    data.nVersion = -1;

	return modifyStyleEx(data);
}

bool WizIndex::deleteStyle(const WIZSTYLEDATA& data, bool bLog, bool bReset /* = true */)
{
    if(!deleteStyleDocuments(data, bReset)) {
        TOLOG1("Failed to delete documents of style: %1", data.strName);
        return false;
    }

    if (bLog) {
        if (!logDeletedGuid(data.strGUID, wizobjectStyle)) {
            TOLOG("Warning: Failed to log deleted style guid!");
        }
    }

    return deleteStyleEx(data);
}

bool WizIndex::modifyDocumentInfo(WIZDOCUMENTDATA& data, bool bReset /* = true */)
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
        data.nInfoChanged = 1;
        data.tModified = WizGetCurrentTime();
        data.nVersion = -1;
        //
        qDebug() << "Reset document info & version, guid: " << data.strGUID << ", title: " << data.strTitle;
    }

	return modifyDocumentInfoEx(data);
}

bool WizIndex::modifyDocumentDateModified(WIZDOCUMENTDATA& data)
{
    if (data.strGUID.isEmpty()) {
        TOLOG("Failed to modify document: guid is empty!");
        return false;
	}

    data.nInfoChanged = 1;
    data.nVersion = -1;

	return modifyDocumentInfoEx(data);
}

bool WizIndex::modifyDocumentDataDateModified(WIZDOCUMENTDATA& data)
{
    if (data.strGUID.isEmpty()) {
        TOLOG("Failed to modify document: guid is empty!");
        return false;
    }

    data.nInfoChanged = 1;
    data.nVersion = -1;

	return modifyDocumentInfoEx(data);
}

bool WizIndex::modifyDocumentDateAccessed(WIZDOCUMENTDATA& data)
{
    Q_ASSERT(data.strKbGUID == kbGUID());

	CString strSQL;
    strSQL.format("update WIZ_DOCUMENT set DT_ACCESSED=%s where DOCUMENT_GUID=%s",
        TIME2SQL(data.tAccessed).utf16(),
        STR2SQL(data.strGUID).utf16()
        );

	if (!execSQL(strSQL))
        return false;

    emit documentAccessDateModified(data);
    return true;
}

bool WizIndex::modifyDocumentReadCount(const WIZDOCUMENTDATA& data)
{
	CString strSQL;
    strSQL.format("update WIZ_DOCUMENT set DOCUMENT_READ_COUNT=%d where DOCUMENT_GUID=%s",
		data.nReadCount,
        STR2SQL(data.strGUID).utf16()
        );

    bool ret = execSQL(strSQL);

    if (ret)
    {
        emit documentReadCountChanged(data);
    }
    return ret;
}

bool WizIndex::modifyDocumentLocation(WIZDOCUMENTDATA& data)
{
    if (data.strGUID.isEmpty()) {
        TOLOG("Failed to modify document: guid is empty!");
        return false;
    }

    data.nInfoChanged = 1;
    data.nVersion = -1;

    return modifyDocumentInfoEx(data);
}

bool WizIndex::deleteDocument(const WIZDOCUMENTDATA& data, bool bLog)
{
    WIZDOCUMENTDATA dataTemp = data;

    // don't have to reset md5 and version
    if (!deleteDocumentTags(dataTemp, false)) {
        TOLOG1("Failed to delete document tags: %1", data.strTitle);
        return false;
    }

    if (bLog) {
        if (!logDeletedGuid(data.strGUID, wizobjectDocument)) {
            TOLOG("Warning: Failed to log deleted document guid!");
        }
    }

    return deleteDocumentEx(data);
}

bool WizIndex::getGroupUnreadDocuments(CWizDocumentDataArray& arrayDocument)
{

    QString strSQL = QString("select %1 from %2 %3").arg(FIELD_LIST_WIZ_DOCUMENT)
            .arg(TABLE_NAME_WIZ_DOCUMENT).arg("where DOCUMENT_READ_COUNT=0 and DOCUMENT_LOCATION not like '/Deleted Items/%' limit 1000");

    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

int WizIndex::getGroupUnreadDocumentCount()
{
    CString strSQL;
    strSQL.format("select count(*) from WIZ_DOCUMENT where DOCUMENT_READ_COUNT=0 and DOCUMENT_LOCATION not like '/Deleted Items/%'");

    CppSQLite3Query query = m_db.execQuery(strSQL);

    if (!query.eof()) {
        int nCount = query.getIntField(0);
        return nCount;
    }


    return 0;
}

void WizIndex::setGroupDocumentsReaded()
{
    CString strSQL = WizFormatString0("update WIZ_DOCUMENT set DOCUMENT_READ_COUNT=1 where DOCUMENT_READ_COUNT=0");
    execSQL(strSQL);
    emit groupDocumentUnreadCountModified(kbGUID());
}

CString WizIndex::kbGuidToSQL()
{
    CString strKbGUIDSQL;
    if (kbGUID().isEmpty())
    {
        strKbGUIDSQL = " (KB_GUID is null or KB_GUID = '') ";
    }
    else
    {
        strKbGUIDSQL = WizFormatString1("KB_GUID = '%1'", kbGUID());
    }
    //
    return strKbGUIDSQL;
}

bool WizIndex::modifyAttachmentInfo(WIZDOCUMENTATTACHMENTDATA& data)
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
	data.strInfoMD5 = calDocumentAttachmentInfoMD5(data);
    data.nVersion = -1;

	return modifyAttachmentInfoEx(data);
}

bool WizIndex::deleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data,
                                 bool bLog, bool bResetDocInfo, bool /*update*/)
{
    if (!deleteAttachmentEx(data)) {
        TOLOG1("Failed to delete attachment: %1", data.strName);
        return false;
    }

    if (bLog) {
        if (!logDeletedGuid(data.strGUID, wizobjectDocumentAttachment)) {
            TOLOG("Warning: Failed to log deleted attachment guid!");
        }
    }

    updateDocumentAttachmentCount(data.strDocumentGUID, bResetDocInfo);

    return true;
}

bool WizIndex::getDocumentsGuidByLocation(const CString& strLocation, CWizStdStringArray& arrayGUID)
{
    CString strSQL;
    strSQL.format("select DOCUMENT_GUID from WIZ_DOCUMENT where DOCUMENT_LOCATION like '%s%%'",
        strLocation.utf16()
        );

	return sqlToStringArray(strSQL, 0, arrayGUID);
}

bool WizIndex::deleteDocumentsTagsByLocation(const CString& strLocation)
{
	CString strSQL;
    strSQL.format("delete from WIZ_DOCUMENT_TAG where DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT where DOCUMENT_LOCATION like '%s%%')",
        strLocation.utf16()
        );
    bool bRet = execSQL(strSQL);

    if (!bRet)
        return false;

    return true;
}

bool WizIndex::deleteDocumentsByLocation(const CString& strLocation)
{
    if (!deleteDocumentsTagsByLocation(strLocation)) {
        TOLOG1("Failed to delete document tags by location: %1", strLocation);
        return false;
	}

    CWizStdStringArray arrayGUID;
    if (getDocumentsGuidByLocation(strLocation, arrayGUID)) {
        if (!logDeletedGuids(arrayGUID, wizobjectDocument)) {
            TOLOG1("Warning: Failed to log document guids by location: %1", strLocation);
		}
	}

	CString strSQL;
    strSQL.format("delete from WIZ_DOCUMENT where DOCUMENT_LOCATION like '%s%%'",
        strLocation.utf16()
        );

    return execSQL(strSQL);
}

bool WizIndex::updateDocumentInfoMD5(WIZDOCUMENTDATA& data)
{
    data.nInfoChanged = 1;
	data.nVersion = -1;
    data.tModified = WizGetCurrentTime();   //记录最后修改时间

	return modifyDocumentInfoEx(data);
}

bool WizIndex::updateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName, bool notifyDataModify /*= true*/)
{
    data.tModified = WizGetCurrentTime();

    data.strDataMD5 = ::WizMd5FileString(strZipFileName);
    data.tDataModified = WizGetCurrentTime();

    data.nDataChanged = 1;
    data.nInfoChanged = 1;
    data.nVersion = -1;

    bool bRet = modifyDocumentInfoEx(data);

    if (notifyDataModify)
    {
        Q_EMIT documentDataModified(data);
    }
    //
    return bRet;
}

bool WizIndex::deleteDocumentTags(WIZDOCUMENTDATA& data, bool bReset /* = true */)
{
    CString strFormat = formatDeleteSQLFormat(TABLE_NAME_WIZ_DOCUMENT_TAG,
                                              "DOCUMENT_GUID");

	CString strSQL;
	strSQL.format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

	if (!execSQL(strSQL))
        return false;

    bool bRet = true;

    if (bReset) {
        bRet = updateDocumentInfoMD5(data);
    }

    Q_EMIT documentTagModified(data);

    return bRet;
}

bool WizIndex::deleteDeletedGuid(const CString& strGUID)
{
    CString strFormat = formatDeleteSQLFormat(TABLE_NAME_WIZ_DELETED_GUID, "DELETED_GUID");

	CString strSQL;
	strSQL.format(strFormat,
        STR2SQL(strGUID).utf16()
        );

    return execSQL(strSQL);
}

bool WizIndex::isObjectDeleted(const CString& strGUID)
{
    CString strWhere = CString("DELETED_GUID = '%1'").arg(strGUID);
    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID, strWhere);
    CWizDeletedGUIDDataArray arrayGUID;
    sqlToDeletedGuidDataArray(strSQL, arrayGUID);
    return arrayGUID.size() > 0;
}

bool WizIndex::updateDocumentsInfoMD5(CWizDocumentDataArray& arrayDocument)
{
    CWizDocumentDataArray::iterator it;
    for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
        if (!updateDocumentInfoMD5(*it)) {
            TOLOG("Failed to update document info!");
		}
	}

    return true;
}

bool WizIndex::deleteTagDocuments(const WIZTAGDATA& data, bool bReset)
{
	CWizDocumentDataArray arrayDocument;
    getDocumentsByTag("", data, arrayDocument, true);

    CString strFormat = formatDeleteSQLFormat(TABLE_NAME_WIZ_DOCUMENT_TAG,
                                              "TAG_GUID");

	CString strSQL;
	strSQL.format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

    if (!execSQL(strSQL)) {
        TOLOG1("Failed to delete documents of tag: %1", data.strName);
        return false;
	}

    if (bReset) {
        updateDocumentsInfoMD5(arrayDocument);
    }

    return true;
}

bool WizIndex::deleteStyleDocuments(const WIZSTYLEDATA& data, bool bReset)
{
	CWizDocumentDataArray arrayDocument;
    getDocumentsByStyle("", data, arrayDocument);

	CString strSQL;
    strSQL.format("update WIZ_DOCUMENT set STYLE_GUID=null where STYLE_GUID=%s",
        STR2SQL(data.strGUID).utf16()
        );

    if (!execSQL(strSQL)) {
        TOLOG1("Failed to delete documents of style: %1", data.strName);
        return false;
	}

    if (bReset) {
        updateDocumentsInfoMD5(arrayDocument);
    }

    return true;
}

bool WizIndex::setDocumentTags(WIZDOCUMENTDATA& data,
                                const CWizStdStringArray& arrayTagGUID,
                                bool bReset /* = true */)
{
    CWizStdStringArray arrayTagOld;
    if (getDocumentTags(data.strGUID, arrayTagOld)) {
        if (WizKMStringArrayIsEqual<CString>(arrayTagOld, arrayTagGUID))
            return true;
    }

    if (!deleteDocumentTags(data, bReset)) {
        TOLOG("Failed to delete document tags");
        return false;
	}

    CWizStdStringArray::const_iterator it;
    for (it = arrayTagGUID.begin(); it != arrayTagGUID.end(); it++) {
        if (!insertDocumentTag(data, *it, bReset)) {
            TOLOG("Failed to insert document-tag record!");
            return false;
		}
	}

    return true;
}


bool WizIndex::setDocumentTags2(WIZDOCUMENTDATA& data,
                                const CWizStdStringArray& arrayTagGUID,
                                bool bReset /* = true */)
{
    CWizStdStringArray arrayTagOld;
    if (getDocumentTags(data.strGUID, arrayTagOld)) {
        if (WizKMStringArrayIsEqual<CString>(arrayTagOld, arrayTagGUID))
            return true;
    }

    //delete old tags
    {
        CString strFormat = formatDeleteSQLFormat(TABLE_NAME_WIZ_DOCUMENT_TAG,
                                                  "DOCUMENT_GUID");

        CString strSQL;
        strSQL.format(strFormat,
            STR2SQL(data.strGUID).utf16()
            );

        if (!execSQL(strSQL))
            return false;
    }

    //
    //add new tags
    {
        CWizStdStringArray::const_iterator it;
        for (it = arrayTagGUID.begin(); it != arrayTagGUID.end(); it++) {
            //
            QString strTagGUID = *it;
            if (strTagGUID.isEmpty()) {
                continue;
            }

            CString strFormat = formatInsertSQLFormat(TABLE_NAME_WIZ_DOCUMENT_TAG,
                                                      FIELD_LIST_WIZ_DOCUMENT_TAG,
                                                      PARAM_LIST_WIZ_DOCUMENT_TAG);

            CString strSQL;
            strSQL.format(strFormat,
                STR2SQL(data.strGUID).utf16(),
                STR2SQL(strTagGUID).utf16()
                );

            if (!execSQL(strSQL))
                return false;
        }
        //
    }
    //
    bool bRet = true;
    if (bReset)
        bRet = updateDocumentInfoMD5(data);

    Q_EMIT documentTagModified(data);

    return bRet;
}

bool WizIndex::getDocumentTags(const CString& strDocumentGUID, CWizStdStringArray& arrayTagGUID)
{
    CString strWhere = WizFormatString1("DOCUMENT_GUID='%1'", strDocumentGUID);
	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT_TAG, FIELD_LIST_WIZ_DOCUMENT_TAG, strWhere); 

    if (!sqlToStringArray(strSQL, 1, arrayTagGUID))
        return false;

    size_t tagCount = arrayTagGUID.size();
    if (tagCount == 0)
        return true;

    WIZDOCUMENTDATA dataDocument;
    if (!documentFromGuid(strDocumentGUID, dataDocument))
        return false;

    for (intptr_t i = tagCount - 1; i >= 0; i--)
    {
        WIZTAGDATA temp;
        if (!tagFromGuid(arrayTagGUID[i], temp))
        {
            deleteDocumentTag(dataDocument, arrayTagGUID[i]);
            arrayTagGUID.erase(arrayTagGUID.begin() + i);
            continue;
        }
    }

    return true;
}

CString WizIndex::getDocumentTagsText(const CString& strDocumentGUID)
{
    CWizStdStringArray arrayTagName;
    getDocumentTagsNameStringArray(strDocumentGUID, arrayTagName);

    CString strText;
    ::WizStringArrayToText(arrayTagName, strText, "; ");

    return strText;
}

bool WizIndex::getDocumentTagsNameStringArray(const CString& strDocumentGUID, CWizStdStringArray& arrayTagName)
{
	CWizTagDataArray arrayTag;
    if (!getDocumentTags(strDocumentGUID, arrayTag)) {
        TOLOG1("Failed to get document tags: %1", strDocumentGUID);
        return false;
	}

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
		arrayTagName.push_back(it->strName);
	}

    return true;
}

int WizIndex::getDocumentTagCount(const CString& strDocumentGUID)
{
    CString strSQL = CString("select count(TAG_GUID) from WIZ_DOCUMENT_TAG where DOCUMENT_GUID='%1'").arg(strDocumentGUID);

    CppSQLite3Query query = m_db.execQuery(strSQL);

    if (!query.eof()) {
        int nCount = query.getIntField(0);
        return nCount;
    }

    return 0;
}

CString WizIndex::getDocumentTagNameText(const CString& strDocumentGUID)
{
	CString strText;

	CWizStdStringArray arrayTagName;
    if (!getDocumentTagsNameStringArray(strDocumentGUID, arrayTagName))
		return strText;

    WizStringArrayToText(arrayTagName, strText, "; ");

	return strText;
}

bool WizIndex::getDocumentTagsDisplayNameStringArray(const CString& strDocumentGUID, CWizStdStringArray& arrayTagDisplayName)
{
    CWizTagDataArray arrayTag;
    if (!getDocumentTags(strDocumentGUID, arrayTag)) {
        TOLOG1("Failed to get document tags: %1", strDocumentGUID);
        return false;
    }

    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        arrayTagDisplayName.push_back(tagNameToDisplayName(it->strName));
    }

    return true;
}

QString WizIndex::getDocumentTagTreeDisplayString(const QString& strDocumentGUID)
{
    CWizTagDataArray arrayTag;
    if (!getDocumentTags(strDocumentGUID, arrayTag)) {
        return QString();
    }

    if (arrayTag.size() == 0)
        return "/";

    //Q_ASSERT(arrayTag.size() == 1);

    QString strTree;
    const WIZTAGDATA& tag = arrayTag.at(0);
    QString strTagGUID = tag.strGUID;
    while (1) {
        WIZTAGDATA data;
        if (!tagFromGuid(strTagGUID, data))
            return "/";

        strTree += ("/" + data.strName);

        if (data.strParentGUID.isEmpty())
            break;

        strTagGUID = data.strParentGUID;
    }

    return strTree + "/";
}

CString WizIndex::getDocumentTagDisplayNameText(const CString& strDocumentGUID)
{
    CString strText;

    CWizStdStringArray arrayTagDisplayName;
    if (!getDocumentTagsDisplayNameStringArray(strDocumentGUID, arrayTagDisplayName))
        return strText;

    WizStringArrayToText(arrayTagDisplayName, strText, "; ");

    return strText;
}

CString WizIndex::getDocumentTagGuidsString(const CString& strDocumentGUID)
{
	CWizStdStringArray arrayTagGUID;
    if (!getDocumentTags(strDocumentGUID, arrayTagGUID))
		return CString();

	std::sort(arrayTagGUID.begin(), arrayTagGUID.end());

	CString strText;
    WizStringArrayToText(arrayTagGUID, strText, ";");
	return strText;
}

bool WizIndex::setDocumentTags(WIZDOCUMENTDATA& data, const CWizTagDataArray& arrayTag)
{
	CWizTagDataArray arrayTagOld;
    if (getDocumentTags(data.strGUID, arrayTagOld)) {        
        if (WizKMObjectArrayIsEqual<WIZTAGDATA>(arrayTagOld, arrayTag))
            return true;
	}

    if (!deleteDocumentTags(data)) {
        TOLOG("Failed to delete document tags!");
        return false;
    }

    CWizTagDataArray::const_iterator it;
    for (it  = arrayTag.begin(); it != arrayTag.end(); it++) {
        if (!insertDocumentTag(data, it->strGUID)) {
            TOLOG("Failed to insert document tag record!");
            return false;
		}
	}

    return true;
}

bool WizIndex::setDocumentTagsText(WIZDOCUMENTDATA& data, const CString& strTagsText)
{
	CWizTagDataArray arrayTag;
    if (!tagsTextToTagArray(strTagsText, arrayTag)) {
        TOLOG1("Failed to convert tags text to tag array, Tags text: %1", strTagsText);
        return false;
	}

    if (!setDocumentTags(data, arrayTag)) {
        TOLOG2("Failed to set document tags, Document: %1, Tags text: %2", data.strTitle, strTagsText);
        return false;
	}

    return true;
}

bool WizIndex::insertDocumentTag(WIZDOCUMENTDATA& data,
                                  const CString& strTagGUID,
                                  bool bReset /* = true */)
{
    if (strTagGUID.isEmpty()) {
        TOLOG("NULL Pointer or empty tag guid: insert into wiz_document_tag,!");
        return false;
	}

    CString strFormat = formatInsertSQLFormat(TABLE_NAME_WIZ_DOCUMENT_TAG,
                                              FIELD_LIST_WIZ_DOCUMENT_TAG,
                                              PARAM_LIST_WIZ_DOCUMENT_TAG);

	CString strSQL;
	strSQL.format(strFormat,
        STR2SQL(data.strGUID).utf16(),
        STR2SQL(strTagGUID).utf16()
        );

    if (!execSQL(strSQL))
        return false;

    bool bRet = true;
    if (bReset)
        bRet = updateDocumentInfoMD5(data);

    Q_EMIT documentTagModified(data);

    return bRet;
}

bool WizIndex::deleteDocumentTag(WIZDOCUMENTDATA& data, const CString& strTagGUID)
{
    if (strTagGUID.isEmpty()) {
        TOLOG("NULL Pointer or empty tag guid: delete from wiz_document_tag,!");
        return false;
	}

	CString strWhere;
    strWhere.format("DOCUMENT_GUID=%s and TAG_GUID=%s",
        STR2SQL(data.strGUID).utf16(),
        STR2SQL(strTagGUID).utf16()
		);

    CString strSQL = WizFormatString1("delete from WIZ_DOCUMENT_TAG where %1", strWhere);

    bool bRet = execSQL(strSQL)
		&& updateDocumentInfoMD5(data);
	if (!bRet)
        return false;

    Q_EMIT documentTagModified(data);

    return true;
}


CString WizIndex::getMetaDef(const CString& strMetaName, const CString& strKey, const CString& strDef /*= NULL*/)
{
	CString str;
    getMeta(strMetaName, strKey, str, strDef);
	return str;
}

bool WizIndex::getMeta(CString strMetaName, CString strKey, CString& strValue, \
                     const CString& strDefault /*= NULL*/, bool* pbMetaExists /*= NULL*/)
{
    if (strMetaName.isEmpty()) {
        TOLOG("Meta name is empty!");
        return false;
    }

    if (strKey.isEmpty()) {
        TOLOG("Meta key is empty!");
        return false;
	}

    if (pbMetaExists) {
        *pbMetaExists = false;
	}

	strMetaName.makeUpper();
	strKey.makeUpper();

    CString strWhere = WizFormatString2("META_NAME=%1 and META_KEY=%2",
		STR2SQL(strMetaName),
		STR2SQL(strKey)
		);

	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_META, FIELD_LIST_WIZ_META, strWhere); 

	CWizMetaDataArray arrayMeta;
    if (!sqlToMetaDataArray(strSQL, arrayMeta)) {
        TOLOG1("Failed tog get meta: %1", strMetaName);
        return false;
	}

    if (arrayMeta.empty()) {
        strValue = CString(strDefault);
    } else {
        strValue = arrayMeta[0].strValue;

        if (arrayMeta.size() > 1) {
            TOLOG1("Warning: too more meta: %1", strMetaName);
		}

        if (pbMetaExists) {
            *pbMetaExists = true;
		}
	}

    return true;
}

bool WizIndex::setMeta(CString strMetaName, CString strKey, const CString& strValue)
{
    if (strMetaName.isEmpty()) {
        TOLOG("Meta name is empty!");
        return false;
    }

    if (strKey.isEmpty()) {
        TOLOG("Meta key is empty!");
        return false;
	}

	CString strOldValue;
    bool bMetaExists = false;
    getMeta(strMetaName, strKey, strOldValue, "", &bMetaExists);

    if (strOldValue == strValue)
        return true;

	strMetaName.makeUpper();
	strKey.makeUpper();

	CString strSQL;
    if (bMetaExists) {
        strSQL.format("update WIZ_META set META_VALUE=%s where META_NAME=%s and META_KEY=%s",
            STR2SQL(strValue).utf16(),
            STR2SQL(strMetaName).utf16(),
            STR2SQL(strKey).utf16()
			);
    } else {
		CString strFormat = formatInsertSQLFormat(TABLE_NAME_WIZ_META, FIELD_LIST_WIZ_META, PARAM_LIST_WIZ_META);

		strSQL.format(strFormat,
            STR2SQL(strMetaName).utf16(),
            STR2SQL(strKey).utf16(),
            STR2SQL(strValue).utf16(),
            TIME2SQL(WizGetCurrentTime()).utf16()
			);
	}

    if (!execSQL(strSQL)) {
        TOLOG3("Failed to update meta, meta name= %1, meta key = %2, meta value = %3", strMetaName, strKey, strValue);
        return false;
	}

	WIZMETADATA data;
	data.strName = strMetaName;
	data.strKey = strKey;
    data.strValue = strValue;
	data.tModified = WizGetCurrentTime();

    return true;
}

bool WizIndex::getAllDocumentsTitle(CWizStdStringArray& arrayDocument)
{
    CString strSQL("select DOCUMENT_TITLE from WIZ_DOCUMENT");

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
		return logSQLException(e, strSQL);
	}
}

bool WizIndex::getDocumentsNoTag(CWizDocumentDataArray& arrayDocument, bool includeTrash /* = false */)
{
    CString strWhere;
    if (includeTrash) {
        strWhere = "DOCUMENT_GUID not in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_TAG)";
    } else {
        strWhere.format("DOCUMENT_GUID not in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_TAG) and DOCUMENT_LOCATION not like %s",
                        STR2SQL(m_strDeletedItemsLocation + "%").utf16()
                        );
    }

    QString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere);

    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getLastestDocuments(CWizDocumentDataArray& arrayDocument, int nMax)
{
    CString strExt;
    strExt.format("where DOCUMENT_LOCATION not like %s order by DT_MODIFIED desc limit 0, %s",
                  STR2SQL(m_strDeletedItemsLocation + "%").utf16(),
                  WizIntToStr(nMax).utf16());
    QString strSQL = formatCanonicSQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strExt);
    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getDocumentsByTag(const CString& strLocation,
                                  const WIZTAGDATA& data,
                                  CWizDocumentDataArray& arrayDocument,
                                  bool includeTrash)
{
	CString strWhere;
    if (!strLocation.isEmpty()) {
        strWhere.format("DOCUMENT_LOCATION like %s and DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID=%s)",
                        STR2SQL(CString(strLocation) + '%').utf16(),
                        STR2SQL(data.strGUID).utf16()
                        );
    } else {
        if (includeTrash) {
            strWhere.format("DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID=%s)",
                            STR2SQL(data.strGUID).utf16()
                            );
        } else {
            strWhere.format("DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID=%s) and DOCUMENT_LOCATION not like %s",
                            STR2SQL(data.strGUID).utf16(),
                            STR2SQL(m_strDeletedItemsLocation + "%").utf16()
                            );
        }
	}

	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 

    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getDocumentsSizeByTag(const WIZTAGDATA& data, int& size)
{
    CString strWhere;
    strWhere.format("DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID=%s) and DOCUMENT_LOCATION not like %s",
                    STR2SQL(data.strGUID).utf16(),
                    STR2SQL(m_strDeletedItemsLocation + "%").utf16()
                    );

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, "COUNT(*)", strWhere);

    return sqlToSize(strSQL, size);
}

bool WizIndex::getAllDocumentsSizeByTag(const WIZTAGDATA& data, int& size)
{
    getDocumentsSizeByTag(data, size);

    CWizTagDataArray arrayTag;
    getAllChildTags(data.strGUID, arrayTag);

    int nSizeCurrent;
    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
        const WIZTAGDATA& tag = *it;
        getDocumentsSizeByTag(tag, nSizeCurrent);
        size += nSizeCurrent;
    }

    return true;
}

bool WizIndex::getDocumentsByStyle(const CString& strLocation, const WIZSTYLEDATA& data, CWizDocumentDataArray& arrayDocument)
{
	CString strWhere;
    if (strLocation && *strLocation) {
        strWhere.format("DOCUMENT_LOCATION like %s and STYLE_GUID=%s",
            STR2SQL(CString(strLocation) + '%').utf16(),
            STR2SQL(data.strGUID).utf16()
			);
    } else {
        strWhere.format("STYLE_GUID=%s",
            STR2SQL(data.strGUID).utf16()
			);
	}

	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 

    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getDocumentsByTags(bool bAnd, const CString& strLocation,
                                const CWizTagDataArray& arrayTag,
                                CWizDocumentDataArray& arrayDocument)
{
	if (arrayTag.empty())
        return true;

	if (arrayTag.size() == 1)
        return getDocumentsByTag(strLocation, arrayTag[0], arrayDocument, true);

    CString strWhere;
    CWizTagDataArray::const_iterator it;
    for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
		CString strLine;

        strLine.format("DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID=%s)",
            STR2SQL(it->strGUID).utf16()
			);

        if (strWhere.isEmpty()) {
			strWhere = strLine;
        } else {
            if (bAnd) {
                strWhere = strWhere + " and " + strLine;
            } else {
                strWhere = strWhere + " or " + strLine;
			}
		}
	}

    if (strLocation && *strLocation) {
        CString strLocation2(strLocation);
        strLocation2.appendChar('%');

        strWhere = WizFormatString2(" (%1) and DOCUMENT_LOCATION like %2", strWhere, STR2SQL(strLocation2));
	}

	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 

    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getDocumentsCountByLocation(const CString& strLocation,
                                           int& count,
                                           bool bIncludeSubFolders /* = false */)
{
    CString strWhere;
    if (bIncludeSubFolders) {
        strWhere.format("DOCUMENT_LOCATION like %s",
                        STR2SQL(strLocation + "%").utf16());
    } else {
        strWhere.format("DOCUMENT_LOCATION like %s",
                        STR2SQL(strLocation).utf16());
    }

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, "COUNT(*)", strWhere);
    return sqlToSize(strSQL, count);
}

bool WizIndex::getDocumentsByLocation(const CString& strLocation,
                                       CWizDocumentDataArray& arrayDocument,
                                       bool bIncludeSubFolders /* = false */)
{
    CString strWhere;
    if (bIncludeSubFolders) {
        strWhere.format("DOCUMENT_LOCATION like %s",
                        STR2SQL(strLocation + "%").utf16());
    } else {
        strWhere.format("DOCUMENT_LOCATION like %s",
                        STR2SQL(strLocation).utf16());
    }

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere);
    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getDocumentsByTitle(const QString& title, CWizDocumentDataArray& arrayDocument)
{
    CString strWhere;
    strWhere.format("DOCUMENT_TITLE like %s",
                    STR2SQL("%" + title + "%").utf16());

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere);
    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

//bool CWizIndex::GetDocumentsByLocationIncludeSubFolders(const CString& strLocation, CWizDocumentDataArray& arrayDocument)
//{
//	CString strWhere;
//	strWhere.Format("DOCUMENT_LOCATION like %s",
//        STR2SQL(strLocation + "%").utf16()
//		);

//	CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere);

//	return SQLToDocumentDataArray(strSQL, arrayDocument);
//}

bool WizIndex::getDocumentsByGuids(const CWizStdStringArray& arrayGUID, CWizDocumentDataArray& arrayDocument)
{
    CWizStdStringArray arrayGUID2;
    CWizStdStringArray::const_iterator it;
    for (it = arrayGUID.begin(); it != arrayGUID.end(); it++) {
		arrayGUID2.push_back(STR2SQL(*it));
	}

	CString strText;
    WizStringArrayToText(arrayGUID2, strText, ",");

    CString strWhere = WizFormatString1("DOCUMENT_GUID in (%1)", strText);

	return getDocumentsBySQLWhere(strWhere, arrayDocument);
}

bool WizIndex::documentFromLocationAndName(const CString& strLocation, const CString& strName, WIZDOCUMENTDATA& data)
{
    if (!strLocation || !*strLocation) {
        TOLOG("Location is empty");
        return false;
    }

    if (!strName || !*strName) {
        TOLOG("Name is empty");
        return false;
	}

	CString strWhere;
    strWhere.format("DOCUMENT_LOCATION like %s AND DOCUMENT_NAME like %s",
        STR2SQL(strLocation).utf16(),
        STR2SQL(strName).utf16()
		);

	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 

	CWizDocumentDataArray arrayDocument;
    if (!sqlToDocumentDataArray(strSQL, arrayDocument)) {
        TOLOG("Failed to get document by guid");
        return false;
	}

    if (arrayDocument.empty()) {
        //TOLOG("Failed to get document by guid, result is empty");
        return false;
    } else if (arrayDocument.size() > 1) {
        CWizDocumentDataArray::const_iterator it;
        for (it = arrayDocument.begin(); it != arrayDocument.end(); it++) {
            TOLOG(WizFormatString5("Warning: Too more file name %1 in %2, Title=%3, FileName=%4, GUID=%5", strName, strLocation, it->strTitle, it->strName, it->strGUID));
		}
	}

	data = arrayDocument[0];
    return true;
}

bool WizIndex::changeDocumentsLocation(const CString& strOldLocation, const CString& strNewLocation)
{
	CString strSQL;
    strSQL.format("update %s set DOCUMENT_LOCATION=%s where DOCUMENT_LOCATION=%s",
        STR2SQL(TABLE_NAME_WIZ_DOCUMENT).utf16(),
        STR2SQL(strNewLocation).utf16(),
        STR2SQL(strOldLocation).utf16()
		);

	return execSQL(strSQL);
}

#ifndef WIZ_NO_OBSOLETE
bool WizIndex::filterDocumentsInDeletedItems(CWizDocumentDataArray& arrayDocument)
{
	if (m_strDeletedItemsLocation.isEmpty())
        return false;

	size_t nCount = arrayDocument.size();
	for (intptr_t i = nCount - 1; i >= 0; i--)
	{
		if (0 == _tcsnicmp(arrayDocument[i].strLocation, m_strDeletedItemsLocation, m_strDeletedItemsLocation.getLength()))
		{
			arrayDocument.erase(arrayDocument.begin() + i);
		}
	}

    return true;
}
#endif

bool WizIndex::getTagsByTime(const WizOleDateTime& t, CWizTagDataArray& arrayData)
{
	CString strSQL = formatQuerySQLByTime(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, FIELD_MODIFIED_WIZ_TAG, t); 
	return sqlToTagDataArray(strSQL, arrayData);
}

bool WizIndex::getStylesByTime(const WizOleDateTime& t, CWizStyleDataArray& arrayData)
{
	CString strSQL = formatQuerySQLByTime(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE, FIELD_MODIFIED_WIZ_STYLE, t); 
	return sqlToStyleDataArray(strSQL, arrayData);
}

bool WizIndex::getDocumentsByTime(const QDateTime& t, CWizDocumentDataArray& arrayData)
{
	CString strSQL = formatQuerySQLByTime3(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, 
		FIELD_INFO_MODIFIED_WIZ_DOCUMENT, 
		FIELD_DATA_MODIFIED_WIZ_DOCUMENT, 
		FIELD_PARAM_MODIFIED_WIZ_DOCUMENT, 
		t); 
    return sqlToDocumentDataArray(strSQL, arrayData);
}

bool WizIndex::getAttachmentsByTime(const WizOleDateTime& t, CWizDocumentAttachmentDataArray& arrayData)
{
	CString strSQL = formatQuerySQLByTime2(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT, 
		FIELD_INFO_MODIFIED_WIZ_DOCUMENT_ATTACHMENT, 
		FIELD_DATA_MODIFIED_WIZ_DOCUMENT_ATTACHMENT, 
		t); 
	return sqlToDocumentAttachmentDataArray(strSQL, arrayData);
}

bool WizIndex::getDeletedGuidsByTime(const WizOleDateTime& t, CWizDeletedGUIDDataArray& arrayData)
{
	CString strSQL = formatQuerySQLByTime(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID, FIELD_MODIFIED_DELETED_GUID, t); 
	return sqlToDeletedGuidDataArray(strSQL, arrayData);
}

#ifndef WIZ_NO_OBSOLETE

bool WizIndex::isDocumentModified()
{
	CWizStdStringArray arrayLocation;
	getLocationsNeedToBeSync(arrayLocation);
	if (arrayLocation.empty())
        return false;

	CString strWhere2 = getLocationArraySQLWhere(arrayLocation);

    CString strSQLDocument = WizFormatString3("select %1 from %2 where WIZ_VERSION = -1 and %3 limit 0, 1",
		FIELD_LIST_WIZ_DOCUMENT, 
		TABLE_NAME_WIZ_DOCUMENT,
		strWhere2);

	DEBUG_TOLOG(strSQLDocument);
	if (hasRecord(strSQLDocument))
        return true;

    return false;
}

bool WizIndex::isModified()
{
	if (isDocumentModified())
        return true;
	//
	//CString strSQLDeletedGUID = FormatModifiedQuerySQL2(TABLE_NAME_WIZ_DELETED_GUID, FIELD_LIST_WIZ_DELETED_GUID, 1); 
	//if (HasRecord(strSQLDeletedGUID))
    //	return true;
	//
	CString strSQLTag = formatModifiedQuerySQL2(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, 1); 
	if (hasRecord(strSQLTag))
        return true;
	//
	CString strSQLStyle = formatModifiedQuerySQL2(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE, 1); 
	if (hasRecord(strSQLStyle))
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

bool WizIndex::getModifiedTags(CWizTagDataArray& arrayData)
{
	CString strSQL = formatModifiedQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG); 
	return sqlToTagDataArray(strSQL, arrayData);
}

bool WizIndex::getModifiedStyles(CWizStyleDataArray& arrayData)
{
	CString strSQL = formatModifiedQuerySQL(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE); 
	return sqlToStyleDataArray(strSQL, arrayData);
}

bool WizIndex::getModifiedParams(CWizDocumentParamDataArray& arrayData)
{
    CString strSQL = formatModifiedQuerySQL(TABLE_NAME_WIZ_DOCUMENT_PARAM, FIELD_LIST_WIZ_DOCUMENT_PARAM);
    return sqlToDocumentParamDataArray(strSQL, arrayData);
}


bool WizIndex::getModifiedDocuments(CWizDocumentDataArray& arrayData)
{
	CString strSQL = formatModifiedQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT);
    return sqlToDocumentDataArray(strSQL, arrayData);
}


bool WizIndex::deleteDocumentParams(const QString& strDocumentGUID)
{
    CString strSQL = WizFormatString2("update %1 set PARAM_VALUE=null, WIZ_VERSION=-1 where DOCUMENT_GUID='%2'",
                                      TABLE_NAME_WIZ_DOCUMENT_PARAM,
                                      strDocumentGUID);
    if (!execSQL(strSQL))
        return false;

    return true;
}

bool WizIndex::setDocumentParam(const QString& strDocumentGUID, const QString& strParamName, const QString& strParamValue)
{
    WIZDOCUMENTPARAMDATA data;
    data.strKbGUID = kbGUID();
    data.strDocumentGuid = strDocumentGUID;
    data.strName = strParamName;
    data.strValue = strParamValue;
    data.nVersion = -1;
    //
    return updateDocumentParam(data);
}

bool WizIndex::setDocumentParams(const QString& strDocumentGuid, const CWizDocumentParamDataArray& arrayParam)
{
    deleteDocumentParams(strDocumentGuid);

    CWizDocumentParamDataArray::const_iterator it;
    for (it = arrayParam.begin(); it != arrayParam.end(); it++) {
        if (!setDocumentParam(strDocumentGuid, it->strName, it->strValue)) {
            TOLOG2("Failed to set document param: %1=%2", it->strName, it->strValue);
        }
    }
    //
    return true;
}

bool WizIndex::modifyDocumentParamVersion(const QString& strDocumentGUID, const QString& strParamName, __int64 version)
{
    QString sql = WizFormatString4("update %1 set wiz_version=%2 where DOCUMENT_GUID='%3' and PARAM_NAME=%4",
                                   TABLE_NAME_WIZ_DOCUMENT_PARAM,
                                   WizInt64ToStr(version),
                                   strDocumentGUID,
                                   STR2SQL(strParamName));
    return execSQL(sql);
}


CString WizIndex::getLocationArraySQLWhere(const CWizStdStringArray& arrayLocation)
{
    CString strWhere;
    CWizStdStringArray::const_iterator it;
    for (it = arrayLocation.begin(); it != arrayLocation.end(); it++) {
        if (strWhere.isEmpty()) {
            strWhere = WizFormatString1(" DOCUMENT_LOCATION like %1 ", STR2SQL(*it + "%"));
        } else {
            strWhere = strWhere + WizFormatString1(" or DOCUMENT_LOCATION like %1 ", STR2SQL(*it + "%"));
        }
    }

    return CString(" (") + strWhere + ")";
}

bool WizIndex::getModifiedDocuments(const CWizStdStringArray& arrayLocation, CWizDocumentDataArray& arrayData)
{
    if (arrayLocation.empty())
        return true;

    CString strSQL = formatModifiedQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT);

    CString strWhere2 = getLocationArraySQLWhere(arrayLocation);

    strSQL = strSQL + " and " + strWhere2;

    return sqlToDocumentDataArray(strSQL, arrayData);
}

bool WizIndex::getModifiedAttachments(CWizDocumentAttachmentDataArray& arrayData)
{
    CString strSQL = formatModifiedQuerySQL(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT);
    return sqlToDocumentAttachmentDataArray(strSQL, arrayData);
}

bool WizIndex::getModifiedAttachments(const CWizStdStringArray& arrayLocation, CWizDocumentAttachmentDataArray& arrayData)
{
    CString strSQL = formatModifiedQuerySQL(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT);

    CString strWhere2 = getLocationArraySQLWhere(arrayLocation);

    strSQL = strSQL + WizFormatString1(" and DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT where %1)", strWhere2);

    return sqlToDocumentAttachmentDataArray(strSQL, arrayData);
}

bool WizIndex::objectExists(const QString& strGUID, const QString& strType, bool& bExists)
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
    if (!sqlToStringArray(strSQL, 0, arrayGUID)) {
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

bool WizIndex::getObjectTableInfo(const CString& strType, CString& strTableName, CString& strKeyFieldName)
{
    if (0 == strType.compareNoCase("tag")) {
		strTableName = TABLE_NAME_WIZ_TAG;
		strKeyFieldName = TABLE_KEY_WIZ_TAG;

    } else if (0 == strType.compareNoCase("style")) {
		strTableName = TABLE_NAME_WIZ_STYLE;
		strKeyFieldName = TABLE_KEY_WIZ_STYLE;

    } else if (0 == strType.compareNoCase("document")) {
		strTableName = TABLE_NAME_WIZ_DOCUMENT;
		strKeyFieldName = TABLE_KEY_WIZ_DOCUMENT;

    } else if (0 == strType.compareNoCase("attachment")) {
		strTableName = TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT;
        strKeyFieldName = TABLE_KEY_WIZ_DOCUMENT_ATTACHMENT;

    } else if (0 == strType.compareNoCase("message")) {
        strTableName = TABLE_NAME_WIZ_MESSAGE;
        strKeyFieldName = TABLE_KEY_WIZ_MESSAGE;

    } else {
        Q_ASSERT(0);
        TOLOG1("Unknown object type: %1", strType);
        return false;
	}

    return true;
}

qint64 WizIndex::getObjectLocalVersion(const QString& strGUID, const QString& strType)
{
    bool objectExists = false;
    return getObjectLocalVersionEx(strGUID, strType, objectExists);
}

qint64 WizIndex::getObjectLocalVersionEx(const QString& strGUID, const QString& strType, bool& bObjectExists)
{
    CString strTableName;
    CString strKeyFieldName;

    if (!getObjectTableInfo(strType, strTableName, strKeyFieldName))
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

bool WizIndex::modifyObjectVersion(const CString& strGUID, const CString& strType, qint64 nVersion)
{
    if (-1 == nVersion)
    {
        qDebug() << "modify object version (-1), type: " << strType << " guid: " << strGUID;
        Q_ASSERT("Should not modify object version to -1");
        return false;
    }

	CString strTableName;
	CString strKeyFieldName;

    if (!getObjectTableInfo(strType, strTableName, strKeyFieldName))
        return false;
    //
    if (strType.toLower() == "document")
    {
        CString strSQL = WizFormatString4("update %1 set WIZ_VERSION=%2, INFO_CHANGED=0, DATA_CHANGED=0 where %3=%4",
            strTableName,
            WizInt64ToStr(nVersion),
            strKeyFieldName,
            STR2SQL(strGUID));

        return execSQL(strSQL);
    }
    else
    {
        //
        CString strSQL = WizFormatString4("update %1 set WIZ_VERSION=%2 where %3=%4",
            strTableName,
            WizInt64ToStr(nVersion),
            strKeyFieldName,
            STR2SQL(strGUID));

        return execSQL(strSQL);
    }
}

bool WizIndex::isObjectDataModified(const CString& strGUID, const CString& strType)
{
    qint64 nVersion = getObjectLocalVersion(strGUID, strType);
    return  -1 == nVersion;
}

bool WizIndex::modifyObjectModifiedTime(const CString& strGUID, const CString& strType, const WizOleDateTime& t)
{
	CString strTableName;
	CString strKeyFieldName;

    if (!getObjectTableInfo(strType, strTableName, strKeyFieldName))
        return false;

	CString strTimeFieldName;
    if (0 == strType.compareNoCase("document")
        || 0 == strType.compareNoCase("attachment"))
	{
        strTimeFieldName = "DT_INFO_MODIFIED";
	}
	else
	{
        strTimeFieldName = "DT_MODIFIED";
	}

    CString strSQL = WizFormatString5("update %1 set %2=%3 where %4=%5",
		strTableName,
		strTimeFieldName,
		TIME2SQL(t),
		strKeyFieldName, 
        STR2SQL(strGUID));

	return execSQL(strSQL);
}

bool WizIndex::getObjectModifiedTime(const CString& strGUID, const CString& strType, WizOleDateTime& t)
{
	CString strTableName;
	CString strKeyFieldName;

    if (!getObjectTableInfo(strType, strTableName, strKeyFieldName))
        return false;

	CString strTimeFieldName;
    if (0 == strType.compareNoCase("document")
        || 0 == strType.compareNoCase("attachment"))
	{
        strTimeFieldName = "DT_INFO_MODIFIED";
	}
	else
	{
        strTimeFieldName = "DT_MODIFIED";
	}

    CString strSQL = WizFormatString4("select %1 from %2 where %3=%4",
		strTimeFieldName,
		strTableName,
		strKeyFieldName, 
        STR2SQL(strGUID));

	CString strTime;
	if (!getFirstRowFieldValue(strSQL, 0, strTime))
        return false;

	t = WizStringToDateTime(strTime);

    return true;
}

bool WizIndex::searchDocumentByTitle(const QString& strTitle,
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

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere);

    if (!sqlToDocumentDataArray(strSQL, arrayDocument))
        return false;

    if (arrayDocument.size() > nMaxCount) {
        arrayDocument.resize(nMaxCount);
    }

    return true;
}

CString URLToSQL(const CString& strURL)
{
	if (strURL.isEmpty())
		return strURL;

    CString strWhere = WizFormatString1("DOCUMENT_URL like %1", STR2SQL_LIKE_BOTH(strURL));
	return strWhere;
}

CString AttachmentNameToSQL(const CString& strName)
{
	if (strName.isEmpty())
		return strName;

    CString strWhere = WizFormatString1("DOCUMENT_GUID in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_ATTACHMENT where ATTACHMENT_NAME like %1)", STR2SQL_LIKE_BOTH(strName));
	return strWhere;
}

CString HasAttachmentToSQL(int nAttachment)
{
	if (-1 == nAttachment)
		return CString();
	//
	if (nAttachment)
	{
        CString strWhere("DOCUMENT_GUID in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_ATTACHMENT)");
		return strWhere;
	}
	else
	{
        CString strWhere("DOCUMENT_GUID not in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_ATTACHMENT)");
		return strWhere;
	}
}

CString TagArrayToSQL(WizIndex& index, const CWizStdStringArray& arrayTagName)
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
		if (strTagName.isEmpty())
			continue;
		//
		CWizTagDataArray arrayTag;
		index.tagArrayByName(strTagName, arrayTag);
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
        CString strWhere = WizFormatString1("DOCUMENT_GUID in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_TAG where TAG_GUID ='%1')", it->strGUID);
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
        ::WizStringArrayToText(arrayWhere, strWhere, " or ");
        strWhere = WizFormatString1("( %1 )", strWhere);
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
        CString strWhere = WizFormatString1("DOCUMENT_FILE_TYPE like %1", STR2SQL_LIKE_BOTH(*it));
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
        ::WizStringArrayToText(arrayWhere, strWhere, " or ");
        strWhere = WizFormatString1("( %1 )", strWhere);
		return strWhere;
	}
}

CString LocationToSQL(const CString& strLocation, bool bIncludeSubFolders)
{
	if (strLocation.isEmpty())
		return strLocation;
	//
	CString strWhereLocation;
	if (bIncludeSubFolders)
	{
        strWhereLocation = " DOCUMENT_LOCATION like " + STR2SQL_LIKE_RIGHT(strLocation);
	}
	else
	{
        strWhereLocation = " DOCUMENT_LOCATION like " + STR2SQL(strLocation);
	}
	//
	return strWhereLocation;
}

CString WizFormatDateInt(int n)
{
	if (n < 10)
        return CString("0") + WizIntToStr(n);
	return WizIntToStr(n);
}

bool WizDateRegularDateString(CString& str)
{
	CWizStdStringArray arrayText;
	::WizSplitTextToArray(str, '-', arrayText);
	if (arrayText.size() != 3)
        return false;
	//
    arrayText[1] = ::WizFormatDateInt(wiz_ttoi(arrayText[1]));
    arrayText[2] = ::WizFormatDateInt(wiz_ttoi(arrayText[2]));
	//
    str = arrayText[0] + "-" + arrayText[1] + "-" + arrayText[2];
    return true;
}

bool WizDateStringToDateTimeOfBegin(CString str, WizOleDateTime& t)
{
	WizDateRegularDateString(str);
    str += " 00:00:00";
	//
	CString strError;
	if (WizStringToDateTime(str, t, strError))
        return true;
	//
    //WizMessageBox(strError);
	//
    return false;
}

bool WizDateStringToDateTimeOfEnd(CString str, WizOleDateTime& t)
{
	WizDateRegularDateString(str);
    str += " 23:59:59";
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
	if (strValue.isEmpty())
		return CString();
	//
    if (!strFieldName || !*strFieldName)
		return CString();
	//
    QChar charOpera = strValue[0];
	if (charOpera == '>')
	{
        strValue.remove(0, 1);
		//
		WizOleDateTime t;
		if (!WizDateStringToDateTimeOfBegin(strValue, t))
			return CString();
		//
        return WizFormatString2("%1 >= '%2'", strFieldName, ::WizDateTimeToString(t));
	}
	else if (charOpera == '<')
	{
        strValue.remove(0, 1);
		//
		WizOleDateTime t;
		if (!WizDateStringToDateTimeOfEnd(strValue, t))
			return CString();
		//
        return WizFormatString2("%1 <= '%2'", strFieldName, ::WizDateTimeToString(t));
	}
	else if (charOpera == '=')
	{
        strValue.remove(0, 1);
	}
	else
	{
		CString strLeft;
		CString strRight;
		if (::WizStringSimpleSplit(strValue, '&', strLeft, strRight))
		{
			WizOleDateTime t1;
			if (!WizDateStringToDateTimeOfBegin(strLeft, t1))
				return CString();
			//
			WizOleDateTime t2;
			if (!WizDateStringToDateTimeOfEnd(strRight, t2))
				return CString();
			//
            return WizFormatString4("(%1 >= '%2' and %3 <= '%4')", strFieldName, ::WizDateTimeToString(t1), strFieldName, ::WizDateTimeToString(t2));
		}
	}
	//
	WizOleDateTime t1;
	if (!WizDateStringToDateTimeOfBegin(strValue, t1))
		return CString();
	//
	WizOleDateTime t2;
	if (!WizDateStringToDateTimeOfEnd(strValue, t2))
		return CString();
	//
    return WizFormatString4("(%1 >= '%2' and %3 <= '%4')", strFieldName, ::WizDateTimeToString(t1), strFieldName, ::WizDateTimeToString(t2));
}

CString TitleToSQL(WizIndex& index, const CString& strKeywords, const WIZSEARCHDATA& data)
{
    bool bAddExtra = false;
	CString strTitle(data.strTitle);
	if (strTitle.isEmpty())
	{
        strTitle = strKeywords;
        bAddExtra = true;
	}
	//
	if (strTitle.isEmpty())
		return CString();
	//
	//
	CWizStdStringArray arrayWhere;
	
	if (bAddExtra)
	{
        arrayWhere.push_back(WizFormatString1("(DOCUMENT_TITLE like %1)", STR2SQL_LIKE_BOTH(strTitle)));
        arrayWhere.push_back(WizFormatString1("(DOCUMENT_NAME like %1)", STR2SQL_LIKE_BOTH(strTitle)));
        arrayWhere.push_back(WizFormatString1("(DOCUMENT_SEO like %1)", STR2SQL_LIKE_BOTH(strTitle)));
        arrayWhere.push_back(WizFormatString1("(DOCUMENT_URL like %1)", STR2SQL_LIKE_BOTH(strTitle)));
        arrayWhere.push_back(WizFormatString1("(DOCUMENT_AUTHOR like %1)", STR2SQL_LIKE_BOTH(strTitle)));
        arrayWhere.push_back(WizFormatString1("(DOCUMENT_KEYWORDS like %1)", STR2SQL_LIKE_BOTH(strTitle)));
		if (data.strAttachmentName.isEmpty())
		{
            arrayWhere.push_back(WizFormatString1("(%1)", AttachmentNameToSQL(strTitle)));
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
			if (!strTagSQL.isEmpty())
			{
                arrayWhere.push_back(WizFormatString1("(%1)", strTagSQL));
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
			if (!strFileTypeSQL.isEmpty())
			{
                arrayWhere.push_back(WizFormatString1("(%1)", strFileTypeSQL));
			}
		}
		CString strWhere;
        ::WizStringArrayToText(arrayWhere, strWhere, " or ");
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
			str.trim();
			if (str.isEmpty())
				continue;
            arrayWhere.push_back(WizFormatString1("(DOCUMENT_TITLE like %1)", STR2SQL_LIKE_BOTH(str)));
		}
		//
		CString strWhere;
        ::WizStringArrayToText(arrayWhere, strWhere, " and ");
		//
		return strWhere;

	}
}

bool WizIndex::search(const CString& strKeywords,
                       const WIZSEARCHDATA& data,
                       const CString& strLocation,
                       bool bIncludeSubFolders,
                       size_t nMaxCount,
                       CWizDocumentDataArray& arrayDocument)
{
	if (!data.strSyntax.isEmpty())
        return true;
	//
	CWizStdStringArray arrayWhere;
	//
	if (!data.strSQL.isEmpty())
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
        arrayWhere.push_back(DateToSQL("DT_CREATED", data.strDateCreated));
        arrayWhere.push_back(DateToSQL("DT_DATA_MODIFIED", data.strDateModified));
        arrayWhere.push_back(DateToSQL("DT_ACCESSED", data.strDateAccessed));
	}
	//
    arrayWhere.push_back(LocationToSQL(CString(strLocation), bIncludeSubFolders));
	//
	::WizStringArrayEraseEmptyLine(arrayWhere);
	if (arrayWhere.empty())
        return true;
	//
	CString strWhere;
    ::WizStringArrayToText(arrayWhere, strWhere, ") and (");
    strWhere = WizFormatString1("( %1 )", strWhere);
	//
    strWhere += WizFormatString1(" order by DT_CREATED desc limit 0, %1", WizIntToStr(int(nMaxCount)));
	//
	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 
	//
	DEBUG_TOLOG(strSQL);
	//
    return sqlToDocumentDataArray(strSQL, arrayDocument);

}

bool WizIndex::search(const CString& strKeywords,
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
    arrayField.push_back("DOCUMENT_TITLE");
    arrayField.push_back("DOCUMENT_NAME");
    arrayField.push_back("DOCUMENT_SEO");
    arrayField.push_back("DOCUMENT_URL");
    arrayField.push_back("DOCUMENT_AUTHOR");
    arrayField.push_back("DOCUMENT_KEYWORDS");
    arrayField.push_back("DOCUMENT_OWNER");
    arrayField.push_back("DOCUMENT_AUTHOR");
	//
	CString strFormat;
	//
	for (CWizStdStringArray::const_iterator it =  arrayField.begin();
		it != arrayField.end();
		it++)
	{
		if (strFormat.isEmpty())
		{
            strFormat = CString("(") + *it + " like '%%1%'" + ")";
		}
		else
		{
            strFormat = strFormat + " OR " + CString("(") + *it + " like '%%1%'" + ")";
		}
	}
	//
    strFormat = "(" + strFormat + ")";
	//
	CString strWhere;
	//
	for (CWizStdStringArray::const_iterator it = arrayKeywords.begin();
		it != arrayKeywords.end();
		it++)
	{
		CString strLine = WizFormatString1(strFormat, *it);
		if (strWhere.isEmpty())
		{
			strWhere = strLine;
		}
		else
		{
            strWhere = strWhere + " AND " + strLine;
		}
	}
	//
	if (!strLocation.isEmpty())
	{
		CString strWhereLocation;
		if (bIncludeSubFolders)
		{
            strWhereLocation = " AND DOCUMENT_LOCATION like " + STR2SQL(WizFormatString1("%%1%", strLocation));
		}
		else
		{
            strWhereLocation = " AND DOCUMENT_LOCATION like " + STR2SQL(WizFormatString1("%%1", strLocation));
		}
		//
		strWhere += strWhereLocation;
	}
	//
	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 
	//
    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getRecentDocuments(long nFlags, const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument)
{
	CString strSQL;
	//
    CString strTimeField = (0 == nFlags) ? "DT_CREATED, DT_DATA_MODIFIED" : "DT_CREATED, DT_MODIFIED, DT_INFO_MODIFIED, DT_DATA_MODIFIED";
	//
	if (strDocumentType.isEmpty())
	{
        strSQL.format("select %s, MAX(%s) AS WIZ_TEMP_MODIFIED from %s where DOCUMENT_LOCATION not like %s order by WIZ_TEMP_MODIFIED desc limit 0, %d",
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            strTimeField.utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(m_strDeletedItemsLocation + "%").utf16(),
			nCount
			);
	}
	else
	{
        strSQL.format("select %s, MAX(%s) AS WIZ_TEMP_MODIFIED from %s where DOCUMENT_TYPE=%s and DOCUMENT_LOCATION not like %s order by WIZ_TEMP_MODIFIED desc limit 0,%d",
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            strTimeField.utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(strDocumentType).utf16(),
            STR2SQL(m_strDeletedItemsLocation + "%").utf16(),
			nCount
			);
	}
	//
    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getRecentDocumentsCreated(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument)
{
	CString strSQL;
	//
	if (strDocumentType.isEmpty())
	{
        strSQL.format("select %s from %s where DOCUMENT_LOCATION not like %s order by DT_CREATED desc limit 0, %d",
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(m_strDeletedItemsLocation + "%").utf16(),
			nCount
			);
	}
	else
	{
        strSQL.format("select %s from %s where DOCUMENT_TYPE=%s and DOCUMENT_LOCATION not like %s order by DT_CREATED desc limit 0,%d",
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(strDocumentType).utf16(),
            STR2SQL(m_strDeletedItemsLocation + "%").utf16(),
			nCount
			);
	}
	//
    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getRecentDocumentsModified(const CString& strDocumentType, int nCount, CWizDocumentDataArray& arrayDocument)
{
    CString strSQL;
	//
	if (strDocumentType.isEmpty())
	{
        strSQL.format("select %s from %s where DOCUMENT_LOCATION not like %s and datetime(DT_CREATED, '+1 minute') < DT_DATA_MODIFIED order by DT_DATA_MODIFIED desc limit 0, %d",
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(m_strDeletedItemsLocation + "%").utf16(),
			nCount
			);
	}
	else
	{
        strSQL.format("select %s from %s where DOCUMENT_TYPE=%s and DOCUMENT_LOCATION not like %s and datetime(DT_CREATED, '+1 minute') < DT_DATA_MODIFIED order by DT_DATA_MODIFIED desc limit 0,%d",
            QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
            QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
            STR2SQL(strDocumentType).utf16(),
            STR2SQL(m_strDeletedItemsLocation + "%").utf16(),
			nCount
			);
	}
	//
    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getRecentDocumentsByCreatedTime(const WizOleDateTime& t, CWizDocumentDataArray& arrayDocument)
{
    CString strTime = TIME2SQL(t);

    CString strSQL;
    strSQL.format("select %s from %s where DOCUMENT_LOCATION not like '/Deleted Items/%%' and DT_CREATED>=%s order by DT_CREATED desc",
        QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
        QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
        strTime.utf16()
        );

    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getRecentDocumentsByModifiedTime(const WizOleDateTime& t, CWizDocumentDataArray& arrayDocument)
{
    CString strTime = TIME2SQL(t);

    CString strSQL;
    strSQL.format("select %s from %s where DOCUMENT_LOCATION not like '/Deleted Items/%%' and DT_MODIFIED>=%s order by DT_MODIFIED desc",
        QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
        QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
        strTime.utf16()
        );

    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndex::getRecentDocumentsByAccessedTime(const WizOleDateTime& t, CWizDocumentDataArray& arrayDocument)
{
    CString strTime = TIME2SQL(t);

    CString strSQL;
    strSQL.format("select %s from %s where DOCUMENT_LOCATION not like '/Deleted Items/%%' and DT_ACCESSED>=%s order by DT_ACCESSED desc",
        QString(FIELD_LIST_WIZ_DOCUMENT).utf16(),
        QString(TABLE_NAME_WIZ_DOCUMENT).utf16(),
        strTime.utf16()
        );

    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

#ifndef WIZ_NO_OBSOLETE
bool WizIndex::getCalendarEvents(const WizOleDateTime& tStart, const WizOleDateTime& tEnd, CWizDocumentDataArray& arrayDocument)
{
	CString strParamNameStart(DOCUMENT_PARAM_NAME_CALENDAR_EVENT_START);
	CString strParamNameEnd(DOCUMENT_PARAM_NAME_CALENDAR_EVENT_END);
	//
	strParamNameStart.makeUpper();
	strParamNameEnd.makeUpper();
	//
	CString strWhere;

	strWhere.format(_T("DOCUMENT_LOCATION not like %s and\
		((DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE>=%s) \
		and DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE<=%s)) \
		or \
		(DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE>=%s) \
		and DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE<=%s)) \
		or \
		(DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE<%s) \
		and DOCUMENT_GUID in (select DOCUMENT_GUID from %s where PARAM_NAME='%s' and PARAM_VALUE>%s))) \
		"),
        STR2SQL(m_strDeletedItemsLocation + "%").utf16(),
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
	return getDocumentsBySQLWhere(strWhere, arrayDocument);
}

bool WizIndex::getAllRecurrenceCalendarEvents(CWizDocumentDataArray& arrayDocument)
{
	CString strParamName(DOCUMENT_PARAM_NAME_CALENDAR_EVENT_RECURRENCE);
	strParamName.makeUpper();

	CString strWhere;
    strWhere.format("DOCUMENT_LOCATION not like %s and DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_PARAM where PARAM_NAME=%s)",
        STR2SQL(m_strDeletedItemsLocation + "%").utf16(),
        STR2SQL(strParamName).utf16()
		);
	//
	//
	CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere); 
	//
	return sqlToDocumentDataArray(strSQL, arrayDocument);
}
#endif

bool WizIndex::getLocations(CWizStdStringArray& arrayLocation)
{
    CString strSQL("select distinct DOCUMENT_LOCATION from WIZ_DOCUMENT");
	return sqlToStringArray(strSQL, 0, arrayLocation);
}

CString WizIndex::getRootLocationName(CString strLocation)
{
    strLocation.trim('/');
	int nIndex = strLocation.find('/');
	if (-1 == nIndex)
		return strLocation;
	else
		return strLocation.left(nIndex);
}

bool WizIndex::isRootLocation(CString strLocation)
{
	strLocation.trim('/');
	return -1 == strLocation.find('/');
}

bool WizIndex::getLocationsNeedToBeSync(CWizStdStringArray& arrayLocation)
{
	if (!getLocations(arrayLocation))
        return false;
	//
	for (intptr_t i = arrayLocation.size() - 1; i >= 0; i--)
	{
		CString strLocation = arrayLocation[i];
		if (!isRootLocation(strLocation)
			|| !getSync(strLocation))
		{
			arrayLocation.erase(arrayLocation.begin() + i);
			continue;
		}
	}
	//
    return true;
}

bool WizIndex::getLocations(CDocumentLocationArray& arrayLocation)
{
    CString strSQL("select DOCUMENT_LOCATION, count(*) as DOCUMENT_COUNT from WIZ_DOCUMENT group by DOCUMENT_LOCATION order by DOCUMENT_LOCATION");
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
		return logSQLException(e, strSQL);
	}
    return true;
}

qint64 WizIndex::getMetaInt64(const CString& strMetaName, const CString& strKey, qint64 nDef)
{
	CString str;
    getMeta(strMetaName, strKey, str);
	if (str.isEmpty())
		return nDef;

    return wiz_ttoi64(str);
}

bool WizIndex::setMetaInt64(const CString& strMetaName, const CString& strKey, qint64 n)
{
    return setMeta(strMetaName, strKey, WizInt64ToStr(n));
}

bool WizIndex::deleteMetasByName(const QString& strMetaName)
{
    CString strFormat = formatDeleteSQLFormat(TABLE_NAME_WIZ_META, "META_NAME");

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(strMetaName).utf16()
        );

    if (!execSQL(strSQL))
        return false;

    return true;
}

bool WizIndex::deleteMetaByKey(const QString& strMetaName, const QString& strMetaKey)
{
    CString strWhere = QString("META_NAME='%1' AND META_KEY='%2'").arg(strMetaName).arg(strMetaKey);
    CString strSQL = formatDeleteSQLByWhere(TABLE_NAME_WIZ_META, strWhere);

    if (!execSQL(strSQL))
        return false;

    return true;
}

int WizIndex::getDocumentAttachmentCount(const CString& strDocumentGUID)
{
    CString strSQL;
    strSQL.format("select count(*) from WIZ_DOCUMENT_ATTACHMENT where DOCUMENT_GUID=%s",
                  STR2SQL(strDocumentGUID).utf16());

    CppSQLite3Query query = m_db.execQuery(strSQL);

    if (!query.eof()) {
        int nCount = query.getIntField(0);
        return nCount;
    }

    return 0;
}

void WizIndex::updateDocumentAttachmentCount(const CString& strDocumentGUID,
                                              bool bResetDocInfo /* = true */)
{
    if (bResetDocInfo)
    {
        WIZDOCUMENTDATA data;
        if (!documentFromGuid(strDocumentGUID, data))
            return;

        data.nAttachmentCount = getDocumentAttachmentCount(strDocumentGUID);

        modifyDocumentInfo(data, bResetDocInfo);
    }
    else
    {
        int nAttachmentCount = getDocumentAttachmentCount(strDocumentGUID);
        QString sql = WizFormatString2("update WIZ_DOCUMENT set DOCUMENT_ATTACHEMENT_COUNT=%1 where DOCUMENT_GUID='%2'", WizIntToStr(nAttachmentCount), strDocumentGUID);
        execSQL(sql);
    }
}

bool WizIndex::getSync(const CString& strLocation)
{
    CString strRootLocationName = getRootLocationName(strLocation);
	
    return getMetaDef("Sync", strRootLocationName, "1") == "1";
}

bool WizIndex::setSync(const CString& strLocation, bool bSync)
{
    CString strRootLocationName = getRootLocationName(strLocation);

    return setMeta("Sync", strRootLocationName, bSync ? "1" : "0");
}

bool WizIndex::getDocumentsNoTagCount(int& nSize, bool includeTrash /* = false */)
{
    CString strWhere;
    if (includeTrash) {
        strWhere = "DOCUMENT_GUID not in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_TAG)";
    } else {
        strWhere.format("DOCUMENT_GUID not in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT_TAG) and DOCUMENT_LOCATION not like %s",
                        STR2SQL(m_strDeletedItemsLocation + "%").utf16()
                        );
    }

    QString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, "COUNT(*)", strWhere);

    return sqlToSize(strSQL, nSize);
}


bool WizIndex::getAllTagsDocumentCount(std::map<CString, int>& mapTagDocumentCount)
{
	CString strSQL;
    strSQL.format("select TAG_GUID, count(*) from WIZ_DOCUMENT_TAG where DOCUMENT_GUID in (select distinct DOCUMENT_GUID from WIZ_DOCUMENT where DOCUMENT_LOCATION not like '/Deleted Items/%%') group by TAG_GUID"
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
		return logSQLException(e, strSQL);
	}
    return true;
}

bool WizIndex::getAllLocationsDocumentCount(std::map<CString, int>& mapLocationDocumentCount)
{
	CString strSQL;
    strSQL.format("select DOCUMENT_LOCATION, count(*) as DOCUMENT_COUNT from WIZ_DOCUMENT group by DOCUMENT_LOCATION");
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
		return logSQLException(e, strSQL);
	}

    return true;
}

int WizIndex::getTrashDocumentCount()
{
    int nTotal = 0;

    CString strSQL;
    strSQL.format("select count(*) as DOCUMENT_COUNT from WIZ_DOCUMENT where DOCUMENT_LOCATION like '/Deleted Items/%'");
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
        return logSQLException(e, strSQL);
    }

    return nTotal;
}

bool WizIndex::getAllDocumentsOwners(CWizStdStringArray& arrayOwners)
{
    CString strSQL;
    strSQL.format("select distinct DOCUMENT_OWNER from WIZ_DOCUMENT");

    return sqlToStringArray(strSQL, 0, arrayOwners);
}


#ifndef WIZ_NO_OBSOLETE
bool WizIndex::getLocationDocumentCount(CString strLocation, bool bIncludeSubFolders, int& nDocumentCount)
{
	nDocumentCount = 0;
	//
	if (bIncludeSubFolders)
        strLocation.append("%");
	//
	CString strSQL;
	//
    strSQL.format("SELECT count(*) as DOCUMENT_COUNT from WIZ_DOCUMENT where DOCUMENT_LOCATION like %s",
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
		return logSQLException(e, strSQL);
	}
	//
    return true;
}
#endif

bool WizIndex::setDocumentVersion(const CString& strDocumentGUID, qint64 nVersion)
{
    if (nVersion == -1)
    {
        qDebug() << "modify document version (-1), guid: " << strDocumentGUID;
    }
    //
    CString strSQL = WizFormatString2("update WIZ_DOCUMENT set WIZ_VERSION=%1 where DOCUMENT_GUID=%2",
		WizInt64ToStr(nVersion),
        STR2SQL(strDocumentGUID));

	if (!execSQL(strSQL))
        return false;

    return true;
}

bool WizIndex::setThumbIndexVersion(const QString& strVersion)
{
    return setMeta("ThumbIndex", "Version", strVersion);
}

QString WizIndex::getThumIndexVersion()
{
    return getMetaDef("ThumbIndex", "Version");
}

bool WizIndex::setDocumentFTSVersion(const QString& strVersion)
{
    return setMeta("FTS", "Version", strVersion);
}

QString WizIndex::getDocumentFTSVersion()
{
    return getMetaDef("FTS", "Version");
}

bool WizIndex::setDocumentFTSEnabled(bool b)
{
    return setMeta("FTS", "Enabled", b ? "1" : "0");
}

bool WizIndex::isDocumentFTSEnabled()
{
    QString str = getMetaDef("FTS", "Enabled");
    if (str == "1" || str.isEmpty())
        return true;

    return false;
}

bool WizIndex::setAllDocumentsSearchIndexed(bool b)
{
    CString strSQL = WizFormatString1("update WIZ_DOCUMENT set DOCUMENT_INDEXED=%1",
        b ? "1" : "0");

	if (!execSQL(strSQL))
        return false;

    return true;
}

bool WizIndex::setDocumentSearchIndexed(const QString& strDocumentGUID, bool b)
{
    CString strSQL = WizFormatString2("update WIZ_DOCUMENT set DOCUMENT_INDEXED=%1 where DOCUMENT_GUID=%2",
        b ? "1" : "0",
        STR2SQL(strDocumentGUID));

	if (!execSQL(strSQL))
        return false;

    return true;
}

bool WizIndex::searchDocumentByWhere(const QString& strWhere, int nMaxCount, CWizDocumentDataArray& arrayDocument)
{
    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strWhere);

    if (!sqlToDocumentDataArray(strSQL, arrayDocument))
        return false;

    if (arrayDocument.size() > nMaxCount) {
        arrayDocument.resize(nMaxCount);
    }

    return true;
}

bool WizIndex::getAllDocumentsNeedToBeSearchIndexed(CWizDocumentDataArray& arrayDocument)
{
    CString strWhere = QString("DOCUMENT_INDEXED=0 and DOCUMENT_GUID in \
                               (select DISTINCT OBJECT_GUID from WIZ_OBJECT_EX where %1=1)").arg(getReservedIntFieldName(DATA_DOWNLOADED_FIELD));

    if (!getDocumentsBySQLWhere(strWhere, arrayDocument)) {
        TOLOG("Failed to get documents by DOCUMENT_INDEXED=0");
        return false;
    }

    return true;
}

bool WizIndex::getAllParentsTagGuid(CString strTagGUID, CWizStdStringArray& arrayGUID)
{
	int nIndex = 0;
	while (1)
	{
		WIZTAGDATA data;
		if (!tagFromGuid(strTagGUID, data))
            return false;

		if (data.strParentGUID.isEmpty())
            return true;

		arrayGUID.push_back(data.strParentGUID);

		strTagGUID = data.strParentGUID;

		nIndex++;

        if (nIndex > 100) {
            TOLOG("Tags data error!");
            return false;
		}
	}
}

bool WizIndex::getAllParentsTagGuid(CString strTagGUID, std::set<CString>& setGUID)
{
	CWizStdStringArray arrayGUID;
    bool bRet = getAllParentsTagGuid(strTagGUID, arrayGUID);

    CWizStdStringArray::const_iterator it;
    for (it = arrayGUID.begin(); it != arrayGUID.end(); it++) {
		setGUID.insert(*it);
	}

	return bRet;
}

bool WizIndex::isLocationEmpty(const CString& strLocation)
{
    CString strSQL = WizFormatString1("select DOCUMENT_LOCATION from WIZ_DOCUMENT where DOCUMENT_LOCATION  like %1 limit 0, 1",
        STR2SQL_LIKE_RIGHT(strLocation));

	return !hasRecord(strSQL);
}

bool WizIndex::getAllLocations(CWizStdStringArray& arrayLocation)
{
    QString strSQL = "select distinct DOCUMENT_LOCATION from WIZ_DOCUMENT";
	return sqlToStringArray(strSQL, 0, arrayLocation);
}

void WizIndex::getAllLocationsWithExtra(CWizStdStringArray& arrayLocation)
{
    getAllLocations(arrayLocation);

    CWizStdStringArray arrayExtra;
    getExtraFolder(arrayExtra);
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

bool WizIndex::getAllChildLocations(const CString& strLocation, CWizStdStringArray& arrayLocation)
{
    CString strSQL = WizFormatString1("select distinct DOCUMENT_LOCATION from WIZ_DOCUMENT where DOCUMENT_LOCATION like %1",
                                      STR2SQL_LIKE_RIGHT(strLocation));

    return sqlToStringArray(strSQL, 0, arrayLocation);
}

bool WizIndex::objectInReserved(const CString& strGUID, const CString& strType)
{
    CString strSQL;
    strSQL = WizFormatString2("select OBJECT_GUID from WIZ_OBJECT_EX where OBJECT_GUID=%1 and OBJECT_TYPE=%2",
                  STR2SQL(strGUID),
                  STR2SQL(strType));

    return hasRecord(strSQL);
}

CString WizIndex::getReservedIntFieldName(WizIndex::WizObjectReservedInt e)
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

CString WizIndex::getReservedStrFieldName(WizObjectReservedStr e)
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

bool WizIndex::setObjectReservedInt(const CString& strGUID, const CString& strType, WizIndex::WizObjectReservedInt e, int val)
{
    CString strSQL;
    if (objectInReserved(strGUID, strType))
    {
        strSQL = WizFormatString4("update WIZ_OBJECT_EX set %1=%2 where OBJECT_GUID=%3 and OBJECT_TYPE=%4",
                                  getReservedIntFieldName(e),
                                  WizIntToStr(val),
                                  STR2SQL(strGUID),
                                  STR2SQL(strType));

    }
    else
    {
        strSQL = WizFormatString4("insert into WIZ_OBJECT_EX (OBJECT_GUID, OBJECT_TYPE, %1) values(%2, %3, %4)",
                                  getReservedIntFieldName(e),
                                  STR2SQL(strGUID),
                                  STR2SQL(strType),
                                  WizIntToStr(val));

    }
    //
    return execSQL(strSQL);
}

bool WizIndex::getObjectReservedInt(const CString& strGUID, const CString& strType, WizObjectReservedInt e, int& val)
{
    if (!objectInReserved(strGUID, strType))
        return false;
    //
    CString strSQL;
    strSQL = WizFormatString3("select %1 from WIZ_OBJECT_EX where OBJECT_GUID=%2 and OBJECT_TYPE=%3",
                            getReservedIntFieldName(e),
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
        return logSQLException(e, strSQL);
    }
}

//
bool WizIndex::setObjectReservedStr(const CString& strGUID, const CString& strType, WizObjectReservedStr e, const CString& val)
{
    CString strSQL;
    if (objectInReserved(strGUID, strType))
    {
        strSQL = WizFormatString4("update WIZ_OBJECT_EX set %1=%2 where OBJECT_GUID=%3 and OBJECT_TYPE=%4",
                                  getReservedStrFieldName(e),
                                  STR2SQL(val),
                                  STR2SQL(strGUID),
                                  STR2SQL(strType));

    }
    else
    {
        strSQL = WizFormatString4("insert into WIZ_OBJECT_EX (OBJECT_GUID, OBJECT_TYPE, %1) values(%2, %3, %4)",
                                  getReservedStrFieldName(e),
                                  STR2SQL(strGUID),
                                  STR2SQL(strType),
                                  STR2SQL(val));

    }
    //
    return execSQL(strSQL);
}

bool WizIndex::getObjectReservedStr(const CString& strGUID, const CString& strType, WizObjectReservedStr e, CString& val)
{
    if (!objectInReserved(strGUID, strType))
        return false;
    //
    CString strSQL;
    strSQL = WizFormatString3("select %1 from WIZ_OBJECT_EX where OBJECT_GUID=%2 and OBJECT_TYPE=%3",
                            getReservedStrFieldName(e),
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
        return logSQLException(e, strSQL);
    }
}

bool WizIndex::setDocumentDataDownloaded(const CString& strGUID, bool bDownloaded)
{
    return setObjectDataDownloaded(strGUID, "document", bDownloaded);
}

bool WizIndex::setAttachmentDataDownloaded(const CString& strGUID, bool bDownloaded)
{
    return setObjectDataDownloaded(strGUID, "attachment", bDownloaded);
}

bool WizIndex::setObjectDataDownloaded(const CString& strGUID, const CString& strType, bool bDownloaded)
{
    return setObjectReservedInt(strGUID, strType, DATA_DOWNLOADED_FIELD, bDownloaded ? 1 : 0);
}

bool WizIndex::isObjectDataDownloaded(const CString& strGUID, const CString& strType)
{
    int nDownloaded = 0;
    getObjectReservedInt(strGUID, strType, DATA_DOWNLOADED_FIELD, nDownloaded);
    return nDownloaded ? true : false;
}

int WizIndex::getNeedToBeDownloadedDocumentCount()
{
    CString strSQL = WizFormatString1("select count(*) from WIZ_DOCUMENT where DOCUMENT_GUID not in (select OBJECT_GUID from WIZ_OBJECT_EX where OBJECT_TYPE='document' and %1=1)",
                                      getReservedIntFieldName(DATA_DOWNLOADED_FIELD));

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
        logSQLException(e, strSQL);
        return 0;
    }
}

int WizIndex::getNeedToBeDownloadedAttachmentCount()
{
    CString strSQL = WizFormatString1("select count(*) from WIZ_DOCUMENT_ATTACHMENT where ATTACHMENT_GUID not in (select OBJECT_GUID from WIZ_OBJECT_EX where OBJECT_TYPE='attachment' and %1=1)",
                                      getReservedIntFieldName(DATA_DOWNLOADED_FIELD));
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
        logSQLException(e, strSQL);
        return 0;
    }
}

bool WizIndex::getNeedToBeDownloadedDocuments(CWizDocumentDataArray& arrayData)
{
    CString strSQL = WizFormatString2("select %1 from WIZ_DOCUMENT where DOCUMENT_GUID not in (select OBJECT_GUID from WIZ_OBJECT_EX where OBJECT_TYPE='document' and %2=1) order by DT_DATA_MODIFIED desc",
                                      FIELD_LIST_WIZ_DOCUMENT,
                                      getReservedIntFieldName(DATA_DOWNLOADED_FIELD));
    //
    CWizDocumentDataArray arrayDocument;
    sqlToDocumentDataArray(strSQL, arrayDocument);
    if (arrayDocument.empty())
        return false;
    //
    arrayData.assign(arrayDocument.begin(), arrayDocument.end());
    return true;
}
bool WizIndex::getNeedToBeDownloadedAttachments(CWizDocumentAttachmentDataArray& arrayData)
{
    CString strSQL = WizFormatString2("select %1 from WIZ_DOCUMENT_ATTACHMENT where ATTACHMENT_GUID not in (select OBJECT_GUID from WIZ_OBJECT_EX where OBJECT_TYPE='attachment' and %2=1) order by DT_DATA_MODIFIED desc",
                                      FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT,
                                      getReservedIntFieldName(DATA_DOWNLOADED_FIELD));
    //
    CWizDocumentAttachmentDataArray arrayAttachment;
    sqlToDocumentAttachmentDataArray(strSQL, arrayAttachment);
    if (arrayAttachment.empty())
        return false;
    //
    arrayData.assign(arrayAttachment.begin(), arrayAttachment.end());
    return true;
}

CString WizIndex::tagDisplayNameToName(const CString& strDisplayName)
{
    CString strName(strDisplayName);
	return strName;
}

CString WizIndex::tagNameToDisplayName(const CString& strName)
{
    CString strDisplayName(strName);

    return strDisplayName;
}

void WizIndex::getExtraFolder(CWizStdStringArray& arrayLocation)
{
    CString str = getMetaDef("FOLDERS", "EXTRA");
    ::WizSplitTextToArray(str, '\\', arrayLocation);
}

void WizIndex::setExtraFolder(const CWizStdStringArray& arrayLocation)
{
    CString strText;
    ::WizStringArrayToText(arrayLocation, strText, "\\");
    setMeta("FOLDERS", "EXTRA", strText);
}

void WizIndex::addExtraFolder(const QString& strLocation)
{
    CWizStdStringArray arrayLocation;
    getExtraFolder(arrayLocation);
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

    setExtraFolder(arrayLocation);
}

void WizIndex::deleteExtraFolder(const QString& strLocation)
{
    CWizStdStringArray arrayLocation;
    getExtraFolder(arrayLocation);

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

    setExtraFolder(arrayLocation);
}

/**
 * @brief CWizIndex::UpdateLocation     just modify document location, do not change document modify date
 * @param strOldLocation
 * @param strNewLocation
 * @return
 */
bool WizIndex::updateLocation(const QString& strOldLocation, const QString& strNewLocation)
{
//    QString sql = QString("update %1 set DOCUMENT_LOCATION='%2' where "
//                          "DOCUMENT_LOCATION='%3'").arg(TABLE_NAME_WIZ_DOCUMENT)
//                          .arg(strNewLocation).arg(strOldLocation);
//    bool result = ExecSQL(sql);
    qDebug() << "update location from : " << strOldLocation << " to : " << strNewLocation;

    CWizDocumentDataArray docArray;
    if (!getDocumentsByLocation(strOldLocation, docArray, true))
        return false;

    //update all include document location
    for (CWizDocumentDataArray::const_iterator it = docArray.begin();
         it != docArray.end();
         it++)
    {
        WIZDOCUMENTDATA doc = *it;
        doc.strLocation.replace(strOldLocation, strNewLocation);
        doc.nInfoChanged = 1;
        doc.nVersion = -1;
        modifyDocumentInfoEx(doc);
    }

    CWizStdStringArray arrayExtra;
    getExtraFolder(arrayExtra);
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
    setExtraFolder(newArray);

    return true;
}
