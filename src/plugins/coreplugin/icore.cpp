#include "icore.h"

using namespace Core::Internal;

namespace Core {

static ICore* m_instance = 0;
static MainWindow* m_mainwindow;

ICore::ICore(MainWindow* mw)
{
    m_instance = this;
    m_mainwindow = mw;
}

ICore::~ICore()
{
    m_instance = 0;
    m_mainwindow = 0;
}

ICore* ICore::instance()
{
    return m_instance;
}

MainWindow* ICore::mainWindow()
{
    return m_mainwindow;
}

void ICore::emitViewNoteRequested(INoteView* view,
                                  const WIZDOCUMENTDATA& doc)
{
    Q_EMIT m_instance->viewNoteRequested(view, doc);
}

void ICore::emitViewNoteLoaded(INoteView* view,
                               const WIZDOCUMENTDATA& doc, bool bOk)
{
    Q_EMIT m_instance->viewNoteLoaded(view, doc, bOk);
}

void ICore::emitCloseNoteRequested(INoteView *view)
{
    Q_EMIT m_instance->closeNoteRequested(view);
}

void ICore::emitMarkdownSettingChanged()
{
    Q_EMIT m_instance->markdownSettingChanged();
}


} // namespace Core
