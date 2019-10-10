#include <QtGlobal>
#include <QApplication>
#include <QTreeWidget>
#include <QMessageBox>
#include <QIcon>
#include <QDir>
#include <QPixmapCache>
#include <QTranslator>
#include <QProcess>
#include <QSettings>
#include <QDesktopServices>
#include <QSslConfiguration>
#include <QNetworkProxy>
#include <QtWebEngine>

#include <sys/stat.h>

#include "utils/WizPathResolve.h"
#include "utils/WizLogger.h"
#include "utils/WizStyleHelper.h"
#include "share/WizSettings.h"
#include "share/WizWin32Helper.h"
#include "share/WizDatabaseManager.h"
#include "share/WizSingleApplication.h"
#include "share/WizThreads.h"
#include "share/WizGlobal.h"

#include "core/WizNoteManager.h"

#ifdef Q_OS_MAC
#include "mac/WizMacHelper.h"
#include "mac/WizIAPHelper.h"
#endif

#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"
#include "sync/WizAvatarHost.h"
#include "WizThumbCache.h"
#include "WizMainWindow.h"
#include "WizDocumentWebEngine.h"
#include "WizLoginDialog.h"


#ifdef Q_OS_MAC
#  define SHARE_PATH "/../Resources"
#else
#  define SHARE_PATH "/../share/wiznote"
#endif


const char* g_lpszDesktopFileName = "\
[Desktop Entry]\n\
Exec=%1WizNote\n\
Icon=wiznote\n\
Type=Application\n\
Terminal=false\n\
Name=%2\n\
GenericName=%3\n\
Categories=WizNote;\n\
Name[en_US]=WizNote\n\
GenericName[en_US.UTF-8]=WizNote\n\
";


#ifdef Q_OS_LINUX
void installOnLinux()
{
    QString appPath = Utils::WizPathResolve::appPath();
    QString strText = WizFormatString3(g_lpszDesktopFileName,
                                       appPath,
                                       QObject::tr("WizNote"),
                                       QObject::tr("WizNote"));
    //
    QString applicationsPath = QDir::homePath() + "/.local/share/applications/";
    ::WizEnsurePathExists(applicationsPath);
    //
    QString iconsBasePath = QDir::homePath() + "/.local/share/icons/hicolor/";
    ::WizEnsurePathExists(iconsBasePath);
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
#endif

int mainCore(int argc, char *argv[])
{

#ifdef Q_OS_LINUX
    // create single application for linux
    WizSingleApplication a(argc, argv, "Special-Message-for-WizNote-SingleApplication");
    if (a.isRunning())
    {
        a.sendMessage(WIZ_SINGLE_APPLICATION);
        return 0;
    }
    //
    QtWebEngine::initialize();
#else
    QApplication a(argc, argv);
    //
    qDebug() << QApplication::font();
    //QFont font = QApplication::font();
    //font.setFamily("Helvetica Neue");
    //QApplication::setFont(font);
    //
#ifdef BUILD4APPSTORE
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("PlugIns");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif
    //
    QtWebEngine::initialize();

#ifdef BUILD4APPSTORE
    WizIAPHelper helper;
    helper.validteReceiptOnLauch();
#endif
#endif

#ifdef Q_OS_WIN
    QFont appFont = WizCreateWindowsUIFont(a, WizGetWindowsFontName());
    QApplication::setFont(appFont);
#endif

    qInstallMessageHandler(Utils::WizLogger::messageHandler);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication::setApplicationName(QObject::tr("WizNote"));
    QApplication::setOrganizationName(QObject::tr("cn.wiz.wiznoteformac"));

    QIcon icon;
    icon.addPixmap(QPixmap(":/logo_16.png"));
    icon.addPixmap(QPixmap(":/logo_32.png"));
    icon.addPixmap(QPixmap(":/logo_48.png"));
    icon.addPixmap(QPixmap(":/logo_64.png"));
    icon.addPixmap(QPixmap(":/logo_128.png"));
    icon.addPixmap(QPixmap(":/logo_256.png"));
    QApplication::setWindowIcon(icon);

#ifdef Q_OS_MAC
    wizMacInitUncaughtExceptionHandler();
    wizMacRegisterSystemService();

    // init sys local for crash report
    QString sysLocal = QLocale::system().name();
    QTranslator translatorSys;
    QString sysLocalFile = Utils::WizPathResolve::localeFileName(sysLocal);
    translatorSys.load(sysLocalFile);
    a.installTranslator(&translatorSys);

    initCrashReporter();

    a.removeTranslator(&translatorSys);

    WizQueuedThreadsInit();

    //FIXME: 在Mac osx安全更新之后存在ssl握手问题，此处进行特殊处理
    QSslConfiguration conf = QSslConfiguration::defaultConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(conf);
#endif

    if (isDarkMode()) {
        a.setStyleSheet("QToolTip { \
                        font: 12px; \
                        color:#cccccc; \
                        padding:0px 1px; \
                        background-color: #F8F8F8; \
                        border:0px;}");

#ifndef Q_OS_MAC
        QString menuStyle = QString("QMenu {color:white;}"
                                    "QMenu::item {color: #a6a6a6;}"
                                    "QMenu::item:selected {background-color: #0058d1; color:#ffffff }"
                                    "QMenu::item:disabled {color: #5c5c5c; }"
                                    );
        a.setStyleSheet(menuStyle);

#endif
    } else {
        a.setStyleSheet("QToolTip { \
                        font: 12px; \
                        color:#000000; \
                        padding:0px 1px; \
                        background-color: #F8F8F8; \
                        border:0px;}");
    }

    // setup settings
    QSettings::setDefaultFormat(QSettings::IniFormat);
    //
    QSettings* globalSettings = new QSettings(Utils::WizPathResolve::globalSettingsFile(), QSettings::IniFormat);
    WizGlobal::setGlobalSettings(globalSettings);
    //

    // use 3 times(30M) of Qt default usage
    int nCacheSize = globalSettings->value("Common/Cache", 10240*3).toInt();
    QPixmapCache::setCacheLimit(nCacheSize);

    QString strUserGuid = globalSettings->value("Users/DefaultUserGuid").toString();
    QList<WizLocalUser> localUsers;
    WizGetLocalUsers(localUsers);    
    QString strAccountFolderName = WizGetLocalFolderName(localUsers, strUserGuid);

    QString strPassword;
    WizUserSettings userSettings(strAccountFolderName);

    QSettings* settings = new QSettings(Utils::WizPathResolve::userSettingsFile(strAccountFolderName), QSettings::IniFormat);
    WizGlobal::setSettings(settings);
    //
    // setup locale for welcome dialog
    QString strLocale = userSettings.locale();
    QLocale::setDefault(strLocale);

    QTranslator translatorWizNote;
    QString strLocaleFile = Utils::WizPathResolve::localeFileName(strLocale);
    translatorWizNote.load(strLocaleFile);
    a.installTranslator(&translatorWizNote);

    QTranslator translatorQt;
    strLocaleFile = Utils::WizPathResolve::qtLocaleFileName(strLocale);
    translatorQt.load(strLocaleFile);
    a.installTranslator(&translatorQt);

#ifdef Q_OS_LINUX
    if (globalSettings->value("Common/Installed", 0).toInt() == 0)
    {
        globalSettings->setValue("Common/Installed", 1);
        installOnLinux();
    }
#endif

    // figure out auto login or manually login
    bool bFallback = true;

    // FIXME: move to WizService initialize
    WizToken token;


    bool bAutoLogin = userSettings.autoLogin();
    strPassword = userSettings.password();

    if (bAutoLogin && !strPassword.isEmpty()) {
        bFallback = false;
    }    

    //
    //set network proxy
    WizSettings wizSettings(Utils::WizPathResolve::globalSettingsFile());
    if (wizSettings.getProxyStatus())
    {
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(wizSettings.getProxyHost());
        proxy.setPort(wizSettings.getProxyPort());
        proxy.setUser(wizSettings.getProxyUserName());
        proxy.setPassword(wizSettings.getProxyPassword());
        QNetworkProxy::setApplicationProxy(proxy);
    }

    //
    QString strUserId = WizGetLocalUserId(localUsers, strUserGuid);
    bool isNewRegisterAccount = false;
    // manually login
    if (bFallback)
    {
        WizLoginDialog loginDialog(strLocale, localUsers);
        if (QDialog::Accepted != loginDialog.exec())
            return 0;

//        qDebug() << "deafult user id : " << strUserGuid << " login dailog user id : " << loginDialog.loginUserGuid();
        if (strUserId.isEmpty() || loginDialog.loginUserGuid() != strUserGuid)
        {
            strAccountFolderName = WizGetLocalFolderName(localUsers, loginDialog.loginUserGuid());
            if (strAccountFolderName.isEmpty())
            {
                strAccountFolderName = loginDialog.userId();
            }
            qDebug() << "login user id : " << loginDialog.userId();
            settings = new QSettings(Utils::WizPathResolve::userSettingsFile(strAccountFolderName), QSettings::IniFormat);
            WizGlobal::setSettings(settings);
        }
        strPassword = loginDialog.password();
        strUserId = loginDialog.userId();
        isNewRegisterAccount = loginDialog.isNewRegisterAccount();
    }
    else
    {
        if (userSettings.serverType() == EnterpriseServer)
        {
            WizCommonApiEntry::setEnterpriseServerIP(userSettings.enterpriseServerIP());
        }
        else if (userSettings.serverType() == WizServer ||
                 (userSettings.serverType() == NoServer && !userSettings.myWizMail().isEmpty()))
        {
            WizCommonApiEntry::setEnterpriseServerIP(WIZNOTE_API_SERVER);
        }
    }

    //
    //
    // reset locale for current user.
    userSettings.setAccountFolderName(strAccountFolderName);
    userSettings.setUserId(strUserId);
    strLocale = userSettings.locale();

    a.removeTranslator(&translatorWizNote);
    strLocaleFile = Utils::WizPathResolve::localeFileName(strLocale);
    translatorWizNote.load(strLocaleFile);
    a.installTranslator(&translatorWizNote);

    a.removeTranslator(&translatorQt);
    strLocaleFile = Utils::WizPathResolve::qtLocaleFileName(strLocale);
    translatorQt.load(strLocaleFile);
    a.installTranslator(&translatorQt);

    WizCommonApiEntry::setLanguage(strLocale);

    WizDatabaseManager dbMgr(strAccountFolderName);
    if (!dbMgr.openAll()) {
        QMessageBox::critical(NULL, "", QObject::tr("Can not open database"));
        return 0;
    }


    qDebug() << "set user id for token ; " << strUserId;
    WizToken::setUserId(strUserId);
    WizToken::setPasswd(strPassword);

    dbMgr.db().setPassword(::WizEncryptPassword(strPassword));
    dbMgr.db().updateInvalidData();

    // FIXME: move to plugins
    WizAvatarHost avatarHost;

    // FIXME: move to core plugin initialize
    WizThumbCache cache;

    WizMainWindow w(dbMgr);
#ifdef Q_OS_LINUX
    QObject::connect(&a, SIGNAL(messageAvailable(QString)), &w,
                     SLOT(on_application_messageAvailable(QString)));
#endif

    //settings->setValue("Users/DefaultUser", strUserId);

    w.show();
    w.init();

#ifdef Q_OS_MAC
    //start and set safari extension
    WIZUSERINFO userInfo;
    dbMgr.db().getUserInfo(userInfo);
    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [strUserId, userInfo](){
        updateShareExtensionAccount(strUserId, userInfo.strUserGUID, userInfo.strMywizEmail ,userInfo.strDisplayName);
        //readShareExtensionAccount();
    });
#endif

    //create introduction note for new register users
    WizNoteManager noteManager(dbMgr);
    noteManager.updateTemplateJS(userSettings.locale());
    noteManager.downloadTemplatePurchaseRecord();
    if (isNewRegisterAccount)
    {
        noteManager.createIntroductionNoteForNewRegisterAccount();
    }

    int ret = a.exec();
    if (w.isLogout()) {
        userSettings.setPassword("");
#ifndef BUILD4APPSTORE
        QProcess::startDetached(argv[0], QStringList());
#else
        QString strAppFile = QApplication::applicationDirPath().remove("/Contents/MacOS");
        if (!QProcess::startDetached("/usr/bin/open -W "+strAppFile))
        {
            QMessageBox::information(0, "Info", "open " + strAppFile + " failed");
        }
#endif
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int ret = mainCore(argc, argv);

    //WizQueuedThreadsShutdown();
    // clean up
    QString strTempPath = Utils::WizPathResolve::tempPath();
    ::WizDeleteAllFilesInFolder(strTempPath);

    return ret;
}

