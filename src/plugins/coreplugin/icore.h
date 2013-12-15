#ifndef CORE_ICORE_H
#define CORE_ICORE_H

#include <QObject>

#include "core_global.h"

struct WIZDOCUMENTDATA;

namespace Core {
class CWizDocumentView;

namespace Internal {
class MainWindow;
}

class CORE_EXPORT ICore : public QObject
{
    Q_OBJECT

    friend class Internal::MainWindow;
    explicit ICore(Internal::MainWindow* mw);
    ~ICore();

public:
    static ICore* instance();
    static Internal::MainWindow* mainWindow();

    static void emitViewNoteRequested(CWizDocumentView* view, const WIZDOCUMENTDATA& doc);
    static void emitViewNoteLoaded(CWizDocumentView* view, const WIZDOCUMENTDATA& doc);
    static void emitViewNoteModeChanged(CWizDocumentView* view, bool bLocked);

Q_SIGNALS:
    void viewNoteRequested(Core::CWizDocumentView* view, const WIZDOCUMENTDATA& doc);
    void viewNoteLoaded(Core::CWizDocumentView* view, const WIZDOCUMENTDATA& doc);
    void viewNoteModeChanged(Core::CWizDocumentView* view, bool bLocked);
    void closeNoteRequested(Core::CWizDocumentView* view);
};

} // namespace Core

#endif // CORE_ICORE_H
