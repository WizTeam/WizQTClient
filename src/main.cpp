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

    QIcon iconApp;
    iconApp.addFile(WizGetSkinResourceFileName("wiznote16"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote24"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote32"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote48"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote64"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote72"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote128"));
    QApplication::setWindowIcon(iconApp);

    // setup locale
    QString strLocale = settings.GetString("Users", "Locale", "");
    if (strLocale.isEmpty())
        //strLocale = QLocale::system().name();
        strLocale = "zh_CN";

    QTranslator translatorWizNote;
    translatorWizNote.load(QString("wiznote_") + strLocale, WizGetResourcesPath() + "languages/");
    a.installTranslator(&translatorWizNote);

    QTranslator translatorQt;
    translatorQt.load(QString("qt_") + strLocale, WizGetResourcesPath() + "languages/");
    a.installTranslator(&translatorQt);

    // figure out auto login or manually login
    QString strDefaultUser = settings.GetString("Users", "DefaultUser", "");
    bool bAutoLogin = settings.GetBool("Users", "AutoLogin", false);

    bool bFallback = true;
    QString strUserId, strPassword;
    if (bAutoLogin) {
        CWizDatabase db;
        if (db.Open(strDefaultUser, ""))
        {
            strUserId = strDefaultUser;
            strPassword = db.GetPassword2();
            bFallback = false;
        }
    }

    if (bFallback) {
        WelcomeDialog dlgWelcome(strDefaultUser);
        if (QDialog::Accepted != dlgWelcome.exec())
            return 0;

        strUserId = dlgWelcome.userId();
        strPassword = dlgWelcome.password();
    }

    // reset password for restart event
    QStringList args = QApplication::arguments();
    if (args.count() >= 4) {
        if (!args.at(1).compare("autologin")) {
            bool bOldAutoLogin = args.at(2).toInt();
            settings.SetBool("Users", "AutoLogin", bOldAutoLogin);
        }

        if (!args.at(3).compare("cleanpassword") &&
                args.at(4).toInt()) {
            CWizDatabase db;
            QString strUser = settings.GetString("Users", "DefaultUser", "");
            CString strPassword;

            if (db.Open(strUser, "")) {
                db.GetPassword(strPassword);
                db.SetPassword(strPassword, "");
            }
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
    {
        CString strTempPath = ::WizGlobal()->GetTempPath();
        ::WizDeleteAllFilesInFolder(strTempPath);
    }

    // restart
    if (w.isRestart())
    {
        // reset auto login
        bAutoLogin = settings.GetBool("Users", "AutoLogin", false);
        settings.SetBool("Users", "AutoLogin", true);

        // reset password
        bool bCleanPassword = false;
        QString strStoredPassword = db.GetEncryptedPassword();
        if (strStoredPassword.isEmpty()) {
            db.SetPassword("", strPassword);
            bCleanPassword = true;
        }

        QStringList argsRestart;
        argsRestart.append(QString("autologin"));
        argsRestart.append(QString(bAutoLogin ? "1" : "0"));
        argsRestart.append(QString("cleanpassword"));
        argsRestart.append(QString(bCleanPassword ? "1" : "0"));
        QProcess::startDetached(argv[0], argsRestart);
    } else if (w.isLogout()) {
        db.SetPassword(strPassword, "");
        QProcess::startDetached(argv[0], QStringList());
    }

    return ret;
}
