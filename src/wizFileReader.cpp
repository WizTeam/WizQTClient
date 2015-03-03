#include "wizFileReader.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDebug>

#include "share/wizmisc.h"
#include "share/wizRtfReader.h"
#include "mac/wizmachelper.h"

CWizFileReader::CWizFileReader(QObject *parent) :
    QThread(parent)
{
    //m_files = new QStringList();
}

void CWizFileReader::loadFiles(const QStringList& strFiles)
{
    m_files = strFiles;
    if (!isRunning())
    {
        start();
    }
}

QString CWizFileReader::loadHtmlFileToHtml(const QString& strFileName)
{
    QFile file(strFileName);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return "";
    QTextStream in(&file);
    QString ret = in.readAll();
    file.close();

    // 检查是否存在与Html文件对于的_files文件夹
    QString strResFolder = strFileName;
    QFileInfo info(strFileName);
    strResFolder.remove("." + info.suffix());
    strResFolder.append("_files");

//    qDebug() << "check if resource folder exists : " << strResFolder;
    QDir dir(strResFolder);
    if (dir.exists())
    {
        QString strOldPath = "./" + info.completeBaseName() + "_files";
        QString strNewPath = "file://" + info.path() + "/" +info.completeBaseName() + "_files";
//        qDebug() << "replace old res path : " << strOldPath << "   to new path : " << strNewPath;
        ret.replace(strOldPath, strNewPath);
    }

    return ret;
}

QString CWizFileReader::loadTextFileToHtml(const QString& strFileName)
{
    QFile file(strFileName);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return "";
    QTextStream in(&file);
    QString ret = in.readAll();
    file.close();
#if QT_VERSION > 0x050000
    ret = ret.toHtmlEscaped();
#else
    ret.replace("<", "&lt;");
    ret.replace(">", "&gt;");
#endif
    ret.replace("\n","<br>");
    ret.replace(" ","&nbsp");

    qDebug() << "load text file to html : " << ret;

    return ret;
}

QString CWizFileReader::loadImageFileToHtml(const QString& strFileName)
{
    return QString("<img border=\"0\" src=\"file://%1\" />").arg(strFileName);
}

QString CWizFileReader::loadRtfFileToHtml(const QString& strFileName)
{
    QString strHtml;
    if (CWizRtfReader::load(strFileName, strHtml))
        return strHtml;

    return "";
}

void CWizFileReader::run()
{
    int nTotal = m_files.count();
    for (int i = 0; i < nTotal; i++)
    {
        QString strFile = m_files.at(i);
        QFileInfo fi(strFile);
        QString strHtml;
        QStringList textExtList, imageExtList, rtfExtList, docExtList, htmlExtList;
        textExtList << "txt" << "md" << "markdown" << "mht" << "cpp" << "h";
        imageExtList << "jpg" << "png" << "gif" << "tiff" << "jpeg" << "bmp" << "svg";
        rtfExtList << "rtf";
        docExtList << "doc" << "docx" << "pages";
        htmlExtList << "html" << "htm";

#ifdef Q_OS_MAC
        QStringList webExtList;
        webExtList << "webarchive";
#endif

        QString docType = fi.suffix();
        if (textExtList.contains(docType,Qt::CaseInsensitive))
        {
            strHtml = loadTextFileToHtml(strFile);
        }
        else if (imageExtList.contains(docType,Qt::CaseInsensitive))
        {
            strHtml = loadImageFileToHtml(strFile);
        }
        else if (htmlExtList.contains(docType, Qt::CaseInsensitive))
        {
            strHtml = loadHtmlFileToHtml(strFile);
        }
#ifdef Q_OS_MAC
        else if (rtfExtList.contains(docType, Qt::CaseInsensitive))
        {
            if (!documentToHtml(strFile, RTFTextDocumentType, strHtml))
                continue;
            WizGetBodyContentFromHtml(strHtml, true);
        }
        else if (docExtList.contains(docType))
        {
            if (!documentToHtml(strFile, DocFormatTextDocumentType, strHtml))
                continue;
            WizGetBodyContentFromHtml(strHtml, true);
        }
        else if (webExtList.contains(docType))
        {
            if (!documentToHtml(strFile, WebArchiveTextDocumentType, strHtml))
                continue;
            WizGetBodyContentFromHtml(strHtml, true);
        }
        else
        {
            strHtml = loadTextFileToHtml(strFile);
        }
#endif

        if (!strHtml.isEmpty())
        {
            QString strTitle = WizExtractFileName(strFile);
            emit fileLoaded(strHtml, strTitle);
        }

        emit loadProgress(nTotal, i + 1);
    }
    emit loadFinished();
    m_files.clear();
}
