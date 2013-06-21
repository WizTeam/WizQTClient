#ifndef WIZXML_H
#define WIZXML_H

#include <QtXml>

#include "wizqthelper.h"


class CWizXMLNode
{
public:
    CWizXMLNode() {}
    CWizXMLNode(const QDomNode& node) { m_node = node; }
    CWizXMLNode(const CWizXMLNode& nodeSrc);

    CWizXMLNode& operator = (const CWizXMLNode& right);

protected:
    QDomNode m_node;

public:
    bool isNull() const { return m_node.isNull(); }
    QDomNode& GetNode() { return m_node; }

    bool GetName(QString& strName);
    QString GetName();
    QString GetType();

    QString GetText(const QString& strDefault = "");
    bool GetText(QString& strText);
    bool SetText(const QString& strText);

    bool GetAttributeText(const QString& strName, QString& strVal);
    bool GetAttributeInt(const QString& strName, int& nVal);
    bool GetAttributeInt64(const QString& strName, qint64& nVal);
    bool GetAttributeUINT(const QString& strName, quint32& nVal);
    bool GetAttributeTimeT(const QString& strName, time_t& nVal);
    bool GetAttributeTimeString(const QString& strName, COleDateTime& t);
    bool GetAttributeBool(const QString& strName, bool& bVal);
    bool GetAttributeDWORD(const QString& strName, DWORD& dwVal);
    QString GetAttributeTextDef(const QString& strName, const QString& strDefault);
    int GetAttributeIntDef(const QString& strName, int nDefault);
    __int64 GetAttributeInt64Def(const QString& strName, __int64 Default);
    bool GetAttributeBoolDef(const QString& strName, bool bDefault);

    bool SetAttributeText(const QString& strName, const QString& strText);
    bool SetAttributeInt(const QString& strName, int nVal);
    bool SetAttributeInt64(const QString& strName, __int64 nVal);
    bool SetAttributeBool(const QString& strName, bool bVal);
    bool SetAttributeTime(const QString& strName, const COleDateTime& t);

    bool FindChildNode(const QString& strNodeName, CWizXMLNode& nodeChild);
    bool AppendChild(const QString& strNodeName, CWizXMLNode& nodeChild);
    bool AppendChild(const QString& strNodeName, const QString& strChildNodeText);
    bool GetChildNode(const QString& strNodeName, CWizXMLNode& nodeChild);
    bool SetChildNodeText(const QString& strNodeName, const QString& strText);
    bool GetChildNodeText(const QString& strNodeName, QString& strText);
    QString GetChildNodeTextDef(const QString& strNodeName, const QString& strDef);

    bool GetAllChildNodes(CWizStdStringArray& arrayNodeName);
    bool GetAllChildNodes(std::deque<CWizXMLNode>& arrayNodes);
    bool GetFirstChildNode(CWizXMLNode& nodeChild);
    QString GetFirstChildNodeText(const QString& strDef = "");

    bool DeleteChild(const QString& strChildName);
    bool DeleteChild(CWizXMLNode& nodeChild);
    bool DeleteAllChild(const QString& strExceptNodeName1 = "", \
                        const QString& strExceptNodeName2 = "", \
                        const QString& strExceptNodeName3 = "");

    bool HasChildNode();
    int GetChildNodesCount();

    bool AppendNodeByPath(const QString& strPath, CWizXMLNode& nodeRet);
    bool AppendNodeSetTextByPath(const QString& strPath, const QString& strText);

    bool FindNodeByPath(const QString& strPath, CWizXMLNode& nodeRet);
    bool FindNodeTextByPath(const QString& strPath, QString& strRet);

    bool GetElementNodeByValue(const QString& strElementName, \
                               const QString& strValueName, \
                               const QString& strValue, \
                               CWizXMLNode& nodeRet);

    bool GetElementOtherNodeByValue(const QString& strElementName, \
                                    const QString& strValueName, \
                                    const QString& strValue, \
                                    const QString& strOtherNodePath, \
                                    CWizXMLNode& nodeRet);

    bool GetElementOtherNodeByValueReturnString(const QString& strElementName, \
                                                const QString& strValueName, \
                                                const QString& strValue, \
                                                const QString& strOtherNodePath, \
                                                QString& strRet);

    bool GetElementOtherNodeByValueReturnInt(const QString& strElementName, \
                                             const QString& strValueName, \
                                             const QString& strValue, \
                                             const QString& strOtherNodePath, \
                                             int& nRet);

    bool GetElementOtherNodeByValueReturnBool(const QString& strElementName, \
                                              const QString& strValueName, \
                                              const QString& strValue, \
                                              const QString& strOtherNodePath, \
                                              bool& bRet);

public:
    static bool FindChildNode(const QDomNodeList& nodes, const QString& strName, QDomNode& nodeRet);
};


class CWizXMLDocument
{
public:
    CWizXMLDocument() {}

protected:
    QDomDocument m_doc;
    QString m_strErrorMessage;
    QString m_strErrorSrcText;

public:
    QDomDocument* GetDoc() { return &m_doc; }

    bool LoadXML(const QString& strXML);
    bool LoadFromFile(const QString& strFileName, bool bPromptError = true);
    void Clear();

    bool ToXML(QString& strText, bool bFormatText);
    bool ToUnicodeFile(const QString& strFileName);

    bool IsFail();
    QString GetErrorMessage() const { return m_strErrorMessage; }
    QString GetErrorSrcText() const { return m_strErrorSrcText; }

    bool FindChildNode(const QString& strName, CWizXMLNode& nodeChild);
    bool AppendChild(const QString& strNodeName, CWizXMLNode& nodeChild);
    bool GetChildNode(const QString& strName, CWizXMLNode& nodeChild);

    bool GetAllChildNodes(CWizStdStringArray& arrayNodeName);
    bool GetAllChildNodes(std::deque<CWizXMLNode>& arrayNodes);

    bool FindNodeByPath(const QString& strPath, CWizXMLNode& nodeRet);
    bool GetNodeTextByPath(const QString& strPath, QString& strRet);

    bool SettingsGetSectionNode(const QString& strRootName, \
                                const QString& strNodeName, \
                                CWizXMLNode& node);

    bool SettingsFindSectionNode(const QString& strRootName, \
                                 const QString& strNodeName, \
                                 CWizXMLNode& node);

    bool SettingsGetChildNode(const QString& strRootName, \
                              const QString& strNodeName, \
                              const QString& strSubNodeName, \
                              CWizXMLNode& node);

    bool SettingsGetStringValue(const QString& strRootName, \
                                const QString& strNodeName, \
                                const QString& strSubNodeName, \
                                const QString& strDefault, \
                                QString& strValue);

    bool SettingsSetStringValue(const QString& strRootName, \
                                const QString& strNodeName, \
                                const QString& strSubNodeName, \
                                const QString& strValue);
};


#endif // WIZXML_H
