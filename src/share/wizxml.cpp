#include "wizxml.h"

#include "wizmisc.h"
#include "../utils/logger.h"


CWizXMLNode::CWizXMLNode(const CWizXMLNode& nodeSrc)
{
    if (!nodeSrc.isNull()) {
        m_node = nodeSrc.m_node;
    } else {
        m_node.clear();
    }
}

CWizXMLNode& CWizXMLNode::operator = (const CWizXMLNode& right)
{
    if (!right.m_node.isNull()) {
        m_node = right.m_node;
    } else {
        m_node.clear();
    }

    return *this;
}

bool CWizXMLNode::GetName(QString& strName)
{
    strName = m_node.nodeName();
    return true;
}

QString CWizXMLNode::GetName()
{
    return m_node.nodeName();
}

QString CWizXMLNode::GetType()
{
    if (m_node.isNull())
        return QString();

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

    return QString();
}

QString CWizXMLNode::GetText(const QString& strDefault /* = "" */)
{
    QString str;
    if (!GetText(str))
    {
        str = strDefault;
    }

    return str;
}

bool CWizXMLNode::GetText(QString& strText)
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

bool CWizXMLNode::SetText(const QString& strText)
{
    Q_ASSERT(!m_node.isNull());

    QDomNode child = m_node.ownerDocument().createTextNode(strText);

    m_node.appendChild(child);
    return true;
}

bool CWizXMLNode::GetAttributeText(const QString& strName, QString& strVal)
{
    QDomNamedNodeMap nodes = m_node.attributes();
    QDomNode node = nodes.namedItem(strName);
    if (node.isNull())
        return false;

    strVal = node.nodeValue();
    return true;
}

bool CWizXMLNode::GetAttributeInt(const QString& strName, int& nVal)
{
    QString strRet;
    if (!GetAttributeText(strName, strRet))
        return false;

    nVal = _ttoi(strRet);
    return true;
}

bool CWizXMLNode::GetAttributeInt64(const QString& strName, qint64& nVal)
{
    QString strRet;
    if (!GetAttributeText(strName, strRet))
        return false;

    nVal = _ttoi64(strRet);
    return true;
}

__int64 CWizXMLNode::GetAttributeInt64Def(const QString& strName, __int64 nDefault)
{
    QString strRet;
    if (!GetAttributeText(strName, strRet))
        return nDefault;

    return _ttoi64(strRet);
}

bool CWizXMLNode::GetAttributeUINT(const QString& strName, quint32& nVal)
{
    int n;
    if (!GetAttributeInt(strName, n))
        return false;

    nVal = n;
    return true;
}

bool CWizXMLNode::GetAttributeTimeT(const QString& strName, time_t& nVal)
{
    qint64 n64;
    if (!GetAttributeInt64(strName, n64))
        return false;

    nVal = time_t(n64);
    return true;
}

bool CWizXMLNode::GetAttributeTimeString(const QString& strName, COleDateTime& t)
{
    QString str;
    if (!GetAttributeText(strName, str))
        return false;

    QString strError;
    if (!WizStringToDateTime(str, t, strError))
    {
        TOLOG(strError);
        return false;
    }

    return true;
}

bool CWizXMLNode::GetAttributeDWORD(const QString& strName, DWORD& dwVal)
{
    int nVal = 0;
    if (!GetAttributeInt(strName, nVal))
        return false;

    dwVal = DWORD(nVal);
    return true;
}

bool CWizXMLNode::GetAttributeBool(const QString& strName, bool& bVal)
{
    QString strRet;
    if (!GetAttributeText(strName, strRet))
        return false;

    strRet = strRet.trimmed();
    bVal = strRet.compare("1") == 0;
    return true;
}

QString CWizXMLNode::GetAttributeTextDef(const QString& strName, const QString& strDefault)
{
    QString strVal;
    if (!GetAttributeText(strName, strVal))
    {
        strVal = strDefault;
    }
    return strVal;
}

int CWizXMLNode::GetAttributeIntDef(const QString& strName, int nDefault)
{
    int nVal = 0;
    if (!GetAttributeInt(strName, nVal))
    {
        nVal = nDefault;
    }
    return nVal;
}

bool CWizXMLNode::GetAttributeBoolDef(const QString& strName, bool bDefault)
{
    bool bVal = false;
    if (!GetAttributeBool(strName, bVal))
    {
        bVal = bDefault;
    }
    return bVal;
}

bool CWizXMLNode::SetAttributeText(const QString& strName, const QString& strText)
{
    Q_ASSERT(!m_node.isNull());

    QDomAttr att = m_node.ownerDocument().createAttribute(strName);
    att.setNodeValue(strText);

    QDomNamedNodeMap nodes = m_node.attributes();
    nodes.setNamedItem(att);

    return true;
}

bool CWizXMLNode::SetAttributeInt(const QString& strName, int nVal)
{
    return SetAttributeText(strName, WizIntToStr(nVal));
}

bool CWizXMLNode::SetAttributeInt64(const QString& strName, __int64 nVal)
{
    return SetAttributeText(strName, WizInt64ToStr(nVal));
}

bool CWizXMLNode::SetAttributeTime(const QString& strName, const COleDateTime& t)
{
    return SetAttributeText(strName, WizDateTimeToString(t));
}

bool CWizXMLNode::SetAttributeBool(const QString& strName, bool bVal)
{
    return SetAttributeText(strName, bVal ? _T("1") : _T("0"));
}

bool CWizXMLNode::FindChildNode(const QString& strNodeName, CWizXMLNode& nodeChild)
{
    QDomNodeList nodes = m_node.childNodes();
    QDomNode node;
    if (!FindChildNode(nodes, strNodeName, node))
        return false;

    nodeChild = node;
    return true;
}

bool CWizXMLNode::GetAllChildNodes(CWizStdStringArray& arrayNodeName)
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);
        QString strName = node.nodeName();

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

    QString str = node.GetType();
    if (str != _T("element"))
        return false;

    nodeChild = node;
    return true;
}

QString CWizXMLNode::GetFirstChildNodeText(const QString& strDef /* = NULL */)
{
    CWizXMLNode nodeChild;
    if (!GetFirstChildNode(nodeChild))
        return strDef;

    return nodeChild.GetText(strDef);
}

bool CWizXMLNode::DeleteChild(const QString& strChildName)
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

bool CWizXMLNode::DeleteAllChild(const QString& strExceptNodeName1, \
                                 const QString& strExceptNodeName2, \
                                 const QString& strExceptNodeName3)
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    for (int i = nCount - 1; i >= 0; i--)
    {
        QDomNode node = nodes.item(i);

        if (!strExceptNodeName1.isEmpty()
            || !strExceptNodeName2.isEmpty()
            || !strExceptNodeName3.isEmpty()
            )
        {
            QString strName = node.nodeName();

            if (!strExceptNodeName1.isEmpty()
                && strName == strExceptNodeName1)
                continue;

            if (!strExceptNodeName2.isEmpty()
                && strName == strExceptNodeName2)
                continue;

            if (!strExceptNodeName3.isEmpty()
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

bool CWizXMLNode::AppendChild(const QString& strNodeName, CWizXMLNode& nodeChild)
{
    QDomNode node = m_node.ownerDocument().createElement(strNodeName);

    nodeChild = m_node.appendChild(node);
    return true;
}

bool CWizXMLNode::AppendChild(const QString& strNodeName, const QString& strChildNodeText)
{
    CWizXMLNode nodeChild;
    if (!AppendChild(strNodeName, nodeChild))
        return false;

    return nodeChild.SetText(strChildNodeText);
}

bool CWizXMLNode::SetChildNodeText(const QString& strNodeName, const QString& strText)
{
    CWizXMLNode nodeChild;
    if (!GetChildNode(strNodeName, nodeChild))
        return false;

    return nodeChild.SetText(strText);
}

bool CWizXMLNode::GetChildNodeText(const QString& strNodeName, QString& strText)
{
    CWizXMLNode nodeChild;
    if (!GetChildNode(strNodeName, nodeChild))
        return false;

    return nodeChild.GetText(strText);
}

QString CWizXMLNode::GetChildNodeTextDef(const QString& strNodeName, const QString& strDef)
{
    QString str;
    if (GetChildNodeText(strNodeName, str))
        return str;

    return strDef;
}

bool CWizXMLNode::GetChildNode(const QString& strNodeName, CWizXMLNode& nodeChild)
{
    if (FindChildNode(strNodeName, nodeChild))
        return true;
    return AppendChild(strNodeName, nodeChild);
}

bool CWizXMLNode::FindChildNode(const QDomNodeList& nodes, const QString& strName, QDomNode& nodeRet)
{
    long nCount = nodes.count();
    for (int i = 0; i < nCount; i++)
    {
        QDomNode node = nodes.item(i);
        QString strNodeName = node.nodeName();

        if (strNodeName.compare(strName, Qt::CaseInsensitive) == 0)
        {
            nodeRet = node;
            return true;
        }
    }
    return false;
}

bool CWizXMLNode::FindNodeByPath(const QString& strPath, CWizXMLNode& nodeRet)
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
        QString strNodeName = arrayText[i];

        strNodeName = strNodeName.trimmed();

        if (strNodeName.isEmpty())
            return false;

        CWizXMLNode nodeChild;
        if (!node.FindChildNode(strNodeName, nodeChild))
            return false;

        node = nodeChild;
    }

    Q_ASSERT(!node.isNull());

    nodeRet = node;

    return true;
}

bool CWizXMLNode::AppendNodeByPath(const QString& strPath, CWizXMLNode& nodeRet)
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
        QString strNodeName = arrayText[i];

        strNodeName = strNodeName.trimmed();

        if (strNodeName.isEmpty())
            return false;

        CWizXMLNode nodeChild;
        if (!node.AppendChild(strNodeName, nodeChild))
        {
            TOLOG1("Failed to append child node: %1", strNodeName);
            return false;
        }

        node = nodeChild;
    }

    Q_ASSERT(!node.isNull());

    nodeRet = node;

    return true;
}

bool CWizXMLNode::AppendNodeSetTextByPath(const QString& strPath, const QString& strText)
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

bool CWizXMLNode::FindNodeTextByPath(const QString& strPath, QString& strRet)
{
    CWizXMLNode node;
    if (!FindNodeByPath(strPath, node))
        return false;

    return node.GetText(strRet);
}

bool CWizXMLNode::GetElementNodeByValue(const QString& strElementName, \
                                        const QString& strValueName, \
                                        const QString& strValue, \
                                        CWizXMLNode& nodeRet)
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

        if (0 == nodeValue.GetText().compare(strValue, Qt::CaseInsensitive))
        {
            nodeRet = node;
            return true;
        }
    }

    return false;
}

bool CWizXMLNode::GetElementOtherNodeByValue(const QString& strElementName, \
                                             const QString& strValueName, \
                                             const QString& strValue, \
                                             const QString& strOtherNodePath, \
                                             CWizXMLNode& nodeRet)
{
    CWizXMLNode nodeElement;
    if (!GetElementNodeByValue(strElementName, strValueName, strValue, nodeElement))
        return false;

    return nodeElement.FindNodeByPath(strOtherNodePath, nodeRet);
}

bool CWizXMLNode::GetElementOtherNodeByValueReturnString(const QString& strElementName, \
                                                         const QString& strValueName, \
                                                         const QString& strValue, \
                                                         const QString& strOtherNodePath, \
                                                         QString& strRet)
{
    CWizXMLNode nodeRet;
    if (!GetElementOtherNodeByValue(strElementName, strValueName, strValue, strOtherNodePath, nodeRet))
        return false;

    strRet = nodeRet.GetText();

    return true;
}

bool CWizXMLNode::GetElementOtherNodeByValueReturnInt(const QString& strElementName, \
                                                      const QString& strValueName, \
                                                      const QString& strValue, \
                                                      const QString& strOtherNodePath, \
                                                      int& nRet)
{
    QString strRet;
    if (!GetElementOtherNodeByValueReturnString(strElementName, strValueName, strValue, strOtherNodePath, strRet))
        return false;

    strRet = strRet.trimmed();

    int nTemp = _ttoi(strRet);
    if (WizIntToStr(nTemp) == strRet)
    {
        nRet = nTemp;
        return true;
    }

    return false;
}

bool CWizXMLNode::GetElementOtherNodeByValueReturnBool(const QString& strElementName, \
                                                       const QString& strValueName, \
                                                       const QString& strValue, \
                                                       const QString& strOtherNodePath, \
                                                       bool& bRet)
{
    QString strRet;
    if (!GetElementOtherNodeByValueReturnString(strElementName, strValueName, strValue, strOtherNodePath, strRet))
        return false;

    strRet = strRet.trimmed();
    strRet = strRet.toLower();

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



bool CWizXMLDocument::LoadXML(const QString& strXML)
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;
    if (m_doc.setContent(strXML, &errorMsg, &errorLine, &errorColumn))
        return true;

    return false;
}

bool CWizXMLDocument::LoadFromFile(const QString& strFileName, bool bPromptError /*= true*/)
{
    Q_UNUSED(bPromptError);

    if (!PathFileExists(strFileName))
        return false;

    QString strXml;
    if (!::WizLoadUnicodeTextFromFile(strFileName, strXml))
        return false;

    return LoadXML(strXml);
}

void CWizXMLDocument::Clear()
{
    m_doc.clear();
}

bool CWizXMLDocument::FindChildNode(const QString& strName, CWizXMLNode& nodeChild)
{
    if (m_doc.isNull())
        return false;

    QDomNodeList nodes = m_doc.childNodes();

    QDomNode node;
    if (!CWizXMLNode::FindChildNode(nodes, strName, node))
        return false;

    nodeChild = node;

    return true;
}

bool CWizXMLDocument::IsFail()
{
    return false;
}

bool CWizXMLDocument::AppendChild(const QString& strNodeName, CWizXMLNode& nodeChild)
{    
    QDomNode node = m_doc.createElement(strNodeName);
    nodeChild = m_doc.appendChild(node);

    return true;
}

bool CWizXMLDocument::GetChildNode(const QString& strName, CWizXMLNode& nodeChild)
{    

    if (FindChildNode(strName, nodeChild))
        return true;
    return AppendChild(strName, nodeChild);
}

bool CWizXMLDocument::ToXML(QString& strText, bool bFormatText)
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

bool CWizXMLDocument::ToUnicodeFile(const QString& strFileName)
{
    if (m_doc.isNull())
        return false;

    QString strText;
    if (!ToXML(strText, true))
        return false;

    return WizSaveUnicodeTextToUtf8File(strFileName, strText);
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
        QString strName = node.nodeName();

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

bool CWizXMLDocument::FindNodeByPath(const QString& strPath, CWizXMLNode& nodeRet)
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
        QString strNodeName = arrayText[i];

        strNodeName = strNodeName.trimmed();

        if (strNodeName.isEmpty())
            return false;

        CWizXMLNode nodeChild;
        if (!node.FindChildNode(strNodeName, nodeChild))
            return false;

        node = nodeChild;
    }

    Q_ASSERT(!node.isNull());

    nodeRet = node;

    return true;
}

bool CWizXMLDocument::GetNodeTextByPath(const QString& strPath, QString& strRet)
{
    CWizXMLNode node;
    if (!FindNodeByPath(strPath, node))
        return false;

    return node.GetText(strRet);
}

bool CWizXMLDocument::SettingsGetSectionNode(const QString& strRootName, const QString& strNodeName, CWizXMLNode& node)
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

bool CWizXMLDocument::SettingsFindSectionNode(const QString& strRootName, const QString& strNodeName, CWizXMLNode& node)
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

bool CWizXMLDocument::SettingsGetChildNode(const QString& strRootName, \
                                           const QString& strNodeName, \
                                           const QString& strSubNodeName, \
                                           CWizXMLNode& node)
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

bool CWizXMLDocument::SettingsGetStringValue(const QString& strRootName, \
                                             const QString& strNodeName, \
                                             const QString& strSubNodeName, \
                                             const QString& strDefault, \
                                             QString& strValue)
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

bool CWizXMLDocument::SettingsSetStringValue(const QString& strRootName, \
                                             const QString& strNodeName, \
                                             const QString& strSubNodeName, \
                                             const QString& strValue)
{
    CWizXMLNode node;
    if (!SettingsGetChildNode(strRootName, strNodeName, strSubNodeName, node))
    {
        TOLOG(_T("Failed to get value node"));
        return false;
    }

    return node.SetText(strValue);
}
