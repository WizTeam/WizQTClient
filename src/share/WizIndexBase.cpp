#include "WizIndexBase.h"
#include "WizDef.h"

#include <QDebug>

#include "utils/WizLogger.h"
#include "utils/WizPathResolve.h"

#include "share/WizThreads.h"

WizIndexBase::WizIndexBase(void)
    : m_bUpdating(false)
{
    qRegisterMetaType<WIZTAGDATA>("WIZTAGDATA");
    qRegisterMetaType<WIZSTYLEDATA>("WIZSTYLEDATA");
    qRegisterMetaType<WIZDOCUMENTDATA>("WIZDOCUMENTDATA");
    qRegisterMetaType<WIZDOCUMENTATTACHMENTDATA>("WIZDOCUMENTATTACHMENTDATA");

    qRegisterMetaType<WIZMESSAGEDATA>("WIZMESSAGEDATA");
    qRegisterMetaType<WIZBIZUSER>("WIZBIZUSER");
}

WizIndexBase::~WizIndexBase(void)
{
    close();
}

bool WizIndexBase::open(const CString& strFileName)
{
    m_strFileName = strFileName;
    //m_strDatabasePath = WizExtractFilePath(strFileName);

    try {
        m_db.open(strFileName);
        //
        for (int i = 0; i < TABLE_COUNT; i++) {
            if (!checkTable(g_arrayTableName[i]))
                return false;
        }
        // upgrade table structure if table structure have been changed
        if (m_db.tableExists(TABLE_NAME_WIZ_META)) {
            int nVersion = getTableStructureVersion().toInt();
            if (nVersion < QString(WIZ_TABLE_STRUCTURE_VERSION).toInt()) {
                updateTableStructure(nVersion);
                setTableStructureVersion(WIZ_TABLE_STRUCTURE_VERSION);
            }
        }
    } catch (const CppSQLite3Exception& e) {
        return logSQLException(e, "open database");
    }


    return true;
}

bool WizIndexBase::isOpened()
{
    return m_db.isOpened();
}

void WizIndexBase::close()
{
    m_db.close();
}

bool WizIndexBase::checkTable(const QString& strTableName)
{
    if (m_db.tableExists(strTableName))
        return true;

    // create table if not exist
    CString strFileName = WizPathAddBackslash2(Utils::WizPathResolve::resourcesPath() + "sql") + strTableName.toLower() + ".sql";
    CString strSQL;
    if (!WizLoadUnicodeTextFromFile(strFileName, strSQL))
        return false;

    bool result = execSQL(strSQL);
    return result;
}

bool WizIndexBase::execSQL(const CString& strSQL)
{
    try {
        m_db.execDML(strSQL);
        return true;
    } catch (const CppSQLite3Exception& e) {
        return logSQLException(e, strSQL);
    }
}

CppSQLite3Query WizIndexBase::Query(const CString& strSQL)
{
    return m_db.execQuery(strSQL);
}

int WizIndexBase::exec(const CString& strSQL)
{
    return m_db.execDML(strSQL);
}

bool WizIndexBase::hasRecord(const CString& strSQL)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        return !query.eof();
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::getFirstRowFieldValue(const CString& strSQL, int nFieldIndex, CString& strValue)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        if (!query.eof()) {
            strValue = query.getStringField(nFieldIndex);
            return true;
        } else {
            return false;
        }
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::logSQLException(const CppSQLite3Exception& e, const CString& strSQL)
{
    TOLOG(e.errorMessage());
    TOLOG(strSQL);
    return false;
}

bool WizIndexBase::repair(const QString& strDestFileName)
{
    return CppSQLite3DB::repair(m_strFileName, strDestFileName) ? true : false;
}

bool WizIndexBase::updateTableStructure(int oldVersion)
{
    qDebug() << "table structure version : " << oldVersion << "  update to version " << WIZ_TABLE_STRUCTURE_VERSION;
    if (oldVersion < 1) {
        if (!m_db.columnExists("WIZ_TAG", "TAG_POS")) {
            exec("ALTER TABLE 'WIZ_TAG' ADD 'TAG_POS' int64; ");
        }
    }
    //
    if (oldVersion < 2) {
        if (!m_db.columnExists("WIZ_MESSAGE", "DELETE_STATUS")) {
            exec("ALTER TABLE 'WIZ_MESSAGE' ADD 'DELETE_STATUS' int;");
        }
        if (!m_db.columnExists("WIZ_MESSAGE", "LOCAL_CHANGED")) {
            exec("ALTER TABLE 'WIZ_MESSAGE' ADD 'LOCAL_CHANGED' int;");
        }
    }

    if (oldVersion < 3) {
        if (!m_db.columnExists("WIZ_MESSAGE", "MESSAGE_NOTE")) {
            exec("ALTER TABLE 'WIZ_MESSAGE' ADD 'MESSAGE_NOTE' varchar(2048);");
        }
    }
    //
    if (oldVersion < 4) {
        if (!m_db.columnExists("WIZ_DOCUMENT", "INFO_CHANGED")) {
            exec("ALTER TABLE 'WIZ_DOCUMENT' ADD 'INFO_CHANGED' int default 1;");
        }
        if (!m_db.columnExists("WIZ_DOCUMENT", "DATA_CHANGED")) {
            exec("ALTER TABLE 'WIZ_DOCUMENT' ADD 'DATA_CHANGED' int default 1;");
        }
    }
    //
    if (oldVersion < 5) {
        if (!m_db.columnExists("WIZ_DOCUMENT_PARAM", "WIZ_VERSION")) {
            exec("ALTER TABLE 'WIZ_DOCUMENT_PARAM' ADD 'WIZ_VERSION' int default -1;");
        }
    }
    //
    return true;
}

CString WizIndexBase::formatCanonicSQL(const CString& strTableName,
                                        const CString& strFieldList,
                                        const CString& strExt)
{
    return WizFormatString3("select %1 from %2 %3",
                            strFieldList,
                            strTableName,
                            strExt);
}

CString WizIndexBase::formatQuerySQL(const CString& strTableName,
                                      const CString& strFieldList)
{
    return WizFormatString2("select %1 from %2",
                            strFieldList,
                            strTableName);
}

CString WizIndexBase::formatQuerySQL(const CString& strTableName,
                                      const CString& strFieldList,
                                      const CString& strWhere)
{
    return WizFormatString3("select %1 from %2 where %3",
                            strFieldList,
                            strTableName,
                            strWhere);
}

CString WizIndexBase::formatInsertSQLFormat(const CString& strTableName,
                                             const CString& strFieldList,
                                             const CString& strParamList)
{
    return WizFormatString3("insert into %1 (%2) values (%3)",
                            strTableName,
                            strFieldList,
                            strParamList);
}

CString WizIndexBase::formatUpdateSQLFormat(const CString& strTableName,
                                             const CString& strFieldList,
                                             const CString& strKey)
{
    return WizFormatString3("update %1 set %2 where %3=%s",
                            strTableName,
                            strFieldList,
                            strKey);
}

CString WizIndexBase::formatUpdateSQLByWhere(const CString& strTableName,
                                              const CString& strFieldList,
                                              const CString& strWhere)
{
    return WizFormatString3("update %1 set %2 where %3",
                            strTableName,
                            strFieldList,
                            strWhere);
}

CString WizIndexBase::formatDeleteSQLFormat(const CString& strTableName,
                                             const CString& strKey)
{
    return WizFormatString2("delete from %1 where %2=%s",
                            strTableName,
                            strKey);
}

CString WizIndexBase::formatDeleteSQLByWhere(const CString& strTableName, const CString& strWhere)
{
    return WizFormatString2("delete from %1 where %2",
                            strTableName,
                            strWhere);
}

CString WizIndexBase::formatQuerySQLByTime(const CString& strTableName,
                                            const CString& strFieldList,
                                            const CString& strFieldName,
                                            const WizOleDateTime& t)
{
    return WizFormatString4("select %1 from %2 where %3 >= %4",
                            strFieldList,
                            strTableName,
                            strFieldName,
                            TIME2SQL(t));
}
CString WizIndexBase::formatQuerySQLByTime2(const CString& strTableName,
                                             const CString& strFieldList,
                                             const CString& strInfoFieldName,
                                             const CString& strDataFieldName,
                                             const WizOleDateTime& t)
{
    return WizFormatString5("select %1 from %2 where %3 >= %5 or %4 >= %5",
                            strFieldList,
                            strTableName,
                            strInfoFieldName,
                            strDataFieldName,
                            TIME2SQL(t));
}
CString WizIndexBase::formatQuerySQLByTime3(const CString& strTableName,
                                             const CString& strFieldList,
                                             const CString& strInfoFieldName,
                                             const CString& strDataFieldName,
                                             const CString& strParamFieldName,
                                             const QDateTime& t)
{
    return WizFormatString6("select %1 from %2 where %3 >= %6 or %4 >= %6 or %5 >= %6",
                            strFieldList,
                            strTableName,
                            strInfoFieldName,
                            strDataFieldName,
                            strParamFieldName,
                            TIME2SQL(t));
}

CString WizIndexBase::formatModifiedQuerySQL(const CString& strTableName,
                                              const CString& strFieldList)
{
    return WizFormatString2("select %1 from %2 where WIZ_VERSION = -1",
                            strFieldList,
                            strTableName);
}

CString WizIndexBase::formatModifiedQuerySQL2(const CString& strTableName,
                                               const CString& strFieldList,
                                               int nCount)
{
    return WizFormatString3("select %1 from %2 where WIZ_VERSION = -1 limit 0, %3",
                            strFieldList,
                            strTableName,
                            WizIntToStr(nCount));
}

bool WizIndexBase::sqlToSize(const CString& strSQL, int& size)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        size = query.getInt64Field(0);
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::sqlToTagDataArray(const CString& strSQL, CWizTagDataArray& arrayTag)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZTAGDATA data;
            data.strKbGUID = kbGUID();
            data.strGUID = query.getStringField(tagTAG_GUID);
            data.strParentGUID = query.getStringField(tagTAG_GROUP_GUID);
            data.strName = query.getStringField(tagTAG_NAME);
            data.strDescription = query.getStringField(tagTAG_DESCRIPTION);
            data.tModified = query.getTimeField(tagDT_MODIFIED);
            data.nVersion = query.getInt64Field(tagVersion);
            data.nPosition = query.getInt64Field(tagTAG_POS);

            arrayTag.push_back(data);
            query.nextRow();
        }

        std::sort(arrayTag.begin(), arrayTag.end());
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::sqlToStyleDataArray(const CString& strSQL, CWizStyleDataArray& arrayStyle)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZSTYLEDATA data;
            data.strKbGUID = kbGUID();
            data.strGUID = query.getStringField(styleSTYLE_GUID);
            data.strName = query.getStringField(styleSTYLE_NAME);
            data.strDescription = query.getStringField(styleSTYLE_DESCRIPTION);
            data.crTextColor = query.getColorField2(styleSTYLE_TEXT_COLOR);
            data.crBackColor = query.getColorField2(styleSTYLE_BACK_COLOR);
            data.bTextBold = query.getBoolField(styleSTYLE_TEXT_BOLD);
            data.nFlagIndex = query.getIntField(styleSTYLE_FLAG_INDEX);
            data.tModified = query.getTimeField(styleDT_MODIFIED);
            data.nVersion = query.getInt64Field(styleVersion);

            arrayStyle.push_back(data);
            query.nextRow();
        }

        std::sort(arrayStyle.begin(), arrayStyle.end());
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::sqlToMetaDataArray(const CString& strSQL, CWizMetaDataArray& arrayMeta)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZMETADATA data;
            data.strKbGUID = kbGUID();
            data.strName = query.getStringField(metaMETA_NAME);
            data.strKey = query.getStringField(metaMETA_KEY);
            data.strValue = query.getStringField(metaMETA_VALUE);
            data.tModified = query.getTimeField(metaDT_MODIFIED);

            arrayMeta.push_back(data);
            query.nextRow();
        }
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::sqlToDeletedGuidDataArray(const CString& strSQL, CWizDeletedGUIDDataArray& arrayGUID)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZDELETEDGUIDDATA data;
            data.strKbGUID = kbGUID();
            data.strGUID = query.getStringField(deletedguidDELETED_GUID);
            data.eType = WIZOBJECTDATA::intToObjectType(query.getIntField(deletedguidGUID_TYPE));
            data.tDeleted = query.getTimeField(deletedguidDT_DELETED);

            arrayGUID.push_back(data);
            query.nextRow();
        }
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::sqlToDocumentParamDataArray(const CString& strSQL,
                            CWizDocumentParamDataArray& arrayParam)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZDOCUMENTPARAMDATA data;
            data.strKbGUID = kbGUID();
            data.strDocumentGuid = query.getStringField(0);
            data.strName = query.getStringField(1);
            data.strValue = query.getStringField(2);
            data.nVersion = query.getInt64Field(3);

            arrayParam.push_back(data);
            query.nextRow();
        }
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}


bool WizIndexBase::sqlToStringArray(const CString& strSQL, int nFieldIndex, CWizStdStringArray& arrayString)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            CString strGUID = query.getStringField(nFieldIndex);

            arrayString.push_back(strGUID);
            query.nextRow();
        }
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}


bool WizIndexBase::initDocumentExFields(CWizDocumentDataArray& arrayDocument, const CWizStdStringArray& arrayGUID, const std::map<QString, int>& mapDocumentIndex)
{
    CString strDocumentGUIDs;
    ::WizStringArrayToText(arrayGUID, strDocumentGUIDs, _T("\',\'"));
    //
    CString strParamSQL = WizFormatString1(_T("select DOCUMENT_GUID, PARAM_NAME, PARAM_VALUE from WIZ_DOCUMENT_PARAM where (PARAM_NAME='DOCUMENT_FLAGS' or PARAM_NAME='RATE' or PARAM_NAME='SYSTEM_TAGS') and DOCUMENT_GUID in('%1')"), strDocumentGUIDs);
    //
    CppSQLite3Query queryParam = m_db.execQuery(strParamSQL);
    //
    while (!queryParam.eof())
    {
        CString strGUID = queryParam.getStringField(0);
        CString strParamName = queryParam.getStringField(1);
        //
        std::map<QString, int>::const_iterator it = mapDocumentIndex.find(strGUID);
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
                    if (strParamName == _T("DOCUMENT_FLAGS"))
                    {
                        int nFlags = queryParam.getIntField(2);
                        data.nFlags = nFlags;
                    }
                    else if (strParamName == _T("RATE"))
                    {
                        int nRate = queryParam.getIntField(2);
                        data.nRate = nRate;
                    }
                }
            }
        }
        //
        queryParam.nextRow();
    }
    //
    return true;
}

bool WizIndexBase::sqlToDocumentDataArray(const CString& strSQL, CWizDocumentDataArray& arrayDocument)
{
    try
    {
        CWizStdStringArray arrayGUID;
        std::map<QString, int> mapDocumentIndex;
        //
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZDOCUMENTDATA data;
            data.strKbGUID = kbGUID();
            data.strGUID = query.getStringField(documentDOCUMENT_GUID);
            data.strTitle = query.getStringField(documentDOCUMENT_TITLE);
            data.strLocation = query.getStringField(documentDOCUMENT_LOCATION);
            data.strName = query.getStringField(documentDOCUMENT_NAME);
            data.strSEO = query.getStringField(documentDOCUMENT_SEO);
            data.strURL = query.getStringField(documentDOCUMENT_URL);
            data.strAuthor = query.getStringField(documentDOCUMENT_AUTHOR);
            data.strKeywords = query.getStringField(documentDOCUMENT_KEYWORDS);
            data.strType = query.getStringField(documentDOCUMENT_TYPE);
            data.strOwner = query.getStringField(documentDOCUMENT_OWNER);
            data.strFileType = query.getStringField(documentDOCUMENT_FILE_TYPE);
            data.strStyleGUID = query.getStringField(documentSTYLE_GUID);
            data.tCreated = query.getTimeField(documentDT_CREATED);
            data.tModified = query.getTimeField(documentDT_MODIFIED);
            data.tAccessed = query.getTimeField(documentDT_ACCESSED);
            data.nProtected = query.getIntField(documentDOCUMENT_PROTECT);
            data.nReadCount = query.getIntField(documentDOCUMENT_READ_COUNT);
            data.nAttachmentCount = query.getIntField(documentDOCUMENT_ATTACHEMENT_COUNT);
            data.nIndexed = query.getIntField(documentDOCUMENT_INDEXED);
            data.tDataModified = query.getTimeField(documentDT_DATA_MODIFIED);
            data.strDataMD5 = query.getStringField(documentDOCUMENT_DATA_MD5);
            data.nVersion = query.getInt64Field(documentVersion);
            data.nInfoChanged = query.getIntField(documentINFO_CHANGED);
            data.nDataChanged = query.getIntField(documentDATA_CHANGED);
            //
            arrayGUID.push_back(data.strGUID);
            arrayDocument.push_back(data);
            //
            mapDocumentIndex[data.strGUID] = int(arrayDocument.size() - 1);
            //
            query.nextRow();
        }
        //
        if (!arrayDocument.empty())
        {
            initDocumentExFields(arrayDocument, arrayGUID, mapDocumentIndex);
        }
        //
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::sqlToDocumentAttachmentDataArray(const CString& strSQL,
                                                     CWizDocumentAttachmentDataArray& arrayAttachment)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZDOCUMENTATTACHMENTDATA data;
            data.strKbGUID = kbGUID();
            data.strGUID = query.getStringField(documentattachmentATTACHMENT_GUID);
            data.strDocumentGUID = query.getStringField(documentattachmentDOCUMENT_GUID);
            data.strName = query.getStringField(documentattachmentATTACHMENT_NAME);
            data.strURL = query.getStringField(documentattachmentATTACHMENT_URL);
            data.strDescription = query.getStringField(documentattachmentATTACHMENT_DESCRIPTION);
            data.tInfoModified = query.getTimeField(documentattachmentDT_INFO_MODIFIED);
            data.strInfoMD5 = query.getStringField(documentattachmentATTACHMENT_INFO_MD5);
            data.tDataModified = query.getTimeField(documentattachmentDT_DATA_MODIFIED);
            data.strDataMD5 = query.getStringField(documentattachmentATTACHMENT_DATA_MD5);
            data.nVersion = query.getInt64Field(documentattachmentVersion);

            arrayAttachment.push_back(data);
            query.nextRow();
        }

        std::sort(arrayAttachment.begin(), arrayAttachment.end());
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::sqlToMessageDataArray(const QString& strSQL,
                                          CWizMessageDataArray& arrayMessage)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZMESSAGEDATA data;
            data.nId = query.getInt64Field(msgMESSAGE_ID);
            data.bizGUID = query.getStringField(msgBIZ_GUID);
            data.kbGUID = query.getStringField(msgKB_GUID);
            data.documentGUID = query.getStringField(msgDOCUMENT_GUID);
            data.senderAlias = query.getStringField(msgSENDER);
            data.senderId = query.getStringField(msgSENDER_ID);
            data.senderGUID = query.getStringField(msgSENDER_GUID);
            data.receiverAlias = query.getStringField(msgRECEIVER);
            data.receiverId = query.getStringField(msgRECEIVER_ID);
            data.receiverGUID = query.getStringField(msgRECEIVER_GUID);
            data.nMessageType = query.getIntField(msgMESSAGE_TYPE);
            data.nReadStatus = query.getIntField(msgREAD_STATUS);
            data.tCreated = query.getTimeField(msgDT_CREATED);
            data.title = query.getStringField(msgMESSAGE_TITLE);
            data.messageBody = query.getStringField(msgMESSAGE_TEXT);
            data.nVersion = query.getInt64Field(msgWIZ_VERSION);
            data.nDeleteStatus = query.getIntField(msgDELETE_STATUS);
            data.nLocalChanged = query.getIntField(msgLOCAL_CHANGED);
            data.note = query.getStringField(msgMESSAGE_NOTE);

            arrayMessage.push_back(data);
            query.nextRow();
        }

        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::sqlToBizUserDataArray(const QString& strSQL,
                                          CWizBizUserDataArray& arrayUser)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZBIZUSER data;
            data.kbGUID = query.getStringField(userBIZ_GUID);
            data.userId = query.getStringField(userUSER_ID);
            data.userGUID = query.getStringField(userUSER_GUID);
            data.alias = query.getStringField(userUSER_ALIAS);
            data.pinyin = query.getStringField(userUSER_PINYIN);

            arrayUser.push_back(data);
            query.nextRow();
        }

        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::createMessageEx(const WIZMESSAGEDATA& data)
{
    qDebug() << "create message, id: " << data.nId;

    CString strFormat = formatInsertSQLFormat(TABLE_NAME_WIZ_MESSAGE,
                                              FIELD_LIST_WIZ_MESSAGE,
                                              PARAM_LIST_WIZ_MESSAGE);

    CString strSQL;
    strSQL.format(strFormat,
                  WizInt64ToStr(data.nId).utf16(),
                  STR2SQL(data.bizGUID).utf16(),
                  STR2SQL(data.kbGUID).utf16(),
                  STR2SQL(data.documentGUID).utf16(),
                  STR2SQL(data.senderAlias).utf16(),
                  STR2SQL(data.senderId).utf16(),
                  STR2SQL(data.senderGUID).utf16(),
                  STR2SQL(data.receiverAlias).utf16(),
                  STR2SQL(data.receiverId).utf16(),
                  STR2SQL(data.receiverGUID).utf16(),
                  data.nMessageType,
                  data.nReadStatus,
                  TIME2SQL(data.tCreated).utf16(),
                  STR2SQL(data.title).utf16(),
                  STR2SQL(data.messageBody).utf16(),
                  WizInt64ToStr(data.nVersion).utf16(),
                  data.nDeleteStatus,
                  data.nLocalChanged,
                  STR2SQL(data.note).utf16()
        );


    if (!execSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit messageCreated(data);
    }

    return true;
}

bool WizIndexBase::modifyMessageEx(const WIZMESSAGEDATA& data)
{
    qDebug() << "modify message, id: " << data.nId;

    WIZMESSAGEDATA dataOld;
    messageFromId(data.nId, dataOld);

    CString strFormat = formatUpdateSQLFormat(TABLE_NAME_WIZ_MESSAGE,
                                              FIELD_LIST_WIZ_MESSAGE_MODIFY,
                                              TABLE_KEY_WIZ_MESSAGE);

    CString strSQL;
    strSQL.format(strFormat,
                  data.nReadStatus,
                  data.nDeleteStatus,
                  WizInt64ToStr(data.nVersion).utf16(),
                  data.nLocalChanged,
                  WizInt64ToStr(data.nId).utf16()
        );

    if (!execSQL(strSQL))
        return false;

    WIZMESSAGEDATA dataNew;
    messageFromId(data.nId, dataNew);

    if (!m_bUpdating) {
        emit messageModified(dataOld, dataNew);
    }

    return true;
}

bool WizIndexBase::deleteMessageEx(const WIZMESSAGEDATA& data)
{
    qDebug() << "delete message, id: " << data.nId;

    CString strFormat = formatDeleteSQLFormat(TABLE_NAME_WIZ_MESSAGE,
                                              TABLE_KEY_WIZ_MESSAGE);

    CString strSQL;
    strSQL.format(strFormat,
        WizInt64ToStr(data.nId).utf16()
        );

    if (!execSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit messageDeleted(data);
    }

    return true;
}

bool WizIndexBase::createUserEx(const WIZBIZUSER& data)
{
    qDebug() << "create user, alias: " << data.alias;

    Q_ASSERT(!data.kbGUID.isEmpty() && !data.userGUID.isEmpty());

    CString strFormat = formatInsertSQLFormat(TABLE_NAME_WIZ_USER,
                                              FIELD_LIST_WIZ_USER,
                                              PARAM_LIST_WIZ_USER);

    CString strSQL;
    strSQL.format(strFormat,
                  STR2SQL(data.kbGUID).utf16(),
                  STR2SQL(data.userId).utf16(),
                  STR2SQL(data.userGUID).utf16(),
                  STR2SQL(data.alias).utf16(),
                  STR2SQL(data.pinyin).utf16()
        );


    if (!execSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit userCreated(data);
    }

    return true;
}

bool WizIndexBase::modifyUserEx(const WIZBIZUSER& user)
{
    qDebug() << "modify user, alias: " << user.alias;

    Q_ASSERT(!user.kbGUID.isEmpty() && !user.userGUID.isEmpty());

    // save old user info
    WIZBIZUSER userOld;
    userFromGUID(user.kbGUID, user.userGUID, userOld);

    CString strWhere = "BIZ_GUID=%1 AND USER_GUID=%2";
    strWhere = strWhere.arg(STR2SQL(user.kbGUID)).arg(STR2SQL(user.userGUID));

    CString strFormat = formatUpdateSQLByWhere(TABLE_NAME_WIZ_USER,
                                              FIELD_LIST_WIZ_USER_MODIFY,
                                              strWhere);

    CString strSQL;
    strSQL.format(strFormat,
                  STR2SQL(user.userId).utf16(),
                  STR2SQL(user.alias).utf16(),
                  STR2SQL(user.pinyin).utf16()
        );

    if (!execSQL(strSQL))
        return false;

    // read new user info
    WIZBIZUSER userNew;
    userFromGUID(user.kbGUID, user.userGUID, userNew);

    if (!m_bUpdating) {
        emit userModified(userOld, userNew);
    }

    return true;
}

bool WizIndexBase::deleteUserEx(const WIZBIZUSER& data)
{
    qDebug() << "delete user, alias: " << data.alias;

    // TODO

    return true;
}

bool WizIndexBase::createTagEx(const WIZTAGDATA& d)
{
    qDebug() << "create tag, name: " << d.strName;

    Q_ASSERT(d.strKbGUID == m_strKbGUID);

    WIZTAGDATA data = d;
    if (data.strGUID == data.strParentGUID) {
        data.strParentGUID.clear();
    }

    CString strFormat = formatInsertSQLFormat(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, PARAM_LIST_WIZ_TAG);

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strGUID).utf16(),
        STR2SQL(data.strParentGUID).utf16(),
        STR2SQL(data.strName).utf16(),
        STR2SQL(data.strDescription).utf16(),
        TIME2SQL(data.tModified).utf16(),
        WizInt64ToStr(data.nVersion).utf16(),
        WizInt64ToStr(data.nPosition).utf16()
        );


    if (!execSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit tagCreated(data);
    }

    return true;
}

bool WizIndexBase::modifyTagEx(const WIZTAGDATA& d)
{
    qDebug() << "modify tag, name: " << d.strName;

    Q_ASSERT(d.strKbGUID == m_strKbGUID);

    WIZTAGDATA dataOld;
    tagFromGuid(d.strGUID, dataOld);

    WIZTAGDATA data = d;
    if (data.strGUID == data.strParentGUID) {
        data.strParentGUID.empty();
    }

    CString strFormat = formatUpdateSQLFormat(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG_MODIFY, TABLE_KEY_WIZ_TAG);

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strParentGUID).utf16(),
        STR2SQL(data.strName).utf16(),
        STR2SQL(data.strDescription).utf16(),
        TIME2SQL(data.tModified).utf16(),
        WizInt64ToStr(data.nVersion).utf16(),
        WizInt64ToStr(data.nPosition).utf16(),
        STR2SQL(data.strGUID).utf16()
        );

    if (!execSQL(strSQL))
        return false;

    WIZTAGDATA dataNew;
    tagFromGuid(d.strGUID, dataNew);

    if (!m_bUpdating) {
        emit tagModified(dataOld, dataNew);
    }

    return true;
}

bool WizIndexBase::deleteTagEx(const WIZTAGDATA& data)
{
    qDebug() << "delete tag, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    CString strFormat = formatDeleteSQLFormat(TABLE_NAME_WIZ_TAG, TABLE_KEY_WIZ_TAG);

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

    if (!execSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit tagDeleted(data);
    }

    return true;
}

bool WizIndexBase::createStyleEx(const WIZSTYLEDATA& data)
{
    qDebug() << "create style, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    CString strFormat = formatInsertSQLFormat(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE, PARAM_LIST_WIZ_STYLE);

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strGUID).utf16(),
        STR2SQL(data.strName).utf16(),
        STR2SQL(data.strDescription).utf16(),
        COLOR2SQL(data.crTextColor).utf16(),
        COLOR2SQL(data.crBackColor).utf16(),
        data.bTextBold ? 1 : 0,
        data.nFlagIndex,
        TIME2SQL(data.tModified).utf16(),
        WizInt64ToStr(data.nVersion).utf16()
        );


    if (!execSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit styleCreated(data);
    }

    return true;
}

bool WizIndexBase::modifyStyleEx(const WIZSTYLEDATA& data)
{
    qDebug() << "modify style, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    WIZSTYLEDATA dataOld;
    styleFromGuid(data.strGUID, dataOld);

    CString strFormat = formatUpdateSQLFormat(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE_MODIFY, TABLE_KEY_WIZ_STYLE);

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strName).utf16(),
        STR2SQL(data.strDescription).utf16(),
        COLOR2SQL(data.crTextColor).utf16(),
        COLOR2SQL(data.crBackColor).utf16(),
        data.bTextBold ? 1 : 0,
        data.nFlagIndex,
        TIME2SQL(data.tModified).utf16(),
        WizInt64ToStr(data.nVersion).utf16(),
        STR2SQL(data.strGUID).utf16()
        );

    if (!execSQL(strSQL))
        return false;

    WIZSTYLEDATA dataNew;
    styleFromGuid(data.strGUID, dataNew);

    if (!m_bUpdating) {
        emit styleModified(dataOld, dataNew);
    }

    return true;
}

bool WizIndexBase::deleteStyleEx(const WIZSTYLEDATA& data)
{
    qDebug() << "delete style, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    CString strFormat = formatDeleteSQLFormat(TABLE_NAME_WIZ_STYLE, TABLE_KEY_WIZ_STYLE);

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

    if (!execSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit styleDeleted(data);
    }

    return true;
}

bool WizIndexBase::createDocumentEx(const WIZDOCUMENTDATA& dataNew)
{
    qDebug() << "create document, title: " << dataNew.strTitle;

    Q_ASSERT(dataNew.strKbGUID == m_strKbGUID);

    WIZDOCUMENTDATA data = dataNew;

    // try to fill the fields not allowed empty
    if (data.strTitle.isEmpty()) {
        TOLOG1("Document Title is empty: %1, Try to rename to the \"New note\"", data.strGUID);
        data.strTitle = "New note";
    }

    if (data.strLocation.isEmpty()) {
        data.strLocation = getDefaultNoteLocation();
        TOLOG2("Document Location is empty: %1, Try to relocation to the %2", data.strTitle, data.strLocation);
    }

    CString strFormat = formatInsertSQLFormat(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, PARAM_LIST_WIZ_DOCUMENT);

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strGUID).utf16(),
        STR2SQL(data.strTitle).utf16(),
        STR2SQL(data.strLocation).utf16(),
        STR2SQL(data.strName).utf16(),
        STR2SQL(data.strSEO).utf16(),
        STR2SQL(data.strURL).utf16(),
        STR2SQL(data.strAuthor).utf16(),
        STR2SQL(data.strKeywords).utf16(),
        STR2SQL(data.strType).utf16(),
        STR2SQL(data.strOwner).utf16(),
        STR2SQL(data.strFileType).utf16(),
        STR2SQL(data.strStyleGUID).utf16(),

        TIME2SQL(data.tCreated).utf16(),
        TIME2SQL(data.tModified).utf16(),
        TIME2SQL(data.tAccessed).utf16(),

        0,//data.nIconIndex,
        0,//data.nSync,
        (int)data.nProtected,
        (int)data.nReadCount,
        (int)data.nAttachmentCount,
        (int)data.nIndexed,

        TIME2SQL(data.tModified).utf16(),//TIME2SQL(data.tInfoModified).utf16(),
        STR2SQL(data.strDataMD5).utf16(),//STR2SQL(data.strInfoMD5).utf16(),
        TIME2SQL(data.tDataModified).utf16(),
        STR2SQL(data.strDataMD5).utf16(),
        TIME2SQL(data.tModified).utf16(),//TIME2SQL(data.tParamModified).utf16(),
        STR2SQL(data.strDataMD5).utf16(),//STR2SQL(data.strParamMD5).utf16(),
        WizInt64ToStr(data.nVersion).utf16(),
        (int)data.nInfoChanged,
        (int)data.nDataChanged
    );

    if (!execSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit documentCreated(data);
    }

    return true;
}

bool WizIndexBase::modifyDocumentInfoEx(const WIZDOCUMENTDATA& dataCur)
{
    qDebug() << "modify document, title: " << dataCur.strTitle;

    if (dataCur.strKbGUID != m_strKbGUID)
    {
        Q_ASSERT(false);
    }

    WIZDOCUMENTDATA dataOld;
    documentFromGuid(dataCur.strGUID, dataOld);

    WIZDOCUMENTDATA data = dataCur;

    // try to fill the fields not allowed empty
    if (data.strTitle.isEmpty()) {
        if (!dataOld.strTitle.isEmpty()) {
            data.strTitle = dataOld.strTitle;
        } else {
            data.strTitle = "New note";
        }

        TOLOG2("Document Title is empty: %1, Try to rename to the %2", data.strGUID, data.strTitle);
    }

    if (data.strLocation.isEmpty()) {
        if (!dataOld.strLocation.isEmpty()) {
            data.strLocation = dataOld.strLocation;
        } else {
            data.strLocation = getDefaultNoteLocation();
        }

        TOLOG2("Document Location is empty: %1, Try to relocation to the %2", data.strTitle, data.strLocation);
    }
    //
    if (data.nVersion >= 0)
    {
        if (data.nDataChanged || data.nInfoChanged)
        {
            qDebug() << "fault error: data changed or info changed is not false";
            data.nDataChanged = 0;
            data.nInfoChanged = 0;
        }
    }

    CString strFormat = formatUpdateSQLFormat(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT_MODIFY, TABLE_KEY_WIZ_DOCUMENT);

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strTitle).utf16(),
        STR2SQL(data.strLocation).utf16(),
        STR2SQL(data.strName).utf16(),
        STR2SQL(data.strSEO).utf16(),
        STR2SQL(data.strURL).utf16(),
        STR2SQL(data.strAuthor).utf16(),
        STR2SQL(data.strKeywords).utf16(),
        STR2SQL(data.strType).utf16(),
        STR2SQL(data.strOwner).utf16(),
        STR2SQL(data.strFileType).utf16(),
        STR2SQL(data.strStyleGUID).utf16(),

        TIME2SQL(data.tCreated).utf16(),
        TIME2SQL(data.tModified).utf16(),
        TIME2SQL(data.tAccessed).utf16(),

        0,//data.nIconIndex,
        0,//data.nSync,
        (int)data.nProtected,
        (int)data.nReadCount,
        (int)data.nAttachmentCount,
        (int)data.nIndexed,

        TIME2SQL(data.tDataModified).utf16(),//TIME2SQL(data.tInfoModified).utf16(),
        STR2SQL(data.strDataMD5 ).utf16(),//STR2SQL(data.strInfoMD5 ).utf16(),
        TIME2SQL(data.tDataModified).utf16(),
        STR2SQL(data.strDataMD5 ).utf16(),
        TIME2SQL(data.tDataModified).utf16(),//TIME2SQL(data.tParamModified).utf16(),
        STR2SQL(data.strDataMD5 ).utf16(),//STR2SQL(data.strParamMD5 ).utf16(),
        WizInt64ToStr(data.nVersion).utf16(),
        (int)data.nInfoChanged,
        (int)data.nDataChanged,

        STR2SQL(data.strGUID).utf16()
    );

    if (!execSQL(strSQL))
        return false;

    WIZDOCUMENTDATA dataNew;
    documentFromGuid(data.strGUID, dataNew);

    if (!m_bUpdating) {
        emit documentModified(dataOld, dataNew);
    }

    return true;
}

bool WizIndexBase::deleteDocumentEx(const WIZDOCUMENTDATA& data)
{
    qDebug() << "delete document, title: " << data.strTitle;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    CString strFormat = formatDeleteSQLFormat(TABLE_NAME_WIZ_DOCUMENT, TABLE_KEY_WIZ_DOCUMENT);

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

    if (!execSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit documentDeleted(data);
    }

    return true;
}

bool WizIndexBase::createAttachmentEx(const WIZDOCUMENTATTACHMENTDATA& data)
{
    qDebug() << "create attachment, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    //CString strInfoMD5 = CalDocumentAttachmentInfoMD5(data);
    //if (strInfoMD5 != data.strInfoMD5) {
    //    TOLOG2("Warning: Attachment info md5 does not match: %1, %2", strInfoMD5, data.strInfoMD5);
    //}

    CString strFormat = formatInsertSQLFormat(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT, PARAM_LIST_WIZ_DOCUMENT_ATTACHMENT);

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strGUID).utf16(),
        STR2SQL(data.strDocumentGUID).utf16(),
        STR2SQL(data.strName).utf16(),
        STR2SQL(data.strURL).utf16(),
        STR2SQL(data.strDescription).utf16(),
        TIME2SQL(data.tInfoModified).utf16(),
        STR2SQL(data.strInfoMD5).utf16(),
        TIME2SQL(data.tDataModified).utf16(),
        STR2SQL(data.strDataMD5).utf16(),
        WizInt64ToStr(data.nVersion).utf16()
    );

    if (!execSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit attachmentCreated(data);
    }

    return true;
}

bool WizIndexBase::modifyAttachmentInfoEx(const WIZDOCUMENTATTACHMENTDATA& data)
{
    qDebug() << "modify attachment, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    WIZDOCUMENTATTACHMENTDATA dataOld;
    attachmentFromGuid(data.strGUID, dataOld);

    CString strFormat = formatUpdateSQLFormat(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT_MODIFY, TABLE_KEY_WIZ_DOCUMENT_ATTACHMENT);

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strName).utf16(),
        STR2SQL(data.strURL).utf16(),
        STR2SQL(data.strDescription).utf16(),
        TIME2SQL(data.tInfoModified).utf16(),
        STR2SQL(data.strInfoMD5 ).utf16(),
        TIME2SQL(data.tDataModified).utf16(),
        STR2SQL(data.strDataMD5 ).utf16(),
        WizInt64ToStr(data.nVersion).utf16(),
        STR2SQL(data.strGUID).utf16()
    );

    if (!execSQL(strSQL))
        return false;

    WIZDOCUMENTATTACHMENTDATA dataNew;
    attachmentFromGuid(data.strGUID, dataNew);

    if (!m_bUpdating) {
        emit attachmentModified(dataOld, dataNew);
    }

    return true;
}

bool WizIndexBase::deleteAttachmentEx(const WIZDOCUMENTATTACHMENTDATA& data)
{
    qDebug() << "delete attachment, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    CString strFormat = formatDeleteSQLFormat(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT,
                                              "ATTACHMENT_GUID");

    CString strSQL;
    strSQL.format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

    if (!execSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit attachmentDeleted(data);
    }

    return true;
}

bool WizIndexBase::updateDocumentParam(const WIZDOCUMENTPARAMDATA& data)
{
    QString strFormat = "replace into %1 (%2) values (%3)";
    strFormat = strFormat.arg(TABLE_NAME_WIZ_DOCUMENT_PARAM, FIELD_LIST_WIZ_DOCUMENT_PARAM, PARAM_LIST_WIZ_DOCUMENT_PARAM);
    //
    CString sql;
    sql.format(strFormat,
               STR2SQL(data.strDocumentGuid).utf16(),
               STR2SQL(data.strName.toUpper()).utf16(),
               STR2SQL(data.strValue).utf16(),
               WizInt64ToStr(data.nVersion).utf16()
               );
    //
    if (!execSQL(sql))
        return false;
    //
    WIZDOCUMENTPARAMDATA tmp = data;
    WizExecuteOnThread(WIZ_THREAD_MAIN, [=] {
        emit documentParamModified(tmp);
    });
    //
    return true;
}


bool WizIndexBase::getAllTags(CWizTagDataArray& arrayTag)
{
    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG);
    return sqlToTagDataArray(strSQL, arrayTag);
}

bool WizIndexBase::getAllTags(std::multimap<CString, WIZTAGDATA>& mapTag)
{
    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG);
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZTAGDATA data;
            data.strKbGUID = kbGUID();
            data.strGUID = query.getStringField(tagTAG_GUID);
            data.strParentGUID = query.getStringField(tagTAG_GROUP_GUID);
            data.strName = query.getStringField(tagTAG_NAME);
            data.strDescription = query.getStringField(tagTAG_DESCRIPTION);
            data.tModified = query.getTimeField(tagDT_MODIFIED);
            data.nVersion = query.getInt64Field(tagVersion);
            data.nPosition = query.getInt64Field(tagTAG_POS);

            mapTag.insert(std::make_pair(data.strParentGUID, data));
            query.nextRow();
        }

        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return logSQLException(e, strSQL);
    }
}

bool WizIndexBase::getRootTags(CWizTagDataArray& arrayTag)
{
    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG,
                                    "(TAG_GROUP_GUID = '' or TAG_GROUP_GUID is null)");

    return sqlToTagDataArray(strSQL, arrayTag);
}

bool WizIndexBase::getChildTags(const CString& strParentTagGUID, CWizTagDataArray& arrayTag)
{
    CString strWhere = strParentTagGUID.isEmpty() ?
                       WizFormatString0("TAG_GROUP_GUID is null") :
                       WizFormatString1("TAG_GROUP_GUID='%1'", strParentTagGUID);

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, strWhere);
    return sqlToTagDataArray(strSQL, arrayTag);
}

bool WizIndexBase::getAllChildTags(const CString& strParentTagGUID, CWizTagDataArray& arrayTag)
{
    CWizTagDataArray arrayTagCurrent;
    getChildTags(strParentTagGUID, arrayTagCurrent);

    arrayTag.insert(arrayTag.begin(), arrayTagCurrent.begin(), arrayTagCurrent.end());

    CWizTagDataArray::const_iterator it;
    for (it = arrayTagCurrent.begin(); it != arrayTagCurrent.end(); it++) {
        getAllChildTags(it->strGUID, arrayTag);
    }

    return true;
}

bool WizIndexBase::getAllTagsWithErrorParent(CWizTagDataArray& arrayTag)
{
    CString strWhere = QString("TAG_GROUP_GUID is not null and TAG_GROUP_GUID "
                               "not in (select distinct TAG_GUID from %1)").arg(TABLE_NAME_WIZ_TAG);

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, strWhere);
    return sqlToTagDataArray(strSQL, arrayTag);
}

bool WizIndexBase::getChildTagsSize(const CString &strParentTagGUID, int &size)
{
    CString strWhere = strParentTagGUID.isEmpty() ?
                       WizFormatString0("TAG_GROUP_GUID is null") :
                       WizFormatString1("TAG_GROUP_GUID='%1'", strParentTagGUID);

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_TAG, "COUNT(*)", strWhere);
    return sqlToSize(strSQL, size);
}

bool WizIndexBase::getAllChildTagsSize(const CString& strParentTagGUID, int& size)
{
    int nSizeCurrent;
    getChildTagsSize(strParentTagGUID, nSizeCurrent);
    size += nSizeCurrent;

    CWizTagDataArray arrayTagCurrent;
    getChildTags(strParentTagGUID, arrayTagCurrent);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTagCurrent.begin(); it != arrayTagCurrent.end(); it++) {
        getAllChildTagsSize(it->strGUID, size);
    }

    return true;
}

bool WizIndexBase::tagFromGuid(const CString& strTagGUID, WIZTAGDATA& data)
{
    if (!strTagGUID || !*strTagGUID) {
        TOLOG("TagGUID is empty");
        return false;
    }

    CString strWhere;
    strWhere.format("TAG_GUID=%s",
        STR2SQL(strTagGUID).utf16()
        );

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, strWhere);

    CWizTagDataArray arrayTag;
    if (!sqlToTagDataArray(strSQL, arrayTag)) {
        TOLOG("Failed to get tag by guid");
        return false;
    }

    if (arrayTag.empty()) {
        //TOLOG("Failed to get tag by guid, result is empty");
        return false;
    }

    data = arrayTag[0];
    return true;
}

bool WizIndexBase::getStyles(CWizStyleDataArray& arrayStyle)
{
    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE);
    return sqlToStyleDataArray(strSQL, arrayStyle);
}

bool WizIndexBase::styleFromGuid(const CString& strStyleGUID, WIZSTYLEDATA& data)
{
    if (!strStyleGUID || !*strStyleGUID) {
        TOLOG("StyleGUID is empty");
        return false;
    }

    CString strWhere;
    strWhere.format("STYLE_GUID=%s",
        STR2SQL(strStyleGUID).utf16()
        );

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE, strWhere);

    CWizStyleDataArray arrayStyle;
    if (!sqlToStyleDataArray(strSQL, arrayStyle)) {
        TOLOG("Failed to get style by guid");
        return false;
    }

    if (arrayStyle.empty()) {
        //TOLOG("Failed to get style by guid, result is empty");
        return false;
    }

    data = arrayStyle[0];
    return true;
}

bool WizIndexBase::getMetas(CWizMetaDataArray& arrayMeta)
{
    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_META, FIELD_LIST_WIZ_META);
    return sqlToMetaDataArray(strSQL, arrayMeta);
}

bool WizIndexBase::getAllDocumentsSize(int& count, bool bIncludeTrash /* = false */)
{
    CString strSQL;
    if (bIncludeTrash) {
        strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, "COUNT(*)");
    } else {
        CString strWhere;
        strWhere.format("DOCUMENT_LOCATION not like %s",
                        STR2SQL(QString(LOCATION_DELETED_ITEMS) + "%").utf16());

        strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, "COUNT(*)", strWhere);
    }

    return sqlToSize(strSQL, count);
}

bool WizIndexBase::getAllDocuments(CWizDocumentDataArray& arrayDocument)
{
    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT);
    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndexBase::getDocumentsBySQLWhere(const CString& strSQLWhere, CWizDocumentDataArray& arrayDocument)
{
    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strSQLWhere);
    return sqlToDocumentDataArray(strSQL, arrayDocument);
}

bool WizIndexBase::documentFromGuid(const CString& strDocumentGUID, WIZDOCUMENTDATA& data)
{
    if (!strDocumentGUID || !*strDocumentGUID) {
        TOLOG("DocumentGUID is empty");
        return false;
    }

    CString strWhere;
    strWhere.format("DOCUMENT_GUID=%s",
        STR2SQL(strDocumentGUID).utf16()
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
    }

    data = arrayDocument[0];
    return true;
}

bool WizIndexBase::getAttachments(CWizDocumentAttachmentDataArray& arrayAttachment)
{
    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT);
    return sqlToDocumentAttachmentDataArray(strSQL, arrayAttachment);
}

bool WizIndexBase::attachmentFromGuid(const CString& strAttachcmentGUID,
                                       WIZDOCUMENTATTACHMENTDATA& data)
{
    if (strAttachcmentGUID.isEmpty()) {
        TOLOG("AttahcmentGUID is empty");
        return false;
    }

    CString strWhere;
    strWhere.format("ATTACHMENT_GUID=%s",
        STR2SQL(strAttachcmentGUID).utf16()
        );

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT, strWhere);

    CWizDocumentAttachmentDataArray arrayDocumentAttachment;
    if (!sqlToDocumentAttachmentDataArray(strSQL, arrayDocumentAttachment)) {
        TOLOG("Failed to get attachment attachment by guid");
        return false;
    }

    if (arrayDocumentAttachment.empty()) {
        //TOLOG("Failed to get attachment by guid, result is empty");
        return false;
    }

    data = arrayDocumentAttachment[0];
    return true;
}

bool WizIndexBase::messageFromId(qint64 nId, WIZMESSAGEDATA& data)
{
    CString strWhere;
    strWhere.format("MESSAGE_ID=%s", WizInt64ToStr(nId).utf16());

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_MESSAGE,
                                    FIELD_LIST_WIZ_MESSAGE,
                                    strWhere);

    CWizMessageDataArray arrayMessage;
    if (!sqlToMessageDataArray(strSQL, arrayMessage)) {
        TOLOG("[messageFromId] failed to get message by id");
        return false;
    }

    if (arrayMessage.empty())
        return false;

    data = arrayMessage[0];
    return true;
}

bool WizIndexBase::messageFromUserGUID(const QString& userGUID, CWizMessageDataArray& arrayMessage)
{
    CString strWhere;
    strWhere.format("SENDER_GUID=%s", STR2SQL(userGUID).utf16());

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_MESSAGE,
                                    FIELD_LIST_WIZ_MESSAGE,
                                    strWhere);

    if (!sqlToMessageDataArray(strSQL, arrayMessage)) {
        TOLOG1("[messageFromId] failed to get message by user guid : %1", userGUID);
        return false;
    }

    return !arrayMessage.empty();
}

bool WizIndexBase::unreadMessageFromUserGUID(const QString& userGUID, CWizMessageDataArray& arrayMessage)
{
    CString strWhere;
    strWhere.format("SENDER_GUID=%s and READ_STATUS=0", STR2SQL(userGUID).utf16());

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_MESSAGE,
                                    FIELD_LIST_WIZ_MESSAGE,
                                    strWhere);

    if (!sqlToMessageDataArray(strSQL, arrayMessage)) {
        TOLOG1("[messageFromId] failed to get unread message by user guid : %1", userGUID);
        return false;
    }

    return !arrayMessage.empty();
}

bool WizIndexBase::messageFromDocumentGUID(const QString& strGUID, WIZMESSAGEDATA& data)
{
    CString strWhere;
    strWhere.format("DOCUMENT_GUID=%s", STR2SQL(strGUID).utf16());

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_MESSAGE,
                                    FIELD_LIST_WIZ_MESSAGE,
                                    strWhere);

    CWizMessageDataArray arrayMessage;
    if (!sqlToMessageDataArray(strSQL, arrayMessage)) {
        TOLOG1("[messageFromId] failed to get message by document guid : %1", strGUID);
        return false;
    }

    if (arrayMessage.empty())
        return false;

    // FIXME: return the lastest message
    data = arrayMessage[0];
    return true;
}

bool WizIndexBase::getAllUsers(CWizBizUserDataArray& arrayUser)
{
    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_USER, FIELD_LIST_WIZ_USER);
    return sqlToBizUserDataArray(strSQL, arrayUser);
}

bool WizIndexBase::userFromGUID(const QString& strUserGUID,
                                 CWizBizUserDataArray& arrayUser)
{
    CString strWhere;
    strWhere.format("USER_GUID=%s", STR2SQL(strUserGUID).utf16());

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_USER,
                                    FIELD_LIST_WIZ_USER,
                                    strWhere);

    if (!sqlToBizUserDataArray(strSQL, arrayUser)) {
        TOLOG("[userFromGUID] failed to get user by user guid");
        return false;
    }

    if (arrayUser.empty())
        return false;

    return true;
}

bool WizIndexBase::userFromGUID(const QString& strKbGUID,
                                 const QString& userGUID,
                                 WIZBIZUSER& user)
{
    CString strWhere = "BIZ_GUID=%1 AND USER_GUID=%2";
    strWhere = strWhere.arg(STR2SQL(strKbGUID)).arg(STR2SQL(userGUID));

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_USER,
                                    FIELD_LIST_WIZ_USER,
                                    strWhere);

    CWizBizUserDataArray arrayUser;
    if (!sqlToBizUserDataArray(strSQL, arrayUser)) {
        TOLOG("[userFromGUID] failed to get user by user guid");
        return false;
    }

    if (arrayUser.empty())
        return false;

    user = arrayUser[0];
    return true;
}

bool WizIndexBase::users(const QString& strKbGUID, CWizBizUserDataArray& arrayUser)
{
    CString strWhere = "BIZ_GUID=%1";
    strWhere = strWhere.arg(STR2SQL(strKbGUID));

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_USER,
                                    FIELD_LIST_WIZ_USER,
                                    strWhere);

    if (!sqlToBizUserDataArray(strSQL, arrayUser)) {
        TOLOG("[users] failed to get users");
        return false;
    }

    //if (arrayUser.empty()) {
    //    qDebug() << "[users] should not be empty, right?";
    //    return false;
    //}

    return true;
}

bool WizIndexBase::userFromID(const QString& strKbGUID, const QString& userID, WIZBIZUSER& user)
{
    CString strWhere = "BIZ_GUID=%1 AND USER_ID=%2";
    strWhere = strWhere.arg(STR2SQL(strKbGUID)).arg(STR2SQL(userID));

    CString strSQL = formatQuerySQL(TABLE_NAME_WIZ_USER,
                                    FIELD_LIST_WIZ_USER,
                                    strWhere);

    CWizBizUserDataArray arrayUser;
    if (!sqlToBizUserDataArray(strSQL, arrayUser)) {
        TOLOG("[userFromGUID] failed to get user by user guid");
        return false;
    }

    if (arrayUser.empty())
        return false;

    user = arrayUser[0];
    return true;
}
