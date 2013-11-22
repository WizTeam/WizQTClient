#include "coreplugin.h"

#include <QtPlugin>
#include <QDebug>
#include <QDateTime>

//#include <coreplugin/actionmanager/actionmanager.h>
//#include <coreplugin/editormanager/editormanager.h>

//#include "wizmainwindow.h"

using namespace Core;
using namespace Core::Internal;

CorePlugin::CorePlugin()// : m_editMode(0), m_designMode(0)
{
    //qRegisterMetaType<Core::Id>();
    //m_mainWindow = new MainWindow;
}

CorePlugin::~CorePlugin()
{
    //if (m_editMode) {
    //    removeObject(m_editMode);
    //    delete m_editMode;
    //}

    //if (m_designMode) {
    //    if (m_designMode->designModeIsRequired())
    //        removeObject(m_designMode);
    //    delete m_designMode;
    //}

    //delete m_mainWindow;
}

//void CorePlugin::parseArguments(const QStringList &arguments)
//{
//    for (int i = 0; i < arguments.size(); ++i) {
//        if (arguments.at(i) == QLatin1String("-color")) {
//            const QString colorcode(arguments.at(i + 1));
//            m_mainWindow->setOverrideColor(QColor(colorcode));
//            i++; // skip the argument
//        }
//        if (arguments.at(i) == QLatin1String("-presentationMode"))
//            ActionManager::setPresentationModeEnabled(true);
//    }
//}

bool CorePlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    //qsrand(QDateTime::currentDateTime().toTime_t());
    //parseArguments(arguments);
    //const bool success = m_mainWindow->init(errorMessage);
    //if (success) {
    //    m_editMode = new EditMode;
    //    addObject(m_editMode);
    //    ModeManager::activateMode(m_editMode->id());
    //    m_designMode = new DesignMode;
    //    InfoBar::initializeGloballySuppressed();
    //}

    //// Make sure we respect the process's umask when creating new files
    //Utils::SaveFile::initializeUmask();

    return true;
}

void CorePlugin::extensionsInitialized()
{
    //MimeDatabase::syncUserModifiedMimeTypes();
    //if (m_designMode->designModeIsRequired())
    //    addObject(m_designMode);
    //m_mainWindow->extensionsInitialized();
}

bool CorePlugin::delayedInitialize()
{
    //HelpManager::setupHelpManager();
    return true;
}

//QObject *CorePlugin::remoteCommand(const QStringList & /* options */, const QStringList &args)
//{
//    IDocument *res = m_mainWindow->openFiles(
//                args, ICore::OpenFilesFlags(ICore::SwitchMode | ICore::CanContainLineNumbers));
//    m_mainWindow->raiseWindow();
//    return res;
//}

//void CorePlugin::fileOpenRequest(const QString &f)
//{
//    remoteCommand(QStringList(), QStringList(f));
//}

//ExtensionSystem::IPlugin::ShutdownFlag CorePlugin::aboutToShutdown()
//{
//    m_mainWindow->aboutToShutdown();
//    return SynchronousShutdown;
//}

Q_EXPORT_PLUGIN(CorePlugin)
