#include "wizhtmlcollector.h"
#include "../share/wizhtml2zip.h"

#include <QDebug>

bool CWizHtmlFileMap::Lookup(const QString& strUrl, QString& strFileName)
{
    QString strKey(strUrl.toLower());
    CWizHtmlFileDataMap::const_iterator it = m_map.find(strKey);
    if (it == m_map.end())
        return false;

    strFileName = it->second.strFileName;
    return true;
}

void CWizHtmlFileMap::Add(const QString& strUrl, const QString& strFileName,
                          WIZHTMLFILEDATA::HtmlFileType eType, bool bProcessed)
{
    QString strKey(strUrl.toLower());

    WIZHTMLFILEDATA data;
    data.strUrl = strUrl;
    data.strFileName = strFileName;
    data.eType = eType;
    data.bProcessed = bProcessed;
    m_map[strKey] = data;
}

void CWizHtmlFileMap::GetAll(std::deque<WIZHTMLFILEDATA>& arrayFile)
{
    for (CWizHtmlFileDataMap::const_iterator it = m_map.begin();
    it != m_map.end();
    it++)
    {
        arrayFile.push_back(it->second);
    }
}


/* --------------------------- CWizHtmlCollector --------------------------- */
CWizHtmlCollector::CWizHtmlCollector()
    : m_bMainPage(false)
{
}

void CWizHtmlCollector::StartTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);

    QString strName = pTag->getTagName().toLower();
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

    m_ret.push_back(pTag->getTag());
}

void CWizHtmlCollector::EndTag(CWizHtmlTag *pTag, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);

    if (!pTag->isOpening()) {
        m_ret.push_back(pTag->getTag());
    }
}

void CWizHtmlCollector::Characters(const CString &rText, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);

    m_ret.push_back(rText);
}

void CWizHtmlCollector::Comment(const CString &rComment, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);

    m_ret.push_back(rComment);
}

void CWizHtmlCollector::ProcessTagValue(CWizHtmlTag *pTag,
                                        const QString& strAttributeName,
                                        WIZHTMLFILEDATA::HtmlFileType eType)
{
    QString strValue = pTag->getValueFromName(strAttributeName);
    if (strValue.isEmpty())
        return;

    QUrl url(strValue);
    if (url.isRelative()) {
        QUrl urlBase = m_url;
        url = urlBase.resolved(url);
    }

    QString strFileName;
    if (!m_files.Lookup(url.toString(), strFileName)) {
        strFileName = url.toLocalFile();
        if (!strFileName.isEmpty() && !PathFileExists(strFileName)) {
            strFileName.clear();
        }

        if (strFileName.isEmpty())
            return;

        m_files.Add(url.toString(), strFileName, eType, false);
    }

    pTag->setValueToName(strAttributeName, ToResourceFileName(strFileName));
}

QString CWizHtmlCollector::ToResourceFileName(const QString& strFileName)
{
    if (m_bMainPage) {
        return "index_files/" + WizExtractFileName(strFileName);
    } else {
        return WizExtractFileName(strFileName);
    }
}

bool CWizHtmlCollector::Collect(const QString& strUrl, \
                                QString& strHtml, \
                                bool mainPage /*= false*/)
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

    CString strHtml2 = strHtml;

    ::WizStringArrayToText(m_ret, strHtml2, "");

    return true;
}

bool CWizHtmlCollector::Html2Zip(const QString& strExtResourcePath, \
                                 const QString& strMetaText, \
                                 const QString& strZipFileName)
{
    //CString strMainHtml(strHtml);
    //if (!Collect(strUrl, strMainHtml, true))
    //    return false;

    std::deque<WIZHTMLFILEDATA> arrayResource;
    m_files.GetAll(arrayResource);

    std::set<QString> files;
    std::deque<WIZHTMLFILEDATA>::const_iterator it;
    for (it = arrayResource.begin(); it != arrayResource.end(); it++) {
        files.insert(it->strFileName);
    }

    CWizStdStringArray arrayExtResource;
    if (!strExtResourcePath.isEmpty())
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

/* -------------------------- CWizHtmlToPlainText -------------------------- */
CWizHtmlToPlainText::CWizHtmlToPlainText()
{
}

bool CWizHtmlToPlainText::toText(const QString& strHtmlAll, QString& strPlainText)
{
    m_strText.clear();

    // remove head or title if exists
    QString strHtml = strHtmlAll;
    strHtml.replace(QRegExp("<head>.*</head>", Qt::CaseInsensitive), "");
    strHtml.replace(QRegExp("<title>.*</title>", Qt::CaseInsensitive), "");

    CWizHtmlReader reader;
    reader.setEventHandler(this);
    reader.setBoolOption(CWizHtmlReader::resolveEntities, true);

    reader.Read(strHtml);

    m_strText.replace('\0', ' '); // otherwise sqlite statement will be failed!
    strPlainText = m_strText.simplified();
    return true;
}

void CWizHtmlToPlainText::Characters(const CString &rText, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);

    m_strText.push_back(rText + " ");
}
