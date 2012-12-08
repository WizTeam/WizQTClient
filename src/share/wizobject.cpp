#include "wizobject.h"

#include "wizxmlrpc.h"


//////////////////////////////////////////////////////////////////////////////
WIZUSERINFO::WIZUSERINFO()
    : nUserLevel(0)
    , nUserPoints(0)
    , nMaxFileSize(10 * 1024 * 1024)
    , bEnableGroup(false)
{

}

int WIZUSERINFO::GetMaxFileSize()
{
    return std::max<int>(20 * 1024 * 1024, nMaxFileSize);
}

bool WIZUSERINFO::LoadFromXmlRpc(CWizXmlRpcStructValue& val)
{
    //CWizXmlRpcStructValue* pStruct = dynamic_cast<CWizXmlRpcStructValue*>(&val);
    //if (!pStruct)
    //{
    //    TOLOG("Failed to cast CWizXmlRpcValue to CWizXmlRpcStructValue");
    //    return false;
    //}

    CWizXmlRpcStructValue& data = val;
    data.GetStr("token", strToken);
    data.GetTime("expried_time", tTokenExpried);
    data.GetStr("kapi_url", strDatabaseServer);
    data.GetStr("download_url", strDownloadDataServer);
    data.GetStr("upload_url", strUploadDataServer);
    data.GetStr("capi_url", strChatServer);
    data.GetStr("kb_guid", strKbGUID);
    data.GetInt("upload_size_limit", nMaxFileSize);
    data.GetStr("user_type", strUserType);
    data.GetStr("show_ad", strShowAD);
    data.GetStr("system_tags", strSystemTags);
    data.GetStr("push_tag", strPushTag);
    data.GetStr("user_level_name", strUserLevelName);
    data.GetInt("user_level", nUserLevel);
    data.GetInt("user_points", nUserPoints);
    data.GetStr("sns_list", strSNSList);
    data.GetInt("enable_group", bEnableGroup);
    data.GetStr("notice", strNotice);

    if (CWizXmlRpcStructValue* pUser = data.GetStruct("user"))
    {
        pUser->GetStr(_T("displayname"), strDisplayName);
        //pUser->GetStr(_T("nickname"), strNickName);
        pUser->GetStr(_T("language"), strLanguage);
        //pUser->GetStr(_T("backupserver"), strBackupDatabaseServer);
    }

    return !strToken.isEmpty()
        && !strKbGUID.isEmpty()
        && !strDatabaseServer.isEmpty();
}

WIZUSERCERT::WIZUSERCERT()
{
}

bool WIZUSERCERT::LoadFromXmlRpc(CWizXmlRpcStructValue& val)
{
    //CWizXmlRpcStructValue* pStruct = dynamic_cast<CWizXmlRpcStructValue*>(&val);
    //if (!pStruct)
    //{
    //    TOLOG("Failed to cast CWizXmlRpcValue to CWizXmlRpcStructValue while onGetUserCert");
    //    return false;
    //}

    CWizXmlRpcStructValue& data = val;
    data.GetStr("n", strN);
    data.GetStr("e", stre);
    data.GetStr("d", strd);
    data.GetStr("hint", strHint);

    return true;
}


WIZKBINFO::WIZKBINFO()
{
    nStorageLimit = 0;
    nStorageUsage = 0;
    nTrafficLimit = 0;
    nTrafficUsage = 0;
}

bool WIZKBINFO::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetInt64(_T("storage_limit"), nStorageLimit);
    data.GetInt64(_T("storage_usage"), nStorageUsage);
    data.GetStr(_T("storage_limit_string"), strStorageLimit);
    data.GetStr(_T("storage_usage_string"), strStorageUsage);
    //
    data.GetInt64(_T("traffic_limit"), nTrafficLimit);
    data.GetInt64(_T("traffic_usage"), nTrafficUsage);
    data.GetStr(_T("traffic_limit_string"), strTrafficLimit);
    data.GetStr(_T("traffic_usage_string"), strTrafficUsage);
    //
    return true;
}

///////////////////////////////////////////////////////////////////

WIZOBJECTPARTDATA::WIZOBJECTPARTDATA()
    : nStartPos(0)
    , nQuerySize(0)
    , nObjectSize(0)
    , bEOF(false)
    , nPartSize(0)
{
}

bool WIZOBJECTPARTDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetInt64(_T("obj_size"), nObjectSize);
    data.GetInt(_T("eof"), bEOF);
    data.GetInt64(_T("part_size"), nPartSize);
    data.GetString(_T("part_md5"), strPartMD5);
    if (!data.GetStream("data", arrayData))
    {
        TOLOG(_T("Fault error, data is null!"));
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////


WIZOBJECTDATA::WIZOBJECTDATA()
    : eObjectType(wizobjectError)
{

}

WIZOBJECTDATA::WIZOBJECTDATA(const WIZOBJECTDATA& data)
{
    strDisplayName = data.strDisplayName;
    strObjectGUID = data.strObjectGUID;
    tTime = data.tTime;
    eObjectType = data.eObjectType;
    arrayData = data.arrayData;
}

WIZOBJECTDATA::WIZOBJECTDATA(const WIZDOCUMENTDATA& data)
{
    strDisplayName = data.strTitle;
    strObjectGUID = data.strGUID;
    tTime = data.tDataModified;
    eObjectType = wizobjectDocument;
}

WIZOBJECTDATA::WIZOBJECTDATA(const WIZDOCUMENTATTACHMENTDATA& data)
{
    strDisplayName = data.strName;
    strObjectGUID = data.strGUID;
    tTime = data.tDataModified;
    eObjectType = wizobjectDocumentAttachment;
}


WizObjectType WIZOBJECTDATA::IntToObjectType(int n)
{
    if (n <= wizobjectError || n > wizobjectDocument)
    {
        ATLASSERT(FALSE);
        return wizobjectError;
    }

    return WizObjectType(n);
}

WizObjectType WIZOBJECTDATA::TypeStringToObjectType(const CString& strType)
{
    if (strType.IsEmpty())
    {
        TOLOG1(_T("Unknown guid type: %1"), strType);
        ATLASSERT(FALSE);
        return wizobjectError;
    }
    //
    if (0 == _tcsicmp(strType, _T("tag")))
        return wizobjectTag;
    if (0 == _tcsicmp(strType, _T("tag_group")))
        return wizobjectTag;
    else if (0 == _tcsicmp(strType, _T("style")))
        return wizobjectStyle;
    else if (0 == _tcsicmp(strType, _T("attachment")))
        return wizobjectDocumentAttachment;
    else if (0 == _tcsicmp(strType, _T("document")))
        return wizobjectDocument;
    //
    TOLOG1(_T("Unknown guid type: %1"), strType);
    ATLASSERT(FALSE);
    return wizobjectError;
}

CString WIZOBJECTDATA::ObjectTypeToTypeString(WizObjectType eType)
{
    switch (eType)
    {
    case wizobjectTag:
        return CString(_T("tag"));
    case wizobjectStyle:
        return CString(_T("style"));
    case wizobjectDocumentAttachment:
        return CString(_T("attachment"));
    case wizobjectDocument:
        return CString(_T("document"));
    default:
        TOLOG1(_T("Unknown guid type value: %1"), WizIntToStr(eType));
        ATLASSERT(FALSE);
        return CString();
    }
}

WIZTAGDATA::WIZTAGDATA()
    : nVersion(-1)
{
}

BOOL WIZTAGDATA::EqualForSync(const WIZTAGDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strName == data.strName
        && strDescription == data.strDescription
        && strParentGUID == data.strParentGUID;
}

BOOL WIZTAGDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    return data.GetStr(_T("tag_guid"), strGUID)
        && data.GetStr(_T("tag_group_guid"), strParentGUID)
        && data.GetStr(_T("tag_name"), strName)
        && data.GetStr(_T("tag_description"), strDescription)
        && data.GetTime(_T("dt_info_modified"), tModified)
        && data.GetInt64(_T("version"), nVersion);
}

BOOL WIZTAGDATA::SaveToXmlRpc(CWizXmlRpcStructValue& data) const
{
    data.AddString(_T("tag_guid"), strGUID);
    data.AddString(_T("tag_group_guid"), strParentGUID);
    data.AddString(_T("tag_name"), strName);
    data.AddString(_T("tag_description"), strDescription);
    data.AddTime(_T("dt_info_modified"), tModified);
    data.AddInt64(_T("version"), nVersion);;

    return TRUE;
}

bool operator< (const WIZTAGDATA& data1, const WIZTAGDATA& data2 ) throw()
{
    return( data1.strName.CompareNoCase( data2.strName) < 0 );
}

WIZTAGDATA::~WIZTAGDATA()
{
}

////////////////////////////////////////////////////////////////////

WIZSTYLEDATA::WIZSTYLEDATA()
{
    bTextBold = FALSE;
    nFlagIndex = 0;
    nVersion = -1;
}

BOOL WIZSTYLEDATA::EqualForSync(const WIZSTYLEDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strName == data.strName
            && strDescription == data.strDescription
            && crTextColor == data.crTextColor
            && crBackColor == data.crBackColor
            && bool(bTextBold ? true : false) == bool(data.bTextBold ? true : false)
            && nFlagIndex == data.nFlagIndex;
}

BOOL WIZSTYLEDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetStr(_T("style_description"), strDescription);

    return data.GetStr(_T("style_guid"), strGUID)
        && data.GetStr(_T("style_name"), strName)
        && data.GetColor(_T("style_textcolor"), crTextColor)
        && data.GetColor(_T("style_backcolor"), crBackColor)
        && data.GetBool(_T("style_text_bold"), bTextBold)
        && data.GetInt(_T("style_flagindex"), nFlagIndex)
        && data.GetTime(_T("dt_info_modified"), tModified)
        && data.GetInt64(_T("version"), nVersion);
}

BOOL WIZSTYLEDATA::SaveToXmlRpc(CWizXmlRpcStructValue& data) const
{
    data.AddString(_T("style_description"), strDescription);;

    data.AddString(_T("style_guid"), strGUID);
    data.AddString(_T("style_name"), strName);
    data.AddColor(_T("style_textcolor"), crTextColor);
    data.AddColor(_T("style_backcolor"), crBackColor);
    data.AddBool(_T("style_text_bold"), bTextBold);
    data.AddInt(_T("style_flagindex"), nFlagIndex);
    data.AddTime(_T("dt_info_modified"), tModified);
    data.AddInt64(_T("version"), nVersion);;

    return TRUE;
}

bool operator< (const WIZSTYLEDATA& data1, const WIZSTYLEDATA& data2 ) throw()
{
    return( data1.strName.CompareNoCase( data2.strName) < 0 );
}

////////////////////////////////////////////////////////////////////
WIZDOCUMENTDATABASE::WIZDOCUMENTDATABASE()
    : nVersion(-1)
    , nObjectPart(0)
{
}

BOOL WIZDOCUMENTDATABASE::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetStr(_T("document_guid"), strGUID);
    data.GetStr(_T("document_title"), strTitle);
    data.GetStr(_T("document_category"), strLocation);

    data.GetTime(_T("dt_info_modified"), tInfoModified);
    data.GetStr(_T("info_md5"), strInfoMD5);
    data.GetTime(_T("dt_data_modified"), tDataModified);
    data.GetStr(_T("data_md5"), strDataMD5);
    data.GetTime(_T("dt_param_modified"), tParamModified);
    data.GetStr(_T("param_md5"), strParamMD5);
    data.GetInt64(_T("version"), nVersion);

    return !strGUID.IsEmpty();
}

WIZDOCUMENTDATA::WIZDOCUMENTDATA()
    : WIZDOCUMENTDATABASE()
    , nIconIndex(0)
    , nSync(0)
    , nProtected(0)
    , nReadCount(0)
    , nAttachmentCount(0)
    , nIndexed(0)
    , nFlags(0)
    , nRate(0)
    , nShareFlags(0)
{
}

BOOL WIZDOCUMENTDATA::EqualForSync(const WIZDOCUMENTDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strInfoMD5 == data.strInfoMD5
            && strDataMD5 == data.strDataMD5
            && strParamMD5 == data.strParamMD5;
}

WIZDOCUMENTDATA::~WIZDOCUMENTDATA()
{
}

////////////////////////////////////////////////////////////////////

BOOL WIZDOCUMENTPARAMDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    return data.GetStr(_T("param_name"), strName)
        && data.GetStr(_T("param_value"), strValue);
}

BOOL WIZDOCUMENTPARAMDATA::SaveToXmlRpc(CWizXmlRpcStructValue& data) const
{
    data.AddString(_T("param_name"), strName);
    data.AddString(_T("param_value"), strValue);
    return TRUE;
}


////////////////////////////////////////////////////////////////////
WIZDELETEDGUIDDATA::WIZDELETEDGUIDDATA()
{
    eType = wizobjectError;
    nVersion = 0;
}

BOOL WIZDELETEDGUIDDATA::EqualForSync(const WIZDELETEDGUIDDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return TRUE;
}

BOOL WIZDELETEDGUIDDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    CString strType;
    //
    BOOL bRet = data.GetStr(_T("deleted_guid"), strGUID)
        && data.GetStr(_T("guid_type"), strType)
        && data.GetTime(_T("dt_deleted"), tDeleted)
        && data.GetInt64(_T("version"), nVersion);
    //
    eType = WIZOBJECTDATA::TypeStringToObjectType(strType);
    //
    return bRet;
}

BOOL WIZDELETEDGUIDDATA::SaveToXmlRpc(CWizXmlRpcStructValue& data) const
{
    data.AddString(_T("deleted_guid"), strGUID);
    data.AddString(_T("guid_type"), WIZOBJECTDATA::ObjectTypeToTypeString(eType));
    data.AddTime(_T("dt_deleted"), tDeleted);
    data.AddInt64(_T("version"), nVersion);
    //
    return TRUE;
}


////////////////////////////////////////////////////////////////////
WIZDOCUMENTATTACHMENTDATA::WIZDOCUMENTATTACHMENTDATA()
    : nVersion(-1)
{
}

BOOL WIZDOCUMENTATTACHMENTDATA::EqualForSync(const WIZDOCUMENTATTACHMENTDATA& data) const
{
    ATLASSERT(strGUID == data.strGUID);
    return strInfoMD5 == data.strInfoMD5
            && strDataMD5 == data.strDataMD5;
}

BOOL WIZDOCUMENTATTACHMENTDATA::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    data.GetStr(_T("attachment_guid"), strGUID);
    data.GetStr(_T("attachment_document_guid"), strDocumentGUID);
    data.GetStr(_T("attachment_name"), strName);
    data.GetStr(_T("attachment_url"), strURL);
    data.GetStr(_T("attachment_description"), strDescription);
    data.GetTime(_T("dt_info_modified"), tInfoModified);
    data.GetStr(_T("info_md5"), strInfoMD5);
    data.GetTime(_T("dt_data_modified"), tDataModified);
    data.GetStr(_T("data_md5"), strDataMD5);
    data.GetInt64(_T("version"), nVersion);

    return !strGUID.IsEmpty() && !strDocumentGUID.IsEmpty();
}

bool operator< (const WIZDOCUMENTATTACHMENTDATA& data1,const WIZDOCUMENTATTACHMENTDATA& data2 ) throw()
{
    return( data1.strName.CompareNoCase( data2.strName) < 0 );
}

WIZDOCUMENTATTACHMENTDATA::~WIZDOCUMENTATTACHMENTDATA()
{
}



////////////////////////////////////////////////////////////////////
WIZDOCUMENTDATAEX::WIZDOCUMENTDATAEX()
{
    bSkipped = false;
}

WIZDOCUMENTDATAEX::WIZDOCUMENTDATAEX(const WIZDOCUMENTDATA& data)
    : WIZDOCUMENTDATA(data)
{
    bSkipped = false;
}

WIZDOCUMENTDATAEX& WIZDOCUMENTDATAEX::operator= (const WIZDOCUMENTDATAEX& right)
{
    WIZDOCUMENTDATA::operator = (right);

    arrayTagGUID.assign(right.arrayTagGUID.begin(), right.arrayTagGUID.end());
    arrayParam.assign(right.arrayParam.begin(), right.arrayParam.end());
    arrayData = right.arrayData;

    bSkipped = right.bSkipped;

    return *this;
}

BOOL WIZDOCUMENTDATAEX::ParamArrayToStringArray(CWizStdStringArray& params) const
{
    for (std::deque<WIZDOCUMENTPARAMDATA>::const_iterator it = arrayParam.begin();
    it != arrayParam.end();
    it++)
    {
        params.push_back(it->strName + _T("=") + it->strValue);
    }

    return TRUE;
}

BOOL WIZDOCUMENTDATAEX::LoadFromXmlRpc(CWizXmlRpcStructValue& data)
{
    BOOL bInfo = FALSE;
    BOOL bData = FALSE;
    BOOL bParam = FALSE;
    data.GetBool(_T("document_info"), bInfo);
    data.GetBool(_T("document_data"), bData);
    data.GetBool(_T("document_param"), bParam);
    ATLASSERT(data.GetInt64(_T("version"), nVersion));
    ATLASSERT(!bData);
    //
    nObjectPart = 0;
    nObjectPart |= bInfo ? WIZKM_XMKRPC_DOCUMENT_PART_INFO : 0;
    nObjectPart |= bData ? WIZKM_XMKRPC_DOCUMENT_PART_DATA : 0;
    nObjectPart |= bParam ? WIZKM_XMKRPC_DOCUMENT_PART_PARAM : 0;
    //
    data.GetStr(_T("document_guid"), strGUID);
    //
    if (bInfo)
    {
        data.GetStr(_T("document_title"), strTitle);
        data.GetStr(_T("document_category"), strLocation);
        data.GetStr(_T("document_filename"), strName);
        data.GetStr(_T("document_seo"), strSEO);
        data.GetStr(_T("document_url"), strURL);
        data.GetStr(_T("document_author"), strAuthor);
        data.GetStr(_T("document_keywords"), strKeywords);
        data.GetStr(_T("document_type"), strType);
        data.GetStr(_T("document_owner"), strOwner);
        data.GetStr(_T("document_filetype"), strFileType);
        data.GetStr(_T("document_styleguid"), strStyleGUID);
        data.GetTime(_T("dt_created"), tCreated);
        data.GetTime(_T("dt_modified"), tModified);
        data.GetTime(_T("dt_accessed"), tAccessed);
        data.GetInt(_T("document_iconindex"), nIconIndex);
        data.GetInt(_T("document_protected"), nProtected);
        //data.GetInt(_T("document_readcount"), nReadCount);
        data.GetInt(_T("document_attachment_count"), nAttachmentCount);
        data.GetTime(_T("dt_info_modified"), tInfoModified);
        data.GetStr(_T("info_md5"), strInfoMD5);
        data.GetTime(_T("dt_data_modified"), tDataModified);
        data.GetStr(_T("data_md5"), strDataMD5);
        data.GetTime(_T("dt_param_modified"), tParamModified);
        data.GetStr(_T("param_md5"), strParamMD5);

        data.GetStringArray(_T("document_tags"), arrayTagGUID);

        data.GetStr(_T("system_tags"), strSystemTags);
    }

    if (bData)
    {
        ATLASSERT(FALSE);
    }

    if (bParam)
    {
        std::deque<WIZDOCUMENTPARAMDATA> params;
        if (!data.GetArray("document_params", params))
        {
            TOLOG(_T("Failed to get document param!"));
            return FALSE;
        }
        arrayParam.assign(params.begin(), params.end());
    }

    return !strGUID.IsEmpty();
}


////////////////////////////////////////////////////////////////////
WIZDOCUMENTATTACHMENTDATAEX::WIZDOCUMENTATTACHMENTDATAEX()
    : nObjectPart(0)
{
    bSkipped = false;
}

WIZDOCUMENTATTACHMENTDATAEX::WIZDOCUMENTATTACHMENTDATAEX(const WIZDOCUMENTATTACHMENTDATA& data)
    : WIZDOCUMENTATTACHMENTDATA(data)
    , nObjectPart(0)
{
    bSkipped = false;
}

WIZDOCUMENTATTACHMENTDATAEX& WIZDOCUMENTATTACHMENTDATAEX::operator= (const WIZDOCUMENTATTACHMENTDATAEX& right)
{
    WIZDOCUMENTATTACHMENTDATA::operator = (right);

    arrayData = right.arrayData;
    bSkipped = right.bSkipped;
    nObjectPart = right.nObjectPart;

    return *this;
}


////////////////////////////////////////////////////////////////////

WIZTODODATA::WIZTODODATA()
{
    nOrder = 0;
    bBlank = FALSE;
    eState = todoState0;
    ePrior = todoPriorNormal;
    //
    InitTime();
}

WIZTODODATA::WIZTODODATA(const CString& str, WIZTODOSTATE state, WIZTODOPRIOR prior)
    : strText(str)
    , eState(state)
    , ePrior(prior)
{
    nOrder = 0;
    bBlank = FALSE;
    //
    InitTime();
}
    //
void WIZTODODATA::InitTime()
{
    static int index = 0;
    index++;
    //
    tCreated = WizGetCurrentTime();
    tModified = WizGetCurrentTime();
    tCompleted = WizGetCurrentTime();
    //
    nOrder = WizTimeGetTimeT(tCreated);
    nOrder = nOrder * 1000;
    nOrder = nOrder + index;
}

void WIZTODODATA::AddLink(const CString& strDocumentGUID)
{
    arrayLinkedDocumentGUID.push_back(strDocumentGUID);
}

CString WIZTODODATA::GetLinkedDocumentGUIDString() const
{
    CString strText;
    WizStringArrayToText(arrayLinkedDocumentGUID, strText, _T(";"));
    return strText;
}

void WIZTODODATA::SetLinkedDocumentGUIDString(const CString& str)
{
    WizSplitTextToArray(str, ';', arrayLinkedDocumentGUID);
}

WIZTODODATAEX::WIZTODODATAEX()
{

}

WIZTODODATAEX::WIZTODODATAEX(const WIZTODODATA& data)
    : WIZTODODATA(data)
{

}

intptr_t WIZTODODATAEX::AddChild(const WIZTODODATAEX& data)
{
    arrayChild.push_back(data);
    return arrayChild.size() - 1;
}

BOOL WIZTODODATAEX::WizTodoDataArrayFindLinkedDocument(const CWizTodoDataExArray& arrayData, const CString& strDocumentGUID)
{
    for (CWizTodoDataExArray::const_iterator it = arrayData.begin();
    it != arrayData.end();
    it++)
    {
        if (-1 != WizFindInArrayNoCase(it->arrayLinkedDocumentGUID, strDocumentGUID))
            return TRUE;
        //
        if (WizTodoDataArrayFindLinkedDocument(it->arrayChild, strDocumentGUID))
            return TRUE;
    }
    //
    return FALSE;
}

intptr_t WIZTODODATAEX::WizTodoDataArrayFindText(const CWizTodoDataExArray& arrayData, const CString& strText)
{
    for (CWizTodoDataExArray::const_iterator it = arrayData.begin();
    it != arrayData.end();
    it++)
    {
        if (0 == it->strText.CompareNoCase(strText))
            return it - arrayData.begin();
    }
    //
    return -1;
}

BOOL WIZTODODATAEX::WizTodoDataItemCopyAndCombine(WIZTODODATAEX& itemDest, const WIZTODODATAEX& itemOther)
{
    BOOL bCombined = FALSE;
    //
    if (itemDest.eState < itemOther.eState)
    {
        itemDest.eState = itemOther.eState;
        bCombined = TRUE;
    }
    if (itemDest.tCompleted < itemOther.tCompleted)
    {
        itemDest.tCompleted = itemOther.tCompleted;
        bCombined = TRUE;
    }
    if (itemDest.tModified < itemOther.tModified)
    {
        itemDest.tModified = itemOther.tModified;
        bCombined = TRUE;
    }
    if (itemDest.tCreated < itemOther.tCreated)
    {
        itemDest.tCreated = itemOther.tCreated;
        bCombined = TRUE;
    }
    if (itemDest.arrayLinkedDocumentGUID != itemOther.arrayLinkedDocumentGUID)
    {
        itemDest.arrayLinkedDocumentGUID.insert(itemDest.arrayLinkedDocumentGUID.begin(), itemOther.arrayLinkedDocumentGUID.begin(), itemOther.arrayLinkedDocumentGUID.end());
        ::WizStringArrayRemoveMultiElement(itemDest.arrayLinkedDocumentGUID);
        bCombined = TRUE;
    }
    //
    if (WizTodoDataArrayCombine(itemDest.arrayChild, itemOther.arrayChild))
    {
        bCombined = TRUE;
    }
    //
    return bCombined;
}

BOOL WIZTODODATAEX::WizTodoDataArrayCombine(WIZTODODATAEX::CWizTodoDataExArray& arrayDest, const WIZTODODATAEX::CWizTodoDataExArray& arrayOther)
{
    BOOL bCombined = FALSE;
    //
    for (WIZTODODATAEX::CWizTodoDataExArray::const_iterator it = arrayOther.begin();
    it != arrayOther.end();
    it++)
    {
        intptr_t nIndex = WIZTODODATAEX::WizTodoDataArrayFindText(arrayDest, it->strText);
        //
        if (-1 == nIndex)
        {
            arrayDest.push_back(*it);
            bCombined = TRUE;
        }
        else
        {
            WIZTODODATAEX& itemDest = arrayDest[nIndex];
            const WIZTODODATAEX& itemOther = *it;
            //
            if (WizTodoDataItemCopyAndCombine(itemDest, itemOther))
            {
                bCombined = TRUE;
            }
        }
    }
    //
    return bCombined;
}

BOOL WIZTODODATAEX::WizTodoDataArrayRemoveMultiItem(WIZTODODATAEX::CWizTodoDataExArray& arrayData)
{
    BOOL bCombined = FALSE;
    //
    std::deque<int> arrayMultiIndex;
    //
    std::map<CString, int> mapTextAndIndex;
    for (WIZTODODATAEX::CWizTodoDataExArray::iterator it = arrayData.begin();
    it != arrayData.end();
    it++)
    {
        WIZTODODATAEX& dataCurr = *it;
        int nIndex = (int)(it - arrayData.begin());
        //
        CString strText = it->strText;
        strText.MakeLower();
        //
        std::map<CString, int>::const_iterator itMap = mapTextAndIndex.find(strText);
        if (itMap == mapTextAndIndex.end())
        {
            mapTextAndIndex[strText] = nIndex;
            //
            if (!dataCurr.arrayChild.empty())
            {
                bCombined = WizTodoDataArrayRemoveMultiItem(dataCurr.arrayChild);
            }
        }
        else
        {
            WIZTODODATAEX& dataDest = arrayData[itMap->second];
            WIZTODODATAEX::WizTodoDataItemCopyAndCombine(dataDest, dataCurr);
            //
            arrayMultiIndex.push_back(nIndex);
            //
            bCombined = TRUE;
        }
    }
    //
    if (arrayMultiIndex.empty())
        return bCombined;
    //
    size_t nMultiIndexCount = arrayMultiIndex.size();
    for (intptr_t i = nMultiIndexCount - 1; i >= 0; i--)
    {
        int nIndex = arrayMultiIndex[i];
        //
        arrayData.erase(arrayData.begin() + nIndex);
    }
    //
    return TRUE;
}

void WIZTODODATAEX::AddCompletedDate(const CString& strTextExt)
{
    CString strTime = WizFormatString1(_T("[%1]"), WizDateToLocalString(tCompleted));
    //
    strText = strTime + strTextExt + strText;
    //
    std::map<CString, int> mapTextAndIndex;
    for (WIZTODODATAEX::CWizTodoDataExArray::iterator it = arrayChild.begin();
    it != arrayChild.end();
    it++)
    {
        WIZTODODATAEX& dataCurr = *it;
        dataCurr.AddCompletedDate(strTextExt);
    }
}


COLORREF WizTodoGetTextColor(const WIZTODODATA& data)
{
    COLORREF cr = WIZ_TODO_TEXT_COLOR_DEFAULT;
    //
    if (todoState100 == data.eState)
    {
        cr = WIZ_TODO_TEXT_COLOR_DEFAULT_COMPLETED;
        switch (data.ePrior)
        {
        case todoPriorUrgent:
            cr = WIZ_TODO_TEXT_COLOR_URGENT_COMPLATED;
            break;
        case todoPriorUrgentAndImportant:
            cr = WIZ_TODO_TEXT_COLOR_URGENTANDIMPORTANT_COMPLETED;
            break;
        default:
            break;
        }
    }
    else
    {
        switch (data.ePrior)
        {
        case todoPriorUrgent:
            cr = WIZ_TODO_TEXT_COLOR_URGENT;
            break;
        case todoPriorUrgentAndImportant:
            cr = WIZ_TODO_TEXT_COLOR_URGENTANDIMPORTANTR;
            break;
        case todoPriorNotUrgentAndNotImportant:
            cr = WIZ_TODO_TEXT_COLOR_NOTURGENTANDNOTIMPORTANT;
            break;
        default:
            break;
        }
    }
    //
    return cr;
}

