#include <QtGlobal>
#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include <QDir>
#include <QPixmapCache>
#include <QTranslator>
#include <QProcess>
#include <QSettings>
#include <QDesktopServices>

#include <sys/stat.h>

#include <extensionsystem/pluginmanager.h>
#include "wizmainwindow.h"
#include "wizLoginDialog.h"
#include "share/wizsettings.h"
#include "share/wizwin32helper.h"
#include "share/wizDatabaseManager.h"

#include "utils/pathresolve.h"
#include "utils/logger.h"
#include "sync/token.h"
#include "sync/apientry.h"
#include "sync/avatar.h"
#include "thumbcache.h"

using namespace ExtensionSystem;
using namespace Core::Internal;

static inline QStringList getPluginPaths()
{
    QStringList rc;
    // Figure out root:  Up one from 'bin'
    QDir rootDir = QApplication::applicationDirPath();
    rootDir.cdUp();
    const QString rootDirPath = rootDir.canonicalPath();
#if !defined(Q_OS_MAC)
    // 1) "plugins" (Win/Linux)
    QString pluginPath = rootDirPath;
    pluginPath += QLatin1Char('/');
    pluginPath += QLatin1String("/lib/wiznote/plugins");
    rc.push_back(pluginPath);
#else
    // 2) "PlugIns" (OS X)
    QString pluginPath = rootDirPath;
    pluginPath += QLatin1String("/PlugIns");
    rc.push_back(pluginPath);
#endif
    // 3) <localappdata>/plugins/<ideversion>
    //    where <localappdata> is e.g.
    //    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
    //    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
    //    "~/Library/Application Support/QtProject/Qt Creator" on Mac
#if QT_VERSION >= 0x050000
    pluginPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    pluginPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
    pluginPath += QLatin1Char('/');

#if !defined(Q_OS_MAC)
    pluginPath += QLatin1String("wiznote");
#else
    pluginPath += QLatin1String("WizNote");
#endif
    pluginPath += QLatin1String("/plugins/");
    rc.push_back(pluginPath);
    return rc;
}


#ifdef Q_OS_MAC
#  define SHARE_PATH "/../Resources"
#else
#  define SHARE_PATH "/../share/wiznote"
#endif

const char* g_lpszDesktopFileName = "\
[Desktop Entry]\n\
Exec=%1wiznote\n\
Icon=wiznote\n\
Type=Application\n\
Terminal=false\n\
Name=%2\n\
GenericName=%3\n\
Categories=WizNote;\n\
Name[en_US]=WizNote\n\
GenericName[en_US.UTF-8]=WizNote\n\
";


void installOnLinux()
{
    QString appPath = WizGetAppPath();
    QString strText = WizFormatString3(g_lpszDesktopFileName,
                                       appPath,
                                       QObject::tr("WizNote"),
                                       QObject::tr("WizNote"));
    //
    QString applicationsPath = QDir::homePath() + "/.local/share/applications/";
    ::WizEnsurePathExists(applicationsPath);
    //
    QString iconsBasePath = QDir::homePath() + "/.local/share/icons/hicolor/";
    ::WizEnsurePathExists(applicationsPath);
    //
    CWizStdStringArray arrayIconSize;
    arrayIconSize.push_back("16");
    arrayIconSize.push_back("32");
    arrayIconSize.push_back("48");
    arrayIconSize.push_back("64");
    arrayIconSize.push_back("128");
    arrayIconSize.push_back("256");
    for (CWizStdStringArray::const_iterator it = arrayIconSize.begin();
        it != arrayIconSize.end();
        it++)
    {
        QString iconSize = *it;
        QString iconPathName = iconSize + "x" + iconSize;
        QString iconFullPath = iconsBasePath + iconPathName + "/apps/";
        WizEnsurePathExists(iconFullPath);
        //
        QString resourceName = ":/logo_" + iconSize + ".png";
        QPixmap pixmap(resourceName);
        if (pixmap.isNull())
            continue;
        //
        pixmap.save(iconFullPath + "wiznote.png");
    }

    QString desktopFileName = applicationsPath + "wiznote.desktop";
    ::WizSaveUnicodeTextToUtf8File(desktopFileName, strText, false);
    //
    chmod(desktopFileName.toUtf8(), ACCESSPERMS);
}

int main(int argc, char *argv[])
{
    //
#if QT_VERSION < 0x050000
    qInstallMsgHandler(Utils::Logger::messageHandler);
#else
    qInstallMessageHandler(Utils::Logger::messageHandler);
#endif

    QApplication a(argc, argv);

    QApplication::setApplicationName(QObject::tr("WizNote"));
    QIcon icon;
    icon.addPixmap(QPixmap(":/logo_16.png"));
    icon.addPixmap(QPixmap(":/logo_32.png"));
    icon.addPixmap(QPixmap(":/logo_48.png"));
    icon.addPixmap(QPixmap(":/logo_96.png"));
    icon.addPixmap(QPixmap(":/logo_128.png"));
    icon.addPixmap(QPixmap(":/logo_256.png"));
    QApplication::setWindowIcon(icon);

#ifdef Q_OS_MAC
    // enable switch between qt widget and alien widget(cocoa)
    // refer to: https://bugreports.qt-project.org/browse/QTBUG-11401
    //a.setAttribute(Qt::AA_NativeWindows);
#endif


    // setup settings
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings* globalSettings = new QSettings(Utils::PathResolve::globalSettingsFilePath(), QSettings::IniFormat);

//#ifdef Q_OS_WIN
//    QString strDefaultFontName = settings.GetString("Common", "DefaultFont", "");
//    QFont f = WizCreateWindowsUIFont(a, strDefaultFontName);
//    a.setFont(f);
//#endif

    // setup plugin manager
    PluginManager pluginManager;
    PluginManager::setFileExtension(QLatin1String("pluginspec"));
    PluginManager::setGlobalSettings(globalSettings);

    const QStringList pluginPaths = getPluginPaths();
    PluginManager::setPluginPaths(pluginPaths);

    // use 3 times(30M) of Qt default usage
    int nCacheSize = globalSettings->value("Common/Cache", 10240*3).toInt();
    QPixmapCache::setCacheLimit(nCacheSize);

    QString strUserId = globalSettings->value("Users/DefaultUser").toString();
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

#ifndef Q_OS_MAC
    if (globalSettings->value("Common/Installed", 0).toInt() == 0)
    {
        globalSettings->setValue("Common/Installed", 1);
        installOnLinux();
    }
#endif

    // check update if needed
    //CWizUpdaterDialog updater;
    //if (updater.checkNeedUpdate()) {
    //    updater.show();
    //    updater.doUpdate();
    //    int ret = a.exec();
    //    QProcess::startDetached(argv[0], QStringList());
    //    return ret;
    //}

    // figure out auto login or manually login
    bool bFallback = true;

    // FIXME: move to WizService plugin initialize
    WizService::Token token;


    bool bAutoLogin = userSettings.autoLogin();
    strPassword = userSettings.password();

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

    QSettings* settings = new QSettings(Utils::PathResolve::userSettingsFilePath(strUserId), QSettings::IniFormat);
    PluginManager::setSettings(settings);
    //
    //
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

    WizService::Token::setUserId(strUserId);
    WizService::Token::setPasswd(strPassword);

    dbMgr.db().SetPassword(::WizEncryptPassword(strPassword));

    // FIXME: move to plugins
    WizService::AvatarHost avatarHost;

    // FIXME: move to core plugin initialize
    Core::ThumbCache cache;



    MainWindow w(dbMgr);

    //settings->setValue("Users/DefaultUser", strUserId);
    PluginManager::loadPlugins();

    w.show();
    w.init();

    int ret = a.exec();

    // clean up
    QString strTempPath = Utils::PathResolve::tempPath();
    ::WizDeleteAllFilesInFolder(strTempPath);

    if (w.isLogout()) {
        QProcess::startDetached(argv[0], QStringList());
    }

    return ret;
}
