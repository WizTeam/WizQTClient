#include "wizFileReader.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include "share/wizmisc.h"
#include "share/wizRtfReader.h"

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
        QStringList textExtList, imageExtList, rtfExtList;
        textExtList << "txt" << "md" << "markdown" << "html" << "htm" << "cpp" << "h";
        imageExtList << "jpg" << "png" << "gif" << "tiff" << "jpeg" << "bmp" << "svg";
        rtfExtList << "rtf";

        if (textExtList.contains(fi.suffix(),Qt::CaseInsensitive))
        {
            strHtml = loadTextFileToHtml(strFile);
        }
        else if (imageExtList.contains(fi.suffix(),Qt::CaseInsensitive))
        {
            strHtml = loadImageFileToHtml(strFile);
        }
        else if (rtfExtList.contains(fi.suffix(), Qt::CaseInsensitive))
        {
            strHtml = loadRtfFileToHtml(strFile);
        }

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
