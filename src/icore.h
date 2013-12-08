#ifndef CORE_ICORE_H
#define CORE_ICORE_H

#include <QObject>

struct WIZDOCUMENTDATA;

namespace Core {

namespace Internal {
class MainWindow;
class CWizDocumentView;
}

class ICore : public QObject
{
    Q_OBJECT

    friend class Internal::MainWindow;
    explicit ICore(Internal::MainWindow* mw);
    ~ICore();

public:
    static ICore* instance();
    static QWidget* mainWindow();

    static void emitViewNoteRequested(Internal::CWizDocumentView* view, const WIZDOCUMENTDATA& doc);
    static void emitViewNoteLoaded(Internal::CWizDocumentView* view, const WIZDOCUMENTDATA& doc);
    static void emitViewNoteModeChanged(Internal::CWizDocumentView* view, bool bLocked);

Q_SIGNALS:
    void viewNoteRequested(Internal::CWizDocumentView* view, const WIZDOCUMENTDATA& doc);
    void viewNoteLoaded(Internal::CWizDocumentView* view, const WIZDOCUMENTDATA& doc);
    void viewNoteModeChanged(Internal::CWizDocumentView* view, bool bLocked);

    void closeNoteRequested(Internal::CWizDocumentView* view);
};

} // namespace Core

#endif // CORE_ICORE_H
