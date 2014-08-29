#include "wizFileReader.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

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
    ret.replace("\n","<br>");
    ret.replace(" ","&nbsp");
    return ret;
}

QString CWizFileReader::loadImageFileToHtml(QString strFileName)
{
    return QString("<img border=\"0\" src=\"file://%1\" />").arg(strFileName);

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
        textExtList<<"txt"<<"md"<<"markdown"<<"html"<<"htm"<<"cpp"<<"h";
        imageExtList<<"jpg"<<"png"<<"gif"<<"tiff"<<"jpeg"<<"bmp"<<"svg";

        if (textExtList.contains(fi.suffix(),Qt::CaseInsensitive))
        {
            strHtml = loadTextFileToHtml(strFile);
        }
        else if (imageExtList.contains(fi.suffix(),Qt::CaseInsensitive))
        {
            strHtml = loadImageFileToHtml(strFile);
        }

        if (!strHtml.isEmpty())
        {
            emit fileLoaded(strHtml);
        }

        emit loadProgress(nTotal, i + 1);
    }
    emit loadFinished();
    m_files.clear();
}
