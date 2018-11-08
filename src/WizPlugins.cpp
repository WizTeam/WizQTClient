#include "WizPlugins.h"
#include "sync/WizToken.h"
#include "WizMainWindow.h"
#include "share/WizGlobal.h"
#include "utils/WizPathResolve.h"
#include "widgets/WizLocalProgressWebView.h"

#include <QMovie>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QNetworkReply>
#include <QWebEngineView>
#include "share/WizSettings.h"
#include "share/WizWebEngineView.h"
#include "share/WizMisc.h"

WizPluginData::WizPluginData(QString path, QObject* parent)
    : QObject(parent)
    , m_path(path)
{
    WizPathAddBackslash(m_path);
    QString fileName = path + "plugin.ini";
    QString section = "Common";
    //
    WizSettings plugin(fileName);
    m_name = plugin.getString(section, "AppName");
    m_type = plugin.getString(section, "Type");
    m_appGuid = plugin.getString(section, "AppGUID");
    m_scriptFileName = m_path + "index.html";
    m_icon = WizLoadSkinIcon("", m_path + "plugin.svg");
}


WizPluginPopupWidget::WizPluginPopupWidget(WizExplorerApp& app, WizPluginData* data, QWidget* parent)
    : WizPopupWidget(parent)
    , m_data(data)
{
    WizWebEngineViewInjectObjects objects = {
        {"WizPluginData", m_data},
        {"WizExplorerApp", app.object()}
    };
    m_web = new WizWebEngineView(objects, this);
    //
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    layout->addWidget(m_web);
    m_web->load(QUrl::fromLocalFile(m_data->scriptFileName()));
}

WizPlugins::WizPlugins(QString basePath)
{
    init(basePath);
};

WizPlugins::~WizPlugins()
{
    for (auto data : m_data) {
        delete data;
    }
    m_data.clear();
};

void WizPlugins::init(QString basePath)
{
    CWizStdStringArray folders;
    WizEnumFolders(basePath, folders, 0);
    //
    for (auto folder : folders) {
        WizPluginData* data = new WizPluginData(folder, nullptr);
        if (data->scriptFileName().isEmpty()) {
            delete data;
            continue;
        }
        //
        m_data.push_back(data);
    }
}

std::vector<WizPluginData*> WizPlugins::pluginsByType(QString type) const
{
    WizPlugins& plugins = WizPlugins::plugins();
    std::vector<WizPluginData*> ret;
    for (auto data : plugins.m_data) {
        if (data->type() == type) {
            ret.push_back(data);
        }
    }
    return ret;
}

WizPlugins& WizPlugins::plugins()
{
    QString pluginBase = Utils::WizPathResolve::pluginsPath();
    static WizPlugins plugins(pluginBase);
    return plugins;
}
