#ifndef WIZINDEXBASE_H
#define WIZINDEXBASE_H

#include <QObject>
#include <QMetaType>

#include "WizQtHelper.h"
#include "cppsqlite3.h"
#include "WizObject.h"

/*
Base class for database operation of sqlite layer
*/
class WizIndexBase : public QObject
{
    Q_OBJECT

public:
    WizIndexBase(void);
    ~WizIndexBase(void);

    virtual bool open(const CString& strFileName);
    bool isOpened();
    void close();
    bool checkTable(const QString& strTableName);
    bool execSQL(const CString& strSQL);
    int exec(const CString& strSQL);
    CppSQLite3Query Query(const CString& strSQL);
    bool hasRecord(const CString& strSQL);
    bool getFirstRowFieldValue(const CString& strSQL, int nFieldIndex, CString& strValue);
    bool repair(const QString& strDestFileName);

    QString kbGUID() const { return m_strKbGUID; }
    void setKbGUID(const QString& guid) { m_strKbGUID = guid; }

    QString getDatabasePath() const { return m_strFileName; }
    virtual QString getDefaultNoteLocation() const { return LOCATION_DEFAULT; }

    virtual bool setTableStructureVersion(const QString& strVersion) = 0;
    virtual QString getTableStructureVersion() = 0;
    bool updateTableStructure(int oldVersion);

    /* Raw query*/

    /* Tags */
    bool getAllTags(CWizTagDataArray& arrayTag);
    bool getAllTags(std::multimap<CString, WIZTAGDATA>& mapTag);
    bool getRootTags(CWizTagDataArray& arrayTag);
    bool getChildTags(const CString& strParentTagGUID, CWizTagDataArray& arrayTag); // 1 level
    bool getAllChildTags(const CString& strParentTagGUID, CWizTagDataArray& arrayTag);
    bool getAllTagsWithErrorParent(CWizTagDataArray& arrayTag);

    // used to test whether tag have child or not
    bool getChildTagsSize(const CString& strParentTagGUID, int& size); // 1 level
    bool getAllChildTagsSize(const CString& strParentTagGUID, int& size);

    bool tagFromGuid(const CString& strTagGUID, WIZTAGDATA& data);

    // styles
    bool getStyles(CWizStyleDataArray& arrayStyle);
    bool styleFromGuid(const CString& strStyleGUID, WIZSTYLEDATA& data);

    // metas
    bool getMetas(CWizMetaDataArray& arrayMeta);

    // documents
    bool getAllDocuments(CWizDocumentDataArray& arrayDocument);
    bool getDocumentsBySQLWhere(const CString& strSQLWhere, CWizDocumentDataArray& arrayDocument);
    bool documentFromGuid(const CString& strDocumentGUID, WIZDOCUMENTDATA& data);

    bool getAllDocumentsSize(int& count, bool bIncludeTrash = false);

    // attachments
    bool getAttachments(CWizDocumentAttachmentDataArray& arrayAttachment);
    bool attachmentFromGuid(const CString& strAttachcmentGUID, WIZDOCUMENTATTACHMENTDATA& data);

    // messages
    bool messageFromId(qint64 id, WIZMESSAGEDATA& data);
    bool messageFromUserGUID(const QString& userGUID, CWizMessageDataArray& arrayMessage);
    bool unreadMessageFromUserGUID(const QString& userGUID, CWizMessageDataArray& arrayMessage);
    bool messageFromDocumentGUID(const QString& strGUID, WIZMESSAGEDATA& data);

    // biz users, one user may in different biz group
    bool getAllUsers(CWizBizUserDataArray& arrayUser);
    bool userFromGUID(const QString& strUserGUID,
                      CWizBizUserDataArray &arrayUser);
    bool userFromGUID(const QString& strKbGUID,
                      const QString& userGUID,
                      WIZBIZUSER& user);
    bool users(const QString& strKbGUID, CWizBizUserDataArray& arrayUser);
    bool userFromID(const QString& strKbGUID,
                    const QString& userID,
                    WIZBIZUSER& user);

protected:
    CppSQLite3DB m_db;

private:
    QString m_strFileName;
    QString m_strKbGUID;
    bool m_bUpdating;
private:
    bool initDocumentExFields(CWizDocumentDataArray& arrayDocument, const CWizStdStringArray& arrayGUID, const std::map<QString, int>& mapDocumentIndex);
protected:
    bool logSQLException(const CppSQLite3Exception& e, const CString& strSQL);

    void beginUpdate() { m_bUpdating = true; }
    void endUpdate() { m_bUpdating = false; }
    bool isUpdating() const { return m_bUpdating; }

    static CString formatCanonicSQL(const CString& strTableName,
                                    const CString& strFieldList,
                                    const CString& strExt);

    static CString formatQuerySQL(const CString& strTableName,
                                  const CString& strFieldList);

    static CString formatQuerySQL(const CString& strTableName,
                                  const CString& strFieldList,
                                  const CString& strWhere);

    static CString formatInsertSQLFormat(const CString& strTableName,
                                         const CString& strFieldList,
                                         const CString& strParamList);

    static CString formatUpdateSQLFormat(const CString& strTableName,
                                         const CString& strFieldList,
                                         const CString& strKey);

    static CString formatUpdateSQLByWhere(const CString& strTableName,
                                          const CString& strFieldList,
                                          const CString& strWhere);

    static CString formatDeleteSQLFormat(const CString& strTableName,
                                         const CString& strKey);

    static CString formatDeleteSQLByWhere(const CString& strTableName,
                                         const CString& strWhere);

    static CString formatQuerySQLByTime(const CString& strTableName,
                                        const CString& strFieldList,
                                        const CString& strFieldName,
                                        const WizOleDateTime& t);

    static CString formatQuerySQLByTime2(const CString& strTableName,
                                         const CString& strFieldList,
                                         const CString& strInfoFieldName,
                                         const CString& strDataFieldName,
                                         const WizOleDateTime& t);

    static CString formatQuerySQLByTime3(const CString& strTableName,
                                         const CString& strFieldList,
                                         const CString& strInfoFieldName,
                                         const CString& strDataFieldName,
                                         const CString& strParamFieldName,
                                         const QDateTime &t);

    static CString formatModifiedQuerySQL(const CString& strTableName,
                                          const CString& strFieldList);

    static CString formatModifiedQuerySQL2(const CString& strTableName,
                                           const CString& strFieldList,
                                           int nCount);

    /* Basic operations */
    bool sqlToSize(const CString& strSQL, int& size);

    bool sqlToTagDataArray(const CString& strSQL,
                           CWizTagDataArray& arrayTag);

    bool sqlToStyleDataArray(const CString& strSQL,
                             CWizStyleDataArray& arrayStyle);

    bool sqlToMetaDataArray(const CString& strSQL,
                            CWizMetaDataArray& arrayMeta);

    bool sqlToDeletedGuidDataArray(const CString& strSQL,
                                   CWizDeletedGUIDDataArray& arrayGUID);

    bool sqlToStringArray(const CString& strSQL, int nFieldIndex,
                          CWizStdStringArray& arrayString);

    bool sqlToDocumentDataArray(const CString& strSQL,
                                CWizDocumentDataArray& arrayDocument);

    bool sqlToDocumentParamDataArray(const CString& strSQL,
                                CWizDocumentParamDataArray& arrayParam);

    bool sqlToDocumentAttachmentDataArray(const CString& strSQL,
                                          CWizDocumentAttachmentDataArray& arrayAttachment);

    bool sqlToMessageDataArray(const QString& strSQL,
                               CWizMessageDataArray& arrayMessage);

    bool sqlToBizUserDataArray(const QString& strSQL,
                               CWizBizUserDataArray& arrayUser);
    //

public:
    bool createMessageEx(const WIZMESSAGEDATA& data);
    bool modifyMessageEx(const WIZMESSAGEDATA& data);
    bool deleteMessageEx(const WIZMESSAGEDATA& data);

    bool createUserEx(const WIZBIZUSER& data);
    bool modifyUserEx(const WIZBIZUSER& data);
    bool deleteUserEx(const WIZBIZUSER& data);

    bool createTagEx(const WIZTAGDATA& data);
    bool modifyTagEx(const WIZTAGDATA& data);
    bool deleteTagEx(const WIZTAGDATA& data);

    bool createStyleEx(const WIZSTYLEDATA& data);
    bool modifyStyleEx(const WIZSTYLEDATA& data);
    bool deleteStyleEx(const WIZSTYLEDATA& data);

    bool createDocumentEx(const WIZDOCUMENTDATA& data);
    bool modifyDocumentInfoEx(const WIZDOCUMENTDATA& data);
    bool deleteDocumentEx(const WIZDOCUMENTDATA& data);

    bool createAttachmentEx(const WIZDOCUMENTATTACHMENTDATA& data);
    bool modifyAttachmentInfoEx(const WIZDOCUMENTATTACHMENTDATA& data);
    bool deleteAttachmentEx(const WIZDOCUMENTATTACHMENTDATA& data);
    //
    bool updateDocumentParam(const WIZDOCUMENTPARAMDATA& data);

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
    void documentTagModified(const WIZDOCUMENTDATA& document);
    void documentAccessDateModified(const WIZDOCUMENTDATA& document);
    void documentReadCountChanged(const WIZDOCUMENTDATA& document);

    void groupDocumentUnreadCountModified(const QString& strKbGUID);

    void attachmentCreated(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void attachmentModified(const WIZDOCUMENTATTACHMENTDATA& attachmentOld,
                            const WIZDOCUMENTATTACHMENTDATA& attachmentNew);
    void attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA& attachment);

    void documentParamModified(const WIZDOCUMENTPARAMDATA& param);

    void folderCreated(const QString& strLocation);
    void folderDeleted(const QString& strLocation);

    void messageCreated(const WIZMESSAGEDATA& msg);
    void messageModified(const WIZMESSAGEDATA& msgOld,
                         const WIZMESSAGEDATA& msgNew);
    void messageDeleted(const WIZMESSAGEDATA& msg);

    void userCreated(const WIZBIZUSER& user);
    void userModified(const WIZBIZUSER& userOld,
                      const WIZBIZUSER& userNew);
    void userDeleted(const WIZBIZUSER& user);
};


/* ------------------------------ WIZ_TAG ------------------------------ */
#define TABLE_NAME_WIZ_TAG  "WIZ_TAG"

#define FIELD_LIST_WIZ_TAG  "\
TAG_GUID, TAG_GROUP_GUID, TAG_NAME, TAG_DESCRIPTION, DT_MODIFIED, WIZ_VERSION, TAG_POS"

#define PARAM_LIST_WIZ_TAG  "%s, %s, %s, %s, %s, %s, %s"

#define FIELD_LIST_WIZ_TAG_MODIFY "\
TAG_GROUP_GUID=%s, TAG_NAME=%s, TAG_DESCRIPTION=%s, DT_MODIFIED=%s, \
WIZ_VERSION=%s, TAG_POS=%s"

#define TABLE_KEY_WIZ_TAG   "TAG_GUID"

#define FIELD_MODIFIED_WIZ_TAG  "DT_MODIFIED"

enum FieldIndex_WizTag
{
    tagTAG_GUID,
    tagTAG_GROUP_GUID,
    tagTAG_NAME,
    tagTAG_DESCRIPTION,
    tagDT_MODIFIED,
    tagVersion,
    tagTAG_POS
};

/* ------------------------------ WIZ_STYLE ------------------------------ */
#define TABLE_NAME_WIZ_STYLE    "WIZ_STYLE"

#define FIELD_LIST_WIZ_STYLE "\
STYLE_GUID, STYLE_NAME, STYLE_DESCRIPTION, STYLE_TEXT_COLOR, STYLE_BACK_COLOR, \
STYLE_TEXT_BOLD, STYLE_FLAG_INDEX, DT_MODIFIED, WIZ_VERSION"

#define PARAM_LIST_WIZ_STYLE    "%s, %s, %s, %s, %s, %d, %d, %s, %s"

#define FIELD_LIST_WIZ_STYLE_MODIFY "\
STYLE_NAME=%s, STYLE_DESCRIPTION=%s, STYLE_TEXT_COLOR=%s, STYLE_BACK_COLOR=%s, \
STYLE_TEXT_BOLD=%d, STYLE_FLAG_INDEX=%d, DT_MODIFIED=%s, WIZ_VERSION=%s"

#define TABLE_KEY_WIZ_STYLE "STYLE_GUID"
#define FIELD_MODIFIED_WIZ_STYLE    "DT_MODIFIED"

enum FieldIndex_WizStyle
{
    styleSTYLE_GUID,
    styleSTYLE_NAME,
    styleSTYLE_DESCRIPTION,
    styleSTYLE_TEXT_COLOR,
    styleSTYLE_BACK_COLOR,
    styleSTYLE_TEXT_BOLD,
    styleSTYLE_FLAG_INDEX,
    styleDT_MODIFIED,
    styleVersion
};

/* --------------------------- WIZ_DOCUMENT_TAG --------------------------- */
#define TABLE_NAME_WIZ_DOCUMENT_TAG "WIZ_DOCUMENT_TAG"
#define FIELD_LIST_WIZ_DOCUMENT_TAG "DOCUMENT_GUID, TAG_GUID"
#define PARAM_LIST_WIZ_DOCUMENT_TAG "%s, %s"


/* ------------------------------ WIZ_DOCUMENT ------------------------------ */
#define TABLE_NAME_WIZ_DOCUMENT "WIZ_DOCUMENT"

#define FIELD_LIST_WIZ_DOCUMENT "\
DOCUMENT_GUID, DOCUMENT_TITLE, DOCUMENT_LOCATION, DOCUMENT_NAME, DOCUMENT_SEO, \
DOCUMENT_URL, DOCUMENT_AUTHOR, DOCUMENT_KEYWORDS, DOCUMENT_TYPE, \
DOCUMENT_OWNER, DOCUMENT_FILE_TYPE, STYLE_GUID, DT_CREATED, DT_MODIFIED, \
DT_ACCESSED, DOCUMENT_ICON_INDEX, DOCUMENT_SYNC, DOCUMENT_PROTECT, \
DOCUMENT_READ_COUNT, DOCUMENT_ATTACHEMENT_COUNT, DOCUMENT_INDEXED, \
DT_INFO_MODIFIED, DOCUMENT_INFO_MD5, DT_DATA_MODIFIED, DOCUMENT_DATA_MD5, \
DT_PARAM_MODIFIED, DOCUMENT_PARAM_MD5, WIZ_VERSION, INFO_CHANGED, DATA_CHANGED"

#define PARAM_LIST_WIZ_DOCUMENT "\
%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %d, %d, %d, %d, \
%d, %d, %s, %s, %s, %s, %s, %s, %s, %d, %d"

#define FIELD_LIST_WIZ_DOCUMENT_INFO_MODIFY "\
DOCUMENT_TITLE=%s, DOCUMENT_LOCATION=%s, DOCUMENT_NAME=%s, DOCUMENT_SEO=%s, \
DOCUMENT_URL=%s, DOCUMENT_AUTHOR=%s, DOCUMENT_KEYWORDS=%s, DOCUMENT_TYPE=%s, \
DOCUMENT_OWNER=%s, DOCUMENT_FILE_TYPE=%s, STYLE_GUID=%s, DT_CREATED=%s, \
DT_MODIFIED=%s, DT_ACCESSED=%s, DOCUMENT_ICON_INDEX=%d, DOCUMENT_SYNC=%d, \
DOCUMENT_PROTECT=%d, DOCUMENT_READ_COUNT=%d, DOCUMENT_ATTACHEMENT_COUNT=%d, \
DOCUMENT_INDEXED=%d, DT_INFO_MODIFIED=%s, DOCUMENT_INFO_MD5=%s, WIZ_VERSION=%s, INFO_CHANGED=%d, DATA_CHANGED=%d"

#define FIELD_LIST_WIZ_DOCUMENT_MODIFY  "\
DOCUMENT_TITLE=%s, DOCUMENT_LOCATION=%s, DOCUMENT_NAME=%s, DOCUMENT_SEO=%s, \
DOCUMENT_URL=%s, DOCUMENT_AUTHOR=%s, DOCUMENT_KEYWORDS=%s, DOCUMENT_TYPE=%s, \
DOCUMENT_OWNER=%s, DOCUMENT_FILE_TYPE=%s, STYLE_GUID=%s, DT_CREATED=%s, \
DT_MODIFIED=%s, DT_ACCESSED=%s, DOCUMENT_ICON_INDEX=%d, DOCUMENT_SYNC=%d, \
DOCUMENT_PROTECT=%d, DOCUMENT_READ_COUNT=%d, DOCUMENT_ATTACHEMENT_COUNT=%d, \
DOCUMENT_INDEXED=%d, DT_INFO_MODIFIED=%s, DOCUMENT_INFO_MD5=%s, \
DT_DATA_MODIFIED=%s, DOCUMENT_DATA_MD5=%s, DT_PARAM_MODIFIED=%s, \
DOCUMENT_PARAM_MD5=%s, WIZ_VERSION=%s, INFO_CHANGED=%d, DATA_CHANGED=%d"

#define TABLE_KEY_WIZ_DOCUMENT  "DOCUMENT_GUID"
#define FIELD_INFO_MODIFIED_WIZ_DOCUMENT    "DT_INFO_MODIFIED"
#define FIELD_DATA_MODIFIED_WIZ_DOCUMENT    "DT_DATA_MODIFIED"
#define FIELD_PARAM_MODIFIED_WIZ_DOCUMENT   "DT_PARAM_MODIFIED"

enum FieldIndex_WizDocument
{
        documentDOCUMENT_GUID = 0,
        documentDOCUMENT_TITLE,
        documentDOCUMENT_LOCATION,
        documentDOCUMENT_NAME,
        documentDOCUMENT_SEO,
        documentDOCUMENT_URL,
        documentDOCUMENT_AUTHOR,
        documentDOCUMENT_KEYWORDS,
        documentDOCUMENT_TYPE,
        documentDOCUMENT_OWNER,
        documentDOCUMENT_FILE_TYPE,
        documentSTYLE_GUID,
        documentDT_CREATED,
        documentDT_MODIFIED,
        documentDT_ACCESSED,
        documentDOCUMENT_ICON_INDEX,
        documentDOCUMENT_SYNC,
        documentDOCUMENT_PROTECT,
        documentDOCUMENT_READ_COUNT,
        documentDOCUMENT_ATTACHEMENT_COUNT,
        documentDOCUMENT_INDEXED,
        documentDT_INFO_MODIFIED,
        documentDOCUMENT_INFO_MD5,
        documentDT_DATA_MODIFIED,
        documentDOCUMENT_DATA_MD5,
        documentDT_PARAM_MODIFIED,
        documentDOCUMENT_PARAM_MD5,
        documentVersion,
        documentINFO_CHANGED,
        documentDATA_CHANGED
};

/* ------------------------ WIZ_DOCUMENT_ATTACHMENT ------------------------ */
#define TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT  "WIZ_DOCUMENT_ATTACHMENT"

#define FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT  "\
ATTACHMENT_GUID, DOCUMENT_GUID, ATTACHMENT_NAME, ATTACHMENT_URL, \
ATTACHMENT_DESCRIPTION, DT_INFO_MODIFIED, ATTACHMENT_INFO_MD5, \
DT_DATA_MODIFIED, ATTACHMENT_DATA_MD5, WIZ_VERSION"

#define PARAM_LIST_WIZ_DOCUMENT_ATTACHMENT  "\
%s, %s, %s, %s, %s, %s, %s, %s, %s, %s"

#define FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT_INFO_MODIFY  "\
ATTACHMENT_NAME=%s, ATTACHMENT_URL=%s, ATTACHMENT_DESCRIPTION=%s, \
DT_INFO_MODIFIED=%s, ATTACHMENT_INFO_MD5=%s, WIZ_VERSION=%s"

#define FIELD_LIST_WIZ_DOCUMENT_ATTACHMENT_MODIFY   "\
ATTACHMENT_NAME=%s, ATTACHMENT_URL=%s, ATTACHMENT_DESCRIPTION=%s, \
DT_INFO_MODIFIED=%s, ATTACHMENT_INFO_MD5=%s, DT_DATA_MODIFIED=%s, \
ATTACHMENT_DATA_MD5=%s, WIZ_VERSION=%s"

#define TABLE_KEY_WIZ_DOCUMENT_ATTACHMENT   "ATTACHMENT_GUID"
#define FIELD_INFO_MODIFIED_WIZ_DOCUMENT_ATTACHMENT "DT_INFO_MODIFIED"
#define FIELD_DATA_MODIFIED_WIZ_DOCUMENT_ATTACHMENT "DT_DATA_MODIFIED"

        enum FieldIndex_WizDocumentAttachment
{
        documentattachmentATTACHMENT_GUID,
        documentattachmentDOCUMENT_GUID,
        documentattachmentATTACHMENT_NAME,
        documentattachmentATTACHMENT_URL,
        documentattachmentATTACHMENT_DESCRIPTION,
        documentattachmentDT_INFO_MODIFIED,
        documentattachmentATTACHMENT_INFO_MD5,
        documentattachmentDT_DATA_MODIFIED,
        documentattachmentATTACHMENT_DATA_MD5,
        documentattachmentVersion
        };

/* ------------------------ WIZ_DOCUMENT_PARAM ------------------------ */
#define TABLE_NAME_WIZ_DOCUMENT_PARAM   "WIZ_DOCUMENT_PARAM"
#define FIELD_LIST_WIZ_DOCUMENT_PARAM   "DOCUMENT_GUID, PARAM_NAME, PARAM_VALUE, WIZ_VERSION"
#define PARAM_LIST_WIZ_DOCUMENT_PARAM   "%s, %s, %s, %s"
#define FIELD_LIST_WIZ_DOCUMENT_PARAM_MODIFY    "PARAM_VALUE=%s"
#define TABLE_KEY_WIZ_DOCUMENT_PARAM_FLAGS  "DOCUMENT_FLAGS"

enum FieldIndex_WizDocumentParam
{
    documentparamDOCUMENT_GUID,
    documentparamPARAM_NAME,
    documentparamPARAM_VALUE
};

/* ------------------------ WIZ_DELETED_GUID ------------------------ */
#define TABLE_NAME_WIZ_DELETED_GUID "WIZ_DELETED_GUID"
#define FIELD_LIST_WIZ_DELETED_GUID "DELETED_GUID, GUID_TYPE, DT_DELETED"
#define PARAM_LIST_WIZ_DELETED_GUID "%s, %d, %s"
#define FIELD_MODIFIED_DELETED_GUID "DT_DELETED"

enum FieldIndex_WizDeletedGUID
{
    deletedguidDELETED_GUID,
    deletedguidGUID_TYPE,
    deletedguidDT_DELETED
};


/* ------------------------------ WIZ_META ------------------------------ */
#define TABLE_NAME_WIZ_META "WIZ_META"
#define FIELD_LIST_WIZ_META "META_NAME, META_KEY, META_VALUE, DT_MODIFIED"
#define PARAM_LIST_WIZ_META "%s, %s, %s, %s"

#define FIELD_LIST_WIZ_META_INFO_MODIFY "\
META_NAME=%s, META_KEY=%s, META_VALUE=%s, DT_MODIFIED=%s"

        enum FieldIndex_WizMeta
        {
            metaMETA_NAME,
            metaMETA_KEY,
            metaMETA_VALUE,
            metaDT_MODIFIED
        };

/* ------------------------------ WIZ_META ------------------------------ */
#define TABLE_NAME_WIZ_OBJECT_EX    "WIZ_OBJECT_EX"


/* ------------------------------ WIZ_MESSAGE ------------------------------ */
#define TABLE_NAME_WIZ_MESSAGE "WIZ_MESSAGE"

#define FIELD_LIST_WIZ_MESSAGE "\
MESSAGE_ID, BIZ_GUID, KB_GUID, DOCUMENT_GUID, SENDER, SENDER_ID, SENDER_GUID, \
RECEIVER, RECEIVER_ID, RECEIVER_GUID, MESSAGE_TYPE, READ_STATUS,\
DT_CREATED, MESSAGE_TITLE, MESSAGE_TEXT, WIZ_VERSION, DELETE_STATUS, LOCAL_CHANGED, MESSAGE_NOTE"

#define PARAM_LIST_WIZ_MESSAGE "\
%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %d, %d,%s, %s, %s, %s, %d, %d, %s"

#define FIELD_LIST_WIZ_MESSAGE_MODIFY "READ_STATUS=%d, DELETE_STATUS=%d, WIZ_VERSION=%s, LOCAL_CHANGED=%d"
#define TABLE_KEY_WIZ_MESSAGE   "MESSAGE_ID"

enum FieldIndex_WizMessage
{
    msgMESSAGE_ID,
    msgBIZ_GUID,
    msgKB_GUID,
    msgDOCUMENT_GUID,
    msgSENDER,
    msgSENDER_ID,
    msgSENDER_GUID,
    msgRECEIVER,
    msgRECEIVER_ID,
    msgRECEIVER_GUID,
    msgMESSAGE_TYPE,
    msgREAD_STATUS,
    msgDT_CREATED,
    msgMESSAGE_TITLE,
    msgMESSAGE_TEXT,
    msgWIZ_VERSION,
    msgDELETE_STATUS,
    msgLOCAL_CHANGED,
    msgMESSAGE_NOTE
};

/* ------------------------------ WIZ_USER ------------------------------ */
#define TABLE_NAME_WIZ_USER "WIZ_USER"

#define FIELD_LIST_WIZ_USER "\
BIZ_GUID, USER_ID, USER_GUID, USER_ALIAS, USER_PINYIN"

#define PARAM_LIST_WIZ_USER "%s, %s, %s, %s, %s"

#define FIELD_LIST_WIZ_USER_MODIFY "USER_ID=%s, USER_ALIAS=%s, USER_PINYIN=%s"

enum FieldIndex_WizUser
{
    userBIZ_GUID,
    userUSER_ID,
    userUSER_GUID,
    userUSER_ALIAS,
    userUSER_PINYIN
};


/* --------------------------------- TOTAL --------------------------------- */
#define TABLE_COUNT 11

const QString g_arrayTableName[TABLE_COUNT] =
{
    TABLE_NAME_WIZ_TAG,
    TABLE_NAME_WIZ_STYLE,
    TABLE_NAME_WIZ_DOCUMENT,
    TABLE_NAME_WIZ_DOCUMENT_ATTACHMENT,
    TABLE_NAME_WIZ_DOCUMENT_PARAM,
    TABLE_NAME_WIZ_DOCUMENT_TAG,
    TABLE_NAME_WIZ_DELETED_GUID,
    TABLE_NAME_WIZ_META,
    TABLE_NAME_WIZ_OBJECT_EX,
    TABLE_NAME_WIZ_MESSAGE,
    TABLE_NAME_WIZ_USER
};


#endif // WIZINDEXBASE_H
