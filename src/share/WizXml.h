#ifndef WIZXML_H
#define WIZXML_H

#include <QtXml>

#include "WizQtHelper.h"


class WizXMLNode
{
public:
    WizXMLNode() {}
    WizXMLNode(const QDomNode& node) { m_node = node; }
    WizXMLNode(const WizXMLNode& nodeSrc);

    WizXMLNode& operator = (const WizXMLNode& right);

protected:
    QDomNode m_node;

public:
    bool isNull() const { return m_node.isNull(); }
    QDomNode& getNode() { return m_node; }

    bool getName(QString& strName);
    QString getName();
    QString getType();

    QString getText(const QString& strDefault = "");
    bool getText(QString& strText);
    bool setText(const QString& strText);

    bool getAttributeText(const QString& strName, QString& strVal);
    bool getAttributeInt(const QString& strName, int& nVal);
    bool getAttributeInt64(const QString& strName, qint64& nVal);
    bool getAttributeUINT(const QString& strName, quint32& nVal);
    bool getAttributeTimeT(const QString& strName, time_t& nVal);
    bool getAttributeTimeString(const QString& strName, WizOleDateTime& t);
    bool getAttributeBool(const QString& strName, bool& bVal);
    bool getAttributeDWORD(const QString& strName, DWORD& dwVal);
    QString getAttributeTextDef(const QString& strName, const QString& strDefault);
    int getAttributeIntDef(const QString& strName, int nDefault);
    __int64 getAttributeInt64Def(const QString& strName, __int64 Default);
    bool getAttributeBoolDef(const QString& strName, bool bDefault);

    bool setAttributeText(const QString& strName, const QString& strText);
    bool setAttributeInt(const QString& strName, int nVal);
    bool setAttributeInt64(const QString& strName, __int64 nVal);
    bool setAttributeBool(const QString& strName, bool bVal);
    bool setAttributeTime(const QString& strName, const WizOleDateTime& t);

    bool findChildNode(const QString& strNodeName, WizXMLNode& nodeChild);
    bool appendChild(const QString& strNodeName, WizXMLNode& nodeChild);
    bool appendChild(const QString& strNodeName, const QString& strChildNodeText);
    bool getChildNode(const QString& strNodeName, WizXMLNode& nodeChild);
    bool setChildNodeText(const QString& strNodeName, const QString& strText);
    bool getChildNodeText(const QString& strNodeName, QString& strText);
    QString getChildNodeTextDef(const QString& strNodeName, const QString& strDef);

    bool getAllChildNodes(CWizStdStringArray& arrayNodeName);
    bool getAllChildNodes(std::deque<WizXMLNode>& arrayNodes);
    bool getFirstChildNode(WizXMLNode& nodeChild);
    QString getFirstChildNodeText(const QString& strDef = "");

    bool deleteChild(const QString& strChildName);
    bool deleteChild(WizXMLNode& nodeChild);
    bool deleteAllChild(const QString& strExceptNodeName1 = "", \
                        const QString& strExceptNodeName2 = "", \
                        const QString& strExceptNodeName3 = "");

    bool hasChildNode();
    int getChildNodesCount();

    bool appendNodeByPath(const QString& strPath, WizXMLNode& nodeRet);
    bool appendNodeSetTextByPath(const QString& strPath, const QString& strText);

    bool findNodeByPath(const QString& strPath, WizXMLNode& nodeRet);
    bool findNodeTextByPath(const QString& strPath, QString& strRet);

    bool getElementNodeByValue(const QString& strElementName, \
                               const QString& strValueName, \
                               const QString& strValue, \
                               WizXMLNode& nodeRet);

    bool getElementOtherNodeByValue(const QString& strElementName, \
                                    const QString& strValueName, \
                                    const QString& strValue, \
                                    const QString& strOtherNodePath, \
                                    WizXMLNode& nodeRet);

    bool getElementOtherNodeByValueReturnString(const QString& strElementName, \
                                                const QString& strValueName, \
                                                const QString& strValue, \
                                                const QString& strOtherNodePath, \
                                                QString& strRet);

    bool getElementOtherNodeByValueReturnInt(const QString& strElementName, \
                                             const QString& strValueName, \
                                             const QString& strValue, \
                                             const QString& strOtherNodePath, \
                                             int& nRet);

    bool getElementOtherNodeByValueReturnBool(const QString& strElementName, \
                                              const QString& strValueName, \
                                              const QString& strValue, \
                                              const QString& strOtherNodePath, \
                                              bool& bRet);

public:
    static bool findChildNode(const QDomNodeList& nodes, const QString& strName, QDomNode& nodeRet);
};


class WizXMLDocument
{
public:
    WizXMLDocument() {}

protected:
    QDomDocument m_doc;
    QString m_strErrorMessage;
    QString m_strErrorSrcText;

public:
    QDomDocument* getDoc() { return &m_doc; }

    bool loadXML(const QString& strXML);
    bool loadFromFile(const QString& strFileName, bool bPromptError = true);
    void clear();

    bool toXML(QString& strText, bool bFormatText);
    bool toUnicodeFile(const QString& strFileName);

    bool isFail();
    QString getErrorMessage() const { return m_strErrorMessage; }
    QString getErrorSrcText() const { return m_strErrorSrcText; }

    bool findChildNode(const QString& strName, WizXMLNode& nodeChild);
    bool appendChild(const QString& strNodeName, WizXMLNode& nodeChild);
    bool getChildNode(const QString& strName, WizXMLNode& nodeChild);

    bool getAllChildNodes(CWizStdStringArray& arrayNodeName);
    bool getAllChildNodes(std::deque<WizXMLNode>& arrayNodes);

    bool findNodeByPath(const QString& strPath, WizXMLNode& nodeRet);
    bool getNodeTextByPath(const QString& strPath, QString& strRet);

    bool settingsGetSectionNode(const QString& strRootName, \
                                const QString& strNodeName, \
                                WizXMLNode& node);

    bool settingsFindSectionNode(const QString& strRootName, \
                                 const QString& strNodeName, \
                                 WizXMLNode& node);

    bool settingsGetChildNode(const QString& strRootName, \
                              const QString& strNodeName, \
                              const QString& strSubNodeName, \
                              WizXMLNode& node);

    bool settingsGetStringValue(const QString& strRootName, \
                                const QString& strNodeName, \
                                const QString& strSubNodeName, \
                                const QString& strDefault, \
                                QString& strValue);

    bool settingsSetStringValue(const QString& strRootName, \
                                const QString& strNodeName, \
                                const QString& strSubNodeName, \
                                const QString& strValue);
};


#endif // WIZXML_H
