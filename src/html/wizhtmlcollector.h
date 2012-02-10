#ifndef WIZHTMLCOLLECTOR_H
#define WIZHTMLCOLLECTOR_H

#include "wizhtmlreader.h"
#include "../share/wizmisc.h"
#include <QUrl>

struct WIZHTMLFILEDATA
{
    enum HtmlFileType { typeResource, typeFrame, typeCSS };
    CString strUrl;
    CString strFileName;
    HtmlFileType eType;
    bool bProcessed;
    //
    WIZHTMLFILEDATA() : eType(typeResource), bProcessed(false) {}
};

class CWizHtmlFileMap
{
public:
protected:
    typedef std::map<CString, WIZHTMLFILEDATA> CWizHtmlFileDataMap;
    CWizHtmlFileDataMap m_map;
protected:
public:
    BOOL Lookup(const CString& strUrl, CString& strFileName);
    void Add(const CString& strUrl, const CString& strFileName, WIZHTMLFILEDATA::HtmlFileType eType, BOOL bProcessed);
    //
    void GetAll(std::deque<WIZHTMLFILEDATA>& arrayFile);
};


class CWizHtmlCollector : public IWizHtmlReaderEvents
{
public:
    CWizHtmlCollector();
protected:
    CWizHtmlFileMap m_files;
    BOOL m_bMainPage;
    QUrl m_url;
    CWizStdStringArray m_ret;
public:
    virtual void StartTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort);
    virtual void EndTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort);
    virtual void Characters(const CString &rText, DWORD dwAppData, bool &bAbort);
    virtual void Comment(const CString &rComment, DWORD dwAppData, bool &bAbort);
protected:
    virtual void ProcessTagValue(CWizHtmlTag *pTag, const CString& strAttributeName, WIZHTMLFILEDATA::HtmlFileType eType);
    virtual CString ToResourceFileName(const CString& strFileName);
public:
    BOOL Collect(const CString& strUrl, CString& strHtml, BOOL mainPage = false);
public:
    BOOL Html2Zip(const CString& strUrl, const CString& strHtml, const CString& strExtResourcePath, const CString& strMetaText, const CString& strZipFileName);
};

#endif // WIZHTMLCOLLECTOR_H
