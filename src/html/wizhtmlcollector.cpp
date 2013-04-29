#include "wizhtmlcollector.h"
#include "../share/wizhtml2zip.h"

BOOL CWizHtmlFileMap::Lookup(const CString& strUrl, CString& strFileName)
{
    CString strKey(strUrl);
    strKey.MakeLower();
    //
    CWizHtmlFileDataMap::const_iterator it = m_map.find(strKey);
    if (it == m_map.end())
        return FALSE;
    //
    strFileName = it->second.strFileName;
    return true;
}

void CWizHtmlFileMap::Add(const CString& strUrl, const CString& strFileName, WIZHTMLFILEDATA::HtmlFileType eType, BOOL bProcessed)
{
    CString strKey(strUrl);
    strKey.MakeLower();
    //
    WIZHTMLFILEDATA data;
    data.strUrl = strUrl;
    data.strFileName = strFileName;
    data.eType = eType;
    data.bProcessed = bProcessed;
    //
    m_map[strKey] = data;
}

//
void CWizHtmlFileMap::GetAll(std::deque<WIZHTMLFILEDATA>& arrayFile)
{
    for (CWizHtmlFileDataMap::const_iterator it = m_map.begin();
    it != m_map.end();
    it++)
    {
        arrayFile.push_back(it->second);
    }
}





/////////////////////////////////////////////
CWizHtmlCollector::CWizHtmlCollector()
    : m_bMainPage(false)
{
}

void CWizHtmlCollector::StartTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);
    //
    //
    CString strName = pTag->getTagName();
    //
    strName.MakeLower();
    if (strName == "script")
    {
        ProcessTagValue(pTag, "src", WIZHTMLFILEDATA::typeResource);
    }
    else if (strName == "img")
    {
        ProcessTagValue(pTag, "src", WIZHTMLFILEDATA::typeResource);
    }
    else if (strName == "link")
    {
        if (pTag->getValueFromName("type") == "text/css")
        {
            ProcessTagValue(pTag, "src", WIZHTMLFILEDATA::typeResource);
        }
    }
    else if (strName == "body")
    {
        if (pTag->getValueFromName("contentEditable") == "true")
        {
            pTag->removeAttribute("contentEditable");
        }
    }
    //
    m_ret.push_back(pTag->getTag());
}

void CWizHtmlCollector::EndTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);
    //
    if (!pTag->isOpening())
    {
        m_ret.push_back(pTag->getTag());
    }
}

void CWizHtmlCollector::Characters(const CString &rText, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);
    //
    m_ret.push_back(rText);
}

void CWizHtmlCollector::Comment(const CString &rComment, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);
    //
    m_ret.push_back(rComment);
}

void CWizHtmlCollector::ProcessTagValue(CWizHtmlTag *pTag, const CString& strAttributeName, WIZHTMLFILEDATA::HtmlFileType eType)
{
    CString strValue = pTag->getValueFromName(strAttributeName);
    if (strValue.IsEmpty())
        return;
    //
    QUrl url(strValue);
    if (url.isRelative())
    {
        QUrl urlBase = m_url;
        url = urlBase.resolved(url);
    }
    //
    CString strFileName;
    if (!m_files.Lookup(url.toString(), strFileName))
    {
        strFileName = url.toLocalFile();
        if (!strFileName.isEmpty() && !PathFileExists(strFileName))
        {
            strFileName.Empty();
        }
        //
        if (strFileName.IsEmpty())
            return;
        //
        m_files.Add(url.toString(), strFileName, eType, false);
    }
    //
    pTag->setValueToName(strAttributeName, ToResourceFileName(strFileName));
}

CString CWizHtmlCollector::ToResourceFileName(const CString& strFileName)
{
    if (m_bMainPage)
    {
        return "index_files/" + WizExtractFileName(strFileName);
    }
    else
    {
        return WizExtractFileName(strFileName);
    }
}

bool CWizHtmlCollector::Collect(const CString& strUrl, \
                                CString& strHtml, \
                                BOOL mainPage /*= false*/)
{
    m_ret.clear();
    m_bMainPage = mainPage;
    if (PathFileExists(strUrl))
    {
        m_url = QUrl::fromLocalFile(strUrl);
    }
    else
    {
        m_url = QUrl(strUrl);
    }

    CWizHtmlReader reader;
    reader.setEventHandler(this);

    reader.Read(strHtml);

    ::WizStringArrayToText(m_ret, strHtml, "");

    return true;
}

bool CWizHtmlCollector::Html2Zip(const CString& strUrl, \
                                 const CString& strHtml, \
                                 const CString& strExtResourcePath, \
                                 const CString& strMetaText, \
                                 const CString& strZipFileName)
{
    CString strMainHtml(strHtml);
    if (!Collect(strUrl, strMainHtml, true))
        return false;

    std::deque<WIZHTMLFILEDATA> arrayResource;
    m_files.GetAll(arrayResource);

    std::set<CString> files;
    std::deque<WIZHTMLFILEDATA>::const_iterator it;
    for (it = arrayResource.begin(); it != arrayResource.end(); it++) {
        files.insert(it->strFileName);
    }

    CWizStdStringArray arrayExtResource;
    if (!strExtResourcePath.IsEmpty())
    {
        ::WizEnumFiles(strExtResourcePath, "*.*", arrayExtResource, 0);
        for (CWizStdStringArray::const_iterator it = arrayExtResource.begin();
            it != arrayExtResource.end();
            it++)
        {
            files.insert(*it);
        }
    }

    CString strRet;
    ::WizStringArrayToText(m_ret, strRet, "");

    CWizStdStringArray arrayAllResource;
    arrayAllResource.assign(files.begin(), files.end());

    return WizHtml2Zip(strRet, arrayAllResource, strMetaText, strZipFileName);
}



CWizHtmlToPlainText::CWizHtmlToPlainText()
{
}

bool CWizHtmlToPlainText::toText(const QString& strHtml, QString& strPlainText)
{
    m_strText.clear();

    CWizHtmlReader reader;
    reader.setEventHandler(this);
    reader.setBoolOption(CWizHtmlReader::resolveEntities, true);

    reader.Read(strHtml);

    strPlainText = m_strText.simplified();

    return true;
}

void CWizHtmlToPlainText::Characters(const CString &rText, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);

    CString strText = rText;
    strText.replace('\000', ' ');
    m_strText.push_back(strText + " ");
}
