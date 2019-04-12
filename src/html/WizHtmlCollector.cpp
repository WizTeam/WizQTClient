#include "WizHtmlCollector.h"
#include "WizMainWindow.h"
#include <QEventLoop>
#include <QFile>
#include <QTimer>
#include <QDebug>
#include <QNetworkCacheMetaData>
#include <QNetworkDiskCache>

#include "../share/WizHtml2Zip.h"
#include "../share/WizObjectDataDownloader.h"
#include "utils/WizMisc.h"
#include "utils/WizLogger.h"

bool WizHtmlFileMap::lookup(const QString& strUrl, QString& strFileName)
{
    QString strKey(strUrl.toLower());
    CWizHtmlFileDataMap::const_iterator it = m_map.find(strKey);
    if (it == m_map.end())
        return false;

    strFileName = it->second.strFileName;
    return true;
}

void WizHtmlFileMap::add(const QString& strUrl, const QString& strFileName,
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

void WizHtmlFileMap::getAll(std::deque<WIZHTMLFILEDATA>& arrayFile)
{
    for (CWizHtmlFileDataMap::const_iterator it = m_map.begin();
    it != m_map.end();
    it++)
    {
        arrayFile.push_back(it->second);
    }
}


/* --------------------------- CWizHtmlCollector --------------------------- */
WizHtmlCollector::WizHtmlCollector()
    : m_bMainPage(false)
{
}

void WizHtmlCollector::startTag(WizHtmlTag *pTag, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);

    QString strName = pTag->getTagName().toLower();
    if (strName == "script")
    {
        processTagValue(pTag, "src", WIZHTMLFILEDATA::typeResource);
    }
    else if (strName == "img")
    {
        //ProcessTagValue(pTag, "src", WIZHTMLFILEDATA::typeResource);
        processImgTagValue(pTag, "src", WIZHTMLFILEDATA::typeResource);
    }
    else if (strName == "link")
    {
        if (pTag->getValueFromName("type") == "text/css")
        {
            processTagValue(pTag, "href", WIZHTMLFILEDATA::typeResource);
            processTagValue(pTag, "src", WIZHTMLFILEDATA::typeResource);
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
    if (pTag->getValueFromName("wiz_style") == "unsave")
    {
        return;
    }

    m_ret.push_back(pTag->getTag());
}

void WizHtmlCollector::endTag(WizHtmlTag *pTag, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);

    if (!pTag->isOpening()) {
        m_ret.push_back(pTag->getTag());
    }
}

void WizHtmlCollector::characters(const CString &rText, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);

    m_ret.push_back(rText);
}

void WizHtmlCollector::comment(const CString &rComment, DWORD dwAppData, bool &bAbort)
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

void WizHtmlCollector::processTagValue(WizHtmlTag *pTag,
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
    if (!m_files.lookup(url.toString(), strFileName)) {
        strFileName = url.toLocalFile();
        if (!strFileName.isEmpty() && !WizPathFileExists(strFileName)) {
            strFileName.clear();
        }

        if (strFileName.isEmpty())
            return;
        //
        QString strName = Utils::WizMisc::extractFileName(strFileName);
        if (!IsRegFileName(strName))
        {
            QString strNewFileName = m_strTempPath + ::WizGenGUIDLowerCaseLetterOnly() + Utils::WizMisc::extractFileExt(strFileName);
            if (WizCopyFile(strFileName, strNewFileName, FALSE))
            {
                strFileName = strNewFileName;
            }
        }
        //
        m_files.add(url.toString(), strFileName, eType, false);
    }

    pTag->setValueToName(strAttributeName, toResourceFileName(strFileName));
}

void WizHtmlCollector::processImgTagValue(WizHtmlTag* pTag, const QString& strAttributeName, WIZHTMLFILEDATA::HtmlFileType eType)
{
    QString strValue = pTag->getValueFromName(strAttributeName);
    if (strValue.isEmpty())
        return;

    QUrl url(strValue);
    QString strScheme = url.scheme().toLower();
    if (strScheme == "http" || strScheme == "https" || strScheme == "ftp")
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
            m_files.add(strAbsFile, strFile, eType, false);
            pTag->setValueToName(strAttributeName, toResourceFileName(strFile));
            return;
        }
    }
    else if (strScheme == "file")
    {
        QString resourcePath = m_url.toLocalFile();
        Utils::WizMisc::addBackslash(resourcePath);
        QString filepath = url.toLocalFile();
        if (!filepath.startsWith(resourcePath))
        {
            if (QFile::exists(filepath))
            {
                QString destFile = resourcePath + ::WizGenGUIDLowerCaseLetterOnly() + Utils::WizMisc::extractFileExt(filepath);
                if (QFile::copy(filepath, destFile))
                {
                    m_files.add(QUrl::fromLocalFile(destFile).toString(), destFile, WIZHTMLFILEDATA::typeResource, false);
                    pTag->setValueToName(strAttributeName, toResourceFileName(destFile));
                    return;
                }
            }

        }
    }

    processTagValue(pTag, strAttributeName, eType);
}

QString WizHtmlCollector::toResourceFileName(const QString& strFileName)
{
    if (m_bMainPage) {
        return "index_files/" + Utils::WizMisc::extractFileName(strFileName);
    } else {
        return Utils::WizMisc::extractFileName(strFileName);
    }
}

bool WizHtmlCollector::loadImageFromCache(const QUrl& url, QString& strFileName)
{
    WizMainWindow *mainWindow = WizMainWindow::instance();
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

        QIODevice *device = cache->data(url);
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



enum WIZIMAGEFORMAT {wizImageUnknown, wizImageBmp, wizImageJpg, wizImageGif, wizImagePng, wizImageSvg};

inline WIZIMAGEFORMAT WizGetImageFormatFromMarkBuffer(const unsigned char* pBuffer, int nBufferLen)
{
    if (nBufferLen < 4)
        return wizImageUnknown;
    //
    const unsigned char* szBuffer = pBuffer;
    //
    if (szBuffer[0] == 'B' && szBuffer[1] == 'M')
        return wizImageBmp;
    else if (szBuffer[0] == 0xFF && szBuffer[1] == 0xD8)
        return wizImageJpg;
    else if (szBuffer[0] == 'G' && szBuffer[1] == 'I' && szBuffer[2] == 'F')
        return wizImageGif;
    else if (szBuffer[0] == '<' && szBuffer[1] == 's' && szBuffer[2] == 'v' && szBuffer[3] == 'g')
        return wizImageSvg;
    else if (szBuffer[0] == 0x89 && szBuffer[1] == 'P' && szBuffer[2] == 'N' && szBuffer[3] == 'G')
        return wizImagePng;
    else
        return wizImageUnknown;
}

inline WIZIMAGEFORMAT WizGetImageFormatFromFile(const QString& strFileName)
{
    FILE* fp = fopen(strFileName.toStdString().c_str(), "rb");
    if (!fp)
        return wizImageUnknown;
    unsigned char szBuffer[17];
    fread(szBuffer, 16, 1, fp);
    fclose(fp);
    //
    return WizGetImageFormatFromMarkBuffer(szBuffer, 16);
}

bool WizHtmlCollector::downloadImage(const QString& strUrl, QString& strFileName)
{
    strFileName = ::WizGenGUIDLowerCaseLetterOnly();
    qDebug() << "[Save] Start to download image : " << strUrl;
    QString file = m_strTempPath + strFileName;
    if (!WizURLDownloadToFile(strUrl, file, false))
        return false;
    //
    QFile fileObj(file);
    QString ext;
    //
    WIZIMAGEFORMAT format = WizGetImageFormatFromFile(file);
    switch (format)
    {
    case wizImageBmp:
        ext = ".bmp";
        break;
    case wizImageJpg:
        ext = ".jpg";
        break;
    case wizImageGif:
        ext = ".gif";
        break;
    case wizImagePng:
        ext = ".png";
        break;
    case wizImageSvg:
        ext = ".svg";
        break;
    default:
        break;
    }
    //
    if (!ext.isEmpty())
    {
        if (fileObj.rename(m_strTempPath + strFileName + ext))
        {
            strFileName += ext;
        }
    }

    return true;
}

bool WizHtmlCollector::collect(const QString& strUrl, \
                                QString& strHtml, \
                                bool mainPage,
                                const QString& strTempPath)
{
    m_ret.clear();
    m_bMainPage = mainPage;
    m_strTempPath = strTempPath;
    //
    if (WizPathFileExists(strUrl))
    {
        m_url = QUrl::fromLocalFile(strUrl);
    }
    else
    {
        m_url = QUrl(strUrl);
    }

    WizHtmlReader reader;
    reader.setEventHandler(this);

    reader.read(strHtml);

    CString strHtml2;
    ::WizStringArrayToText(m_ret, strHtml2, "");
    strHtml = strHtml2;

    return true;
}

bool WizHtmlCollector::html2Zip(const QString& strExtResourcePath, \
                                 const QString& strZipFileName)
{
    std::deque<WIZHTMLFILEDATA> arrayResource;
    m_files.getAll(arrayResource);

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


    //remove unused images
    QString allText = strRet;
    for (auto file : files)
    {
        QString ext = Utils::WizMisc::extractFileExt(file);
        ext = ext.toLower();
        if (ext == ".html"
                || ext == ".htm"
                || ext == ".js"
                || ext == ".css")
        {
            QString text;
            if (::WizLoadUnicodeTextFromFile(file, text))
            {
                allText += text;
            }
        }
    }
    //
    std::set<QString> retFiles;
    for (auto file: files)
    {
        QString ext = Utils::WizMisc::extractFileExt(file);
        ext = ext.toLower();
        if (ext == ".png"
                || ext == ".jpg"
                || ext == ".bmp"
                || ext == ".gif"
                || ext == ".jpeg"
                || ext == ".webp")
        {
            QString imageName = Utils::WizMisc::extractFileName(file);
            if (allText.contains(imageName, Qt::CaseInsensitive))
            {
                retFiles.insert(file);
            }
            else
            {
                DEBUG_TOLOG1("%1 has been deleted, remove it", imageName);
            }
        }
        else
        {
            retFiles.insert(file);
        }
    }
    //
    CWizStdStringArray arrayAllResource;
    arrayAllResource.assign(retFiles.begin(), retFiles.end());
    //
    return WizHtml2Zip(strRet, arrayAllResource, strZipFileName);
}

/* -------------------------- CWizHtmlToPlainText -------------------------- */
WizHtmlToPlainText::WizHtmlToPlainText()
{
}

bool WizHtmlToPlainText::toText(const QString& strHtmlAll, QString& strPlainText)
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

void WizHtmlToPlainText::characters(const CString &rText, DWORD dwAppData, bool &bAbort)
{
    Q_UNUSED(dwAppData);
    Q_UNUSED(bAbort);

    m_strText.push_back(rText + " ");
}
