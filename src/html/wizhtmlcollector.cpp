#include "wizhtmlcollector.h"
#include "../share/wizhtml2zip.h"
#include "../share/wizObjectDataDownloader.h"
#include "utils/misc.h"
#include "wizmainwindow.h"
#include <QEventLoop>
#include <QFile>
#include <QTimer>
#include <QDebug>
#include <QNetworkCacheMetaData>
#include <QNetworkDiskCache>

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
        //ProcessTagValue(pTag, "src", WIZHTMLFILEDATA::typeResource);
        ProcessImgTagValue(pTag, "src", WIZHTMLFILEDATA::typeResource);
    }
    else if (strName == "link")
    {
        if (pTag->getValueFromName("type") == "text/css")
        {
            ProcessTagValue(pTag, "href", WIZHTMLFILEDATA::typeResource);
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
static bool IsRegFileName(const QString& name)
{
    if (name.isEmpty())
        return false;
    //
    for (int i = 0; i < name.length(); i++)
    {
        QChar ch = name[i];
        if (ch >= 'a' && ch <= 'z')
            continue;
        if (ch >= 'A' && ch <= 'Z')
            continue;
        //
        if (ch == '.')
            continue;
        if (ch == '_')
            continue;
        if (ch == '-')
            continue;
        //
        if (ch >= '0' && ch <= '9')
            continue;
        //
        if (ch == '@')
            continue;
        if (ch == '(' || ch == ')')
            continue;
        if (ch == '{' || ch == '}')
            continue;
        //
        return false;
    }
    //
    return true;
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
        //
        QString strName = Utils::Misc::extractFileName(strFileName);
        if (!IsRegFileName(strName))
        {
            QString strNewFileName = m_strTempPath + ::WizGenGUIDLowerCaseLetterOnly() + Utils::Misc::extractFileExt(strFileName);
            if (WizCopyFile(strFileName, strNewFileName, FALSE))
            {
                strFileName = strNewFileName;
            }
        }
        //
        m_files.Add(url.toString(), strFileName, eType, false);
    }

    pTag->setValueToName(strAttributeName, ToResourceFileName(strFileName));
}

void CWizHtmlCollector::ProcessImgTagValue(CWizHtmlTag* pTag, const QString& strAttributeName, WIZHTMLFILEDATA::HtmlFileType eType)
{
    QString strValue = pTag->getValueFromName(strAttributeName);
    if (strValue.isEmpty())
        return;

    QUrl url(strValue);
    QString strShme = url.scheme().toLower();
    if (strShme == "http" || strShme == "https" || strShme == "ftp")
    {
        QString strFileName;
        if (loadImageFromCache(url, strFileName))
        {
            qDebug() << "find image in cache : " << strValue;
        }
        else
        {
            downloadImage(strValue, strFileName);
        }
        //

        QString strFile = m_strTempPath + strFileName;
        if (QFile::exists(strFile))
        {
            qDebug() <<"[Save] change to local image : " << strFile;
            QString strAbsFile = "file://" + strFile;
            m_files.Add(strAbsFile, strFile, eType, false);
            pTag->setValueToName(strAttributeName, ToResourceFileName(strFile));
            return;
        }
    }

    ProcessTagValue(pTag, strAttributeName, eType);
}

QString CWizHtmlCollector::ToResourceFileName(const QString& strFileName)
{
    if (m_bMainPage) {
        return "index_files/" + Utils::Misc::extractFileName(strFileName);
    } else {
        return Utils::Misc::extractFileName(strFileName);
    }
}

bool CWizHtmlCollector::loadImageFromCache(const QUrl& url, QString& strFileName)
{
    Core::Internal::MainWindow *mainWindow = Core::Internal::MainWindow::instance();
    QNetworkDiskCache *cache = mainWindow->webViewNetworkCache();
    if (cache)
    {
        QNetworkCacheMetaData cacheMeta = cache->metaData(url);
        if (!cacheMeta.isValid())
            return false;

        QString strImageType;
        QNetworkCacheMetaData::RawHeaderList headList = cacheMeta.rawHeaders();
        foreach (QNetworkCacheMetaData::RawHeader header, headList)
        {
            if (header.first == "Content-Type")
            {
                strImageType = header.second;
                break;
            }
        }

        QIODevice *device = mainWindow->webViewNetworkCache()->data(url);
        if (device && strImageType.contains("image/"))
        {
            strImageType.remove("image/");
            QImage image;
            strFileName = ::WizGenGUIDLowerCaseLetterOnly() + "." + strImageType;
            image.load(device, strImageType.toUpper().toUtf8());
            return image.save(m_strTempPath + strFileName);
        }
    }
    return false;
}

bool CWizHtmlCollector::downloadImage(const QString& strUrl, QString& strFileName)
{
    strFileName = ::WizGenGUIDLowerCaseLetterOnly()
            + strUrl.right(strUrl.length() - strUrl.lastIndexOf('.'));
    qDebug() << "[Save] Start to download image : " << strUrl;
    CWizFileDownloader* downloader = new CWizFileDownloader(strUrl, strFileName, m_strTempPath);
    QEventLoop loop;
    loop.connect(downloader, SIGNAL(downloadDone(QString,bool)), &loop, SLOT(quit()));
    //  just wait for 15 seconds
    QTimer::singleShot(15 * 1000, &loop, SLOT(quit()));
    downloader->startDownload();
    loop.exec();

    return true;
}

bool CWizHtmlCollector::Collect(const QString& strUrl, \
                                QString& strHtml, \
                                bool mainPage,
                                const QString& strTempPath)
{
    m_ret.clear();
    m_bMainPage = mainPage;
    m_strTempPath = strTempPath;
    //
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
    WizHtml2Text(strHtmlAll,strPlainText);
    strPlainText.replace("\n", " ");

    //CWizHtmlReader 的解析方法存在残留标签的问题，需要修正
//    m_strText.clear();

//    // remove head or title if exists
//    QString strHtml = strHtmlAll;
//    strHtml.replace(QRegExp("<head>.*</head>", Qt::CaseInsensitive), "");
//    strHtml.replace(QRegExp("<title>.*</title>", Qt::CaseInsensitive), "");

//    CWizHtmlReader reader;
//    reader.setEventHandler(this);
//    reader.setBoolOption(CWizHtmlReader::resolveEntities, true);

//    reader.Read(strHtml);

//    m_strText.replace('\0', ' '); // otherwise sqlite statement will be failed!
//    strPlainText = m_strText.simplified();
    return true;
}

void CWizHtmlToPlainText::Characters(const CString &rText, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);

    m_strText.push_back(rText + " ");
}
