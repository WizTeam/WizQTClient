#ifndef WIZHTMLCOLLECTOR_H
#define WIZHTMLCOLLECTOR_H

#include "WizHtmlReader.h"
#include "../share/WizMisc.h"
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
class WizHtmlFileMap
{
protected:
    typedef std::map<QString, WIZHTMLFILEDATA> CWizHtmlFileDataMap;
    CWizHtmlFileDataMap m_map;

public:
    bool lookup(const QString& strUrl, QString& strFileName);
    void add(const QString& strUrl, const QString& strFileName,
             WIZHTMLFILEDATA::HtmlFileType eType, bool bProcessed);
    void getAll(std::deque<WIZHTMLFILEDATA>& arrayFile);
};


/*
 * Collect media resources path list from html file
 */
class WizHtmlCollector : public WizHtmlReaderEvents
{
public:
    WizHtmlCollector();

    bool collect(const QString &strUrl, QString &strHtml, bool mainPage, const QString& strTempPath);
    bool html2Zip(const QString& strExtResourcePath, const QString& strZipFileName);

protected:
    virtual void startTag(WizHtmlTag *pTag, DWORD dwAppData, bool &bAbort);
    virtual void endTag(WizHtmlTag *pTag, DWORD dwAppData, bool &bAbort);
    virtual void characters(const CString &rText, DWORD dwAppData, bool &bAbort);
    virtual void comment(const CString &rComment, DWORD dwAppData, bool &bAbort);

protected:
    WizHtmlFileMap m_files;
    bool m_bMainPage;
    QUrl m_url;
    CWizStdStringArray m_ret;
    QString m_strTempPath;

    void processTagValue(WizHtmlTag *pTag, const QString& strAttributeName,
                         WIZHTMLFILEDATA::HtmlFileType eType);
    void processImgTagValue(WizHtmlTag *pTag, const QString& strAttributeName,
                         WIZHTMLFILEDATA::HtmlFileType eType);
    QString toResourceFileName(const QString &strFileName);

    //
    bool loadImageFromCache(const QUrl& url, QString& strFileName);
    bool downloadImage(const QString& strUrl, QString& strFileName);
};

class WizHtmlToPlainText : public WizHtmlReaderEvents
{
public:
    WizHtmlToPlainText();
    bool toText(const QString& strHtml, QString& strPlainText);

protected:
    virtual void characters(const CString& rText, DWORD dwAppData, bool& bAbort);

private:
    QString m_strText;
};

#endif // WIZHTMLCOLLECTOR_H
