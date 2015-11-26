#include "wizNoteManager.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#if QT_VERSION > 0x050000
#include <QtConcurrent>
#else
#include <QtConcurrentRun>
#endif

#include "share/wizobject.h"
#include "utils/pathresolve.h"
#include "share/wizDatabase.h"
#include "share/wizDatabaseManager.h"

CWizNoteManager* CWizNoteManager::m_instance = nullptr;

CWizNoteManager* CWizNoteManager::instance()
{
    Q_ASSERT(m_instance);

    return m_instance;
}

bool CWizNoteManager::createSingleton(CWizExplorerApp& app)
{
    m_instance = new CWizNoteManager(app);
    return true;
}

void CWizNoteManager::createIntroductionNoteForNewRegisterAccount()
{
    QtConcurrent::run([=](){
        //get local note
        QDir dir(Utils::PathResolve::introductionNotePath());
        QStringList introductions = dir.entryList(QDir::Files);
        if (introductions.isEmpty())
            return;

        QSettings settings(Utils::PathResolve::introductionNotePath() + "settings.ini", QSettings::IniFormat);

        //copy note to new account
        CWizDatabase& db = m_app.databaseManager().db();
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
                qDebug() << "create introduction note failed : " << filePath;
            }
        }
    });
}

CWizNoteManager::CWizNoteManager(CWizExplorerApp& app)
    : m_app(app)
{
}

