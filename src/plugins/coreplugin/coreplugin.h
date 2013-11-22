#ifndef COREPLUGIN_H
#define COREPLUGIN_H

#include <extensionsystem/iplugin.h>

namespace Core {
namespace Internal {

class MainWindow;

class CorePlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "wiz.plugin.system.internal.coreplugin")

public:
    CorePlugin();
    ~CorePlugin();

    bool initialize(const QStringList &arguments, QString *errorMessage = 0);
    void extensionsInitialized();
    bool delayedInitialize();
    //ShutdownFlag aboutToShutdown();
    //QObject *remoteCommand(const QStringList & /* options */, const QStringList &args);

//public slots:
//    void fileOpenRequest(const QString&);

//private:
//    void parseArguments(const QStringList & arguments);

    //MainWindow *m_mainWindow;
};

} // namespace Internal
} // namespace Core

#endif // COREPLUGIN_H
