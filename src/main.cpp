#include <QtCore>
#include <QtGui>

#include "wizmainwindow.h"
#include "wizupdaterprogressdialog.h"
#include "wizwelcomedialog.h"
#include "share/wizsettings.h"
#include "share/wizwin32helper.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName(QObject::tr("WizNote"));
    IWizGlobal::instance()->setVersion("1.2.0");

#if defined Q_OS_MAC
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("PlugIns");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#elif defined Q_OS_LINUX
    QDir dir(QApplication::applicationDirPath());
    dir.cd("plugins");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

    CWizSettings settings(QDir::homePath() + "/.wiznote/wiznote.ini");

#ifdef Q_OS_WIN
    QString strDefaultFontName = settings.GetString("Common", "DefaultFont", "");
    QFont f = WizCreateWindowsUIFont(a, strDefaultFontName);
    a.setFont(f);
#endif

    // set icon
    QIcon iconApp;
    iconApp.addFile(WizGetResourcesPath() + "skins/wiznote16.png");
    iconApp.addFile(WizGetResourcesPath() + "skins/wiznote24.png");
    iconApp.addFile(WizGetResourcesPath() + "skins/wiznote32.png");
    iconApp.addFile(WizGetResourcesPath() + "skins/wiznote48.png");
    iconApp.addFile(WizGetResourcesPath() + "skins/wiznote64.png");
    iconApp.addFile(WizGetResourcesPath() + "skins/wiznote128.png");
    iconApp.addFile(WizGetResourcesPath() + "skins/wiznote256.png");
    QApplication::setWindowIcon(iconApp);

    QString strUserId = settings.GetString("Users", "DefaultUser", "");
    QString strPassword;

    CWizUserSettings userSettings(strUserId);

    // setup locale for welcome dialog
    QString strLocale = userSettings.locale();

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
    if (bFallback) {

        WelcomeDialog dlgWelcome(strUserId, strLocale);
        if (QDialog::Accepted != dlgWelcome.exec())
            return 0;

        strUserId = dlgWelcome.userId();
        strPassword = dlgWelcome.password();
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

    // ready
    CWizDatabase db;
    if (!db.Open(strUserId, strPassword))
    {
        QMessageBox::critical(NULL, "", QObject::tr("Can not open account"));
        return 0;
    }

    MainWindow w(db);
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
