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

    WIZHTMLFILEDATA() : eType(typeResource), bProcessed(false) {}
};

class CWizHtmlFileMap
{
protected:
    typedef std::map<CString, WIZHTMLFILEDATA> CWizHtmlFileDataMap;
    CWizHtmlFileDataMap m_map;

public:
    bool Lookup(const CString& strUrl, CString& strFileName);
    void Add(const CString& strUrl, const CString& strFileName, WIZHTMLFILEDATA::HtmlFileType eType, BOOL bProcessed);
    void GetAll(std::deque<WIZHTMLFILEDATA>& arrayFile);
};


class CWizHtmlCollector : public IWizHtmlReaderEvents
{
public:
    CWizHtmlCollector();

    bool Collect(const CString& strUrl, CString& strHtml, BOOL mainPage = false);
    bool Html2Zip(const CString& strUrl, const CString& strHtml, \
                  const CString& strExtResourcePath, const CString& strMetaText, \
                  const CString& strZipFileName);

    virtual void StartTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort);
    virtual void EndTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort);
    virtual void Characters(const CString &rText, DWORD dwAppData, bool &bAbort);
    virtual void Comment(const CString &rComment, DWORD dwAppData, bool &bAbort);

protected:
    CWizHtmlFileMap m_files;
    BOOL m_bMainPage;
    QUrl m_url;
    CWizStdStringArray m_ret;

    virtual void ProcessTagValue(CWizHtmlTag *pTag, const CString& strAttributeName, WIZHTMLFILEDATA::HtmlFileType eType);
    virtual CString ToResourceFileName(const CString& strFileName);
};

class CWizHtmlToPlainText : public IWizHtmlReaderEvents
{
public:
    CWizHtmlToPlainText();
    bool toText(const QString& strHtml, QString& strPlainText);

protected:
    virtual void Characters(const CString &rText, DWORD dwAppData, bool &bAbort);

private:
    QString m_strText;
};

#endif // WIZHTMLCOLLECTOR_H
