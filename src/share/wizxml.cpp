#include "wizxml.h"


CWizXMLNode::CWizXMLNode()
{
}

CWizXMLNode::CWizXMLNode(const QDomNode& node)
{
    InitData(node);
}

CWizXMLNode::CWizXMLNode(const CWizXMLNode& nodeSrc)
{
    if (nodeSrc.Valid())
    {
        InitData(nodeSrc.m_node);
    }
    else
    {
        Clear();
    }
}

CWizXMLNode::~CWizXMLNode()
{
}

CWizXMLNode& CWizXMLNode::operator = (const CWizXMLNode& right)
{
    if (!right.m_node.isNull())
    {
        InitData(right.m_node);
    }
    else
    {
        Clear();
    }

    return *this;
}

void CWizXMLNode::InitData(const QDomNode& node)
{
    m_node = node;
}

bool CWizXMLNode::GetName(CString& strName)
{
    strName = m_node.nodeName();
    return true;
}

CString CWizXMLNode::GetName()
{
    CString strName;
    GetName(strName);
    return strName;
}

CString CWizXMLNode::GetType()
{
    if (m_node.isNull())
        return CString();

    switch (m_node.nodeType())
    {
    case QDomNode::ElementNode              :
        return "element";
    case QDomNode::AttributeNode            :
        return "attribute";
    case QDomNode::TextNode                 :
        return "text";
    case QDomNode::CDATASectionNode         :
        return "cdatasection";
    case QDomNode::EntityReferenceNode      :
        return "entityreference";
    case QDomNode::EntityNode               :
        return "entity";
    case QDomNode::ProcessingInstructionNode:
        return "processinginstruction";
    case QDomNode::CommentNode              :
        return "comment";
    case QDomNode::DocumentNode             :
        return "document";
    case QDomNode::DocumentTypeNode         :
        return "documenttype";
    case QDomNode::DocumentFragmentNode     :
        return "documentfragment";
    case QDomNode::NotationNode             :
        return "notation";
    case QDomNode::BaseNode                 :
        return "BaseNode";
    case QDomNode::CharacterDataNode        :
        return "CharacterDataNode";
    }
    return CString();
}

CString CWizXMLNode::GetText(const CString& strDefault /* = "" */)
{
    CString str;
    if (!GetText(str))
    {
        str = strDefault;
    }

    return str;
}

bool CWizXMLNode::GetText(CString& strText)
{
    QDomNodeList nodes =  m_node.childNodes();
    if (nodes.isEmpty())
        return false;

    if (nodes.size() != 1)
        return false;

    QDomNode node = nodes.item(0);
    if (node.nodeType() != QDomNode::TextNode)
        return false;

    strText = node.nodeValue();
    return true;
}

bool CWizXMLNode::SetText(const CString& strText)
{
    Q_ASSERT(!m_node.isNull());

    QDomNode child = m_node.ownerDocument().createTextNode(strText);

    m_node.appendChild(child);
    return true;
}

bool CWizXMLNode::GetAttributeText(const CString& strName, CString& strVal)
{
    QDomNamedNodeMap nodes = m_node.attributes();
    QDomNode node = nodes.namedItem(strName);
    if (node.isNull())
        return false;

    strVal = node.nodeValue();
    return true;
}

bool CWizXMLNode::GetAttributeInt(const CString& strName, int& nVal)
{
    CString strRet;
    if (!GetAttributeText(strName, strRet))
        return false;

    nVal = _ttoi(strRet);
    return true;
}

bool CWizXMLNode::GetAttributeInt64(const CString& strName, __int64& nVal)
{
    CString strRet;
    if (!GetAttributeText(strName, strRet))
        return false;

    nVal = _ttoi64(strRet);
    return true;
}

__int64 CWizXMLNode::GetAttributeInt64Def(const CString& strName, __int64 nDefault)
{
    CString strRet;
    if (!GetAttributeText(strName, strRet))
        return nDefault;

    return _ttoi64(strRet);
}

bool CWizXMLNode::GetAttributeUINT(const CString& strName, UINT& nVal)
{
    int n;
    if (!GetAttributeInt(strName, n))
        return false;

    nVal = n;
    return true;
}

bool CWizXMLNode::GetAttributeTimeT(const CString& strName, time_t& nVal)
{
    __int64 n64;
    if (!GetAttributeInt64(strName, n64))
        return false;

    nVal = time_t(n64);
    return true;
}

bool CWizXMLNode::GetAttributeTimeString(const CString& strName, COleDateTime& t)
{
    CString str;
    if (!GetAttributeText(strName, str))
        return false;

    CString strError;
    if (!WizStringToDateTime(str, t, strError))
    {
        TOLOG(strError);
        return false;
    }

    return true;
}

bool CWizXMLNode::GetAttributeDWORD(const CString& strName, DWORD& dwVal)
{
    int nVal = 0;
    if (!GetAttributeInt(strName, nVal))
        return false;

    dwVal = DWORD(nVal);
    return true;
}

bool CWizXMLNode::GetAttributeBool(const CString& strName, bool& bVal)
{
    CString strRet;
    if (!GetAttributeText(strName, strRet))
        return false;

    strRet.Trim();
    bVal = strRet.Compare(_T("1")) == 0;
    return true;
}

CString CWizXMLNode::GetAttributeTextDef(const CString& strName, const CString& strDefault)
{
    CString strVal;
    if (!GetAttributeText(strName, strVal))
    {
        strVal = strDefault;
    }
    return strVal;
}

int CWizXMLNode::GetAttributeIntDef(const CString& strName, int nDefault)
{
    int nVal = 0;
    if (!GetAttributeInt(strName, nVal))
    {
        nVal = nDefault;
    }
    return nVal;
}

bool CWizXMLNode::GetAttributeBoolDef(const CString& strName, bool bDefault)
{
    bool bVal = false;
    if (!GetAttributeBool(strName, bVal))
    {
        bVal = bDefault;
    }
    return bVal;
}

bool CWizXMLNode::SetAttributeText(const CString& strName, const CString& strText)
{
    Q_ASSERT(!m_node.isNull());

    QDomAttr att = m_node.ownerDocument().createAttribute(strName);
    att.setNodeValue(strText);

    QDomNamedNodeMap nodes = m_node.attributes();
    nodes.setNamedItem(att);

    return true;
}

bool CWizXMLNode::SetAttributeInt(const CString& strName, int nVal)
{
    return SetAttributeText(strName, WizIntToStr(nVal));
}

bool CWizXMLNode::SetAttributeInt64(const CString& strName, __int64 nVal)
{
    return SetAttributeText(strName, WizInt64ToStr(nVal));
}

bool CWizXMLNode::SetAttributeTime(const CString& strName, const COleDateTime& t)
{
    return SetAttributeText(strName, WizDateTimeToString(t));
}

bool CWizXMLNode::SetAttributeBool(const CString& strName, bool bVal)
{
    return SetAttributeText(strName, bVal ? _T("1") : _T("0"));
}

bool CWizXMLNode::FindChildNode(const CString& strNodeName, CWizXMLNode& nodeChild)
{
    QDomNodeList nodes = m_node.childNodes();
    QDomNode node;
    if (!FindChildNode(nodes, strNodeName, node))
        return false;

    nodeChild.InitData(node);
    return true;
}

bool CWizXMLNode::GetAllChildNodes(CWizStdStringArray& arrayNodeName)
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);
        CString strName = node.nodeName();

        arrayNodeName.push_back(strName);
    }
    return true;
}

bool CWizXMLNode::GetAllChildNodes(std::deque<CWizXMLNode>& arrayNodes)
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);
        arrayNodes.push_back(node);
    }

    return true;
}

bool CWizXMLNode::GetFirstChildNode(CWizXMLNode& nodeChild)
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    if (nCount <= 0)
        return false;

    CWizXMLNode node = nodes.item(0);

    CString str = node.GetType();
    if (str != _T("element"))
        return false;

    nodeChild = node;
    return true;
}

CString CWizXMLNode::GetFirstChildNodeText(const CString& strDef /* = NULL */)
{
    CWizXMLNode nodeChild;
    if (!GetFirstChildNode(nodeChild))
        return strDef;

    return nodeChild.GetText(strDef);
}

bool CWizXMLNode::DeleteChild(const CString& strChildName)
{
    CWizXMLNode node;
    if (!FindChildNode(strChildName, node))
        return true;

    m_node.removeChild(node.m_node);
    return true;
}

bool CWizXMLNode::DeleteChild(CWizXMLNode& nodeChild)
{
    m_node.removeChild(nodeChild.m_node);
    return true;
}

bool CWizXMLNode::DeleteAllChild(const CString& strExceptNodeName1, \
                                 const CString& strExceptNodeName2, \
                                 const CString& strExceptNodeName3)
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    for (int i = nCount - 1; i >= 0; i--)
    {
        QDomNode node = nodes.item(i);

        if (!strExceptNodeName1.IsEmpty()
            || !strExceptNodeName2.IsEmpty()
            || !strExceptNodeName3.IsEmpty()
            )
        {
            CString strName = node.nodeName();

            if (!strExceptNodeName1.IsEmpty()
                && strName == strExceptNodeName1)
                continue;

            if (!strExceptNodeName2.IsEmpty()
                && strName == strExceptNodeName2)
                continue;

            if (!strExceptNodeName3.IsEmpty()
                && strName == strExceptNodeName3)
                continue;
        }

        m_node.removeChild(node);
    }
    return true;
}

bool CWizXMLNode::HasChildNode()
{
    return m_node.hasChildNodes();
}

int CWizXMLNode::GetChildNodesCount()
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    int nChildCount = nCount;

    for (int i = 0; i < nCount; i++)
    {
        CWizXMLNode node = nodes.item(i);

        nChildCount += node.GetChildNodesCount();

    }
    return nChildCount;
}

bool CWizXMLNode::AppendChild(const CString& strNodeName, CWizXMLNode& nodeChild)
{
    QDomNode node = m_node.ownerDocument().createElement(strNodeName);

    nodeChild = m_node.appendChild(node);
    return true;
}

bool CWizXMLNode::AppendChild(const CString& strNodeName, const CString& strChildNodeText)
{
    CWizXMLNode nodeChild;
    if (!AppendChild(strNodeName, nodeChild))
        return false;

    return nodeChild.SetText(strChildNodeText);
}

bool CWizXMLNode::SetChildNodeText(const CString& strNodeName, const CString& strText)
{
    CWizXMLNode nodeChild;
    if (!GetChildNode(strNodeName, nodeChild))
        return false;

    return nodeChild.SetText(strText);
}

bool CWizXMLNode::GetChildNodeText(const CString& strNodeName, CString& strText)
{
    CWizXMLNode nodeChild;
    if (!GetChildNode(strNodeName, nodeChild))
        return false;

    return nodeChild.GetText(strText);
}

CString CWizXMLNode::GetChildNodeTextDef(const CString& strNodeName, const CString& strDef)
{
    CString str;
    if (GetChildNodeText(strNodeName, str))
        return str;

    return strDef;
}

bool CWizXMLNode::GetChildNode(const CString& strNodeName, CWizXMLNode& nodeChild)
{
    if (FindChildNode(strNodeName, nodeChild))
        return true;
    return AppendChild(strNodeName, nodeChild);
}

bool CWizXMLNode::FindChildNode(const QDomNodeList& nodes, const CString& strName, QDomNode& nodeRet)
{
    long nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);
        CString strNodeName = node.nodeName();

        if (strNodeName.CompareNoCase(strName) == 0)
        {
            nodeRet = node;
            return true;
        }
    }
    return false;
}

bool CWizXMLNode::FindNodeByPath(const CString& strPath, CWizXMLNode& nodeRet)
{
    CWizStdStringArray arrayText;
    if (!WizSplitTextToArray(strPath, '/', arrayText))
        return false;

    WizStringArrayEraseEmptyLine(arrayText);

    if (arrayText.empty())
        return false;

    CWizXMLNode node = *this;

    for (size_t i = 0; i < arrayText.size(); i++)
    {
        CString strNodeName = arrayText[i];

        strNodeName.Trim();

        if (strNodeName.IsEmpty())
            return false;

        CWizXMLNode nodeChild;
        if (!node.FindChildNode(strNodeName, nodeChild))
            return false;

        node = nodeChild;
    }

    Q_ASSERT(node.Valid());

    nodeRet = node;

    return true;
}

bool CWizXMLNode::AppendNodeByPath(const CString& strPath, CWizXMLNode& nodeRet)
{
    CWizStdStringArray arrayText;
    if (!WizSplitTextToArray(strPath, '/', arrayText))
        return false;

    WizStringArrayEraseEmptyLine(arrayText);

    if (arrayText.empty())
        return false;

    CWizXMLNode node = *this;

    for (size_t i = 0; i < arrayText.size(); i++)
    {
        CString strNodeName = arrayText[i];

        strNodeName.Trim();

        if (strNodeName.IsEmpty())
            return false;

        CWizXMLNode nodeChild;
        if (!node.AppendChild(strNodeName, nodeChild))
        {
            TOLOG1(_T("Failed to append child node: %1"), strNodeName);
            return false;
        }

        node = nodeChild;
    }

    Q_ASSERT(node.Valid());

    nodeRet = node;

    return true;
}

bool CWizXMLNode::AppendNodeSetTextByPath(const CString& strPath, const CString& strText)
{
    CWizXMLNode nodeRet;
    if (!AppendNodeByPath(strPath, nodeRet))
    {
        TOLOG1(_T("Failed to append node by path: %!"), strPath);
        return false;
    }

    bool bRet = nodeRet.SetText(strText);
    if (!bRet)
    {
        TOLOG(_T("Failed to set node text"));
        return false;
    }

    return true;
}

bool CWizXMLNode::FindNodeTextByPath(const CString& strPath, CString& strRet)
{
    CWizXMLNode node;
    if (!FindNodeByPath(strPath, node))
        return false;

    return node.GetText(strRet);
}

bool CWizXMLNode::GetElementNodeByValue(const CString& strElementName, const CString& strValueName, const CString& strValue, CWizXMLNode& nodeRet)
{
    std::deque<CWizXMLNode> arrayNodes;
    if (!GetAllChildNodes(arrayNodes))
        return false;

    size_t nCount = arrayNodes.size();
    for (size_t i = 0; i < nCount; i++)
    {
        CWizXMLNode& node = arrayNodes[i];

        if (node.GetName() != strElementName)
            continue;

        CWizXMLNode nodeValue;
        if (!node.FindNodeByPath(strValueName, nodeValue))
            continue;

        if (0 == nodeValue.GetText().CompareNoCase(strValue))
        {
            nodeRet = node;
            return true;
        }
    }

    return false;
}

bool CWizXMLNode::GetElementOtherNodeByValue(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, CWizXMLNode& nodeRet)
{
    CWizXMLNode nodeElement;
    if (!GetElementNodeByValue(strElementName, strValueName, strValue, nodeElement))
        return false;

    return nodeElement.FindNodeByPath(strOtherNodePath, nodeRet);
}

bool CWizXMLNode::GetElementOtherNodeByValueReturnString(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, CString& strRet)
{
    CWizXMLNode nodeRet;
    if (!GetElementOtherNodeByValue(strElementName, strValueName, strValue, strOtherNodePath, nodeRet))
        return false;

    strRet = nodeRet.GetText();

    return true;
}

bool CWizXMLNode::GetElementOtherNodeByValueReturnInt(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, int& nRet)
{
    CString strRet;
    if (!GetElementOtherNodeByValueReturnString(strElementName, strValueName, strValue, strOtherNodePath, strRet))
        return false;

    strRet.Trim();

    int nTemp = _ttoi(strRet);
    if (WizIntToStr(nTemp) == strRet)
    {
        nRet = nTemp;
        return true;
    }

    return false;
}

bool CWizXMLNode::GetElementOtherNodeByValueReturnBool(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, bool& bRet)
{
    CString strRet;
    if (!GetElementOtherNodeByValueReturnString(strElementName, strValueName, strValue, strOtherNodePath, strRet))
        return false;

    strRet.Trim();
    strRet.MakeLower();

    if (strRet == _T("0")
        || strRet == _T("false"))
    {
        bRet = false;

        return true;
    }
    else if (strRet == _T("1")
        || strRet == _T("true"))
    {
        bRet = true;

        return true;
    }
    else
    {
        return false;
    }

    return true;
}


CWizXMLDocument::CWizXMLDocument()
{
    Create();
}

CWizXMLDocument::~CWizXMLDocument()
{
}

bool CWizXMLDocument::Create()
{
    return true;
}

bool CWizXMLDocument::LoadXML(const CString& strXML)
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;
    if (m_doc.setContent(strXML, &errorMsg, &errorLine, &errorColumn))
        return true;

    return false;
}

bool CWizXMLDocument::LoadFromFile(const CString& strFileName, bool bPromptError /*= true*/)
{
    Q_UNUSED(bPromptError);

    if (!PathFileExists(strFileName))
        return false;

    CString strXml;
    if (!::WizLoadUnicodeTextFromFile(strFileName, strXml))
        return false;

    return LoadXML(strXml);
}

void CWizXMLDocument::Clear()
{
    m_doc.clear();
    Create();
}

bool CWizXMLDocument::FindChildNode(const CString& strName, CWizXMLNode& nodeChild)
{
    if (m_doc.isNull())
        return false;

    QDomNodeList nodes = m_doc.childNodes();

    QDomNode node;
    if (!CWizXMLNode::FindChildNode(nodes, strName, node))
        return false;

    nodeChild.InitData(node);

    return true;
}

bool CWizXMLDocument::IsFail()
{
    return false;
}

bool CWizXMLDocument::AppendChild(const CString& strNodeName, CWizXMLNode& nodeChild)
{
    if (m_doc.isNull())
    {
        m_doc = QDomDocument(strNodeName);
    }

    QDomNode node = m_doc.createElement(strNodeName);
    nodeChild = m_doc.appendChild(node);

    return true;
}

bool CWizXMLDocument::GetChildNode(const CString& strName, CWizXMLNode& nodeChild)
{
    if (m_doc.isNull())
    {
        m_doc = QDomDocument(strName);
    }

    if (FindChildNode(strName, nodeChild))
        return true;
    return AppendChild(strName, nodeChild);
}

bool CWizXMLDocument::ToXML(CString& strText, bool bFormatText)
{
    if (m_doc.isNull())
        return false;

    strText = m_doc.toString();

    if (bFormatText)
    {
        //WizFormatXML(strText);
    }

    return true;
}

bool CWizXMLDocument::ToUnicodeFile(const CString& strFileName)
{
    if (m_doc.isNull())
        return false;

    CString strText;
    if (!ToXML(strText, true))
        return false;

    return WizSaveUnicodeTextToUnicodeFile(strFileName, strText);
}

bool CWizXMLDocument::GetAllChildNodes(CWizStdStringArray& arrayNodeName)
{
    if (m_doc.isNull())
        return false;

    QDomNodeList nodes = m_doc.childNodes();
    int nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);
        CString strName = node.nodeName();

        arrayNodeName.push_back(strName);
    }
    return true;
}

bool CWizXMLDocument::GetAllChildNodes(std::deque<CWizXMLNode>& arrayNodes)
{
    if (m_doc.isNull())
        return false;

    QDomNodeList nodes = m_doc.childNodes();
    int nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);

        arrayNodes.push_back(node);
    }

    return true;
}

bool CWizXMLDocument::FindNodeByPath(const CString& strPath, CWizXMLNode& nodeRet)
{
    CWizStdStringArray arrayText;
    if (!WizSplitTextToArray(strPath, '/', arrayText))
        return false;

    WizStringArrayEraseEmptyLine(arrayText);

    if (arrayText.empty())
        return false;

    CWizXMLNode nodeRoot;
    if (!FindChildNode(arrayText[0], nodeRoot))
        return false;

    CWizXMLNode node = nodeRoot;

    for (size_t i = 1; i < arrayText.size(); i++)
    {
        CString strNodeName = arrayText[i];

        strNodeName.Trim();

        if (strNodeName.IsEmpty())
            return false;

        CWizXMLNode nodeChild;
        if (!node.FindChildNode(strNodeName, nodeChild))
            return false;

        node = nodeChild;
    }

    Q_ASSERT(node.Valid());

    nodeRet = node;

    return true;
}

bool CWizXMLDocument::GetNodeTextByPath(const CString& strPath, CString& strRet)
{
    CWizXMLNode node;
    if (!FindNodeByPath(strPath, node))
        return false;

    return node.GetText(strRet);
}

bool CWizXMLDocument::SettingsGetSectionNode(const CString& strRootName, const CString& strNodeName, CWizXMLNode& node)
{
    CWizXMLNode nodeRoot;
    if (!GetChildNode(strRootName, nodeRoot))
    {
        //TOLOG1(_T("Failed to get root node by name: %1"), strRootName);
        return false;
    }

    if (!nodeRoot.GetChildNode(strNodeName, node))
    {
        //TOLOG1(_T("Failed to get section node by name: %1"), strNodeName);
        return false;
    }

    return true;
}

bool CWizXMLDocument::SettingsFindSectionNode(const CString& strRootName, const CString& strNodeName, CWizXMLNode& node)
{
    CWizXMLNode nodeRoot;
    if (!FindChildNode(strRootName, nodeRoot))
    {
        //TOLOG1(_T("Failed to get root node by name: %1"), strRootName);
        return false;
    }

    if (!nodeRoot.FindChildNode(strNodeName, node))
    {
        //TOLOG1(_T("Failed to get section node by name: %1"), strNodeName);
        return false;
    }

    return true;
}

bool CWizXMLDocument::SettingsGetChildNode(const CString& strRootName, const CString& strNodeName, const CString& strSubNodeName, CWizXMLNode& node)
{
    CWizXMLNode nodeRoot;
    if (!GetChildNode(strRootName, nodeRoot))
    {
        //TOLOG1(_T("Failed to get root node by name: %1"), strRootName);
        return false;
    }

    CWizXMLNode nodeParent;
    if (!nodeRoot.GetChildNode(strNodeName, nodeParent))
    {
        //TOLOG1(_T("Failed to get section node by name: %1"), strNodeName);
        return false;
    }

    if (!nodeParent.GetChildNode(strSubNodeName, node))
    {
        TOLOG1(_T("Failed to get key node by name: %1"), strSubNodeName);
        return false;
    }

    return true;
}

bool CWizXMLDocument::SettingsGetStringValue(const CString& strRootName, const CString& strNodeName, const CString& strSubNodeName, const CString& strDefault, CString& strValue)
{
    CWizXMLNode node;
    if (!SettingsGetChildNode(strRootName, strNodeName, strSubNodeName, node))
    {
        TOLOG(_T("Failed to get value node"));
        return false;
    }

    strValue = node.GetText(strDefault);

    return true;
}

bool CWizXMLDocument::SettingsSetStringValue(const CString& strRootName, const CString& strNodeName, const CString& strSubNodeName, const CString& strValue)
{
    CWizXMLNode node;
    if (!SettingsGetChildNode(strRootName, strNodeName, strSubNodeName, node))
    {
        TOLOG(_T("Failed to get value node"));
        return false;
    }

    return node.SetText(strValue);
}
