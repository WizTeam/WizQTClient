#include "wizNoteManager.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include "utils/pathresolve.h"
#include "utils/misc.h"
#include "share/wizobject.h"
#include "share/wizDatabase.h"
#include "share/wizDatabaseManager.h"
#include "share/wizthreads.h"

void CWizNoteManager::createIntroductionNoteForNewRegisterAccount()
{
    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [=](){
        //get local note
        QDir dir(Utils::PathResolve::introductionNotePath());
        QStringList introductions = dir.entryList(QDir::Files);
        if (introductions.isEmpty())
            return;

        QSettings settings(Utils::PathResolve::introductionNotePath() + "settings.ini", QSettings::IniFormat);
        //copy note to new account
        CWizDatabase& db = m_dbMgr.db();
        for (QString fileName : introductions)
        {
            QString filePath = Utils::PathResolve::introductionNotePath() + fileName;
            QFileInfo info(filePath);
            if (info.suffix() == "ini")
                continue;
            settings.beginGroup("Location");
            QString location = settings.value(info.baseName(), "/My Notes/").toByteArray();
            settings.endGroup();
            WIZTAGDATA tag;
            WIZDOCUMENTDATA doc;
            settings.beginGroup("Title");
            doc.strTitle = settings.value(info.baseName()).toString();
            settings.endGroup();
            if (!db.CreateDocumentByTemplate(filePath, location, tag, doc))
            {
                qCritical() << "create introduction note failed : " << filePath;
            }
        }
    });
}

bool CWizNoteManager::createNote(WIZDOCUMENTDATA& data)
{
    return createNote(data, "", QObject::tr("Untitled"), "<p><br/></p>", "");
}

bool CWizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID)
{
    return createNote(data, strKbGUID, QObject::tr("Untitled"), "<p><br/></p>", "");
}

bool CWizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strLocation)
{
    return createNote(data, strKbGUID, QObject::tr("Untitled"), "<p><br/></p>", strLocation);
}

bool CWizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const WIZTAGDATA& tag)
{
    return createNote(data, strKbGUID, QObject::tr("Untitled"), "<p><br/></p>", "", tag);
}

bool CWizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strLocation, const WIZTAGDATA& tag)
{
    return createNote(data, strKbGUID, QObject::tr("Untitled"), "<p><br/></p>", strLocation, tag);
}

bool CWizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strTitle, const QString& strHtml)
{
    return createNote(data, strKbGUID, strTitle, strHtml, "");
}

bool CWizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strTitle, const QString& strHtml,
                                 const QString& strLocation)
{
    QString location = strLocation;
    if (location.isEmpty())
    {
        location = m_dbMgr.db(strKbGUID).GetDefaultNoteLocation();
    }

    QString strBody = Utils::Misc::getHtmlBodyContent(strHtml);
    if (!m_dbMgr.db(strKbGUID).CreateDocumentAndInit(strBody, "", 0, strTitle, "newnote", location, "", data))
    {
        qCritical() << "Failed to new document!";
        return false;
    }

    return true;
}


bool CWizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strTitle, const QString& strHtml,
                                 const WIZTAGDATA& tag)
{
    QString location = m_dbMgr.db(strKbGUID).GetDefaultNoteLocation();
    return createNote(data, strKbGUID, strTitle, strHtml, location, tag);
}

bool CWizNoteManager::createNote(WIZDOCUMENTDATA& data, const QString& strKbGUID,
                                 const QString& strTitle, const QString& strHtml,
                                 const QString& strLocation, const WIZTAGDATA& tag)
{
    if (!createNote(data, strKbGUID, strTitle, strHtml, strLocation))
        return false;

    if (!tag.strGUID.IsEmpty())
    {
        CWizDocument doc(m_dbMgr.db(strKbGUID), data);
        doc.AddTag(tag);
    }

    return true;
}


CWizNoteManager::CWizNoteManager(CWizDatabaseManager& dbMgr)
    : m_dbMgr(dbMgr)
{
}

