#include "wizFileReader.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include "share/wizmisc.h"
#include "share/wizRtfReader.h"
#include "mac/wizmachelper.h"

CWizFileReader::CWizFileReader(QObject *parent) :
    QThread(parent)
{
    //m_files = new QStringList();
}

void CWizFileReader::loadFiles(QStringList strFiles)
{
    m_files = strFiles;
    if (!isRunning())
    {
        start();
    }
}

QString CWizFileReader::loadTextFileToHtml(QString strFileName)
{
    // QTextStream
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
    return ret;
}

QString CWizFileReader::loadImageFileToHtml(QString strFileName)
{
    return QString("<img border=\"0\" src=\"file://%1\" />").arg(strFileName);
}

QString CWizFileReader::loadRtfFileToHtml(QString strFileName)
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
        QStringList textExtList, imageExtList, rtfExtList, docExtList;
        textExtList << "txt" << "md" << "markdown" << "html" << "htm" << "cpp" << "h";
        imageExtList << "jpg" << "png" << "gif" << "tiff" << "jpeg" << "bmp" << "svg";
        rtfExtList << "rtf";
        docExtList << "doc" << "docx";

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
