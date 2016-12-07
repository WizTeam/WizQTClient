#include "WizUpdaterProgressDialog.h"
#include "ui_WizUpdaterProgressDialog.h"

#include <QMouseEvent>
#include <QDesktopWidget>
#include <QDir>
#include <QTimer>
#include <QMessageBox>

#include "share/WizMisc.h"
#include "share/WizSettings.h"

WizUpdaterDialog::WizUpdaterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizUpdaterDialog)
    , m_isOldDatabase(false)
    , m_bUpdateDatabase(false)
    , m_bUpdateApp(false)
{
    ui->setupUi(this);

    center();
    setFixedSize(size());
    setWindowFlags(Qt::CustomizeWindowHint);

    QPixmap pixmap(::WizGetResourcesPath() + "skins/wizupdater.png");
    ui->labelIcon->setPixmap(pixmap);
}

void WizUpdaterDialog::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_bMovable = true;
        m_lastPos = event->pos();
    }
}

void WizUpdaterDialog::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons().testFlag(Qt::LeftButton) && m_bMovable) {
        move(pos() + (event->pos() - m_lastPos));
    }
}

void WizUpdaterDialog::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_bMovable = false;
    }
}

void WizUpdaterDialog::center()
{
    setGeometry(QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    size(),
                    qApp->desktop()->availableGeometry()
    ));
}

void WizUpdaterDialog::setGuiNotify(const QString& text)
{
    ui->label->setText(text);
    ui->progressUpdate->setValue(ui->progressUpdate->value() + 1);
}

WizUpdaterDialog::~WizUpdaterDialog()
{
    delete ui;
}

bool WizUpdaterDialog::checkNeedUpdate()
{
    if (needUpdateDatabase()) {
        return true;
    }

    if (needUpdateApp()) {
        return true;
    }

    return false;
}

bool WizUpdaterDialog::needUpdateDatabase()
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

bool WizUpdaterDialog::needUpdateApp()
{
    // check stub here, created by CWizUpdaterNotifyDialog
    QFile fileUpdate(QDir::homePath() + "/.wiznote/update/WIZNOTE_READY_FOR_UPGRADE");
    if (fileUpdate.exists()) {
        m_bUpdateApp = true;
    }

    return m_bUpdateApp;
}

void WizUpdaterDialog::doUpdate()
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

void WizUpdaterDialog::doUpdateAppSetSteps()
{
    int steps = 0;

    QString strUpdate = ::WizGetUpgradePath();
    QDir rootDir(strUpdate);

    steps = rootDir.entryList().count();

    ui->progressUpdate->setMaximum(steps);
}

void WizUpdaterDialog::doUpdateApp()
{
    // read metadata
    QList<QStringList> files;

    setGuiNotify("Prepare upgrade");

    QString strConfig = ::WizGetUpgradePath() + "config.txt";
    WizSettings* config = new WizSettings(strConfig);

    int i = 0;
    QString strFileEntry = config->getString("Files", QString::number(i));
    while (!strFileEntry.isEmpty()) {
        QStringList strFileMeta = strFileEntry.split("*");

        QStringList strFile;
        strFile << strFileMeta.at(0) << strFileMeta.at(1);
        files.append(strFile);

        i++;
        strFileEntry = config->getString("Files", QString::number(i));
    }

    QList<QStringList>::const_iterator it;
    for(it = files.constBegin(); it!= files.constEnd(); it++) {
        QString strLocal = ::WizGetAppPath() + (*it).at(0);
        QString strLocalPath = ::WizExtractFilePath(strLocal);
        QString strUpdate = ::WizGetUpgradePath() + (*it).at(0);
        QString strUpdatePath = ::WizExtractFilePath(strUpdate);

        QFile fileUpdate(strUpdate);
        if (fileUpdate.exists()) {
            // compare MD5
            QString md5Remote = (*it).at(1);
            QString md5Download = ::WizMd5FileString(strUpdate);

            if (md5Remote == md5Download) {
                ::WizEnsurePathExists(strLocalPath);
                fileUpdate.copy(strLocal);
                fileUpdate.remove();
                setGuiNotify(QString("Copying %1").arg(strLocal));
            }
        }

        ::WizDeleteFolder(strUpdatePath);
    }

    // remove config file
    QFile fileConfig(strConfig);
    fileConfig.remove();
    QFile fileZip(::WizGetUpgradePath() + "update.zip");
    fileZip.remove();

    // remove stub
    QFile fileStub(::WizGetUpgradePath() + "WIZNOTE_READY_FOR_UPGRADE");
    fileStub.remove();

    setGuiNotify("Upgrade done");
    ui->progressUpdate->setValue(ui->progressUpdate->maximum());
}

bool WizUpdaterDialog::detectIsOldDatabaseStruct()
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

void WizUpdaterDialog::doOldDatabaseUpgradeSetSteps()
{
    int steps = 0;

    QString oldDataStorePath = QDir::homePath() + "/WizNote/";
    QDir rootDir(oldDataStorePath);

    steps = rootDir.entryList().count();

    ui->progressUpdate->setMaximum(steps + 10);
}

void WizUpdaterDialog::doOldDatabaseUpgrade()
{
    QDir homeDir(QDir::homePath());
    QString oldDataStorePath = QDir::homePath() + "/WizNote/";
    QString newDataStorePath = QDir::homePath() + "/.wiznote/";

    bool ret = homeDir.rmdir(newDataStorePath);

    if (ret) {
        homeDir.rename(oldDataStorePath, newDataStorePath);
    } else {
        QMessageBox::critical(this, tr("Error"), tr("the .wiznote directory should be empty, please contect R&D team!"));
        return;
    }

    QDir rootDir(newDataStorePath);

    rootDir.remove("wiznote.ini");
    setGuiNotify("Remove wiznote.ini");
    rootDir.remove("wiznote.log");
    setGuiNotify("Remove wiznote.log");

    // enum each user
    CWizStdStringArray folders;
    WizEnumFolders(newDataStorePath, folders, 0);
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
            setGuiNotify(QString("Rename %1").arg(note));
        }

        // rename attachments data
        CWizStdStringArray attaches;
        WizEnumFiles(strOldAttaches, "*.dat", attaches, 0);
        foreach (const QString& attach, attaches) {
            QDir dirAttach(strOldAttaches);
            QString strNewName = strOldAttaches + "{" + attach.right(40).left(36) + "}";
            dirAttach.rename(attach, strNewName);
            setGuiNotify(QString("Rename %1").arg(attach));
        }

        // move user data to new dir
        setGuiNotify("reconstruct directories");
        userDir.rename(strOldNotes, folder + "notes/");
        userDir.rename(strOldAttaches, folder + "attachments/");

        userDir.mkdir("data");
        userDir.rename(folder + "notes", folder + "data/notes");
        userDir.rename(folder + "attachments", folder + "data/attachments");
        userDir.rename(folder + "index.db", folder + "data/index.db");
        userDir.rename(folder + "wizthumb.db", folder + "data/wizthumb.db");

        ui->progressUpdate->setValue(ui->progressUpdate->maximum());
        setGuiNotify("Update Done!");
    }
}

void WizUpdaterDialog::doDatabaseUpgrade()
{
    // stub here
}
