#ifndef WIZHTMLCOLLECTOR_H
#define WIZHTMLCOLLECTOR_H

#include "wizhtmlreader.h"
#include "../share/wizmisc.h"
#include <QUrl>

struct WIZHTMLFILEDATA
{
    enum HtmlFileType { typeResource, typeFrame, typeCSS };
    QString strUrl;
    QString strFileName;
    HtmlFileType eType;
    bool bProcessed;

    WIZHTMLFILEDATA() : eType(typeResource), bProcessed(false) {}
};

/* ---------------------------- CWizHtmlFileMap ---------------------------- */
class CWizHtmlFileMap
{
protected:
    typedef std::map<QString, WIZHTMLFILEDATA> CWizHtmlFileDataMap;
    CWizHtmlFileDataMap m_map;

public:
    bool Lookup(const QString& strUrl, QString& strFileName);
    void Add(const QString& strUrl, const QString& strFileName,
             WIZHTMLFILEDATA::HtmlFileType eType, bool bProcessed);
    void GetAll(std::deque<WIZHTMLFILEDATA>& arrayFile);
};


/*
 * Collect media resources path list from html file
 */
class CWizHtmlCollector : public IWizHtmlReaderEvents
{
public:
    CWizHtmlCollector();

    bool Collect(const QString &strUrl, QString &strHtml, bool mainPage, const QString& strTempPath);
    bool Html2Zip(const QString& strExtResourcePath, const QString& strMetaText, \
                  const QString& strZipFileName);

protected:
    virtual void StartTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort);
    virtual void EndTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort);
    virtual void Characters(const CString &rText, DWORD dwAppData, bool &bAbort);
    virtual void Comment(const CString &rComment, DWORD dwAppData, bool &bAbort);

private:
    CWizHtmlFileMap m_files;
    bool m_bMainPage;
    QUrl m_url;
    CWizStdStringArray m_ret;
    QString m_strTempPath;

    void ProcessTagValue(CWizHtmlTag *pTag, const QString& strAttributeName,
                         WIZHTMLFILEDATA::HtmlFileType eType);
    void ProcessImgTagValue(CWizHtmlTag *pTag, const QString& strAttributeName,
                         WIZHTMLFILEDATA::HtmlFileType eType);
    QString ToResourceFileName(const QString &strFileName);

    //
    bool loadImageFromCache(const QUrl& url, QString& strFileName);
    bool downloadImage(const QString& strUrl, QString& strFileName);
};

class CWizHtmlToPlainText : public IWizHtmlReaderEvents
{
public:
    CWizHtmlToPlainText();
    bool toText(const QString& strHtml, QString& strPlainText);

protected:
    virtual void Characters(const CString& rText, DWORD dwAppData, bool& bAbort);

private:
    QString m_strText;
};

#endif // WIZHTMLCOLLECTOR_H
