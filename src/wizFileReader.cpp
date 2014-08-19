#include "wizFileReader.h"
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

CWizFileReader::CWizFileReader(QObject *parent) :
    QThread(parent)
{
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
        return;
    QTextStream in(&file);
    return in.readAll();
}

QString CWizFileReader::loadImageFileToHtml(QString strFileName)
{
    // image to html
}

void CWizFileReader::run()
{
    int nTotal = m_files.count();
    for (int i = 0; i < nTotal; i++)
    {
        QString strFile = m_files.at(i);
        QFileInfo fi(strFile);
        QString strHtml;
        QStringList textExtList,imageExtList;
        textExtList<<"txt"<<"md"<<"markdown"<<"html"<<"htm";
        imageExtList<<"jpg"<<"png"<<"gif"<<"tiff"<<"jpeg"<<"bmp"<<"svg";

        if (textExtList.contains(&fi.suffix(),Qt::CaseInsensitive))
        {
            strHtml = loadTextFileToHtml(strFile)
        }
        else if (imageExtList.contains(&fi.suffix(),Qt::CaseInsensitive))
        {
            strHtml = loadImageFileToHtml(strFile);
        }

        if (!strHtml.isEmpty())
        {
            emit fileLoaded(strHtml);
        }

        emit loadProgress(i + 1, nTotal);
    }
}
