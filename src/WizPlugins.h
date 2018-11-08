#ifndef WIZPLUGINS_H
#define WIZPLUGINS_H

#include <QDialog>
#include <QIcon>
#include "share/WizWebEngineView.h"
#include "share/WizPopupWidget.h"

class WizWebEngineView;
class WizExplorerApp;

class WizPluginData : public QObject
{
    Q_OBJECT
public:
    WizPluginData(QString path, QObject* parent);
    //
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString guid READ guid)
    Q_PROPERTY(QString appGuid READ appGuid)
    Q_PROPERTY(QString scriptFileName READ scriptFileName)
public:
    QString path() const { return m_path; }
    QString guid() const { return m_guid; }
    QString appGuid() const { return m_appGuid; }
    QString scriptFileName() const { return m_scriptFileName; }
    QIcon icon() const { return m_icon; }
    QString type() const { return m_type; }
    QString name() const { return m_name; }
private:
    QString m_path;
    QString m_guid;
    QString m_type;
    QString m_appGuid;
    QString m_scriptFileName;
    QIcon m_icon;
    QString m_name;
    //
    friend class WizPluginPopupWidget;
};

class WizPluginPopupWidget : public WizPopupWidget
{
public:
    WizPluginPopupWidget(WizExplorerApp& app, WizPluginData* data, QWidget* parent);
private:
    WizWebEngineView* m_web;
    WizPluginData* m_data;
};


class WizPlugins
{
private:
    WizPlugins(QString basePath);
public:
    ~WizPlugins();
private:
    void init(QString basePath);
private:
    std::vector<WizPluginData*> m_data;
public:
    static WizPlugins& plugins();
    std::vector<WizPluginData*> pluginsByType(QString type) const;
    std::vector<WizPluginData*> pluginsAll() const { return m_data; }
};



#endif // WIZPLUGINS_H
