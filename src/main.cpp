#include <QtGui/QApplication>
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
    //
#ifdef Q_OS_WIN
    QString strDefaultFontName = ::WizGetString("Common", "DefaultFont", "");
    QFont f = WizCreateWindowsUIFont(a, strDefaultFontName);
    a.setFont(f);
    //
#endif
    //
    QIcon iconApp;
    iconApp.addFile(WizGetSkinResourceFileName("wiznote16"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote24"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote32"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote48"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote64"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote72"));
    iconApp.addFile(WizGetSkinResourceFileName("wiznote128"));
    QApplication::setWindowIcon(iconApp);

    QString localName = QLocale::system().name();

    QTranslator translatorWizNote;
    translatorWizNote.load(QString("wiznote_") + localName, WizGetResourcesPath() + "languages/");
    a.installTranslator(&translatorWizNote);

    QTranslator translatorQt;
    translatorQt.load(QString("qt_") + localName, WizGetResourcesPath() + "languages/");
    a.installTranslator(&translatorQt);

    const CString strCommon("Common");
    //
    CString strUserId = WizGetString(strCommon, "UserId", "");
    CString strPassword = WizGetEncryptedString(strCommon, "Password", "");
    //
    if (strUserId.IsEmpty() || strPassword.IsEmpty())
    {
        WelcomeDialog dlgWelcome;
        dlgWelcome.setUserId(strUserId);
        if (QDialog::Accepted != dlgWelcome.exec())
            return 0;
        //
        strUserId = dlgWelcome.userId();
        strPassword = dlgWelcome.password();
        if (strUserId.IsEmpty()
            || strPassword.IsEmpty())
            return 0;
        //
        WizSetString(strCommon, "UserId", strUserId);
        WizSetEncryptedString(strCommon, "Password", strPassword);
    }
    //
    CWizDatabase db;
    if (!db.Open(strUserId, strPassword))
    {
        QMessageBox::critical(NULL, "", QObject::tr("Can not open account"));
        return 0;
    }
    //
    MainWindow w(db);
    w.show();

    int ret = a.exec();
    //
    if (w.isRestart())
    {
        QProcess::execute(argv[0], QStringList());
    }

    //
    return ret;
}
