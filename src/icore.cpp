#include "icore.h"

#include "wizmainwindow.h"

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

QWidget* ICore::mainWindow()
{
    return m_mainwindow;
}

void ICore::emitViewNoteRequested(CWizDocumentView* view,
                                  const WIZDOCUMENTDATA& doc)
{
    Q_EMIT m_instance->viewNoteRequested(view, doc);
}

void ICore::emitViewNoteLoaded(CWizDocumentView* view,
                               const WIZDOCUMENTDATA& doc)
{
    Q_EMIT m_instance->viewNoteLoaded(view, doc);
}


} // namespace Core
