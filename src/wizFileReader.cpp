#include "wizFileReader.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDebug>

#include "utils/pathresolve.h"
#include "utils/misc.h"
#include "html/wizhtmlcollector.h"
#include "share/wizmisc.h"
#include "share/wizRtfReader.h"
#include "share/wizDatabase.h"
#include "core/wizNoteManager.h"
#include "mac/wizmachelper.h"

CWizFileImporter::CWizFileImporter(CWizDatabaseManager& dbMgr, QObject *parent)
    : QObject(parent)
    , m_dbMgr(dbMgr)
{
}

void CWizFileImporter::importFiles(const QStringList& strFiles, const QString& strTargetFolderLocation)
{
    WIZTAGDATA tag;
    importFiles(strFiles, "", strTargetFolderLocation, tag);
}

void CWizFileImporter::importFiles(const QStringList& strFiles, const QString& strKbGUID, const WIZTAGDATA& tag)
{
    QString location = m_dbMgr.db(strKbGUID).GetDefaultNoteLocation();
    importFiles(strFiles, strKbGUID, location, tag);
}

void CWizFileImporter::importFiles(const QStringList& strFiles, const QString& strKbGUID,
                                   const QString& strTargetFolderLocation, const WIZTAGDATA& tag)
{
    int nTotal = strFiles.count();
    int nFailed = 0;
    QString text(tr(" file(s) import failed: \n"));
    for (int i = 0; i < nTotal; ++i)
    {
        QString strFile = strFiles.at(i);
        if (!importFile(strFile, strKbGUID, strTargetFolderLocation, tag))
        {
            ++nFailed;
            text.append(strFile).append("\n");
        }

        emit importProgress(nTotal, i + 1);
    }
    text = QString::number(nFailed) + text;
    emit importFinished(nFailed == 0, text);
}

QString CWizFileImporter::loadHtmlFileToHtml(const QString& strFileName)
{
    QFile file(strFileName);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return "";
    QTextStream in(&file);
    QString ret = in.readAll();
    file.close();

    return ret;
}

QString CWizFileImporter::loadTextFileToHtml(const QString& strFileName)
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

    return ret;
}

QString CWizFileImporter::loadImageFileToHtml(const QString& strFileName)
{
    return QString("<img border=\"0\" src=\"file://%1\" />").arg(strFileName);
}

QString CWizFileImporter::loadRtfFileToHtml(const QString& strFileName)
{
    QString strHtml;
    if (CWizRtfReader::load(strFileName, strHtml))
        return strHtml;

    return "";
}

bool CWizFileImporter::importFile(const QString& strFile, const QString& strKbGUID,
                                  const QString& strLocation, const WIZTAGDATA& tag)
{
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
    bool addAttach = false;
    bool containsImage = false;
    QString docType = fi.suffix();
    if (textExtList.contains(docType,Qt::CaseInsensitive))
    {
        strHtml = loadTextFileToHtml(strFile);
        addAttach = true;
    }
    else if (imageExtList.contains(docType,Qt::CaseInsensitive))
    {
        strHtml = loadImageFileToHtml(strFile);
        containsImage = true;
    }
    else if (htmlExtList.contains(docType, Qt::CaseInsensitive))
    {
        strHtml = loadHtmlFileToHtml(strFile);
        containsImage = true;
        addAttach = true;
    }
#ifdef Q_OS_MAC
    else if (rtfExtList.contains(docType, Qt::CaseInsensitive))
    {
        if (!documentToHtml(strFile, RTFTextDocumentType, strHtml))
            return false;
        WizGetBodyContentFromHtml(strHtml, true);
        addAttach = true;
    }
    else if (docExtList.contains(docType))
    {
        if (!documentToHtml(strFile, DocFormatTextDocumentType, strHtml))
            return false;
        WizGetBodyContentFromHtml(strHtml, true);
        addAttach = true;
    }
    else if (webExtList.contains(docType))
    {
        if (!documentToHtml(strFile, WebArchiveTextDocumentType, strHtml))
            return false;
        WizGetBodyContentFromHtml(strHtml, true);
        addAttach = true;
    }
    else
    {
        addAttach = true;
    }
#endif
    QString strTitle = Utils::Misc::extractFileName(strFile);

    CWizNoteManager manager(m_dbMgr);
    WIZDOCUMENTDATA doc;
    bool ret = manager.createNote(doc, strKbGUID, strTitle, strHtml, strLocation, tag);
    if (!ret)
    {
        qCritical() << "create note faile : " << strTitle;
        return false;
    }

    CWizDatabase& db = m_dbMgr.db(strKbGUID);
    if (addAttach)
    {
        WIZDOCUMENTATTACHMENTDATA attach;
        if (!db.AddAttachment(doc, strFile, attach))
        {
            qWarning() << "add attachment failed , " << strFile;
        }
    }
    else if (containsImage)
    {
        //为了提取和file路径相关联的图片，在创建之后更新笔记内容
        db.UpdateDocumentData(doc, strHtml, strFile, 0);
    }

    return true;
}
