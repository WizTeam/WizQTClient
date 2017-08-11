#include "WizXmlRpc.h"

#include <QUrl>

#include "WizMisc.h"
#include "../utils/WizLogger.h"


bool WizXmlRpcValueFromXml(WizXMLNode& nodeValue, WizXmlRpcValue** ppRet)
{
    *ppRet = NULL;

    WizXMLNode nodeTypeTest;
    nodeValue.getFirstChildNode(nodeTypeTest);
    if (nodeTypeTest.isNull()) {
        QString strText = nodeValue.getText();
        *ppRet = new WizXmlRpcStringValue(strText);
        return true;
    }

    WizXmlRpcValue* pValue = NULL;
    CString strValueType = nodeTypeTest.getName();

    if (0 == strValueType.compareNoCase("int")
        || 0 == strValueType.compareNoCase("i4"))
    {
        pValue = new WizXmlRpcIntValue();
    }
    else if (0 == strValueType.compareNoCase("string")
        || 0 == strValueType.compareNoCase("ex:nil")
        || 0 == strValueType.compareNoCase("ex:i8"))
    {
        pValue = new WizXmlRpcStringValue();
    }
    else if (0 == strValueType.compareNoCase("nil")
        || 0 == strValueType.compareNoCase("i8"))
    {
        pValue = new WizXmlRpcStringValue();
    }
    else if (0 == strValueType.compareNoCase("struct"))
    {
        pValue = new WizXmlRpcStructValue();
    }
    else if (0 == strValueType.compareNoCase("array"))
    {
        pValue = new WizXmlRpcArrayValue();
    }
    else if (0 == strValueType.compareNoCase("base64"))
    {
        pValue = new WizXmlRpcBase64Value();
    }
    else if (0 == strValueType.compareNoCase("boolean")
        || 0 == strValueType.compareNoCase("bool"))
    {
        pValue = new WizXmlRpcBoolValue();
    }
    else if (0 == strValueType.compareNoCase("dateTime.iso8601"))
    {
        pValue = new WizXmlRpcTimeValue();
    }
    else
    {
        TOLOG1("Unknown xmlrpc value type:%1", strValueType);
        return false;
    }

    Q_ASSERT(pValue);

    if (pValue->read(nodeValue))
    {
        *ppRet = pValue;
        return true;
    }

    TOLOG("Failed to read xmlrpc value!");
    delete pValue;

    return false;
}

bool WizXmlRpcResultFromXml(WizXMLDocument& doc, WizXmlRpcValue** ppRet)
{
    WizXMLNode nodeMethodResponse;
    doc.findChildNode("methodResponse", nodeMethodResponse);
    if (nodeMethodResponse.isNull())
    {
        TOLOG("Failed to get methodResponse node!");
        return false;
    }

    WizXMLNode nodeTest;
    if (!nodeMethodResponse.getFirstChildNode(nodeTest))
    {
        TOLOG("Failed to get methodResponse child node!");
        return false;
    }

    QString strTestName = nodeTest.getName();

    if (0 == strTestName.compare("params", Qt::CaseInsensitive))
    {
        WizXMLNode nodeParamValue;
        nodeTest.findNodeByPath("param/value", nodeParamValue);
        if (nodeParamValue.isNull())
        {
            TOLOG("Failed to get param value node of params!");
            return false;
        }

        return WizXmlRpcValueFromXml(nodeParamValue, ppRet);
    }
    else if (0 == strTestName.compare("fault", Qt::CaseInsensitive))
    {
        WizXMLNode nodeFaultValue;
        nodeTest.findChildNode("value", nodeFaultValue);
        if (nodeFaultValue.isNull())
        {
            TOLOG("Failed to get fault value node!");
            return false;
        }

        WizXmlRpcFaultValue* pFault = new WizXmlRpcFaultValue();
        pFault->read(nodeFaultValue);

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
WizXmlRpcRequest::WizXmlRpcRequest(const QString& strMethodName)
{
    WizXMLNode nodeMethodCall;
    m_doc.appendChild("methodCall", nodeMethodCall);
    nodeMethodCall.setChildNodeText("methodName", strMethodName);

    WizXMLNode nodeParams;
    nodeMethodCall.appendChild("params", nodeParams);
}

void WizXmlRpcRequest::addParam(WizXmlRpcValue* pParam)
{
    Q_ASSERT(pParam);

    WizXMLNode nodeParams;
    m_doc.findNodeByPath("methodCall/params", nodeParams);

    WizXMLNode nodeParamValue;
    nodeParams.appendNodeByPath("param/value", nodeParamValue);

    pParam->write(nodeParamValue);
}

QByteArray WizXmlRpcRequest::toData()
{
    QString strText;
    if (!m_doc.toXML(strText, true)) {
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
WizXmlRpcIntValue::WizXmlRpcIntValue(int n /* = 0 */)
    : m_n(n)
{
}

bool WizXmlRpcIntValue::write(WizXMLNode& nodeValue)
{
    nodeValue.setChildNodeText("int", QString::number(m_n));
    return true;
}

bool WizXmlRpcIntValue::read(WizXMLNode& nodeValue)
{
    QString strValue = nodeValue.getFirstChildNodeText();
    m_n = strValue.toInt();
    return true;
}

QString WizXmlRpcIntValue::toString() const
{
    return QString::number(m_n);
}

WizXmlRpcIntValue::operator int()
{
	return m_n;
}


/* ------------------------- CWizXmlRpcBoolValue ------------------------- */
WizXmlRpcBoolValue::WizXmlRpcBoolValue(bool b /* = false */)
    : m_b(b)
{
}

bool WizXmlRpcBoolValue::write(WizXMLNode& nodeValue)
{
    nodeValue.setChildNodeText("boolean", m_b ? "1" : "0");
    return true;
}

bool WizXmlRpcBoolValue::read(WizXMLNode& nodeValue)
{
    QString strValue = nodeValue.getFirstChildNodeText();
    m_b = (strValue == "1" || 0 == strValue.compare("true", Qt::CaseInsensitive));
    return true;
}

QString WizXmlRpcBoolValue::toString() const
{
    return m_b ? "1" : "0";
}

WizXmlRpcBoolValue::operator bool()
{
	return m_b;
}


/* ------------------------- CWizXmlRpcStringValue ------------------------- */
WizXmlRpcStringValue::WizXmlRpcStringValue(const QString& strDef /* = "" */)
    : m_str(strDef)
{
}

bool WizXmlRpcStringValue::write(WizXMLNode& nodeValue)
{
    nodeValue.setChildNodeText("string", m_str);
    return true;
}

bool WizXmlRpcStringValue::read(WizXMLNode& nodeValue)
{
	m_str = nodeValue.getFirstChildNodeText();
    return true;
}

QString WizXmlRpcStringValue::toString() const
{
    return m_str;
}

WizXmlRpcStringValue::operator QString()
{
	return m_str;
}


/* ------------------------- CWizXmlRpcTimeValue ------------------------- */
WizXmlRpcTimeValue::WizXmlRpcTimeValue()
    : m_t(::WizGetCurrentTime())
{
}

WizXmlRpcTimeValue::WizXmlRpcTimeValue(const WizOleDateTime& t)
	: m_t(t)
{
}

bool WizXmlRpcTimeValue::write(WizXMLNode& nodeValue)
{
    QString str = WizDateTimeToIso8601String(m_t);
    nodeValue.setChildNodeText("dateTime.iso8601", str);
    return true;
}

bool WizXmlRpcTimeValue::read(WizXMLNode& nodeValue)
{
    CString str = nodeValue.getFirstChildNodeText();
    CString strError;
    if (!WizIso8601StringToDateTime(str, m_t, strError)) {
		TOLOG(strError);
        return false;
	}

    return true;
}

QString WizXmlRpcTimeValue::toString() const
{
	return ::WizDateTimeToString(m_t);
}

WizXmlRpcTimeValue::operator WizOleDateTime()
{
	return m_t;
}


/* ------------------------- CWizXmlRpcBase64Value ------------------------- */
WizXmlRpcBase64Value::WizXmlRpcBase64Value(const QByteArray& arrayData)
    : m_arrayData(arrayData)
{
}

bool WizXmlRpcBase64Value::write(WizXMLNode& nodeValue)
{
    QString strText;
    WizBase64Encode(m_arrayData, strText);
    nodeValue.setChildNodeText("base64", strText);
    return true;
}

bool WizXmlRpcBase64Value::read(WizXMLNode& nodeValue)
{
    QString str = nodeValue.getFirstChildNodeText();
    m_arrayData.clear();;
    return WizBase64Decode(str, m_arrayData);
}

QString WizXmlRpcBase64Value::toString() const
{
    QString strText;
    WizBase64Encode(m_arrayData, strText);
    return strText;
}

bool WizXmlRpcBase64Value::getStream(QByteArray& arrayData)
{
    arrayData = m_arrayData;
    return true;
}


/* ------------------------- CWizXmlRpcArrayValue ------------------------- */
WizXmlRpcArrayValue::WizXmlRpcArrayValue()
{
}

WizXmlRpcArrayValue::WizXmlRpcArrayValue(const CWizStdStringArray& arrayData)
{
	setStringArray(arrayData);
}

WizXmlRpcArrayValue::~WizXmlRpcArrayValue ()
{
	clear();
}

bool WizXmlRpcArrayValue::write(WizXMLNode& nodeValue)
{
	WizXMLNode nodeData;
    nodeValue.appendNodeByPath("array/data", nodeData);

    std::deque<WizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++)
	{
		WizXmlRpcValue* pValue = *it;

		WizXMLNode nodeElementValue;
        nodeData.appendChild("value", nodeElementValue);
		pValue->write(nodeElementValue);
	}

    return true;
}

bool WizXmlRpcArrayValue::read(WizXMLNode& nodeValue)
{
	WizXMLNode nodeData;
    if (!nodeValue.findNodeByPath("array/data", nodeData))
	{
        TOLOG("Failed to get array data node!");
        return false;
	}

    std::deque<WizXMLNode> arrayValue;
	nodeData.getAllChildNodes(arrayValue);

    std::deque<WizXMLNode>::iterator it;
    for (it = arrayValue.begin(); it != arrayValue.end(); it++)
	{
		WizXMLNode& nodeElementValue = *it;

		WizXmlRpcValue* pElementValue = NULL;
        if (!WizXmlRpcValueFromXml(nodeElementValue, &pElementValue))
		{
            TOLOG("Failed to load array element value from node!");
            return false;
		}

        Q_ASSERT(pElementValue);
		add(pElementValue);
	}

    return true;
}

QString WizXmlRpcArrayValue::toString() const
{
    QString str;

    std::deque<WizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++)
    {
        str += (*it)->toString();

        if (it != m_array.end() - 1) {
            str += ", ";
        }
    }

    return str;
}

void WizXmlRpcArrayValue::add(WizXmlRpcValue* pValue)
{
	m_array.push_back(pValue);
}

void WizXmlRpcArrayValue::clear()
{
    std::deque<WizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++)
	{
		delete *it;
    }

	m_array.clear();
}

bool WizXmlRpcArrayValue::toStringArray(CWizStdStringArray& arrayRet)
{
    std::deque<WizXmlRpcValue*>::const_iterator it;
    for (it = m_array.begin(); it != m_array.end(); it++)
	{
		WizXmlRpcStringValue* pValue = dynamic_cast<WizXmlRpcStringValue*>(*it);
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

bool WizXmlRpcArrayValue::setStringArray(const CWizStdStringArray& arrayData)
{
    CWizStdStringArray::const_iterator it;
    for (it = arrayData.begin(); it != arrayData.end(); it++)
	{
		add(new WizXmlRpcStringValue(*it));
    }

    return true;
}


/* ------------------------- CWizXmlRpcStructValue ------------------------- */
void WizXmlRpcStructValue::addValue(const QString& strName, WizXmlRpcValue* pValue)
{
    removeValue(strName);
    m_map[strName] = pValue;
}

WizXmlRpcValue* WizXmlRpcStructValue::getMappedValue(const QString& strName) const
{
    std::map<QString, WizXmlRpcValue*>::const_iterator it = m_map.find(strName);
	if (it == m_map.end())
		return NULL;

	return it->second;
}

WizXmlRpcStructValue::~WizXmlRpcStructValue()
{
	clear();
}

bool WizXmlRpcStructValue::write(WizXMLNode& nodeValue)
{
	WizXMLNode nodeStruct;
    nodeValue.appendChild("struct", nodeStruct);

    std::map<QString, WizXmlRpcValue*>::const_iterator it;
    for (it = m_map.begin(); it != m_map.end(); it++) {
        QString strName = it->first;
		WizXmlRpcValue* pValue = it->second;

		WizXMLNode nodeMember;
        nodeStruct.appendChild("member", nodeMember);

        nodeMember.setChildNodeText("name", strName);

		WizXMLNode nodeElementValue;
        nodeMember.appendChild("value", nodeElementValue);

		pValue->write(nodeElementValue);
	}

    return true;
}

bool WizXmlRpcStructValue::read(WizXMLNode& nodeValue)
{
	WizXMLNode nodeStruct;
    if (!nodeValue.findChildNode("struct", nodeStruct)) {
        TOLOG("Failed to get struct node!");
        return false;
	}

    std::deque<WizXMLNode> arrayMember;
	nodeStruct.getAllChildNodes(arrayMember);

    std::deque<WizXMLNode>::iterator it;
    for (it = arrayMember.begin(); it != arrayMember.end(); it++)
    {
		WizXMLNode& nodeMember = *it;

        QString strName;
        if (!nodeMember.getChildNodeText("name", strName))
        {
            TOLOG("Failed to get struct member name!");
            return false;
		}

		WizXMLNode nodeMemberValue;
        if (!nodeMember.findChildNode("value", nodeMemberValue))
        {
            TOLOG("Failed to get struct member value!");
            return false;
		}

		WizXmlRpcValue* pMemberValue = NULL;
        if (!WizXmlRpcValueFromXml(nodeMemberValue, &pMemberValue))
		{
            TOLOG("Failed to load struct member value from node!");
            return false;
		}

        Q_ASSERT(pMemberValue);

		addValue(strName, pMemberValue);
	}

    return true;
}

QString WizXmlRpcStructValue::toString() const
{
    QString str;

    std::map<QString, WizXmlRpcValue*>::const_iterator it;
    for (it = m_map.begin(); it != m_map.end(); it++)
    {
        QString strKeyValue;
        WizXmlRpcStructValue* p = dynamic_cast<WizXmlRpcStructValue*>(it->second);
        if (p) {
            strKeyValue = "{" + it->first + ":" + it->second->toString() + "}";
            str += strKeyValue;
            continue;
        }

        WizXmlRpcArrayValue* p2 = dynamic_cast<WizXmlRpcArrayValue*>(it->second);
        if (p2) {
            strKeyValue = "[" + it->first + ":" + it->second->toString() + "]";
            str += strKeyValue;
            continue;
        }

        strKeyValue = "(" + it->first + ":" + it->second->toString() + ")";
        str += strKeyValue;
    }

    return "{" + str + "}\n";
}

void WizXmlRpcStructValue::addInt(const QString& strName, int n)
{
    addValue(strName, new WizXmlRpcIntValue(n));
}

void WizXmlRpcStructValue::addString(const QString& strName, const QString& str)
{
    addValue(strName, new WizXmlRpcStringValue(str));
}

void WizXmlRpcStructValue::addBool(const QString& strName, bool b)
{
    addValue(strName, new WizXmlRpcBoolValue(b));
}

void WizXmlRpcStructValue::addTime(const QString& strName, const WizOleDateTime& t)
{
    addValue(strName, new WizXmlRpcTimeValue(t));
}

void WizXmlRpcStructValue::addBase64(const QString& strName, const QByteArray& arrayData)
{
    addValue(strName, new WizXmlRpcBase64Value(arrayData));
}

void WizXmlRpcStructValue::addStringArray(const QString& strName, const CWizStdStringArray& arrayData)
{
    addValue(strName, new WizXmlRpcArrayValue(arrayData));
}

void WizXmlRpcStructValue::addInt64(const QString& strName, __int64 n)
{
    addValue(strName, new WizXmlRpcStringValue(WizInt64ToStr(n)));
}

void WizXmlRpcStructValue::addColor(const QString& strName, COLORREF cr)
{
    addValue(strName, new WizXmlRpcStringValue(WizColorToString(cr)));
}

void WizXmlRpcStructValue::addStruct(const QString& strName, WizXmlRpcStructValue* pStruct)
{
    addValue(strName, pStruct);
}

void WizXmlRpcStructValue::addArray(const QString& strName, WizXmlRpcValue* pArray)
{
    addValue(strName, pArray);
}

void WizXmlRpcStructValue::clear()
{
    std::map<QString, WizXmlRpcValue*>::const_iterator it;
    for (it = m_map.begin(); it != m_map.end(); it++) {
		deleteValue(it->second);
	}

	m_map.clear();
}

void WizXmlRpcStructValue::removeValue(const QString& strName)
{
    std::map<QString, WizXmlRpcValue*>::iterator it = m_map.find(strName);
	if (it == m_map.end())
		return;

	deleteValue(it->second);
	m_map.erase(it);
}

void WizXmlRpcStructValue::deleteValue(WizXmlRpcValue* pValue)
{
	delete pValue;
}

bool WizXmlRpcStructValue::getBool(const QString& strName, bool& b) const
{
    WizXmlRpcBoolValue* p = getValue<WizXmlRpcBoolValue>(strName);
	if (!p)
        return false;

	b = *p;
    return true;
}

bool WizXmlRpcStructValue::getInt(const QString& strName, int& n) const
{
    if (WizXmlRpcIntValue* p = getValue<WizXmlRpcIntValue>(strName)) {
		n = *p;
        return true;
	}

    if (WizXmlRpcStringValue* p = getValue<WizXmlRpcStringValue>(strName)) {
        n = QString(*p).toInt();
        return true;
    }

    return false;
}

bool WizXmlRpcStructValue::getString(const QString& strName, QString& str) const
{
    if (WizXmlRpcIntValue* p = getValue<WizXmlRpcIntValue>(strName)) {
        str = QString::number(*p);
        return true;
	}

    if (WizXmlRpcStringValue* p = getValue<WizXmlRpcStringValue>(strName)) {
		str = *p;
        return true;
    }

    return false;
}

bool WizXmlRpcStructValue::getTime(const QString& strName, WizOleDateTime& t) const
{
    if (WizXmlRpcTimeValue* p = getValue<WizXmlRpcTimeValue>(strName))
	{
		t = *p;
        return true;
    }

    return false;
}

bool WizXmlRpcStructValue::getStream(const QString& strName, QByteArray& arrayData) const
{
    if (WizXmlRpcBase64Value* p = getValue<WizXmlRpcBase64Value>(strName))
    {
        return p->getStream(arrayData);
    }

    return false;
}

WizXmlRpcStructValue* WizXmlRpcStructValue::getStruct(const QString& strName) const
{
    return getValue<WizXmlRpcStructValue>(strName);
}

WizXmlRpcArrayValue* WizXmlRpcStructValue::getArray(const QString& strName) const
{
    return getValue<WizXmlRpcArrayValue>(strName);
}

bool WizXmlRpcStructValue::getInt64(const QString& strName, qint64& n) const
{
    QString str;
    if (!getString(strName, str))
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

bool WizXmlRpcStructValue::getInt(const QString& strName, long& n) const
{
	int i = 0;
    if (!getInt(strName, i))
        return false;

	n = i;
    return true;
}

bool WizXmlRpcStructValue::getColor(const QString& strName, COLORREF& cr) const
{
    QString str;
    if (!getString(strName, str)) {
        TOLOG1("Failed to get member %1", strName);
        return false;
	}
	cr = WizStringToColor(str);

    return true;
}

bool WizXmlRpcStructValue::getStringArray(const QString& strName, CWizStdStringArray& arrayData) const
{
    WizXmlRpcArrayValue* pArray = getArray(strName);
    if (!pArray) {
        TOLOG("Failed to get array data in struct");
        return false;
	}

	return pArray->toStringArray(arrayData);
}



bool WizXmlRpcStructValue::toStringMap(std::map<QString, QString>& ret) const
{
    for (std::map<QString, WizXmlRpcValue*>::const_iterator it = m_map.begin();
        it != m_map.end();
        it++)
    {
        ret[it->first] = it->second->toString();
    }
    //
    return TRUE;
}





/* ------------------------- CWizXmlRpcFaultValue ------------------------- */
WizXmlRpcFaultValue::~WizXmlRpcFaultValue()
{
}

bool WizXmlRpcFaultValue::read(WizXMLNode& nodeValue)
{
	return m_val.read(nodeValue);
}

QString WizXmlRpcFaultValue::toString() const
{
    return WizFormatString2("Fault error: %1, %2", WizIntToStr(getFaultCode()), getFaultString());
}

int WizXmlRpcFaultValue::getFaultCode() const
{
	int nCode = 0;
	m_val.getInt("faultCode", nCode);
    ////
	return nCode;
}

QString WizXmlRpcFaultValue::getFaultString() const
{
    QString str;
	m_val.getString("faultString", str);
	return str;
}




WizXmlRpcResult::WizXmlRpcResult()
    : m_pResult(NULL)
    , m_nFaultCode(0)
    , m_bXmlRpcSucceeded(FALSE)
    , m_bFault(FALSE)
{
}
//
WizXmlRpcResult::~WizXmlRpcResult()
{
    if (m_pResult)
    {
        delete m_pResult;
        m_pResult = NULL;
    }
}
//
void WizXmlRpcResult::setResult(const QString& strMethodName, WizXmlRpcValue* pRet)
{
    m_pResult = pRet;
    if (!m_pResult)
    {
        TOLOG1("Can not execute xml-rpc: %1", strMethodName);
        m_bXmlRpcSucceeded = FALSE;
        m_bFault = FALSE;
        return;
    }
    //
    m_bXmlRpcSucceeded = TRUE;
    //
    if (WizXmlRpcFaultValue* pFault = getResultValue<WizXmlRpcFaultValue>())
    {
        m_bFault = TRUE;
        //
        m_nFaultCode = pFault->getFaultCode();
        m_strFaultString = pFault->getFaultString();
        TOLOG3("Failed to call xml rpc %1: %2, %3", strMethodName, WizIntToStr(m_nFaultCode), m_strFaultString);
    }
}
//
BOOL WizXmlRpcResult::isXmlRpcSucceeded() const
{
    return m_bXmlRpcSucceeded;
}
BOOL WizXmlRpcResult::isFault() const
{
    return m_bFault;
}
BOOL WizXmlRpcResult::isNoError() const
{
    return m_bXmlRpcSucceeded && !m_bFault && m_pResult;
}
BOOL WizXmlRpcResult::getString(QString& str) const
{
    if (WizXmlRpcStringValue* pValue = getResultValue<WizXmlRpcStringValue>())
    {
        str = *pValue;
        return TRUE;
    }
    //
    return FALSE;
}
BOOL WizXmlRpcResult::getBool(BOOL& b) const
{
    if (WizXmlRpcBoolValue* pValue = getResultValue<WizXmlRpcBoolValue>())
    {
        b = *pValue;
        return TRUE;
    }
    //
    return FALSE;
}

