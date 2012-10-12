#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QTranslator>
#include <QProcess>

#include "mainwindow.h"
#include "welcomedialog.h"
#include "share/wizsettings.h"
#include "share/wizwin32helper.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName(QObject::tr("WizNote"));

    CWizSettings settings(::WizGetDataStorePath() + "wiznote.ini");

#ifdef Q_OS_WIN
    QString strDefaultFontName = settings.GetString("Common", "DefaultFont", "");
    QFont f = WizCreateWindowsUIFont(a, strDefaultFontName);
    a.setFont(f);
#endif

    // set icon
    QIcon iconApp;
    iconApp.addFile(WizGetResourcesPath() + "wiznote16.png");
    iconApp.addFile(WizGetResourcesPath() + "wiznote24.png");
    iconApp.addFile(WizGetResourcesPath() + "wiznote32.png");
    iconApp.addFile(WizGetResourcesPath() + "wiznote48.png");
    iconApp.addFile(WizGetResourcesPath() + "wiznote64.png");
    iconApp.addFile(WizGetResourcesPath() + "wiznote128.png");
    //iconApp.addFile(WizGetSkinResourceFileName("wiznote16"));
    //iconApp.addFile(WizGetSkinResourceFileName("wiznote24"));
    //iconApp.addFile(WizGetSkinResourceFileName("wiznote32"));
    //iconApp.addFile(WizGetSkinResourceFileName("wiznote48"));
    //iconApp.addFile(WizGetSkinResourceFileName("wiznote64"));
    //iconApp.addFile(WizGetSkinResourceFileName("wiznote72"));
    //iconApp.addFile(WizGetSkinResourceFileName("wiznote128"));
    QApplication::setWindowIcon(iconApp);

    QString strDefaultUser = settings.GetString("Users", "DefaultUser", "");

    CWizUserSettings userSettings(strDefaultUser);

    // setup locale
    QString strLocale = userSettings.locale();
    if (strLocale.isEmpty())
        strLocale = settings.GetString("Common", "Locale", "");
    if (strLocale.isEmpty()) {
        strLocale = QLocale::system().name();
        settings.SetString("Common", "Locale", strLocale);
    }

    QTranslator translatorWizNote;
    translatorWizNote.load(QString("wiznote_") + strLocale, WizGetResourcesPath() + "languages/");
    a.installTranslator(&translatorWizNote);

    QTranslator translatorQt;
    translatorQt.load(QString("qt_") + strLocale, WizGetResourcesPath() + "languages/");
    a.installTranslator(&translatorQt);

    // figure out auto login or manually login
    bool bAutoLogin = settings.GetBool("Users", "AutoLogin", false);

    bool bFallback = true;
    QString strUserId, strPassword;
    if (bAutoLogin) {
        strUserId = strDefaultUser;
        strPassword = ::WizDecryptPassword(userSettings.password());
        bFallback = false;
    }

    // manually login
    if (bFallback) {
        WelcomeDialog dlgWelcome(strDefaultUser);
        if (QDialog::Accepted != dlgWelcome.exec())
            return 0;

        strUserId = dlgWelcome.userId();
        strPassword = dlgWelcome.password();
    }

    // reset password for restart event
    QStringList args = QApplication::arguments();
    if (args.count() >= 3) {
        if (!args.at(1).compare("--autologin=0")) {
            settings.SetBool("Users", "AutoLogin", false);
        }

        if (!args.at(2).compare("--cleanpassword=1")) {
            userSettings.setUser(strDefaultUser);
            userSettings.setPassword();
        }
    }

    // ready
    CWizDatabase db;
    if (!db.Open(strUserId, strPassword))
    {
        QMessageBox::critical(NULL, "", QObject::tr("Can not open account"));
        return 0;
    }

    MainWindow w(db);
    w.showMaximized();
    w.show();
    w.init();

    int ret = a.exec();

    // clean up
    QString strTempPath = ::WizGlobal()->GetTempPath();
    ::WizDeleteAllFilesInFolder(strTempPath);

    // restart
    if (w.isRestart())
    {
        // reset auto login
        bAutoLogin = settings.GetBool("Users", "AutoLogin", false);
        settings.SetBool("Users", "AutoLogin", true);

        // reset password
        // if user did not choose remember password, stored password already cleaned from database
        // we need store it back.
        bool bCleanPassword = false;
        userSettings.setUser(strUserId);

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
            argsRestart.append(QString("--cleanpassword=1"));
        }

        QProcess::startDetached(argv[0], argsRestart);
    } else if (w.isLogout()) {
        userSettings.setUser(strUserId);
        userSettings.setPassword();
        QProcess::startDetached(argv[0], QStringList());
    }

    return ret;
}
