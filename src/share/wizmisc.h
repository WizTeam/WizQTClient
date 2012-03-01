#ifndef WIZMISC_H
#define WIZMISC_H

#ifndef WIZQTHELPER_H
#include "wizqthelper.h"
#endif

#ifndef QDOM_H
#include <QDomDocument>
#endif

#ifndef QICON_H
#include <QIcon>
#endif

#ifndef WIZMD5_H
#include "wizmd5.h"
#endif


struct IWizGlobal
{
    CString GetTempPath();
    void WriteLog(const CString& str);
    void WriteDebugLog(const CString& str);
};


IWizGlobal* WizGlobal();


COleDateTime WizGetCurrentTime();
BOOL WizStringToDateTime(const CString& str, COleDateTime& t, CString& strError);
COleDateTime WizStringToDateTime(const CString& str);
COLORREF WizStringToColor(const CString& str);
QColor WizStringToColor2(const CString& str);

std::string WizBSTR2UTF8(const CString& str);

CString WizFormatString0(const CString& str);
CString WizFormatString1(const CString& strFormat, const CString& strParam1);
CString WizFormatString2(const CString& strFormat, const CString& strParam1, const CString& strParam2);
CString WizFormatString3(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3);
CString WizFormatString4(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4);
CString WizFormatString5(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5);
CString WizFormatString6(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6);
CString WizFormatString7(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6, const CString& strParam7);
CString WizFormatString8(const CString& strFormat, const CString& strParam1, const CString& strParam2, const CString& strParam3, const CString& strParam4, const CString& strParam5, const CString& strParam6, const CString& strParam7, const CString& strParam8);
CString WizFormatInt(__int64 n);

time_t WizTimeGetTimeT(const COleDateTime& t);

CString WizIntToStr(int n);

BOOL WizSplitTextToArray(const CString& strText, QChar ch, CWizStdStringArray& arrayResult);
BOOL WizSplitTextToArray(CString strText, const CString& strSplitterText, BOOL bMatchCase, CWizStdStringArray& arrayResult);
void WizStringArrayToText(const CWizStdStringArray& arrayText, CString& strText, const CString& strSplitter);
int WizFindInArray(const CWizStdStringArray& arrayText, const CString& strFind);
int WizFindInArrayNoCase(const CWizStdStringArray& arrayText, const CString& strFind);
void WizStringArrayEraseEmptyLine(CWizStdStringArray& arrayText);
void WizStringArrayRemoveMultiElement(CWizStdStringArray& arrayText);
void WizStringArrayRemoveMultiElementNoCase(CWizStdStringArray& arrayText);


CString WizStringArrayGetValue(const CWizStdStringArray& arrayText, const CString& valueName);
void WizCommandLineToStringArray(const CString& commandLine, CWizStdStringArray& arrayLine);
CString WizGetCommandLineValue(const CString& strCommandLine, const CString& strKey);

BOOL WizStringSimpleSplit(const CString& str, char ch, CString& strLeft, CString& strRight);

CString WizDateToLocalString(const COleDateTime& t);

CString WizGetAppPath();
CString WizGetAppFileName();
CString WizGetResourcesPath();
CString WizGetDataStorePath();
CString WizGetSettingsFileName();

__int64 WizGetFileSize(const CString& strFileName);
void WizPathAddBackslash(CString& strPath);
void WizPathRemoveBackslash(CString& strPath);
CString WizPathAddBackslash2(const CString& strPath);
CString WizPathRemoveBackslash2(const CString& strPath);
void WizEnsurePathExists(const CString& strPath);

CString WizExtractFilePath(const CString& strFileName);
CString WizExtractFileName(const CString& strFileName);
CString WizExtractFileTitle(const CString& strFileName);
CString WizExtractTitleTemplate(const CString& strFileName);
CString WizExtractFileExt(const CString& strFileName);

#define EF_INCLUDEHIDDEN			0x01
#define EF_INCLUDESUBDIR			0x02

void WizEnumFiles(const CString& strPath, const CString& strExts, CWizStdStringArray& arrayFiles, UINT uFlags);
void WizEnumFolders(const CString& strPath, CWizStdStringArray& arrayFolders, UINT uFlags);


BOOL WizCopyFile(const CString& strSrcFileName, const CString& strDestFileName, BOOL bFailIfExists);
void WizGetNextFileName(CString& strFileName);

CString WizEncryptPassword(const CString& strPassword);
CString WizDecryptPassword(const CString& strEncryptedText);


BOOL WizLoadUnicodeTextFromFile(const CString& strFileName, CString& steText);
BOOL WizSaveUnicodeTextToUnicodeFile(const CString& strFileName, const CString& strText);
BOOL WizSaveUnicodeTextToUtf8File(const CString& strFileName, const CString& strText);

CString WizDateTimeToIso8601String(const COleDateTime& t);
BOOL WizIso8601StringToDateTime(CString str, COleDateTime& t, CString& strError);
CString WizDateTimeToString(const COleDateTime& t);
CString WizStringToSQL(const CString& str);
CString WizTimeToSQL(const COleDateTime& t);
CString WizColorToString(COLORREF cr);
CString WizColorToSQL(COLORREF cr);

intptr_t WizStrStrI_Pos(const CString& str, const CString& Find, int nStart = 0);

CString WizInt64ToStr(__int64 n);

CString WizGenGUIDLowerCaseLetterOnly();


CString WizGetComputerName();

#define TOLOG(x)                        WizGlobal()->WriteLog(x)
#define TOLOG1(x, p1)                   WizGlobal()->WriteLog(WizFormatString1(x, p1))
#define TOLOG2(x, p1, p2)               WizGlobal()->WriteLog(WizFormatString2(x, p1, p2))
#define TOLOG3(x, p1, p2, p3)           WizGlobal()->WriteLog(WizFormatString3(x, p1, p2, p3))
#define TOLOG4(x, p1, p2, p3, p4)       WizGlobal()->WriteLog(WizFormatString4(x, p1, p2, p3, p4))


#define DEBUG_TOLOG(x)                        WizGlobal()->WriteDebugLog(x)
#define DEBUG_TOLOG1(x, p1)                   WizGlobal()->WriteDebugLog(WizFormatString1(x, p1))
#define DEBUG_TOLOG2(x, p1, p2)               WizGlobal()->WriteDebugLog(WizFormatString2(x, p1, p2))
#define DEBUG_TOLOG3(x, p1, p2, p3)           WizGlobal()->WriteDebugLog(WizFormatString3(x, p1, p2, p3))
#define DEBUG_TOLOG4(x, p1, p2, p3, p4)       WizGlobal()->WriteDebugLog(WizFormatString4(x, p1, p2, p3, p4))


BOOL WizBase64Encode(const QByteArray& arrayData, CString& str);
BOOL WizBase64Decode(const CString& str, QByteArray& arrayData);

CString WizStringToBase64(const CString& strSource);
CString WizStringFromBase64(const CString& strBase64);

CString WizGetSkinPath();
CString WizGetSkinResourceFileName(const CString& strName);
QIcon WizLoadSkinIcon(const CString& strIconName);


void WizHtml2Text(const CString& strHtml, CString& strText);
void WizDeleteFolder(const CString& strPath);
void WizDeleteFile(const CString& strFileName);
BOOL WizDeleteAllFilesInFolder(const CString& strPath);

BOOL WizIsValidFileNameNoPath(const CString& strFileName);
void WizMakeValidFileNameNoPath(CString& strFileName);
void WizMakeValidFileNameNoPathLimitLength(CString& strFileName, int nMaxTitleLength);
void WizMakeValidFileNameNoPathLimitFullNameLength(CString& strFileName, int nMaxFullNameLength);
CString WizMakeValidFileNameNoPathReturn(const CString& strFileName);

BOOL WizSaveDataToFile(const CString& strFileName, const QByteArray& arrayData);
BOOL WizLoadDataFromFile(const CString& strFileName, QByteArray& arrayData);


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
    BOOL Valid() const { return !m_node.isNull(); }
    void Clear() { m_node.clear(); }
public:
    BOOL GetName(CString& strName);
    CString GetName();
    CString GetType();
    //
    CString GetText(const CString& strDefault = "");
    BOOL GetText(CString& strText);
    BOOL SetText(const CString& strText);
    //
    BOOL GetAttributeText(const CString& strName, CString& strVal);
    BOOL GetAttributeInt(const CString& strName, int& nVal);
    BOOL GetAttributeInt64(const CString& strName, __int64& nVal);
    BOOL GetAttributeUINT(const CString& strName, UINT& nVal);
    BOOL GetAttributeTimeT(const CString& strName, time_t& nVal);
    BOOL GetAttributeTimeString(const CString& strName, COleDateTime& t);
    BOOL GetAttributeBool(const CString& strName, BOOL& bVal);
    BOOL GetAttributeDWORD(const CString& strName, DWORD& dwVal);
    CString GetAttributeTextDef(const CString& strName, const CString& strDefault);
    int GetAttributeIntDef(const CString& strName, int nDefault);
    __int64 GetAttributeInt64Def(const CString& strName, __int64 Default);
    BOOL GetAttributeBoolDef(const CString& strName, BOOL bDefault);
    //
    BOOL SetAttributeText(const CString& strName, const CString& strText);
    BOOL SetAttributeInt(const CString& strName, int nVal);
    BOOL SetAttributeInt64(const CString& strName, __int64 nVal);
    BOOL SetAttributeBool(const CString& strName, BOOL bVal);
    BOOL SetAttributeTime(const CString& strName, const COleDateTime& t);
    //
    BOOL FindChildNode(const CString& strNodeName, CWizXMLNode& nodeChild);
    BOOL AppendChild(const CString& strNodeName, CWizXMLNode& nodeChild);
    BOOL AppendChild(const CString& strNodeName, const CString& strChildNodeText);
    BOOL GetChildNode(const CString& strNodeName, CWizXMLNode& nodeChild);
    BOOL SetChildNodeText(const CString& strNodeName, const CString& strText);
    BOOL GetChildNodeText(const CString& strNodeName, CString& strText);
    CString GetChildNodeTextDef(const CString& strNodeName, const CString& strDef);
    //
    BOOL GetAllChildNodes(CWizStdStringArray& arrayNodeName);
    BOOL GetAllChildNodes(std::deque<CWizXMLNode>& arrayNodes);
    BOOL GetFirstChildNode(CWizXMLNode& nodeChild);
    CString GetFirstChildNodeText(const CString& strDef = "");
    //
    BOOL DeleteChild(const CString& strChildName);
    BOOL DeleteChild(CWizXMLNode& nodeChild);
    BOOL DeleteAllChild(const CString& strExceptNodeName1 = "", const CString& strExceptNodeName2 = "", const CString& strExceptNodeName3 = "");
    //
    BOOL HasChildNode();
    int GetChildNodesCount();
    //
    BOOL AppendNodeByPath(const CString& strPath, CWizXMLNode& nodeRet);
    BOOL AppendNodeSetTextByPath(const CString& strPath, const CString& strText);
    //
    BOOL FindNodeByPath(const CString& strPath, CWizXMLNode& nodeRet);
    BOOL FindNodeTextByPath(const CString& strPath, CString& strRet);
    //
    BOOL GetElementNodeByValue(const CString& strElementName, const CString& strValueName, const CString& strValue, CWizXMLNode& nodeRet);
    BOOL GetElementOtherNodeByValue(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, CWizXMLNode& nodeRet);
    BOOL GetElementOtherNodeByValueReturnString(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, CString& strRet);
    BOOL GetElementOtherNodeByValueReturnInt(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, int& nRet);
    BOOL GetElementOtherNodeByValueReturnBool(const CString& strElementName, const CString& strValueName, const CString& strValue, const CString& strOtherNodePath, BOOL& bRet);
public:
    static BOOL FindChildNode(const QDomNodeList& nodes, const CString& strName, QDomNode& nodeRet);
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
protected:
    BOOL Create();
public:
    QDomDocument* GetDoc() { return &m_doc; }
    //
    BOOL LoadXML(const CString& strXML);
    BOOL LoadFromFile(const CString& strFileName, BOOL bPromptError = TRUE);
    void Clear();
    //
    BOOL ToXML(CString& strText, BOOL bFormatText);
    BOOL ToUnicodeFile(const CString& strFileName);
    //
    BOOL IsFail();
    CString GetErrorMessage() const { return m_strErrorMessage; }
    CString GetErrorSrcText() const { return m_strErrorSrcText; }
    //
    BOOL FindChildNode(const CString& strName, CWizXMLNode& nodeChild);
    BOOL AppendChild(const CString& strNodeName, CWizXMLNode& nodeChild);
    BOOL GetChildNode(const CString& strName, CWizXMLNode& nodeChild);
    //
    BOOL GetAllChildNodes(CWizStdStringArray& arrayNodeName);
    BOOL GetAllChildNodes(std::deque<CWizXMLNode>& arrayNodes);
    //
    BOOL FindNodeByPath(const CString& strPath, CWizXMLNode& nodeRet);
    BOOL GetNodeTextByPath(const CString& strPath, CString& strRet);
    //
    BOOL SettingsGetSectionNode(const CString& strRootName, const CString& strNodeName, CWizXMLNode& node);
    BOOL SettingsFindSectionNode(const CString& strRootName, const CString& strNodeName, CWizXMLNode& node);
    BOOL SettingsGetChildNode(const CString& strRootName, const CString& strNodeName, const CString& strSubNodeName, CWizXMLNode& node);
    BOOL SettingsGetStringValue(const CString& strRootName, const CString& strNodeName, const CString& strSubNodeName, const CString& strDefault, CString& strValue);
    BOOL SettingsSetStringValue(const CString& strRootName, const CString& strNodeName, const CString& strSubNodeName, const CString& strValue);
};



class CWizBufferAlloc
{
public:
    CWizBufferAlloc(int nInitSize = 0);
    ~CWizBufferAlloc();
private:
    BYTE* m_pBuffer;
    int m_nSize;
public:
    BYTE* GetBuffer();
    BOOL SetNewSize(int nNewSize);
};


class CWaitCursor
{
public:
    CWaitCursor();
    ~CWaitCursor();
};

#endif // WIZMISC_H
