#include "wizupdaterdialog.h"
#include "ui_wizupdaterdialog.h"

#include <QTimer>
#include <QDir>

#include "share/wizmisc.h"

CWizUpdaterDialog::CWizUpdaterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizUpdaterDialog)
{
    ui->setupUi(this);
    setFixedSize(size());
    setWindowFlags(Qt::CustomizeWindowHint);

    QPixmap pixmap(::WizGetResourcesPath() + "skins/wiznote64.png");
    ui->labelIcon->setPixmap(pixmap);
}

void CWizUpdaterDialog::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_bMovable = true;
        m_lastPos = event->pos();
    }
}

void CWizUpdaterDialog::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons().testFlag(Qt::LeftButton) && m_bMovable) {
        move(pos() + (event->pos() - m_lastPos));
    }
}

void CWizUpdaterDialog::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_bMovable = false;
    }
}

CWizUpdaterDialog::~CWizUpdaterDialog()
{
    delete ui;
}

bool CWizUpdaterDialog::isPrepared()
{
    return true;
}

void CWizUpdaterDialog::prepare()
{
    m_strExecPath = WizGetDataStorePath() + "update/" + WizGetAppFileName();
}

bool CWizUpdaterDialog::checkNeedUpdate()
{
    if (needUpdateDatabase()) {
        return true;
    }

    if (needUpdateApp()) {
        return true;
    }

    return false;
}

bool CWizUpdaterDialog::needUpdateDatabase()
{
    // from v1.0 to new database structure
    if (detectIsOldDatabaseStruct()) {
        m_isOldDatabase = true;
        return true;
    }

    // for newer database update, just check inside user index.db and do update
    m_bUpdateDatabase = false;

    return false;
}

bool CWizUpdaterDialog::needUpdateApp()
{
    m_bUpdateApp = false;
    return false;
}

void CWizUpdaterDialog::doUpdate()
{
    if (m_isOldDatabase) {
        doOldDatabaseUpgrade();
    }

    if (m_bUpdateDatabase) {
        doDatabaseUpgrade();
    }

    if (m_bUpdateApp) {
        doUpdateApp();
    }

    QTimer::singleShot(3000, this, SLOT(close()));
}

bool CWizUpdaterDialog::detectIsOldDatabaseStruct()
{
    // old structure store on $HOME/WizNote/ on linux/mac/windows
    // do not envoke WizGetDataStorePath()
    QString oldDataStorePath = QDir::homePath() + "/WizNote/";
    if(PathFileExists(oldDataStorePath)) {
        CWizStdStringArray folders;
        WizEnumFolders(oldDataStorePath, folders, 0);
        foreach (const QString& folder, folders) {
            if (PathFileExists(folder + "index.db") && \
                    PathFileExists(folder + "wizthumb.db") && \
                    PathFileExists(folder + "Notes"))
                return true;
        }
    }

    return false;
}

void CWizUpdaterDialog::doOldDatabaseUpgrade()
{
    QString oldDataStorePath = QDir::homePath() + "/WizNote/";
    QDir rootDir(oldDataStorePath);

    rootDir.remove("wiznote.ini");
    rootDir.remove("wiznote.log");

    // enum each user
    CWizStdStringArray folders;
    WizEnumFolders(oldDataStorePath, folders, 0);
    foreach (const QString& folder, folders) {
        QDir userDir(folder);

        QString strOldNotes = folder + "Notes/";
        QString strOldAttaches = folder + "Attachments/";

        // rename notes data
        CWizStdStringArray notes;
        WizEnumFiles(strOldNotes, "*.ziw", notes, 0);
        foreach (const QString& note, notes) {
            QDir dirNote(strOldNotes);
            // 36 chars for GUID and 4 chars for extention
            QString strNewName = strOldNotes + "{" + note.right(40).left(36) + "}";
            dirNote.rename(note, strNewName);
        }

        // rename attachments data
        CWizStdStringArray attaches;
        WizEnumFiles(strOldAttaches, "*.dat", attaches, 0);
        foreach (const QString& attach, attaches) {
            QDir dirAttach(strOldAttaches);
            QString strNewName = strOldAttaches + "{" + attach.right(40).left(36) + "}";
            dirAttach.rename(attach, strNewName);
        }

        // move user data to new dir
        userDir.rename(strOldNotes, folder + "notes/");
        userDir.rename(strOldAttaches, folder + "attachments/");

        userDir.mkdir("data");
        userDir.rename(folder + "notes", folder + "data/notes");
        userDir.rename(folder + "attachments", folder + "data/attachments");
        userDir.rename(folder + "index.db", folder + "data/index.db");
        userDir.rename(folder + "wizthumb.db", folder + "data/wizthumb.db");
    }
}

void CWizUpdaterDialog::doDatabaseUpgrade()
{

}

void CWizUpdaterDialog::doUpdateApp()
{

}
