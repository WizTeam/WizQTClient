#include "wizIndexBase.h"
#include "wizdef.h"

#include <QDebug>

#include "utils/logger.h"
#include "utils/pathresolve.h"


CWizIndexBase::CWizIndexBase(void)
    : m_bUpdating(false)
{
    qRegisterMetaType<WIZTAGDATA>("WIZTAGDATA");
    qRegisterMetaType<WIZSTYLEDATA>("WIZSTYLEDATA");
    qRegisterMetaType<WIZDOCUMENTDATA>("WIZDOCUMENTDATA");
    qRegisterMetaType<WIZDOCUMENTATTACHMENTDATA>("WIZDOCUMENTATTACHMENTDATA");

    qRegisterMetaType<WIZMESSAGEDATA>("WIZMESSAGEDATA");
    qRegisterMetaType<WIZBIZUSER>("WIZBIZUSER");
}

CWizIndexBase::~CWizIndexBase(void)
{
    Close();
}

bool CWizIndexBase::Open(const CString& strFileName)
{
    m_strFileName = strFileName;
    //m_strDatabasePath = WizExtractFilePath(strFileName);

    try {
        m_db.open(strFileName);
        // upgrade table structure if table structure have been changed
        if (m_db.tableExists(TABLE_NAME_WIZ_META)) {
            int nVersion = getTableStructureVersion().toInt();
            if (nVersion < QString(WIZ_TABLE_STRUCTURE_VERSION).toInt()) {
                updateTableStructure(nVersion);
            }
        }
    } catch (const CppSQLite3Exception& e) {
        return LogSQLException(e, _T("open database"));
    }

    for (int i = 0; i < TABLE_COUNT; i++) {
        if (!CheckTable(g_arrayTableName[i]))
            return false;
    }
    setTableStructureVersion(WIZ_TABLE_STRUCTURE_VERSION);

    return true;
}

bool CWizIndexBase::IsOpened()
{
    return m_db.IsOpened();
}

void CWizIndexBase::Close()
{
    m_db.close();
}

bool CWizIndexBase::CheckTable(const QString& strTableName)
{
    if (m_db.tableExists(strTableName))
        return true;

    // create table if not exist
    CString strFileName = WizPathAddBackslash2(Utils::PathResolve::resourcesPath() + "sql") + strTableName.toLower() + ".sql";
    CString strSQL;
    if (!WizLoadUnicodeTextFromFile(strFileName, strSQL))
        return false;

    bool result = ExecSQL(strSQL);
    return result;
}

bool CWizIndexBase::ExecSQL(const CString& strSQL)
{
    try {
        m_db.execDML(strSQL);
        return true;
    } catch (const CppSQLite3Exception& e) {
        return LogSQLException(e, strSQL);
    }
}

CppSQLite3Query CWizIndexBase::Query(const CString& strSQL)
{
    return m_db.execQuery(strSQL);
}

int CWizIndexBase::Exec(const CString& strSQL)
{
    return m_db.execDML(strSQL);
}

bool CWizIndexBase::HasRecord(const CString& strSQL)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        return !query.eof();
    }
    catch (const CppSQLite3Exception& e)
    {
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::GetFirstRowFieldValue(const CString& strSQL, int nFieldIndex, CString& strValue)
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
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::LogSQLException(const CppSQLite3Exception& e, const CString& strSQL)
{
    TOLOG(e.errorMessage());
    TOLOG(strSQL);
    return false;
}

bool CWizIndexBase::Repair(const QString& strDestFileName)
{
    return CppSQLite3DB::repair(m_strFileName, strDestFileName) ? true : false;
}

bool CWizIndexBase::updateTableStructure(int oldVersion)
{
    qDebug() << "table structure version : " << oldVersion << "  update to version " << WIZ_TABLE_STRUCTURE_VERSION;
    if (oldVersion < 1) {
        Exec("ALTER TABLE 'WIZ_TAG' ADD 'TAG_POS' int64; ");
    }
    //
    if (oldVersion < 2) {
        Exec("ALTER TABLE 'WIZ_MESSAGE' ADD 'DELETE_STATUS' int;");
        Exec("ALTER TABLE 'WIZ_MESSAGE' ADD 'LOCAL_CHANGED' int;");
    }

    if (oldVersion < 3) {
        Exec("ALTER TABLE 'WIZ_MESSAGE' ADD 'MESSAGE_NOTE' varchar(2048);");
    }
    //
    setTableStructureVersion(WIZ_TABLE_STRUCTURE_VERSION);
    return true;
}

CString CWizIndexBase::FormatCanonicSQL(const CString& strTableName,
                                        const CString& strFieldList,
                                        const CString& strExt)
{
    return WizFormatString3("select %1 from %2 %3",
                            strFieldList,
                            strTableName,
                            strExt);
}

CString CWizIndexBase::FormatQuerySQL(const CString& strTableName,
                                      const CString& strFieldList)
{
    return WizFormatString2(_T("select %1 from %2"),
                            strFieldList,
                            strTableName);
}

CString CWizIndexBase::FormatQuerySQL(const CString& strTableName,
                                      const CString& strFieldList,
                                      const CString& strWhere)
{
    return WizFormatString3(_T("select %1 from %2 where %3"),
                            strFieldList,
                            strTableName,
                            strWhere);
}

CString CWizIndexBase::FormatInsertSQLFormat(const CString& strTableName,
                                             const CString& strFieldList,
                                             const CString& strParamList)
{
    return WizFormatString3(_T("insert into %1 (%2) values (%3)"),
                            strTableName,
                            strFieldList,
                            strParamList);
}

CString CWizIndexBase::FormatUpdateSQLFormat(const CString& strTableName,
                                             const CString& strFieldList,
                                             const CString& strKey)
{
    return WizFormatString3(_T("update %1 set %2 where %3=%s"),
                            strTableName,
                            strFieldList,
                            strKey);
}

CString CWizIndexBase::FormatUpdateSQLByWhere(const CString& strTableName,
                                              const CString& strFieldList,
                                              const CString& strWhere)
{
    return WizFormatString3("update %1 set %2 where %3",
                            strTableName,
                            strFieldList,
                            strWhere);
}

CString CWizIndexBase::FormatDeleteSQLFormat(const CString& strTableName,
                                             const CString& strKey)
{
    return WizFormatString2(_T("delete from %1 where %2=%s"),
                            strTableName,
                            strKey);
}

CString CWizIndexBase::FormatDeleteSQLByWhere(const CString& strTableName, const CString& strWhere)
{
    return WizFormatString2(_T("delete from %1 where %2"),
                            strTableName,
                            strWhere);
}

CString CWizIndexBase::FormatQuerySQLByTime(const CString& strTableName,
                                            const CString& strFieldList,
                                            const CString& strFieldName,
                                            const COleDateTime& t)
{
    return WizFormatString4(_T("select %1 from %2 where %3 >= %4"),
                            strFieldList,
                            strTableName,
                            strFieldName,
                            TIME2SQL(t));
}
CString CWizIndexBase::FormatQuerySQLByTime2(const CString& strTableName,
                                             const CString& strFieldList,
                                             const CString& strInfoFieldName,
                                             const CString& strDataFieldName,
                                             const COleDateTime& t)
{
    return WizFormatString5(_T("select %1 from %2 where %3 >= %5 or %4 >= %5"),
                            strFieldList,
                            strTableName,
                            strInfoFieldName,
                            strDataFieldName,
                            TIME2SQL(t));
}
CString CWizIndexBase::FormatQuerySQLByTime3(const CString& strTableName,
                                             const CString& strFieldList,
                                             const CString& strInfoFieldName,
                                             const CString& strDataFieldName,
                                             const CString& strParamFieldName,
                                             const QDateTime& t)
{
    return WizFormatString6(_T("select %1 from %2 where %3 >= %6 or %4 >= %6 or %5 >= %6"),
                            strFieldList,
                            strTableName,
                            strInfoFieldName,
                            strDataFieldName,
                            strParamFieldName,
                            TIME2SQL(t));
}

CString CWizIndexBase::FormatModifiedQuerySQL(const CString& strTableName,
                                              const CString& strFieldList)
{
    return WizFormatString2(_T("select %1 from %2 where WIZ_VERSION = -1"),
                            strFieldList,
                            strTableName);
}

CString CWizIndexBase::FormatModifiedQuerySQL2(const CString& strTableName,
                                               const CString& strFieldList,
                                               int nCount)
{
    return WizFormatString3(_T("select %1 from %2 where WIZ_VERSION = -1 limit 0, %3"),
                            strFieldList,
                            strTableName,
                            WizIntToStr(nCount));
}

bool CWizIndexBase::SQLToSize(const CString& strSQL, int& size)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        size = query.getInt64Field(0);
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::SQLToTagDataArray(const CString& strSQL, CWizTagDataArray& arrayTag)
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
            data.nPostion = query.getInt64Field(tagTAG_POS);

            arrayTag.push_back(data);
            query.nextRow();
        }

        std::sort(arrayTag.begin(), arrayTag.end());
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::SQLToStyleDataArray(const CString& strSQL, CWizStyleDataArray& arrayStyle)
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
            data.crTextColor = query.getColorField(styleSTYLE_TEXT_COLOR);
            data.crBackColor = query.getColorField(styleSTYLE_BACK_COLOR);
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
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::SQLToMetaDataArray(const CString& strSQL, CWizMetaDataArray& arrayMeta)
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
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::SQLToDeletedGUIDDataArray(const CString& strSQL, CWizDeletedGUIDDataArray& arrayGUID)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZDELETEDGUIDDATA data;
            data.strKbGUID = kbGUID();
            data.strGUID = query.getStringField(deletedguidDELETED_GUID);
            data.eType = WIZOBJECTDATA::IntToObjectType(query.getIntField(deletedguidGUID_TYPE));
            data.tDeleted = query.getTimeField(deletedguidDT_DELETED);

            arrayGUID.push_back(data);
            query.nextRow();
        }
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::SQLToStringArray(const CString& strSQL, int nFieldIndex, CWizStdStringArray& arrayString)
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
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::SQLToDocumentParamDataArray(const CString& strSQL, CWizDocumentParamDataArray& arrayParam)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZDOCUMENTPARAMDATA data;
            data.strKbGUID = kbGUID();
            data.strDocumentGUID = query.getStringField(documentparamDOCUMENT_GUID);
            data.strName = query.getStringField(documentparamPARAM_NAME);
            data.strValue = query.getStringField(documentparamPARAM_VALUE);

            arrayParam.push_back(data);
            query.nextRow();
        }
        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::SQLToDocumentDataArray(const CString& strSQL, CWizDocumentDataArray& arrayDocument)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        CWizStdStringArray arrayGUID;
        std::map<CString, int> mapDocumentIndex;
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
            data.nIconIndex = query.getIntField(documentDOCUMENT_ICON_INDEX);
            data.nSync = query.getIntField(documentDOCUMENT_SYNC);
            data.nProtected = query.getIntField(documentDOCUMENT_PROTECT);
            data.nReadCount = query.getIntField(documentDOCUMENT_READ_COUNT);
            data.nAttachmentCount = query.getIntField(documentDOCUMENT_ATTACHEMENT_COUNT);
            data.nIndexed = query.getIntField(documentDOCUMENT_INDEXED);
            data.tInfoModified = query.getTimeField(documentDT_INFO_MODIFIED);
            data.strInfoMD5 = query.getStringField(documentDOCUMENT_INFO_MD5);
            data.tDataModified = query.getTimeField(documentDT_DATA_MODIFIED);
            data.strDataMD5 = query.getStringField(documentDOCUMENT_DATA_MD5);
            data.tParamModified = query.getTimeField(documentDT_PARAM_MODIFIED);
            data.strParamMD5 = query.getStringField(documentDOCUMENT_PARAM_MD5);
            data.nVersion = query.getInt64Field(documentVersion);

            arrayGUID.push_back(data.strGUID);
            arrayDocument.push_back(data);
            mapDocumentIndex[data.strGUID] = int(arrayDocument.size() - 1);
            query.nextRow();
        }

        if (!arrayGUID.empty()) {
            InitDocumentExFields(arrayDocument, arrayGUID, mapDocumentIndex);
        }

        return true;
    }
    catch (const CppSQLite3Exception& e)
    {
        return LogSQLException(e, strSQL);
    }
}




bool CWizIndexBase::SQLToDocumentAttachmentDataArray(const CString& strSQL,
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
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::SQLToMessageDataArray(const QString& strSQL,
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
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::SQLToBizUserDataArray(const QString& strSQL,
                                          CWizBizUserDataArray& arrayUser)
{
    try
    {
        CppSQLite3Query query = m_db.execQuery(strSQL);
        while (!query.eof())
        {
            WIZBIZUSER data;
            data.bizGUID = query.getStringField(userBIZ_GUID);
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
        return LogSQLException(e, strSQL);
    }
}

bool CWizIndexBase::createMessageEx(const WIZMESSAGEDATA& data)
{
    qDebug() << "create message, id: " << data.nId;

    CString strFormat = FormatInsertSQLFormat(TABLE_NAME_WIZ_MESSAGE,
                                              FIELD_LIST_WIZ_MESSAGE,
                                              PARAM_LIST_WIZ_MESSAGE);

    CString strSQL;
    strSQL.Format(strFormat,
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


    if (!ExecSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit messageCreated(data);
    }

    return true;
}

bool CWizIndexBase::modifyMessageEx(const WIZMESSAGEDATA& data)
{
    qDebug() << "modify message, id: " << data.nId;

    WIZMESSAGEDATA dataOld;
    messageFromId(data.nId, dataOld);

    CString strFormat = FormatUpdateSQLFormat(TABLE_NAME_WIZ_MESSAGE,
                                              FIELD_LIST_WIZ_MESSAGE_MODIFY,
                                              TABLE_KEY_WIZ_MESSAGE);

    CString strSQL;
    strSQL.Format(strFormat,
                  data.nReadStatus,
                  data.nDeleteStatus,
                  WizInt64ToStr(data.nVersion).utf16(),
                  data.nLocalChanged,
                  WizInt64ToStr(data.nId).utf16()
        );

    if (!ExecSQL(strSQL))
        return false;

    WIZMESSAGEDATA dataNew;
    messageFromId(data.nId, dataNew);

    if (!m_bUpdating) {
        emit messageModified(dataOld, dataNew);
    }

    return true;
}

bool CWizIndexBase::deleteMessageEx(const WIZMESSAGEDATA& data)
{
    qDebug() << "delete message, id: " << data.nId;

    CString strFormat = FormatDeleteSQLFormat(TABLE_NAME_WIZ_MESSAGE,
                                              TABLE_KEY_WIZ_MESSAGE);

    CString strSQL;
    strSQL.Format(strFormat,
        WizInt64ToStr(data.nId).utf16()
        );

    if (!ExecSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit messageDeleted(data);
    }

    return true;
}

bool CWizIndexBase::createUserEx(const WIZBIZUSER& data)
{
    qDebug() << "create user, alias: " << data.alias;

    Q_ASSERT(!data.bizGUID.isEmpty() && !data.userGUID.isEmpty());

    CString strFormat = FormatInsertSQLFormat(TABLE_NAME_WIZ_USER,
                                              FIELD_LIST_WIZ_USER,
                                              PARAM_LIST_WIZ_USER);

    CString strSQL;
    strSQL.Format(strFormat,
                  STR2SQL(data.bizGUID).utf16(),
                  STR2SQL(data.userId).utf16(),
                  STR2SQL(data.userGUID).utf16(),
                  STR2SQL(data.alias).utf16(),
                  STR2SQL(data.pinyin).utf16()
        );


    if (!ExecSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit userCreated(data);
    }

    return true;
}

bool CWizIndexBase::modifyUserEx(const WIZBIZUSER& user)
{
    qDebug() << "modify user, alias: " << user.alias;

    Q_ASSERT(!user.bizGUID.isEmpty() && !user.userGUID.isEmpty());

    // save old user info
    WIZBIZUSER userOld;
    userFromGUID(user.bizGUID, user.userGUID, userOld);

    CString strWhere = "BIZ_GUID=%1 AND USER_GUID=%2";
    strWhere = strWhere.arg(STR2SQL(user.bizGUID)).arg(STR2SQL(user.userGUID));

    CString strFormat = FormatUpdateSQLByWhere(TABLE_NAME_WIZ_USER,
                                              FIELD_LIST_WIZ_USER_MODIFY,
                                              strWhere);

    CString strSQL;
    strSQL.Format(strFormat,
                  STR2SQL(user.userId).utf16(),
                  STR2SQL(user.alias).utf16(),
                  STR2SQL(user.pinyin).utf16()
        );

    if (!ExecSQL(strSQL))
        return false;

    // read new user info
    WIZBIZUSER userNew;
    userFromGUID(user.bizGUID, user.userGUID, userNew);

    if (!m_bUpdating) {
        emit userModified(userOld, userNew);
    }

    return true;
}

bool CWizIndexBase::deleteUserEx(const WIZBIZUSER& data)
{
    qDebug() << "delete user, alias: " << data.alias;

    // TODO

    return true;
}

bool CWizIndexBase::CreateTagEx(const WIZTAGDATA& d)
{
    qDebug() << "create tag, name: " << d.strName;

    Q_ASSERT(d.strKbGUID == m_strKbGUID);

    WIZTAGDATA data = d;
    if (data.strGUID == data.strParentGUID) {
        data.strParentGUID.clear();
    }

    CString strFormat = FormatInsertSQLFormat(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, PARAM_LIST_WIZ_TAG);

    CString strSQL;
    strSQL.Format(strFormat,
        STR2SQL(data.strGUID).utf16(),
        STR2SQL(data.strParentGUID).utf16(),
        STR2SQL(data.strName).utf16(),
        STR2SQL(data.strDescription).utf16(),
        TIME2SQL(data.tModified).utf16(),
        WizInt64ToStr(data.nVersion).utf16(),
        WizInt64ToStr(data.nPostion).utf16()
        );


    if (!ExecSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit tagCreated(data);
    }

    return true;
}

bool CWizIndexBase::ModifyTagEx(const WIZTAGDATA& d)
{
    qDebug() << "modify tag, name: " << d.strName;

    Q_ASSERT(d.strKbGUID == m_strKbGUID);

    WIZTAGDATA dataOld;
    TagFromGUID(d.strGUID, dataOld);

    WIZTAGDATA data = d;
    if (data.strGUID == data.strParentGUID) {
        data.strParentGUID.Empty();
    }

    CString strFormat = FormatUpdateSQLFormat(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG_MODIFY, TABLE_KEY_WIZ_TAG);

    CString strSQL;
    strSQL.Format(strFormat,
        STR2SQL(data.strParentGUID).utf16(),
        STR2SQL(data.strName).utf16(),
        STR2SQL(data.strDescription).utf16(),
        TIME2SQL(data.tModified).utf16(),
        WizInt64ToStr(data.nVersion).utf16(),
        WizInt64ToStr(data.nPostion).utf16(),
        STR2SQL(data.strGUID).utf16()
        );

    if (!ExecSQL(strSQL))
        return false;

    WIZTAGDATA dataNew;
    TagFromGUID(d.strGUID, dataNew);

    if (!m_bUpdating) {
        emit tagModified(dataOld, dataNew);
    }

    return true;
}

bool CWizIndexBase::DeleteTagEx(const WIZTAGDATA& data)
{
    qDebug() << "delete tag, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    CString strFormat = FormatDeleteSQLFormat(TABLE_NAME_WIZ_TAG, TABLE_KEY_WIZ_TAG);

    CString strSQL;
    strSQL.Format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

    if (!ExecSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit tagDeleted(data);
    }

    return true;
}

bool CWizIndexBase::CreateStyleEx(const WIZSTYLEDATA& data)
{
    qDebug() << "create style, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    CString strFormat = FormatInsertSQLFormat(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE, PARAM_LIST_WIZ_STYLE);

    CString strSQL;
    strSQL.Format(strFormat,
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


    if (!ExecSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit styleCreated(data);
    }

    return true;
}

bool CWizIndexBase::ModifyStyleEx(const WIZSTYLEDATA& data)
{
    qDebug() << "modify style, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    WIZSTYLEDATA dataOld;
    StyleFromGUID(data.strGUID, dataOld);

    CString strFormat = FormatUpdateSQLFormat(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE_MODIFY, TABLE_KEY_WIZ_STYLE);

    CString strSQL;
    strSQL.Format(strFormat,
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

    if (!ExecSQL(strSQL))
        return false;

    WIZSTYLEDATA dataNew;
    StyleFromGUID(data.strGUID, dataNew);

    if (!m_bUpdating) {
        emit styleModified(dataOld, dataNew);
    }

    return true;
}

bool CWizIndexBase::DeleteStyleEx(const WIZSTYLEDATA& data)
{
    qDebug() << "delete style, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    CString strFormat = FormatDeleteSQLFormat(TABLE_NAME_WIZ_STYLE, TABLE_KEY_WIZ_STYLE);

    CString strSQL;
    strSQL.Format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

    if (!ExecSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit styleDeleted(data);
    }

    return true;
}

bool CWizIndexBase::CreateDocumentEx(const WIZDOCUMENTDATA& dataNew)
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
        data.strLocation = GetDefaultNoteLocation();
        TOLOG2("Document Location is empty: %1, Try to relocation to the %2", data.strTitle, data.strLocation);
    }

    CString strFormat = FormatInsertSQLFormat(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, PARAM_LIST_WIZ_DOCUMENT);

    CString strSQL;
    strSQL.Format(strFormat,
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

        data.nIconIndex,
        data.nSync,
        data.nProtected,
        data.nReadCount,
        data.nAttachmentCount,
        data.nIndexed,

        TIME2SQL(data.tInfoModified).utf16(),
        STR2SQL(data.strInfoMD5).utf16(),
        TIME2SQL(data.tDataModified).utf16(),
        STR2SQL(data.strDataMD5).utf16(),
        TIME2SQL(data.tParamModified).utf16(),
        STR2SQL(data.strParamMD5).utf16(),
        WizInt64ToStr(data.nVersion).utf16()
    );

    if (!ExecSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit documentCreated(data);
    }

    return true;
}

bool CWizIndexBase::ModifyDocumentInfoEx(const WIZDOCUMENTDATA& dataCur)
{
    qDebug() << "modify document, title: " << dataCur.strTitle;

    Q_ASSERT(dataCur.strKbGUID == m_strKbGUID);

    WIZDOCUMENTDATA dataOld;
    DocumentFromGUID(dataCur.strGUID, dataOld);

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
            data.strLocation = GetDefaultNoteLocation();
        }

        TOLOG2("Document Location is empty: %1, Try to relocation to the %2", data.strTitle, data.strLocation);
    }

    CString strFormat = FormatUpdateSQLFormat(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT_MODIFY, TABLE_KEY_WIZ_DOCUMENT);

    CString strSQL;
    strSQL.Format(strFormat,
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

        data.nIconIndex,
        data.nSync,
        data.nProtected,
        data.nReadCount,
        data.nAttachmentCount,
        data.nIndexed,

        TIME2SQL(data.tInfoModified).utf16(),
        STR2SQL(data.strInfoMD5 ).utf16(),
        TIME2SQL(data.tDataModified).utf16(),
        STR2SQL(data.strDataMD5 ).utf16(),
        TIME2SQL(data.tParamModified).utf16(),
        STR2SQL(data.strParamMD5 ).utf16(),
        WizInt64ToStr(data.nVersion).utf16(),

        STR2SQL(data.strGUID).utf16()
    );

    if (!ExecSQL(strSQL))
        return false;

    WIZDOCUMENTDATA dataNew;
    DocumentFromGUID(data.strGUID, dataNew);

    if (!m_bUpdating) {
        emit documentModified(dataOld, dataNew);
    }

    return true;
}

bool CWizIndexBase::DeleteDocumentEx(const WIZDOCUMENTDATA& data)
{
    qDebug() << "delete document, title: " << data.strTitle;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    CString strFormat = FormatDeleteSQLFormat(TABLE_NAME_WIZ_DOCUMENT, TABLE_KEY_WIZ_DOCUMENT);

    CString strSQL;
    strSQL.Format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

    if (!ExecSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit documentDeleted(data);
    }

    return true;
}

bool CWizIndexBase::CreateAttachmentEx(const WIZDOCUMENTATTACHMENTDATA& data)
{
    qDebug() << "create attachment, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    //CString strInfoMD5 = CalDocumentAttachmentInfoMD5(data);
    //if (strInfoMD5 != data.strInfoMD5) {
    //    TOLOG2(_T("Warning: Attachment info md5 does not match: %1, %2"), strInfoMD5, data.strInfoMD5);
    //}

    CString strFormat = FormatInsertSQLFormat(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT, PARAM_LIST_WIZ_DOCUMENT_ATTACHMENT);

    CString strSQL;
    strSQL.Format(strFormat,
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

    if (!ExecSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit attachmentCreated(data);
    }

    return true;
}

bool CWizIndexBase::ModifyAttachmentInfoEx(const WIZDOCUMENTATTACHMENTDATA& data)
{
    qDebug() << "modify attachment, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    WIZDOCUMENTATTACHMENTDATA dataOld;
    AttachmentFromGUID(data.strGUID, dataOld);

    CString strFormat = FormatUpdateSQLFormat(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT_MODIFY, TABLE_KEY_WIZ_DOCUMENT_ATTACHMENT);

    CString strSQL;
    strSQL.Format(strFormat,
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

    if (!ExecSQL(strSQL))
        return false;

    WIZDOCUMENTATTACHMENTDATA dataNew;
    AttachmentFromGUID(data.strGUID, dataNew);

    if (!m_bUpdating) {
        emit attachmentModified(dataOld, dataNew);
    }

    return true;
}

bool CWizIndexBase::DeleteAttachmentEx(const WIZDOCUMENTATTACHMENTDATA& data)
{
    qDebug() << "delete attachment, name: " << data.strName;

    Q_ASSERT(data.strKbGUID == m_strKbGUID);

    CString strFormat = FormatDeleteSQLFormat(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT,
                                              "ATTACHMENT_GUID");

    CString strSQL;
    strSQL.Format(strFormat,
        STR2SQL(data.strGUID).utf16()
        );

    if (!ExecSQL(strSQL))
        return false;

    if (!m_bUpdating) {
        emit attachmentDeleted(data);
    }

    return true;
}

bool CWizIndexBase::GetAllTags(CWizTagDataArray& arrayTag)
{
    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG);
    return SQLToTagDataArray(strSQL, arrayTag);
}

bool CWizIndexBase::GetRootTags(CWizTagDataArray& arrayTag)
{
    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG,
                                    _T("(TAG_GROUP_GUID = '' or TAG_GROUP_GUID is null)"));

    return SQLToTagDataArray(strSQL, arrayTag);
}

bool CWizIndexBase::GetChildTags(const CString& strParentTagGUID, CWizTagDataArray& arrayTag)
{
    CString strWhere = strParentTagGUID.isEmpty() ?
                       WizFormatString0(_T("TAG_GROUP_GUID is null")) :
                       WizFormatString1(_T("TAG_GROUP_GUID='%1'"), strParentTagGUID);

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, strWhere);
    return SQLToTagDataArray(strSQL, arrayTag);
}

bool CWizIndexBase::GetAllChildTags(const CString& strParentTagGUID, CWizTagDataArray& arrayTag)
{
    CWizTagDataArray arrayTagCurrent;
    GetChildTags(strParentTagGUID, arrayTagCurrent);

    arrayTag.insert(arrayTag.begin(), arrayTagCurrent.begin(), arrayTagCurrent.end());

    CWizTagDataArray::const_iterator it;
    for (it = arrayTagCurrent.begin(); it != arrayTagCurrent.end(); it++) {
        GetAllChildTags(it->strGUID, arrayTag);
    }

    return true;
}

bool CWizIndexBase::GetAllTagsWithErrorParent(CWizTagDataArray& arrayTag)
{
    CString strWhere = QString("TAG_GROUP_GUID is not null and TAG_GROUP_GUID "
                               "not in (select distinct TAG_GUID from %1)").arg(TABLE_NAME_WIZ_TAG);

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, strWhere);
    return SQLToTagDataArray(strSQL, arrayTag);
}

bool CWizIndexBase::GetChildTagsSize(const CString &strParentTagGUID, int &size)
{
    CString strWhere = strParentTagGUID.isEmpty() ?
                       WizFormatString0(_T("TAG_GROUP_GUID is null")) :
                       WizFormatString1(_T("TAG_GROUP_GUID='%1'"), strParentTagGUID);

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_TAG, "COUNT(*)", strWhere);
    return SQLToSize(strSQL, size);
}

bool CWizIndexBase::GetAllChildTagsSize(const CString& strParentTagGUID, int& size)
{
    int nSizeCurrent;
    GetChildTagsSize(strParentTagGUID, nSizeCurrent);
    size += nSizeCurrent;

    CWizTagDataArray arrayTagCurrent;
    GetChildTags(strParentTagGUID, arrayTagCurrent);

    CWizTagDataArray::const_iterator it;
    for (it = arrayTagCurrent.begin(); it != arrayTagCurrent.end(); it++) {
        GetAllChildTagsSize(it->strGUID, size);
    }

    return true;
}

bool CWizIndexBase::TagFromGUID(const CString& strTagGUID, WIZTAGDATA& data)
{
    if (!strTagGUID || !*strTagGUID) {
        TOLOG(_T("TagGUID is empty"));
        return false;
    }

    CString strWhere;
    strWhere.Format(_T("TAG_GUID=%s"),
        STR2SQL(strTagGUID).utf16()
        );

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_TAG, FIELD_LIST_WIZ_TAG, strWhere);

    CWizTagDataArray arrayTag;
    if (!SQLToTagDataArray(strSQL, arrayTag)) {
        TOLOG(_T("Failed to get tag by guid"));
        return false;
    }

    if (arrayTag.empty()) {
        //TOLOG(_T("Failed to get tag by guid, result is empty"));
        return false;
    }

    data = arrayTag[0];
    return true;
}

bool CWizIndexBase::GetStyles(CWizStyleDataArray& arrayStyle)
{
    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE);
    return SQLToStyleDataArray(strSQL, arrayStyle);
}

bool CWizIndexBase::StyleFromGUID(const CString& strStyleGUID, WIZSTYLEDATA& data)
{
    if (!strStyleGUID || !*strStyleGUID) {
        TOLOG(_T("StyleGUID is empty"));
        return false;
    }

    CString strWhere;
    strWhere.Format(_T("STYLE_GUID=%s"),
        STR2SQL(strStyleGUID).utf16()
        );

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_STYLE, FIELD_LIST_WIZ_STYLE, strWhere);

    CWizStyleDataArray arrayStyle;
    if (!SQLToStyleDataArray(strSQL, arrayStyle)) {
        TOLOG(_T("Failed to get style by guid"));
        return false;
    }

    if (arrayStyle.empty()) {
        //TOLOG(_T("Failed to get style by guid, result is empty"));
        return false;
    }

    data = arrayStyle[0];
    return true;
}

bool CWizIndexBase::GetMetas(CWizMetaDataArray& arrayMeta)
{
    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_META, FIELD_LIST_WIZ_META);
    return SQLToMetaDataArray(strSQL, arrayMeta);
}

bool CWizIndexBase::GetAllDocumentsSize(int& count, bool bIncludeTrash /* = false */)
{
    CString strSQL;
    if (bIncludeTrash) {
        strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, "COUNT(*)");
    } else {
        CString strWhere;
        strWhere.Format("DOCUMENT_LOCATION not like %s",
                        STR2SQL(QString(LOCATION_DELETED_ITEMS) + _T("%")).utf16());

        strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, "COUNT(*)", strWhere);
    }

    return SQLToSize(strSQL, count);
}

bool CWizIndexBase::GetAllDocuments(CWizDocumentDataArray& arrayDocument)
{
    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT);
    return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndexBase::GetDocumentsBySQLWhere(const CString& strSQLWhere, CWizDocumentDataArray& arrayDocument)
{
    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT, FIELD_LIST_WIZ_DOCUMENT, strSQLWhere);
    return SQLToDocumentDataArray(strSQL, arrayDocument);
}

bool CWizIndexBase::DocumentFromGUID(const CString& strDocumentGUID, WIZDOCUMENTDATA& data)
{
    if (!strDocumentGUID || !*strDocumentGUID) {
        TOLOG(_T("DocumentGUID is empty"));
        return false;
    }

    CString strWhere;
    strWhere.Format(_T("DOCUMENT_GUID=%s"),
        STR2SQL(strDocumentGUID).utf16()
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
    }

    data = arrayDocument[0];
    return true;
}

bool CWizIndexBase::DocumentWithExFieldsFromGUID(const CString& strDocumentGUID, WIZDOCUMENTDATA& data)
{
    if (DocumentFromGUID(strDocumentGUID, data))
    {
        CString strParamSQL = WizFormatString1(_T("select DOCUMENT_GUID, PARAM_NAME, PARAM_VALUE from WIZ_DOCUMENT_PARAM where (PARAM_NAME='DOCUMENT_FLAGS' or PARAM_NAME='RATE' or PARAM_NAME='SYSTEM_TAGS') and DOCUMENT_GUID = '%1'"), strDocumentGUID);

        CppSQLite3Query queryParam = m_db.execQuery(strParamSQL);
        while (!queryParam.eof())
        {
            CString strGUID = queryParam.getStringField(0);
            CString strParamName = queryParam.getStringField(1);

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

            queryParam.nextRow();
        }

        return true;
    }

    return false;
}

bool CWizIndexBase::GetAttachments(CWizDocumentAttachmentDataArray& arrayAttachment)
{
    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT);
    return SQLToDocumentAttachmentDataArray(strSQL, arrayAttachment);
}

bool CWizIndexBase::AttachmentFromGUID(const CString& strAttachcmentGUID,
                                       WIZDOCUMENTATTACHMENTDATA& data)
{
    if (strAttachcmentGUID.IsEmpty()) {
        TOLOG(_T("AttahcmentGUID is empty"));
        return false;
    }

    CString strWhere;
    strWhere.Format(_T("ATTACHMENT_GUID=%s"),
        STR2SQL(strAttachcmentGUID).utf16()
        );

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT, FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT, strWhere);

    CWizDocumentAttachmentDataArray arrayDocumentAttachment;
    if (!SQLToDocumentAttachmentDataArray(strSQL, arrayDocumentAttachment)) {
        TOLOG(_T("Failed to get attachment attachment by guid"));
        return false;
    }

    if (arrayDocumentAttachment.empty()) {
        //TOLOG(_T("Failed to get attachment by guid, result is empty"));
        return false;
    }

    data = arrayDocumentAttachment[0];
    return true;
}

bool CWizIndexBase::messageFromId(qint64 nId, WIZMESSAGEDATA& data)
{
    CString strWhere;
    strWhere.Format("MESSAGE_ID=%s", WizInt64ToStr(nId).utf16());

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_MESSAGE,
                                    FIELD_LIST_WIZ_MESSAGE,
                                    strWhere);

    CWizMessageDataArray arrayMessage;
    if (!SQLToMessageDataArray(strSQL, arrayMessage)) {
        TOLOG("[messageFromId] failed to get message by id");
        return false;
    }

    if (arrayMessage.empty())
        return false;

    data = arrayMessage[0];
    return true;
}

bool CWizIndexBase::messageFromUserGUID(const QString& userGUID, CWizMessageDataArray& arrayMessage)
{
    CString strWhere;
    strWhere.Format("SENDER_GUID=%s", STR2SQL(userGUID).utf16());

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_MESSAGE,
                                    FIELD_LIST_WIZ_MESSAGE,
                                    strWhere);

    if (!SQLToMessageDataArray(strSQL, arrayMessage)) {
        TOLOG1("[messageFromId] failed to get message by user guid : %1", userGUID);
        return false;
    }

    return !arrayMessage.empty();
}

bool CWizIndexBase::unreadMessageFromUserGUID(const QString& userGUID, CWizMessageDataArray& arrayMessage)
{
    CString strWhere;
    strWhere.Format("SENDER_GUID=%s and READ_STATUS=0", STR2SQL(userGUID).utf16());

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_MESSAGE,
                                    FIELD_LIST_WIZ_MESSAGE,
                                    strWhere);

    if (!SQLToMessageDataArray(strSQL, arrayMessage)) {
        TOLOG1("[messageFromId] failed to get unread message by user guid : %1", userGUID);
        return false;
    }

    return !arrayMessage.empty();
}

bool CWizIndexBase::messageFromDocumentGUID(const QString& strGUID, WIZMESSAGEDATA& data)
{
    CString strWhere;
    strWhere.Format("DOCUMENT_GUID=%s", STR2SQL(strGUID).utf16());

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_MESSAGE,
                                    FIELD_LIST_WIZ_MESSAGE,
                                    strWhere);

    CWizMessageDataArray arrayMessage;
    if (!SQLToMessageDataArray(strSQL, arrayMessage)) {
        TOLOG1("[messageFromId] failed to get message by document guid : %1", strGUID);
        return false;
    }

    if (arrayMessage.empty())
        return false;

    // FIXME: return the lastest message
    data = arrayMessage[0];
    return true;
}

bool CWizIndexBase::GetAllUsers(CWizBizUserDataArray& arrayUser)
{
    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_USER, FIELD_LIST_WIZ_USER);
    return SQLToBizUserDataArray(strSQL, arrayUser);
}

bool CWizIndexBase::userFromGUID(const QString& strUserGUID,
                                 CWizBizUserDataArray& arrayUser)
{
    CString strWhere;
    strWhere.Format("USER_GUID=%s", STR2SQL(strUserGUID).utf16());

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_USER,
                                    FIELD_LIST_WIZ_USER,
                                    strWhere);

    if (!SQLToBizUserDataArray(strSQL, arrayUser)) {
        TOLOG("[userFromGUID] failed to get user by user guid");
        return false;
    }

    if (arrayUser.empty())
        return false;

    return true;
}

bool CWizIndexBase::userFromGUID(const QString& strKbGUID,
                                 const QString& userGUID,
                                 WIZBIZUSER& user)
{
    CString strWhere = "BIZ_GUID=%1 AND USER_GUID=%2";
    strWhere = strWhere.arg(STR2SQL(strKbGUID)).arg(STR2SQL(userGUID));

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_USER,
                                    FIELD_LIST_WIZ_USER,
                                    strWhere);

    CWizBizUserDataArray arrayUser;
    if (!SQLToBizUserDataArray(strSQL, arrayUser)) {
        TOLOG("[userFromGUID] failed to get user by user guid");
        return false;
    }

    if (arrayUser.empty())
        return false;

    user = arrayUser[0];
    return true;
}

bool CWizIndexBase::users(const QString& strKbGUID, CWizBizUserDataArray& arrayUser)
{
    CString strWhere = "BIZ_GUID=%1";
    strWhere = strWhere.arg(STR2SQL(strKbGUID));

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_USER,
                                    FIELD_LIST_WIZ_USER,
                                    strWhere);

    if (!SQLToBizUserDataArray(strSQL, arrayUser)) {
        TOLOG("[users] failed to get users");
        return false;
    }

    //if (arrayUser.empty()) {
    //    qDebug() << "[users] should not be empty, right?";
    //    return false;
    //}

    return true;
}

bool CWizIndexBase::userFromID(const QString& strKbGUID, const QString& userID, WIZBIZUSER& user)
{
    CString strWhere = "BIZ_GUID=%1 AND USER_ID=%2";
    strWhere = strWhere.arg(STR2SQL(strKbGUID)).arg(STR2SQL(userID));

    CString strSQL = FormatQuerySQL(TABLE_NAME_WIZ_USER,
                                    FIELD_LIST_WIZ_USER,
                                    strWhere);

    CWizBizUserDataArray arrayUser;
    if (!SQLToBizUserDataArray(strSQL, arrayUser)) {
        TOLOG("[userFromGUID] failed to get user by user guid");
        return false;
    }

    if (arrayUser.empty())
        return false;

    user = arrayUser[0];
    return true;
}
