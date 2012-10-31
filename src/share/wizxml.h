#ifndef WIZXML_H
#define WIZXML_H

#include <QtXml>

#include "wizmisc.h"
#include "wizqthelper.h"


class CWizXMLNode
{
public:
    CWizXMLNode();
    CWizXMLNode(const QDomNode& node);
    CWizXMLNode(const CWizXMLNode& nodeSrc);
    virtual ~CWizXMLNode();

public:
    CWizXMLNode& operator = (const CWizXMLNode& right);

protected:
    QDomNode m_node;

public:
    void InitData(const QDomNode& node);
    QDomNode& GetNode() { return m_node; }
    bool Valid() const { return !m_node.isNull(); }
    void Clear() { m_node.clear(); }

    bool GetName(CString& strName);
    CString GetName();
    CString GetType();

    CString GetText(const CString& strDefault = "");
    bool GetText(CString& strText);
    bool SetText(const CString& strText);

    bool GetAttributeText(const CString& strName, CString& strVal);
    bool GetAttributeInt(const CString& strName, int& nVal);
    bool GetAttributeInt64(const CString& strName, __int64& nVal);
    bool GetAttributeUINT(const CString& strName, UINT& nVal);
    bool GetAttributeTimeT(const CString& strName, time_t& nVal);
    bool GetAttributeTimeString(const CString& strName, COleDateTime& t);
    bool GetAttributeBool(const CString& strName, bool& bVal);
    bool GetAttributeDWORD(const CString& strName, DWORD& dwVal);
    CString GetAttributeTextDef(const CString& strName, const CString& strDefault);
    int GetAttributeIntDef(const CString& strName, int nDefault);
    __int64 GetAttributeInt64Def(const CString& strName, __int64 Default);
    bool GetAttributeBoolDef(const CString& strName, bool bDefault);

    bool SetAttributeText(const CString& strName, const CString& strText);
    bool SetAttributeInt(const CString& strName, int nVal);
    bool SetAttributeInt64(const CString& strName, __int64 nVal);
    bool SetAttributeBool(const CString& strName, bool bVal);
    bool SetAttributeTime(const CString& strName, const COleDateTime& t);

    bool FindChildNode(const CString& strNodeName, CWizXMLNode& nodeChild);
    bool AppendChild(const CString& strNodeName, CWizXMLNode& nodeChild);
    bool AppendChild(const CString& strNodeName, const CString& strChildNodeText);
    bool GetChildNode(const CString& strNodeName, CWizXMLNode& nodeChild);
    bool SetChildNodeText(const CString& strNodeName, const CString& strText);
    bool GetChildNodeText(const CString& strNodeName, CString& strText);
    CString GetChildNodeTextDef(const CString& strNodeName, const CString& strDef);

    bool GetAllChildNodes(CWizStdStringArray& arrayNodeName);
    bool GetAllChildNodes(std::deque<CWizXMLNode>& arrayNodes);
    bool GetFirstChildNode(CWizXMLNode& nodeChild);
    CString GetFirstChildNodeText(const CString& strDef = "");

    bool DeleteChild(const CString& strChildName);
    bool DeleteChild(CWizXMLNode& nodeChild);
    bool DeleteAllChild(const CString& strExceptNodeName1 = "", \
                        const CString& strExceptNodeName2 = "", \
                        const CString& strExceptNodeName3 = "");

    bool HasChildNode();
    int GetChildNodesCount();

    bool AppendNodeByPath(const CString& strPath, CWizXMLNode& nodeRet);
    bool AppendNodeSetTextByPath(const CString& strPath, const CString& strText);

    bool FindNodeByPath(const CString& strPath, CWizXMLNode& nodeRet);
    bool FindNodeTextByPath(const CString& strPath, CString& strRet);

    bool GetElementNodeByValue(const CString& strElementName, \
                               const CString& strValueName, \
                               const CString& strValue, \
                               CWizXMLNode& nodeRet);

    bool GetElementOtherNodeByValue(const CString& strElementName, \
                                    const CString& strValueName, \
                                    const CString& strValue, \
                                    const CString& strOtherNodePath, \
                                    CWizXMLNode& nodeRet);

    bool GetElementOtherNodeByValueReturnString(const CString& strElementName, \
                                                const CString& strValueName, \
                                                const CString& strValue, \
                                                const CString& strOtherNodePath, \
                                                CString& strRet);

    bool GetElementOtherNodeByValueReturnInt(const CString& strElementName, \
                                             const CString& strValueName, \
                                             const CString& strValue, \
                                             const CString& strOtherNodePath, \
                                             int& nRet);

    bool GetElementOtherNodeByValueReturnBool(const CString& strElementName, \
                                              const CString& strValueName, \
                                              const CString& strValue, \
                                              const CString& strOtherNodePath, \
                                              bool& bRet);

public:
    static bool FindChildNode(const QDomNodeList& nodes, const CString& strName, QDomNode& nodeRet);
};


class CWizXMLDocument
{
public:
    CWizXMLDocument();
    virtual ~CWizXMLDocument();

protected:
    QDomDocument m_doc;
    CString m_strErrorMessage;
    CString m_strErrorSrcText;

    bool Create();

public:
    QDomDocument* GetDoc() { return &m_doc; }

    bool LoadXML(const CString& strXML);
    bool LoadFromFile(const CString& strFileName, bool bPromptError = true);
    void Clear();

    bool ToXML(CString& strText, bool bFormatText);
    bool ToUnicodeFile(const CString& strFileName);

    bool IsFail();
    CString GetErrorMessage() const { return m_strErrorMessage; }
    CString GetErrorSrcText() const { return m_strErrorSrcText; }

    bool FindChildNode(const CString& strName, CWizXMLNode& nodeChild);
    bool AppendChild(const CString& strNodeName, CWizXMLNode& nodeChild);
    bool GetChildNode(const CString& strName, CWizXMLNode& nodeChild);

    bool GetAllChildNodes(CWizStdStringArray& arrayNodeName);
    bool GetAllChildNodes(std::deque<CWizXMLNode>& arrayNodes);

    bool FindNodeByPath(const CString& strPath, CWizXMLNode& nodeRet);
    bool GetNodeTextByPath(const CString& strPath, CString& strRet);

    bool SettingsGetSectionNode(const CString& strRootName, \
                                const CString& strNodeName, \
                                CWizXMLNode& node);

    bool SettingsFindSectionNode(const CString& strRootName, \
                                 const CString& strNodeName, \
                                 CWizXMLNode& node);

    bool SettingsGetChildNode(const CString& strRootName, \
                              const CString& strNodeName, \
                              const CString& strSubNodeName, \
                              CWizXMLNode& node);

    bool SettingsGetStringValue(const CString& strRootName, \
                                const CString& strNodeName, \
                                const CString& strSubNodeName, \
                                const CString& strDefault, \
                                CString& strValue);

    bool SettingsSetStringValue(const CString& strRootName, \
                                const CString& strNodeName, \
                                const CString& strSubNodeName, \
                                const CString& strValue);
};


#endif // WIZXML_H
