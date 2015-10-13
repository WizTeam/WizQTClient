#include "wizxmlrpc.h"

#include <QUrl>

#include "wizmisc.h"
#include "../utils/logger.h"


bool WizXmlRpcValueFromXml(CWizXMLNode& nodeValue, CWizXmlRpcValue** ppRet)
{
    *ppRet = NULL;

    CWizXMLNode nodeTypeTest;
    nodeValue.GetFirstChildNode(nodeTypeTest);
    if (nodeTypeTest.isNull()) {
        QString strText = nodeValue.GetText();
        *ppRet = new CWizXmlRpcStringValue(strText);
        return true;
    }

    CWizXmlRpcValue* pValue = NULL;
    CString strValueType = nodeTypeTest.GetName();

    if (0 == strValueType.CompareNoCase("int")
        || 0 == strValueType.CompareNoCase("i4"))
    {
        pValue = new CWizXmlRpcIntValue();
    }
    else if (0 == strValueType.CompareNoCase("string")
        || 0 == strValueType.CompareNoCase("ex:nil")
        || 0 == strValueType.CompareNoCase("ex:i8"))
    {
        pValue = new CWizXmlRpcStringValue();
    }
    else if (0 == strValueType.CompareNoCase("nil")
        || 0 == strValueType.CompareNoCase("i8"))
    {
        pValue = new CWizXmlRpcStringValue();
    }
    else if (0 == strValueType.CompareNoCase("struct"))
    {
        pValue = new CWizXmlRpcStructValue();
    }
    else if (0 == strValueType.CompareNoCase("array"))
    {
        pValue = new CWizXmlRpcArrayValue();
    }
    else if (0 == strValueType.CompareNoCase("base64"))
    {
        pValue = new CWizXmlRpcBase64Value();
    }
    else if (0 == strValueType.CompareNoCase("boolean")
        || 0 == strValueType.CompareNoCase("bool"))
    {
        pValue = new CWizXmlRpcBoolValue();
    }
    else if (0 == strValueType.CompareNoCase("dateTime.iso8601"))
    {
        pValue = new CWizXmlRpcTimeValue();
    }
    else
    {
        TOLOG1("Unknown xmlrpc value type:%1", strValueType);
        return false;
    }

    Q_ASSERT(pValue);

    if (pValue->Read(nodeValue))
    {
        *ppRet = pValue;
        return true;
    }

    TOLOG("Failed to read xmlrpc value!");
    delete pValue;

    return false;
}

bool WizXmlRpcResultFromXml(CWizXMLDocument& doc, CWizXmlRpcValue** ppRet)
{
    CWizXMLNode nodeMethodResponse;
    doc.FindChildNode("methodResponse", nodeMethodResponse);
    if (nodeMethodResponse.isNull())
    {
        TOLOG("Failed to get methodResponse node!");
        return false;
    }

    CWizXMLNode nodeTest;
    if (!nodeMethodResponse.GetFirstChildNode(nodeTest))
    {
        TOLOG("Failed to get methodResponse child node!");
        return false;
    }

    QString strTestName = nodeTest.GetName();

    if (0 == strTestName.compare("params", Qt::CaseInsensitive))
    {
        CWizXMLNode nodeParamValue;
        nodeTest.FindNodeByPath("param/value", nodeParamValue);
        if (nodeParamValue.isNull())
        {
            TOLOG("Failed to get param value node of params!");
            return false;
        }

        return WizXmlRpcValueFromXml(nodeParamValue, ppRet);
    }
    else if (0 == strTestName.compare("fault", Qt::CaseInsensitive))
    {
        CWizXMLNode nodeFaultValue;
        nodeTest.FindChildNode("value", nodeFaultValue);
        if (nodeFaultValue.isNull())
        {
            TOLOG("Failed to get fault value node!");
            return false;
        }

        CWizXmlRpcFaultValue* pFault = new CWizXmlRpcFaultValue();
        pFault->Read(nodeFaultValue);

        *ppRet = pFault;

        return true;
    }
    else
    {
        TOLOG1("Unknown response node name: %1", strTestName);
        return false;
    }
}


/* ------------------------- CWizXmlRpcIntValue ------------------------- */
CWizXmlRpcRequest::CWizXmlRpcRequest(const QString& strMethodName)
{
    CWizXMLNode nodeMethodCall;
    m_doc.AppendChild("methodCall", nodeMethodCall);
    nodeMethodCall.SetChildNodeText("methodName", strMethodName);

    CWizXMLNode nodeParams;
    nodeMethodCall.AppendChild("params", nodeParams);
}

void CWizXmlRpcRequest::addParam(CWizXmlRpcValue* pParam)
{
    Q_ASSERT(pParam);

    CWizXMLNode nodeParams;
    m_doc.FindNodeByPath("methodCall/params", nodeParams);

    CWizXMLNode nodeParamValue;
    nodeParams.AppendNodeByPath("param/value", nodeParamValue);

    pParam->Write(nodeParamValue);
}

QByteArray CWizXmlRpcRequest::toData()
{
    QString strText;
    if (!m_doc.ToXML(strText, true)) {
        TOLOG("Failed to get xml text!");
        return QByteArray();
    }

    if (strText.length() < 10) {
        TOLOG1("Invalidate xml: %1", strText);
        return QByteArray();
    }

    if (strText[0] == '<' && strText[1] == '?')
    {
    }
    else
    {
        strText.insert(0, "<?xml version=\"1.0\"?>\n");
    }

    return strText.toUtf8();
}


/* ------------------------- CWizXmlRpcIntValue ------------------------- */
CWizXmlRpcIntValue::CWizXmlRpcIntValue(int n /* = 0 */)
    : m_n(n)
{
}

bool CWizXmlRpcIntValue::Write(CWizXMLNode& nodeValue)
{
    nodeValue.SetChildNodeText("int", QString::number(m_n));
    return true;
}

bool CWizXmlRpcIntValue::Read(CWizXMLNode& nodeValue)
{
    QString strValue = nodeValue.GetFirstChildNodeText();
    m_n = strValue.toInt();
    return true;
}

QString CWizXmlRpcIntValue::ToString() const
{
    return QString::number(m_n);
}

CWizXmlRpcIntValue::operator int()
{
	return m_n;
}


/* ------------------------- CWizXmlRpcBoolValue ------------------------- */
CWizXmlRpcBoolValue::CWizXmlRpcBoolValue(bool b /* = false */)
    : m_b(b)
{
}

bool CWizXmlRpcBoolValue::Write(CWizXMLNode& nodeValue)
{
    nodeValue.SetChildNodeText("boolean", m_b ? "1" : "0");
    return true;
}

bool CWizXmlRpcBoolValue::Read(CWizXMLNode& nodeValue)
{
    QString strValue = nodeValue.GetFirstChildNodeText();
    m_b = (strValue == "1" || 0 == strValue.compare("true", Qt::CaseInsensitive));
    return true;
}

QString CWizXmlRpcBoolValue::ToString() const
{
    return m_b ? "1" : "0";
}

CWizXmlRpcBoolValue::operator bool()
{
	return m_b;
}


/* ------------------------- CWizXmlRpcStringValue ------------------------- */
CWizXmlRpcStringValue::CWizXmlRpcStringValue(const QString& strDef /* = "" */)
    : m_str(strDef)
{
}

bool CWizXmlRpcStringValue::Write(CWizXMLNode& nodeValue)
{
    nodeValue.SetChildNodeText("string", m_str);
    return true;
}

bool CWizXmlRpcStringValue::Read(CWizXMLNode& nodeValue)
{
	m_str = nodeValue.GetFirstChildNodeText();
    return true;
}

QString CWizXmlRpcStringValue::ToString() const
{
    return m_str;
}

CWizXmlRpcStringValue::operator QString()
{
	return m_str;
}


/* ------------------------- CWizXmlRpcTimeValue ------------------------- */
CWizXmlRpcTimeValue::CWizXmlRpcTimeValue()
    : m_t(::WizGetCurrentTime())
{
}

CWizXmlRpcTimeValue::CWizXmlRpcTimeValue(const COleDateTime& t)
	: m_t(t)
{
}

bool CWizXmlRpcTimeValue::Write(CWizXMLNode& nodeValue)
{
    QString str = WizDateTimeToIso8601String(m_t);
    nodeValue.SetChildNodeText("dateTime.iso8601", str);
    return true;
}

bool CWizXmlRpcTimeValue::Read(CWizXMLNode& nodeValue)
{
    CString str = nodeValue.GetFirstChildNodeText();
    CString strError;
    if (!WizIso8601StringToDateTime(str, m_t, strError)) {
		TOLOG(strError);
        return false;
	}

    return true;
}

QString CWizXmlRpcTimeValue::ToString() const
{
	return ::WizDateTimeToString(m_t);
}

CWizXmlRpcTimeValue::operator COleDateTime()
{
	return m_t;
}


/* ------------------------- CWizXmlRpcBase64Value ------------------------- */
CWizXmlRpcBase64Value::CWizXmlRpcBase64Value(const QByteArray& arrayData)
    : m_arrayData(arrayData)
{
}

bool CWizXmlRpcBase64Value::Write(CWizXMLNode& nodeValue)
{
    QString strText;
    WizBase64Encode(m_arrayData, strText);
    nodeValue.SetChildNodeText("base64", strText);
    return true;
}

bool CWizXmlRpcBase64Value::Read(CWizXMLNode& nodeValue)
{
    QString str = nodeValue.GetFirstChildNodeText();
    m_arrayData.clear();;
    return WizBase64Decode(str, m_arrayData);
}

QString CWizXmlRpcBase64Value::ToString() const
{
    QString strText;
    WizBase64Encode(m_arrayData, strText);
    return strText;
}

bool CWizXmlRpcBase64Value::GetStream(QByteArray& arrayData)
{
    arrayData = m_arrayData;
    return true;
}


/* ------------------------- CWizXmlRpcArrayValue ------------------------- */
CWizXmlRpcArrayValue::CWizXmlRpcArrayValue()
{
}

CWizXmlRpcArrayValue::CWizXmlRpcArrayValue(const CWizStdStringArray& arrayData)
{
	SetStringArray(arrayData);
}

CWizXmlRpcArrayValue::~CWizXmlRpcArrayValue ()
{
	Clear();
}

bool CWizXmlRpcArrayValue::Write(CWizXMLNode& nodeValue)
{
	CWizXMLNode nodeData;
    nodeValue.AppendNodeByPath("array/data", nodeData);

    std::deque<CWizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++)
	{
		CWizXmlRpcValue* pValue = *it;

		CWizXMLNode nodeElementValue;
        nodeData.AppendChild("value", nodeElementValue);
		pValue->Write(nodeElementValue);
	}

    return true;
}

bool CWizXmlRpcArrayValue::Read(CWizXMLNode& nodeValue)
{
	CWizXMLNode nodeData;
    if (!nodeValue.FindNodeByPath("array/data", nodeData))
	{
        TOLOG("Failed to get array data node!");
        return false;
	}

    std::deque<CWizXMLNode> arrayValue;
	nodeData.GetAllChildNodes(arrayValue);

    std::deque<CWizXMLNode>::iterator it;
    for (it = arrayValue.begin(); it != arrayValue.end(); it++)
	{
		CWizXMLNode& nodeElementValue = *it;

		CWizXmlRpcValue* pElementValue = NULL;
        if (!WizXmlRpcValueFromXml(nodeElementValue, &pElementValue))
		{
            TOLOG("Failed to load array element value from node!");
            return false;
		}

        Q_ASSERT(pElementValue);
		Add(pElementValue);
	}

    return true;
}

QString CWizXmlRpcArrayValue::ToString() const
{
    QString str;

    std::deque<CWizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++)
    {
        str += (*it)->ToString();

        if (it != m_array.end() - 1) {
            str += ", ";
        }
    }

    return str;
}

void CWizXmlRpcArrayValue::Add(CWizXmlRpcValue* pValue)
{
	m_array.push_back(pValue);
}

void CWizXmlRpcArrayValue::Clear()
{
    std::deque<CWizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++)
	{
		delete *it;
    }

	m_array.clear();
}

bool CWizXmlRpcArrayValue::ToStringArray(CWizStdStringArray& arrayRet)
{
    std::deque<CWizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++)
	{
		CWizXmlRpcStringValue* pValue = dynamic_cast<CWizXmlRpcStringValue*>(*it);
		if (!pValue)
		{
            TOLOG("Fault error: element of array is null or not a string");
            return false;
		}

        QString str = *pValue;
        arrayRet.push_back(str);
	}

    return true;
}

bool CWizXmlRpcArrayValue::SetStringArray(const CWizStdStringArray& arrayData)
{
    CWizStdStringArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++)
	{
		Add(new CWizXmlRpcStringValue(*it));
    }

    return true;
}


/* ------------------------- CWizXmlRpcStructValue ------------------------- */
void CWizXmlRpcStructValue::AddValue(const QString& strName, CWizXmlRpcValue* pValue)
{
    RemoveValue(strName);
    m_map[strName] = pValue;
}

CWizXmlRpcValue* CWizXmlRpcStructValue::GetMappedValue(const QString& strName) const
{
    std::map<QString, CWizXmlRpcValue*>::const_iterator it = m_map.find(strName);
	if (it == m_map.end())
		return NULL;

	return it->second;
}

CWizXmlRpcStructValue::~CWizXmlRpcStructValue()
{
	Clear();
}

bool CWizXmlRpcStructValue::Write(CWizXMLNode& nodeValue)
{
	CWizXMLNode nodeStruct;
    nodeValue.AppendChild("struct", nodeStruct);

    std::map<QString, CWizXmlRpcValue*>::const_iterator it;
    for (it = m_map.begin(); it != m_map.end(); it++) {
        QString strName = it->first;
		CWizXmlRpcValue* pValue = it->second;

		CWizXMLNode nodeMember;
        nodeStruct.AppendChild("member", nodeMember);

        nodeMember.SetChildNodeText("name", strName);

		CWizXMLNode nodeElementValue;
        nodeMember.AppendChild("value", nodeElementValue);

		pValue->Write(nodeElementValue);
	}

    return true;
}

bool CWizXmlRpcStructValue::Read(CWizXMLNode& nodeValue)
{
	CWizXMLNode nodeStruct;
    if (!nodeValue.FindChildNode("struct", nodeStruct)) {
        TOLOG("Failed to get struct node!");
        return false;
	}

    std::deque<CWizXMLNode> arrayMember;
	nodeStruct.GetAllChildNodes(arrayMember);

    std::deque<CWizXMLNode>::iterator it;
    for (it = arrayMember.begin(); it != arrayMember.end(); it++)
    {
		CWizXMLNode& nodeMember = *it;

        QString strName;
        if (!nodeMember.GetChildNodeText("name", strName))
        {
            TOLOG("Failed to get struct member name!");
            return false;
		}

		CWizXMLNode nodeMemberValue;
        if (!nodeMember.FindChildNode("value", nodeMemberValue))
        {
            TOLOG("Failed to get struct member value!");
            return false;
		}

		CWizXmlRpcValue* pMemberValue = NULL;
        if (!WizXmlRpcValueFromXml(nodeMemberValue, &pMemberValue))
		{
            TOLOG("Failed to load struct member value from node!");
            return false;
		}

        Q_ASSERT(pMemberValue);

		AddValue(strName, pMemberValue);
	}

    return true;
}

QString CWizXmlRpcStructValue::ToString() const
{
    QString str;

    std::map<QString, CWizXmlRpcValue*>::const_iterator it;
    for (it = m_map.begin(); it != m_map.end(); it++)
    {
        QString strKeyValue;
        CWizXmlRpcStructValue* p = dynamic_cast<CWizXmlRpcStructValue*>(it->second);
        if (p) {
            strKeyValue = "{" + it->first + ":" + it->second->ToString() + "}";
            str += strKeyValue;
            continue;
        }

        CWizXmlRpcArrayValue* p2 = dynamic_cast<CWizXmlRpcArrayValue*>(it->second);
        if (p2) {
            strKeyValue = "[" + it->first + ":" + it->second->ToString() + "]";
            str += strKeyValue;
            continue;
        }

        strKeyValue = "(" + it->first + ":" + it->second->ToString() + ")";
        str += strKeyValue;
    }

    return "{" + str + "}\n";
}

void CWizXmlRpcStructValue::AddInt(const QString& strName, int n)
{
    AddValue(strName, new CWizXmlRpcIntValue(n));
}

void CWizXmlRpcStructValue::AddString(const QString& strName, const QString& str)
{
    AddValue(strName, new CWizXmlRpcStringValue(str));
}

void CWizXmlRpcStructValue::AddBool(const QString& strName, bool b)
{
    AddValue(strName, new CWizXmlRpcBoolValue(b));
}

void CWizXmlRpcStructValue::AddTime(const QString& strName, const COleDateTime& t)
{
    AddValue(strName, new CWizXmlRpcTimeValue(t));
}

void CWizXmlRpcStructValue::AddBase64(const QString& strName, const QByteArray& arrayData)
{
    AddValue(strName, new CWizXmlRpcBase64Value(arrayData));
}

void CWizXmlRpcStructValue::AddStringArray(const QString& strName, const CWizStdStringArray& arrayData)
{
    AddValue(strName, new CWizXmlRpcArrayValue(arrayData));
}

void CWizXmlRpcStructValue::AddInt64(const QString& strName, __int64 n)
{
    AddValue(strName, new CWizXmlRpcStringValue(WizInt64ToStr(n)));
}

void CWizXmlRpcStructValue::AddColor(const QString& strName, COLORREF cr)
{
    AddValue(strName, new CWizXmlRpcStringValue(WizColorToString(cr)));
}

void CWizXmlRpcStructValue::AddStruct(const QString& strName, CWizXmlRpcStructValue* pStruct)
{
    AddValue(strName, pStruct);
}

void CWizXmlRpcStructValue::AddArray(const QString& strName, CWizXmlRpcValue* pArray)
{
    AddValue(strName, pArray);
}

void CWizXmlRpcStructValue::Clear()
{
    std::map<QString, CWizXmlRpcValue*>::const_iterator it;
    for (it = m_map.begin(); it != m_map.end(); it++) {
		DeleteValue(it->second);
	}

	m_map.clear();
}

void CWizXmlRpcStructValue::RemoveValue(const QString& strName)
{
    std::map<QString, CWizXmlRpcValue*>::iterator it = m_map.find(strName);
	if (it == m_map.end())
		return;

	DeleteValue(it->second);
	m_map.erase(it);
}

void CWizXmlRpcStructValue::DeleteValue(CWizXmlRpcValue* pValue)
{
	delete pValue;
}

bool CWizXmlRpcStructValue::GetBool(const QString& strName, bool& b) const
{
    CWizXmlRpcBoolValue* p = GetValue<CWizXmlRpcBoolValue>(strName);
	if (!p)
        return false;

	b = *p;
    return true;
}

bool CWizXmlRpcStructValue::GetInt(const QString& strName, int& n) const
{
    if (CWizXmlRpcIntValue* p = GetValue<CWizXmlRpcIntValue>(strName)) {
		n = *p;
        return true;
	}

    if (CWizXmlRpcStringValue* p = GetValue<CWizXmlRpcStringValue>(strName)) {
        n = QString(*p).toInt();
        return true;
    }

    return false;
}

bool CWizXmlRpcStructValue::GetString(const QString& strName, QString& str) const
{
    if (CWizXmlRpcIntValue* p = GetValue<CWizXmlRpcIntValue>(strName)) {
        str = QString::number(*p);
        return true;
	}

    if (CWizXmlRpcStringValue* p = GetValue<CWizXmlRpcStringValue>(strName)) {
		str = *p;
        return true;
    }

    return false;
}

bool CWizXmlRpcStructValue::GetTime(const QString& strName, COleDateTime& t) const
{
    if (CWizXmlRpcTimeValue* p = GetValue<CWizXmlRpcTimeValue>(strName))
	{
		t = *p;
        return true;
    }

    return false;
}

bool CWizXmlRpcStructValue::GetStream(const QString& strName, QByteArray& arrayData) const
{
    if (CWizXmlRpcBase64Value* p = GetValue<CWizXmlRpcBase64Value>(strName))
    {
        return p->GetStream(arrayData);
    }

    return false;
}

CWizXmlRpcStructValue* CWizXmlRpcStructValue::GetStruct(const QString& strName) const
{
    return GetValue<CWizXmlRpcStructValue>(strName);
}

CWizXmlRpcArrayValue* CWizXmlRpcStructValue::GetArray(const QString& strName) const
{
    return GetValue<CWizXmlRpcArrayValue>(strName);
}

bool CWizXmlRpcStructValue::GetInt64(const QString& strName, qint64& n) const
{
    QString str;
    if (!GetString(strName, str))
        return false;

    n = str.toLongLong();
    return true;
}

//bool CWizXmlRpcStructValue::GetInt64(const QString& strName, quint64& n) const
//{
//    QString str;
//    if (!GetString(strName, str))
//        return false;

//    n = str.toULongLong();
//    return true;
//}

//bool CWizXmlRpcStructValue::GetInt(const QString &strName, quint32& n) const
//{
//    int i = 0;
//    if (!GetInt(strName, i))
//        return false;

//    n = i;
//    return true;
//}

bool CWizXmlRpcStructValue::GetInt(const QString& strName, long& n) const
{
	int i = 0;
    if (!GetInt(strName, i))
        return false;

	n = i;
    return true;
}

bool CWizXmlRpcStructValue::GetColor(const QString& strName, COLORREF& cr) const
{
    QString str;
    if (!GetString(strName, str)) {
        TOLOG1("Failed to get member %1", strName);
        return false;
	}
	cr = WizStringToColor(str);

    return true;
}

bool CWizXmlRpcStructValue::GetStringArray(const QString& strName, CWizStdStringArray& arrayData) const
{
    CWizXmlRpcArrayValue* pArray = GetArray(strName);
    if (!pArray) {
        TOLOG("Failed to get array data in struct");
        return false;
	}

	return pArray->ToStringArray(arrayData);
}



BOOL CWizXmlRpcStructValue::ToStringMap(std::map<QString, QString>& ret) const
{
    for (std::map<QString, CWizXmlRpcValue*>::const_iterator it = m_map.begin();
        it != m_map.end();
        it++)
    {
        ret[it->first] = it->second->ToString();
    }
    //
    return TRUE;
}





/* ------------------------- CWizXmlRpcFaultValue ------------------------- */
CWizXmlRpcFaultValue::~CWizXmlRpcFaultValue()
{
}

bool CWizXmlRpcFaultValue::Read(CWizXMLNode& nodeValue)
{
	return m_val.Read(nodeValue);
}

QString CWizXmlRpcFaultValue::ToString() const
{
    return WizFormatString2(_T("Fault error: %1, %2"), WizIntToStr(GetFaultCode()), GetFaultString());
}

int CWizXmlRpcFaultValue::GetFaultCode() const
{
	int nCode = 0;
	m_val.GetInt(_T("faultCode"), nCode);
    /* 此处不能返回Wiz服务器定义的 301,因为QT将 301 定义为连接协议无效.现在需要通过错误代码判断连接状态.
    NOTE：如果Wiz服务器错误代码变更,或者QT错误代码变更,需要修改此处*/
    if (301 == nCode) {
        return WIZKM_XMLRPC_ERROR_INVALID_TOKEN;
    } else if (302 == nCode) {
        return WIZKM_XMLRPC_ERROR_PERMISSION_EXCEPTION;
    } else if (399 == nCode) {
        return WIZKM_XMLRPC_ERROR_WIZ_ALL_ERROR;
    }

	return nCode;
}

QString CWizXmlRpcFaultValue::GetFaultString() const
{
    QString str;
	m_val.GetString(_T("faultString"), str);
	return str;
}




CWizXmlRpcResult::CWizXmlRpcResult()
    : m_pResult(NULL)
    , m_nFaultCode(0)
    , m_bXmlRpcSucceeded(FALSE)
    , m_bFault(FALSE)
{
}
//
CWizXmlRpcResult::~CWizXmlRpcResult()
{
    if (m_pResult)
    {
        delete m_pResult;
        m_pResult = NULL;
    }
}
//
void CWizXmlRpcResult::SetResult(const QString& strMethodName, CWizXmlRpcValue* pRet)
{
    m_pResult = pRet;
    if (!m_pResult)
    {
        TOLOG1(_T("Can not execute xml-rpc: %1"), strMethodName);
        m_bXmlRpcSucceeded = FALSE;
        m_bFault = FALSE;
        return;
    }
    //
    m_bXmlRpcSucceeded = TRUE;
    //
    if (CWizXmlRpcFaultValue* pFault = GetResultValue<CWizXmlRpcFaultValue>())
    {
        m_bFault = TRUE;
        //
        m_nFaultCode = pFault->GetFaultCode();
        m_strFaultString = pFault->GetFaultString();
        TOLOG3(_T("Failed to call xml rpc %1: %2, %3"), strMethodName, WizIntToStr(m_nFaultCode), m_strFaultString);
    }
}
//
BOOL CWizXmlRpcResult::IsXmlRpcSucceeded() const
{
    return m_bXmlRpcSucceeded;
}
BOOL CWizXmlRpcResult::IsFault() const
{
    return m_bFault;
}
BOOL CWizXmlRpcResult::IsNoError() const
{
    return m_bXmlRpcSucceeded && !m_bFault && m_pResult;
}
BOOL CWizXmlRpcResult::GetString(QString& str) const
{
    if (CWizXmlRpcStringValue* pValue = GetResultValue<CWizXmlRpcStringValue>())
    {
        str = *pValue;
        return TRUE;
    }
    //
    return FALSE;
}
BOOL CWizXmlRpcResult::GetBool(BOOL& b) const
{
    if (CWizXmlRpcBoolValue* pValue = GetResultValue<CWizXmlRpcBoolValue>())
    {
        b = *pValue;
        return TRUE;
    }
    //
    return FALSE;
}
