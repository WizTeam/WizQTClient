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

#ifdef Q_OS_WIN
    QString strDefaultFontName = ::WizGetString("Common", "DefaultFont", "");
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

    CWizSettings settings(::WizGetDataStorePath() + "wiznote.ini");

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
            strPassword = db.GetEncryptedPassword();
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
    {
        CString strTempPath = ::WizGlobal()->GetTempPath();
        ::WizDeleteAllFilesInFolder(strTempPath);
    }

    // restart
    if (w.isRestart())
    {
        QProcess::startDetached(argv[0], QStringList());
    }

    return ret;
}
