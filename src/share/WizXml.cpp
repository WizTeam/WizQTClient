#include "WizXml.h"

#include "WizMisc.h"
#include "../utils/WizLogger.h"


WizXMLNode::WizXMLNode(const WizXMLNode& nodeSrc)
{
    if (!nodeSrc.isNull()) {
        m_node = nodeSrc.m_node;
    } else {
        m_node.clear();
    }
}

WizXMLNode& WizXMLNode::operator = (const WizXMLNode& right)
{
    if (!right.m_node.isNull()) {
        m_node = right.m_node;
    } else {
        m_node.clear();
    }

    return *this;
}

bool WizXMLNode::getName(QString& strName)
{
    strName = m_node.nodeName();
    return true;
}

QString WizXMLNode::getName()
{
    return m_node.nodeName();
}

QString WizXMLNode::getType()
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

QString WizXMLNode::getText(const QString& strDefault /* = "" */)
{
    QString str;
    if (!getText(str))
    {
        str = strDefault;
    }

    return str;
}

bool WizXMLNode::getText(QString& strText)
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

bool WizXMLNode::setText(const QString& strText)
{
    Q_ASSERT(!m_node.isNull());

    QDomNode child = m_node.ownerDocument().createTextNode(strText);

    m_node.appendChild(child);
    return true;
}

bool WizXMLNode::getAttributeText(const QString& strName, QString& strVal)
{
    QDomNamedNodeMap nodes = m_node.attributes();
    QDomNode node = nodes.namedItem(strName);
    if (node.isNull())
        return false;

    strVal = node.nodeValue();
    return true;
}

bool WizXMLNode::getAttributeInt(const QString& strName, int& nVal)
{
    QString strRet;
    if (!getAttributeText(strName, strRet))
        return false;

    nVal = wiz_ttoi(strRet);
    return true;
}

bool WizXMLNode::getAttributeInt64(const QString& strName, qint64& nVal)
{
    QString strRet;
    if (!getAttributeText(strName, strRet))
        return false;

    nVal = wiz_ttoi64(strRet);
    return true;
}

__int64 WizXMLNode::getAttributeInt64Def(const QString& strName, __int64 nDefault)
{
    QString strRet;
    if (!getAttributeText(strName, strRet))
        return nDefault;

    return wiz_ttoi64(strRet);
}

bool WizXMLNode::getAttributeUINT(const QString& strName, quint32& nVal)
{
    int n;
    if (!getAttributeInt(strName, n))
        return false;

    nVal = n;
    return true;
}

bool WizXMLNode::getAttributeTimeT(const QString& strName, time_t& nVal)
{
    qint64 n64;
    if (!getAttributeInt64(strName, n64))
        return false;

    nVal = time_t(n64);
    return true;
}

bool WizXMLNode::getAttributeTimeString(const QString& strName, WizOleDateTime& t)
{
    QString str;
    if (!getAttributeText(strName, str))
        return false;

    QString strError;
    if (!WizStringToDateTime(str, t, strError))
    {
        TOLOG(strError);
        return false;
    }

    return true;
}

bool WizXMLNode::getAttributeDWORD(const QString& strName, DWORD& dwVal)
{
    int nVal = 0;
    if (!getAttributeInt(strName, nVal))
        return false;

    dwVal = DWORD(nVal);
    return true;
}

bool WizXMLNode::getAttributeBool(const QString& strName, bool& bVal)
{
    QString strRet;
    if (!getAttributeText(strName, strRet))
        return false;

    strRet = strRet.trimmed();
    bVal = strRet.compare("1") == 0;
    return true;
}

QString WizXMLNode::getAttributeTextDef(const QString& strName, const QString& strDefault)
{
    QString strVal;
    if (!getAttributeText(strName, strVal))
    {
        strVal = strDefault;
    }
    return strVal;
}

int WizXMLNode::getAttributeIntDef(const QString& strName, int nDefault)
{
    int nVal = 0;
    if (!getAttributeInt(strName, nVal))
    {
        nVal = nDefault;
    }
    return nVal;
}

bool WizXMLNode::getAttributeBoolDef(const QString& strName, bool bDefault)
{
    bool bVal = false;
    if (!getAttributeBool(strName, bVal))
    {
        bVal = bDefault;
    }
    return bVal;
}

bool WizXMLNode::setAttributeText(const QString& strName, const QString& strText)
{
    Q_ASSERT(!m_node.isNull());

    QDomAttr att = m_node.ownerDocument().createAttribute(strName);
    att.setNodeValue(strText);

    QDomNamedNodeMap nodes = m_node.attributes();
    nodes.setNamedItem(att);

    return true;
}

bool WizXMLNode::setAttributeInt(const QString& strName, int nVal)
{
    return setAttributeText(strName, WizIntToStr(nVal));
}

bool WizXMLNode::setAttributeInt64(const QString& strName, __int64 nVal)
{
    return setAttributeText(strName, WizInt64ToStr(nVal));
}

bool WizXMLNode::setAttributeTime(const QString& strName, const WizOleDateTime& t)
{
    return setAttributeText(strName, WizDateTimeToString(t));
}

bool WizXMLNode::setAttributeBool(const QString& strName, bool bVal)
{
    return setAttributeText(strName, bVal ? "1" : "0");
}

bool WizXMLNode::findChildNode(const QString& strNodeName, WizXMLNode& nodeChild)
{
    QDomNodeList nodes = m_node.childNodes();
    QDomNode node;
    if (!findChildNode(nodes, strNodeName, node))
        return false;

    nodeChild = node;
    return true;
}

bool WizXMLNode::getAllChildNodes(CWizStdStringArray& arrayNodeName)
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

bool WizXMLNode::getAllChildNodes(std::deque<WizXMLNode>& arrayNodes)
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

bool WizXMLNode::getFirstChildNode(WizXMLNode& nodeChild)
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    if (nCount <= 0)
        return false;

    WizXMLNode node = nodes.item(0);

    QString str = node.getType();
    if (str != "element")
        return false;

    nodeChild = node;
    return true;
}

QString WizXMLNode::getFirstChildNodeText(const QString& strDef /* = NULL */)
{
    WizXMLNode nodeChild;
    if (!getFirstChildNode(nodeChild))
        return strDef;

    return nodeChild.getText(strDef);
}

bool WizXMLNode::deleteChild(const QString& strChildName)
{
    WizXMLNode node;
    if (!findChildNode(strChildName, node))
        return true;

    m_node.removeChild(node.m_node);
    return true;
}

bool WizXMLNode::deleteChild(WizXMLNode& nodeChild)
{
    m_node.removeChild(nodeChild.m_node);
    return true;
}

bool WizXMLNode::deleteAllChild(const QString& strExceptNodeName1, \
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

bool WizXMLNode::hasChildNode()
{
    return m_node.hasChildNodes();
}

int WizXMLNode::getChildNodesCount()
{
    QDomNodeList nodes = m_node.childNodes();
    int nCount = nodes.count();
    int nChildCount = nCount;

    for (int i = 0; i < nCount; i++)
    {
        WizXMLNode node = nodes.item(i);

        nChildCount += node.getChildNodesCount();

    }
    return nChildCount;
}

bool WizXMLNode::appendChild(const QString& strNodeName, WizXMLNode& nodeChild)
{
    QDomNode node = m_node.ownerDocument().createElement(strNodeName);

    nodeChild = m_node.appendChild(node);
    return true;
}

bool WizXMLNode::appendChild(const QString& strNodeName, const QString& strChildNodeText)
{
    WizXMLNode nodeChild;
    if (!appendChild(strNodeName, nodeChild))
        return false;

    return nodeChild.setText(strChildNodeText);
}

bool WizXMLNode::setChildNodeText(const QString& strNodeName, const QString& strText)
{
    WizXMLNode nodeChild;
    if (!getChildNode(strNodeName, nodeChild))
        return false;

    return nodeChild.setText(strText);
}

bool WizXMLNode::getChildNodeText(const QString& strNodeName, QString& strText)
{
    WizXMLNode nodeChild;
    if (!getChildNode(strNodeName, nodeChild))
        return false;

    return nodeChild.getText(strText);
}

QString WizXMLNode::getChildNodeTextDef(const QString& strNodeName, const QString& strDef)
{
    QString str;
    if (getChildNodeText(strNodeName, str))
        return str;

    return strDef;
}

bool WizXMLNode::getChildNode(const QString& strNodeName, WizXMLNode& nodeChild)
{
    if (findChildNode(strNodeName, nodeChild))
        return true;
    return appendChild(strNodeName, nodeChild);
}

bool WizXMLNode::findChildNode(const QDomNodeList& nodes, const QString& strName, QDomNode& nodeRet)
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

bool WizXMLNode::findNodeByPath(const QString& strPath, WizXMLNode& nodeRet)
{
    CWizStdStringArray arrayText;
    if (!WizSplitTextToArray(strPath, '/', arrayText))
        return false;

    WizStringArrayEraseEmptyLine(arrayText);

    if (arrayText.empty())
        return false;

    WizXMLNode node = *this;

    for (size_t i = 0; i < arrayText.size(); i++)
    {
        QString strNodeName = arrayText[i];

        strNodeName = strNodeName.trimmed();

        if (strNodeName.isEmpty())
            return false;

        WizXMLNode nodeChild;
        if (!node.findChildNode(strNodeName, nodeChild))
            return false;

        node = nodeChild;
    }

    Q_ASSERT(!node.isNull());

    nodeRet = node;

    return true;
}

bool WizXMLNode::appendNodeByPath(const QString& strPath, WizXMLNode& nodeRet)
{
    CWizStdStringArray arrayText;
    if (!WizSplitTextToArray(strPath, '/', arrayText))
        return false;

    WizStringArrayEraseEmptyLine(arrayText);

    if (arrayText.empty())
        return false;

    WizXMLNode node = *this;

    for (size_t i = 0; i < arrayText.size(); i++)
    {
        QString strNodeName = arrayText[i];

        strNodeName = strNodeName.trimmed();

        if (strNodeName.isEmpty())
            return false;

        WizXMLNode nodeChild;
        if (!node.appendChild(strNodeName, nodeChild))
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

bool WizXMLNode::appendNodeSetTextByPath(const QString& strPath, const QString& strText)
{
    WizXMLNode nodeRet;
    if (!appendNodeByPath(strPath, nodeRet))
    {
        TOLOG1("Failed to append node by path: %!", strPath);
        return false;
    }

    bool bRet = nodeRet.setText(strText);
    if (!bRet)
    {
        TOLOG("Failed to set node text");
        return false;
    }

    return true;
}

bool WizXMLNode::findNodeTextByPath(const QString& strPath, QString& strRet)
{
    WizXMLNode node;
    if (!findNodeByPath(strPath, node))
        return false;

    return node.getText(strRet);
}

bool WizXMLNode::getElementNodeByValue(const QString& strElementName, \
                                        const QString& strValueName, \
                                        const QString& strValue, \
                                        WizXMLNode& nodeRet)
{
    std::deque<WizXMLNode> arrayNodes;
    if (!getAllChildNodes(arrayNodes))
        return false;

    size_t nCount = arrayNodes.size();
    for (size_t i = 0; i < nCount; i++)
    {
        WizXMLNode& node = arrayNodes[i];

        if (node.getName() != strElementName)
            continue;

        WizXMLNode nodeValue;
        if (!node.findNodeByPath(strValueName, nodeValue))
            continue;

        if (0 == nodeValue.getText().compare(strValue, Qt::CaseInsensitive))
        {
            nodeRet = node;
            return true;
        }
    }

    return false;
}

bool WizXMLNode::getElementOtherNodeByValue(const QString& strElementName, \
                                             const QString& strValueName, \
                                             const QString& strValue, \
                                             const QString& strOtherNodePath, \
                                             WizXMLNode& nodeRet)
{
    WizXMLNode nodeElement;
    if (!getElementNodeByValue(strElementName, strValueName, strValue, nodeElement))
        return false;

    return nodeElement.findNodeByPath(strOtherNodePath, nodeRet);
}

bool WizXMLNode::getElementOtherNodeByValueReturnString(const QString& strElementName, \
                                                         const QString& strValueName, \
                                                         const QString& strValue, \
                                                         const QString& strOtherNodePath, \
                                                         QString& strRet)
{
    WizXMLNode nodeRet;
    if (!getElementOtherNodeByValue(strElementName, strValueName, strValue, strOtherNodePath, nodeRet))
        return false;

    strRet = nodeRet.getText();

    return true;
}

bool WizXMLNode::getElementOtherNodeByValueReturnInt(const QString& strElementName, \
                                                      const QString& strValueName, \
                                                      const QString& strValue, \
                                                      const QString& strOtherNodePath, \
                                                      int& nRet)
{
    QString strRet;
    if (!getElementOtherNodeByValueReturnString(strElementName, strValueName, strValue, strOtherNodePath, strRet))
        return false;

    strRet = strRet.trimmed();

    int nTemp = wiz_ttoi(strRet);
    if (WizIntToStr(nTemp) == strRet)
    {
        nRet = nTemp;
        return true;
    }

    return false;
}

bool WizXMLNode::getElementOtherNodeByValueReturnBool(const QString& strElementName, \
                                                       const QString& strValueName, \
                                                       const QString& strValue, \
                                                       const QString& strOtherNodePath, \
                                                       bool& bRet)
{
    QString strRet;
    if (!getElementOtherNodeByValueReturnString(strElementName, strValueName, strValue, strOtherNodePath, strRet))
        return false;

    strRet = strRet.trimmed();
    strRet = strRet.toLower();

    if (strRet == "0"
        || strRet == "false")
    {
        bRet = false;

        return true;
    }
    else if (strRet == "1"
        || strRet == "true")
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



bool WizXMLDocument::loadXML(const QString& strXML)
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;
    if (m_doc.setContent(strXML, &errorMsg, &errorLine, &errorColumn))
        return true;

    return false;
}

bool WizXMLDocument::loadFromFile(const QString& strFileName, bool bPromptError /*= true*/)
{
    Q_UNUSED(bPromptError);

    if (!WizPathFileExists(strFileName))
        return false;

    QString strXml;
    if (!::WizLoadUnicodeTextFromFile(strFileName, strXml))
        return false;

    return loadXML(strXml);
}

void WizXMLDocument::clear()
{
    m_doc.clear();
}

bool WizXMLDocument::findChildNode(const QString& strName, WizXMLNode& nodeChild)
{
    if (m_doc.isNull())
        return false;

    QDomNodeList nodes = m_doc.childNodes();

    QDomNode node;
    if (!WizXMLNode::findChildNode(nodes, strName, node))
        return false;

    nodeChild = node;

    return true;
}

bool WizXMLDocument::isFail()
{
    return false;
}

bool WizXMLDocument::appendChild(const QString& strNodeName, WizXMLNode& nodeChild)
{    
    QDomNode node = m_doc.createElement(strNodeName);
    nodeChild = m_doc.appendChild(node);

    return true;
}

bool WizXMLDocument::getChildNode(const QString& strName, WizXMLNode& nodeChild)
{    

    if (findChildNode(strName, nodeChild))
        return true;
    return appendChild(strName, nodeChild);
}

bool WizXMLDocument::toXML(QString& strText, bool bFormatText)
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

bool WizXMLDocument::toUnicodeFile(const QString& strFileName)
{
    if (m_doc.isNull())
        return false;

    QString strText;
    if (!toXML(strText, true))
        return false;

    return WizSaveUnicodeTextToUtf8File(strFileName, strText);
}

bool WizXMLDocument::getAllChildNodes(CWizStdStringArray& arrayNodeName)
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

bool WizXMLDocument::getAllChildNodes(std::deque<WizXMLNode>& arrayNodes)
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

bool WizXMLDocument::findNodeByPath(const QString& strPath, WizXMLNode& nodeRet)
{
    CWizStdStringArray arrayText;
    if (!WizSplitTextToArray(strPath, '/', arrayText))
        return false;

    WizStringArrayEraseEmptyLine(arrayText);

    if (arrayText.empty())
        return false;

    WizXMLNode nodeRoot;
    if (!findChildNode(arrayText[0], nodeRoot))
        return false;

    WizXMLNode node = nodeRoot;

    for (size_t i = 1; i < arrayText.size(); i++)
    {
        QString strNodeName = arrayText[i];

        strNodeName = strNodeName.trimmed();

        if (strNodeName.isEmpty())
            return false;

        WizXMLNode nodeChild;
        if (!node.findChildNode(strNodeName, nodeChild))
            return false;

        node = nodeChild;
    }

    Q_ASSERT(!node.isNull());

    nodeRet = node;

    return true;
}

bool WizXMLDocument::getNodeTextByPath(const QString& strPath, QString& strRet)
{
    WizXMLNode node;
    if (!findNodeByPath(strPath, node))
        return false;

    return node.getText(strRet);
}

bool WizXMLDocument::settingsGetSectionNode(const QString& strRootName, const QString& strNodeName, WizXMLNode& node)
{
    WizXMLNode nodeRoot;
    if (!getChildNode(strRootName, nodeRoot))
    {
        //TOLOG1("Failed to get root node by name: %1", strRootName);
        return false;
    }

    if (!nodeRoot.getChildNode(strNodeName, node))
    {
        //TOLOG1("Failed to get section node by name: %1", strNodeName);
        return false;
    }

    return true;
}

bool WizXMLDocument::settingsFindSectionNode(const QString& strRootName, const QString& strNodeName, WizXMLNode& node)
{
    WizXMLNode nodeRoot;
    if (!findChildNode(strRootName, nodeRoot))
    {
        //TOLOG1("Failed to get root node by name: %1", strRootName);
        return false;
    }

    if (!nodeRoot.findChildNode(strNodeName, node))
    {
        //TOLOG1("Failed to get section node by name: %1", strNodeName);
        return false;
    }

    return true;
}

bool WizXMLDocument::settingsGetChildNode(const QString& strRootName, \
                                           const QString& strNodeName, \
                                           const QString& strSubNodeName, \
                                           WizXMLNode& node)
{
    WizXMLNode nodeRoot;
    if (!getChildNode(strRootName, nodeRoot))
    {
        //TOLOG1("Failed to get root node by name: %1", strRootName);
        return false;
    }

    WizXMLNode nodeParent;
    if (!nodeRoot.getChildNode(strNodeName, nodeParent))
    {
        //TOLOG1("Failed to get section node by name: %1", strNodeName);
        return false;
    }

    if (!nodeParent.getChildNode(strSubNodeName, node))
    {
        TOLOG1("Failed to get key node by name: %1", strSubNodeName);
        return false;
    }

    return true;
}

bool WizXMLDocument::settingsGetStringValue(const QString& strRootName, \
                                             const QString& strNodeName, \
                                             const QString& strSubNodeName, \
                                             const QString& strDefault, \
                                             QString& strValue)
{
    WizXMLNode node;
    if (!settingsGetChildNode(strRootName, strNodeName, strSubNodeName, node))
    {
        TOLOG("Failed to get value node");
        return false;
    }

    strValue = node.getText(strDefault);

    return true;
}

bool WizXMLDocument::settingsSetStringValue(const QString& strRootName, \
                                             const QString& strNodeName, \
                                             const QString& strSubNodeName, \
                                             const QString& strValue)
{
    WizXMLNode node;
    if (!settingsGetChildNode(strRootName, strNodeName, strSubNodeName, node))
    {
        TOLOG("Failed to get value node");
        return false;
    }

    return node.setText(strValue);
}
