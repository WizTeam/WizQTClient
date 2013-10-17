#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include <QDir>
#include <QPixmapCache>
#include <QTranslator>
#include <QProcess>

#include "wizmainwindow.h"
#include "wizupdaterprogressdialog.h"
#include "wizLoginDialog.h"
#include "share/wizsettings.h"
#include "share/wizwin32helper.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // enable swtich between qt widget and alien widget(cocoa)
    // refer to: https://bugreports.qt-project.org/browse/QTBUG-11401
    a.setAttribute(Qt::AA_NativeWindows);

    QApplication::setApplicationName(QObject::tr("WizNote"));
    QApplication::setWindowIcon(QIcon(":/logo.png"));

#ifndef Q_OS_MAC
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("plugins");
    QApplication::addLibraryPath(dir.absolutePath());
#endif

    CWizSettings settings(QDir::homePath() + "/.wiznote/wiznote.ini");

    // use 3 times(30M) of Qt default usage
    int nCacheSize = settings.value("Common/Cache", 10240*3).toInt();
    QPixmapCache::setCacheLimit(nCacheSize);

#ifdef Q_OS_WIN
    QString strDefaultFontName = settings.GetString("Common", "DefaultFont", "");
    QFont f = WizCreateWindowsUIFont(a, strDefaultFontName);
    a.setFont(f);
#endif

    QString strUserId = settings.GetString("Users", "DefaultUser", "");
    QString strPassword;

    CWizUserSettings userSettings(strUserId);

    // setup locale for welcome dialog
    QString strLocale = userSettings.locale();
    QLocale::setDefault(strLocale);

    QTranslator translatorWizNote;
    QString strLocaleFile = WizGetLocaleFileName(strLocale);
    translatorWizNote.load(strLocaleFile);
    a.installTranslator(&translatorWizNote);

    QTranslator translatorQt;
    strLocaleFile = WizGetQtLocaleFileName(strLocale);
    translatorQt.load(strLocaleFile);
    a.installTranslator(&translatorQt);

    // check update if needed
    CWizUpdaterDialog updater;
    if (updater.checkNeedUpdate()) {
        updater.show();
        updater.doUpdate();
        int ret = a.exec();
        QProcess::startDetached(argv[0], QStringList());
        return ret;
    }

    // figure out auto login or manually login
    bool bFallback = true;

    bool bAutoLogin = userSettings.autoLogin();
    strPassword = ::WizDecryptPassword(userSettings.password());

    if (bAutoLogin && !strPassword.isEmpty()) {
        bFallback = false;
    }

    // manually login
    CWizLoginDialog loginDialog(strUserId, strLocale);
    if (bFallback) {
        if (QDialog::Accepted != loginDialog.exec())
            return 0;

        strUserId = loginDialog.userId();
        strPassword = loginDialog.password();
    }

    // reset password for restart event, will not touch welcome dialog
    QStringList args = QApplication::arguments();
    if (args.count() >= 3) {
        for (int i = 0; i < args.count(); i++) {
            if (!args.at(i).compare("--autologin=0")) {
                userSettings.setAutoLogin(false);
            } else if(!args.at(i).compare("--autologin=1")) {
                userSettings.setAutoLogin(true);
            } else if(!args.at(i).compare("--cleanpassword=1")) {
                userSettings.setPassword();
            }
        }
    }

    // reset locale for current user.
    userSettings.setUser(strUserId);
    strLocale = userSettings.locale();

    a.removeTranslator(&translatorWizNote);
    strLocaleFile = WizGetLocaleFileName(strLocale);
    translatorWizNote.load(strLocaleFile);
    a.installTranslator(&translatorWizNote);

    a.removeTranslator(&translatorQt);
    strLocaleFile = WizGetQtLocaleFileName(strLocale);
    translatorQt.load(strLocaleFile);
    a.installTranslator(&translatorQt);

    CWizDatabaseManager dbMgr(strUserId);
    if (!dbMgr.openAll()) {
        QMessageBox::critical(NULL, "", QObject::tr("Can not open database"));
        return 0;
    }

    dbMgr.db().SetPassword(strPassword);

    MainWindow w(dbMgr);
    w.show();
    w.init();

    int ret = a.exec();

    // clean up
    QString strTempPath = ::WizGlobal()->GetTempPath();
    ::WizDeleteAllFilesInFolder(strTempPath);

    // restart
    if (w.isRestart())
    {
        userSettings.setUser(strUserId);

        // reset auto login
        bAutoLogin = userSettings.autoLogin();
        userSettings.setAutoLogin(true);

        // reset password
        // if user did not choose remember password, stored password already cleaned from database
        // we need store it back.
        bool bCleanPassword = false;
        if (userSettings.password().isEmpty()) {
            userSettings.setPassword(::WizEncryptPassword(strPassword));
            bCleanPassword = true;
        }

        // generate arguments
        QStringList argsRestart;
        if (bAutoLogin) {
            argsRestart.append(QString("--autologin=1"));
        } else {
            argsRestart.append(QString("--autologin=0"));
        }

        if (bCleanPassword) {
            argsRestart.append(QString("--cleanpassword=1"));
        } else {
            argsRestart.append(QString("--cleanpassword=0"));
        }

        QProcess::startDetached(argv[0], argsRestart);
    } else if (w.isLogout()) {
        QProcess::startDetached(argv[0], QStringList());
    }

    return ret;
}
