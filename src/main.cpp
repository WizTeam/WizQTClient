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

#include <sys/stat.h>

#include <extensionsystem/pluginmanager.h>
#include "utils/pathresolve.h"
#include "utils/logger.h"
#include "utils/stylehelper.h"
#include "share/wizsettings.h"
#include "share/wizwin32helper.h"
#include "share/wizDatabaseManager.h"
#include "share/wizSingleApplication.h"
#include "core/wizNoteManager.h"

#ifdef Q_OS_MAC
#include "mac/wizmachelper.h"
#include "mac/wizIAPHelper.h"
#endif

#include "sync/token.h"
#include "sync/apientry.h"
#include "sync/avatar.h"
#include "thumbcache.h"
#include "wizmainwindow.h"
#include "wizDocumentWebEngine.h"
#include "wizLoginDialog.h"

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
    #ifdef XCODEBUILD
    pluginPath += QLatin1String("/PlugIns/Debug");
    #else
    pluginPath += QLatin1String("/PlugIns");
    #endif
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

static inline QStringList getPluginSpecPaths()
{
#ifdef Q_OS_MAC
    QStringList rc;
    // Figure out root:  Up one from 'bin'
    QDir rootDir = QApplication::applicationDirPath();
    rootDir.cdUp();
    const QString rootDirPath = rootDir.canonicalPath();
    QString pluginSpecPath = rootDirPath;
    pluginSpecPath += QLatin1String("/Resources/plugIns");
    rc.push_back(pluginSpecPath);
    return rc;
#endif
    return getPluginPaths();
}

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


void installOnLinux()
{
    QString appPath = Utils::PathResolve::appPath();
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

int mainCore(int argc, char *argv[])
{
#ifdef BUILD4APPSTORE
   QDir dir(argv[0]);  // e.g. appdir/Contents/MacOS/appname
   dir.cdUp();
   dir.cdUp();
   dir.cd("PlugIns");
   QCoreApplication::setLibraryPaths(QStringList(dir.absolutePath()));
   printf("after change, libraryPaths=(%s)\n", QCoreApplication::libraryPaths().join(",").toUtf8().data());
#endif


#ifdef Q_OS_LINUX
   // create single application for linux
    CWizSingleApplication a(argc, argv, "Special-Message-for-WizNote-SingleApplication");
    if (a.isRunning())
    {
        a.sendMessage(WIZ_SINGLE_APPLICATION);
        return 0;
    }
#else
    QApplication a(argc, argv);

#ifdef BUILD4APPSTORE
    CWizIAPHelper helper;
    helper.validteReceiptOnLauch();
#endif
#endif


#if QT_VERSION > 0x050000
   qInstallMessageHandler(Utils::Logger::messageHandler);
   QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

#else
    qInstallMsgHandler(Utils::Logger::messageHandler);
#endif

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
    QString sysLocalFile = Utils::PathResolve::localeFileName(sysLocal);
    translatorSys.load(sysLocalFile);
    a.installTranslator(&translatorSys);

    initCrashReporter();

    a.removeTranslator(&translatorSys);

    //FIXME: 在Mac osx安全更新之后存在ssl握手问题，此处进行特殊处理
    QSslConfiguration conf = QSslConfiguration::defaultConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(conf);
#endif

    a.setStyleSheet("QToolTip { \
                    font: 12px; \
                    color:#000000; \
                    padding:0px 1px; \
                    background-color: #F8F8F8; \
                    border:0px;}");

    // setup settings
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings* globalSettings = new QSettings(Utils::PathResolve::globalSettingsFile(), QSettings::IniFormat);

//#ifdef Q_OS_WIN
//    QString strDefaultFontName = settings.GetString("Common", "DefaultFont", "");
//    QFont f = WizCreateWindowsUIFont(a, strDefaultFontName);
//    a.setFont(f);
//#endif

    // setup plugin manager
    PluginManager pluginManager;
    PluginManager::setFileExtension(QLatin1String("pluginspec"));
    PluginManager::setGlobalSettings(globalSettings);

    const QStringList specPaths = getPluginSpecPaths();
    const QStringList pluginPaths = getPluginPaths();
    PluginManager::setPluginPaths(specPaths, pluginPaths);

    // use 3 times(30M) of Qt default usage
    int nCacheSize = globalSettings->value("Common/Cache", 10240*3).toInt();
    QPixmapCache::setCacheLimit(nCacheSize);

    QString strUserGuid = globalSettings->value("Users/DefaultUserGuid").toString();
    QList<WizLocalUser> localUsers;
    WizGetLocalUsers(localUsers);    
    QString strAccountFolderName = WizGetLocalFolderName(localUsers, strUserGuid);

    QString strPassword;
    CWizUserSettings userSettings(strAccountFolderName);

    // setup locale for welcome dialog
    QString strLocale = userSettings.locale();
    QLocale::setDefault(strLocale);

    QTranslator translatorWizNote;
    QString strLocaleFile = Utils::PathResolve::localeFileName(strLocale);
    translatorWizNote.load(strLocaleFile);
    a.installTranslator(&translatorWizNote);

    QTranslator translatorQt;
    strLocaleFile = Utils::PathResolve::qtLocaleFileName(strLocale);
    translatorQt.load(strLocaleFile);
    a.installTranslator(&translatorQt);

#ifndef Q_OS_MAC
    if (globalSettings->value("Common/Installed", 0).toInt() == 0)
    {
        globalSettings->setValue("Common/Installed", 1);
        installOnLinux();
    }
#endif

    // figure out auto login or manually login
    bool bFallback = true;

    // FIXME: move to WizService plugin initialize
    WizService::Token token;


    bool bAutoLogin = userSettings.autoLogin();
    strPassword = userSettings.password();

    if (bAutoLogin && !strPassword.isEmpty()) {
        bFallback = false;
    }    

    //
    QSettings* settings = new QSettings(Utils::PathResolve::userSettingsFile(strAccountFolderName), QSettings::IniFormat);
    PluginManager::setSettings(settings);
    //set network proxy
    CWizSettings wizSettings(Utils::PathResolve::globalSettingsFile());
    if (wizSettings.GetProxyStatus())
    {
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(wizSettings.GetProxyHost());
        proxy.setPort(wizSettings.GetProxyPort());
        proxy.setUser(wizSettings.GetProxyUserName());
        proxy.setPassword(wizSettings.GetProxyPassword());
        QNetworkProxy::setApplicationProxy(proxy);
    }

    //
    QString strUserId = WizGetLocalUserId(localUsers, strUserGuid);
    bool isNewRegisterAccount = false;
    // manually login
    if (bFallback)
    {
        CWizLoginDialog loginDialog(strLocale, localUsers);
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
            settings = new QSettings(Utils::PathResolve::userSettingsFile(strAccountFolderName), QSettings::IniFormat);
            PluginManager::setSettings(settings);
        }
        strPassword = loginDialog.password();
        strUserId = loginDialog.userId();
        isNewRegisterAccount = loginDialog.isNewRegisterAccount();
    }
    else
    {
        if (userSettings.serverType() == EnterpriseServer)
        {
            WizService::CommonApiEntry::setEnterpriseServerIP(userSettings.enterpriseServerIP());
        }
        else if (userSettings.serverType() == WizServer ||
                 (userSettings.serverType() == NoServer && !userSettings.myWizMail().isEmpty()))
        {
            WizService::CommonApiEntry::setEnterpriseServerIP(WIZNOTE_API_SERVER);
        }
    }

    //
    //
    // reset locale for current user.
    userSettings.setAccountFolderName(strAccountFolderName);
    userSettings.setUserId(strUserId);
    strLocale = userSettings.locale();

    a.removeTranslator(&translatorWizNote);
    strLocaleFile = Utils::PathResolve::localeFileName(strLocale);
    translatorWizNote.load(strLocaleFile);
    a.installTranslator(&translatorWizNote);

    a.removeTranslator(&translatorQt);
    strLocaleFile = Utils::PathResolve::qtLocaleFileName(strLocale);
    translatorQt.load(strLocaleFile);
    a.installTranslator(&translatorQt);

    WizService::CommonApiEntry::setLanguage(strLocale);

    CWizDatabaseManager dbMgr(strAccountFolderName);
    if (!dbMgr.openAll()) {
        QMessageBox::critical(NULL, "", QObject::tr("Can not open database"));
        return 0;
    }


    qDebug() << "set user id for token ; " << strUserId;
    WizService::Token::setUserId(strUserId);
    WizService::Token::setPasswd(strPassword);

    dbMgr.db().SetPassword(::WizEncryptPassword(strPassword));
    dbMgr.db().UpdateInvalidData();

    // FIXME: move to plugins
    WizService::AvatarHost avatarHost;

    // FIXME: move to core plugin initialize
    Core::ThumbCache cache;



    MainWindow w(dbMgr);
#ifdef Q_OS_LINUX
    QObject::connect(&a, SIGNAL(messageAvailable(QString)), &w,
                     SLOT(on_application_messageAvailable(QString)));
#endif

    //settings->setValue("Users/DefaultUser", strUserId);
    PluginManager::loadPlugins();

    w.show();
    w.init();   

    //
    CWizNoteManager::createSingleton(w);

    if (isNewRegisterAccount)
    {
        CWizNoteManager* noteManager = CWizNoteManager::instance();
        noteManager->createIntroductionNoteForNewRegisterAccount();
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

    // clean up
    QString strTempPath = Utils::PathResolve::tempPath();
    ::WizDeleteAllFilesInFolder(strTempPath);

    return ret;
}


//#include <QPainter>
//#include <QPaintEvent>
//class TransWindow : public QMainWindow
//{
//public:
//    TransWindow(QWidget* parent = 0) : QMainWindow(parent)
//    {
//    }

//protected:
//    void paintEvent(QPaintEvent*)
//    {
//        QPainter p(this);
//        p.setCompositionMode( QPainter::CompositionMode_Clear );
//        p.fillRect(rect(), Qt::SolidPattern );

//    //    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
////        p.fillRect( 10, 10, 300, 300, QColor(255, 255, 255, m_alpha));
//    }
//};

//class TransWidget : public QWidget
//{
//public:
//    TransWidget(QWidget* parent = 0) : QWidget(parent)
//    {
//    }

//protected:
//    void paintEvent(QPaintEvent*)
//    {
//        QPainter p(this);
//        p.setCompositionMode( QPainter::CompositionMode_Clear );
//        p.fillRect(rect(), Qt::SolidPattern );

//    //    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
////        p.fillRect( 10, 10, 300, 300, QColor(255, 255, 255, m_alpha));
//    }
//};


//// test
//#include <QLabel>
//#include <QVBoxLayout>
//#include <QToolBar>
//int main(int argc, char *argv[])
//{
//    QApplication a(argc, argv);

//    TransWindow window;
//    window.setGeometry(500, 500, 500, 300);

//    window.setAutoFillBackground(false);
//    window.setAttribute(Qt::WA_TranslucentBackground, true);
////    window.setUnifiedTitleAndToolBarOnMac(true);


//    QLabel* label = new QLabel(&window);
//    label->setText("Hello World");
//    QToolBar* toolBar = new QToolBar(&window);
//    window.addToolBar(toolBar);
//    QWidget* wgt = new QWidget(&window);
//    QVBoxLayout* layout = new QVBoxLayout(wgt);
//    wgt->setLayout(layout);
//    layout->setContentsMargins(0, 0, 0, 0);
//    layout->addWidget(label);
//    TransWidget* transWgt = new TransWidget(&window);
//    transWgt->setMinimumHeight(50);
//    layout->addWidget(transWgt);
//    QTreeWidget* tree = new QTreeWidget(wgt);
//    tree->setStyleSheet("background-color:transparent;");
//    tree->setAutoFillBackground(true);
//    layout->addWidget(tree);
//    window.setCentralWidget(wgt);

//    window.show();

//    enableBehindBlurOnOSX10_10(wgt);

//    return a.exec();
//}

