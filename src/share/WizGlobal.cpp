#include "WizGlobal.h"
#include <QSettings>

static WizMainWindow* g_mainWindow = NULL;
static QSettings* g_settings = NULL;
static QSettings* g_globalSettings = NULL;


WizGlobal::WizGlobal()
{
}

WizGlobal::~WizGlobal()
{
    g_mainWindow = NULL;
    g_settings = NULL;
    g_globalSettings = NULL;
}

WizGlobal* WizGlobal::instance()
{
    static WizGlobal instance;
    return &instance;
}

void WizGlobal::setMainWindow(WizMainWindow* mw)
{
    g_mainWindow = mw;
}

WizMainWindow* WizGlobal::mainWindow()
{
    return g_mainWindow;
}

void WizGlobal::setSettings(QSettings *settings)
{
    if (g_settings)
    {
        delete g_settings;
        g_settings = NULL;
    }
    //
    g_settings = settings;
    g_settings->setParent(WizGlobal::instance());
}

QSettings *WizGlobal::settings()
{
    return g_settings;
}

void WizGlobal::setGlobalSettings(QSettings *settings)
{
    if (g_globalSettings)
    {
        delete g_globalSettings;
        g_globalSettings = NULL;
    }
    //
    g_globalSettings = settings;
    g_globalSettings->setParent(WizGlobal::instance());
}

QSettings *WizGlobal::globalSettings()
{
    return g_globalSettings;
}


void WizGlobal::emitViewNoteRequested(WizDocumentView* view,
                                  const WIZDOCUMENTDATAEX& doc, bool forceEditing)
{
    Q_EMIT instance()->viewNoteRequested(view, doc, forceEditing);
}

void WizGlobal::emitViewNoteLoaded(WizDocumentView* view,
                               const WIZDOCUMENTDATAEX& doc, bool bOk)
{
    Q_EMIT instance()->viewNoteLoaded(view, doc, bOk);
}

void WizGlobal::emitCloseNoteRequested(WizDocumentView *view)
{
    Q_EMIT instance()->closeNoteRequested(view);
}

void WizGlobal::emitMarkdownSettingChanged()
{
    Q_EMIT instance()->markdownSettingChanged();
}


