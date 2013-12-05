#ifndef CORE_ICORE_H
#define CORE_ICORE_H

#include <QObject>

struct WIZDOCUMENTDATA;
class CWizDocumentView;

namespace Core {

namespace Internal {
class MainWindow;
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
    static void emitViewNoteRequested(CWizDocumentView* view, const WIZDOCUMENTDATA& doc);
    static void emitViewNoteLoaded(CWizDocumentView* view, const WIZDOCUMENTDATA& doc);

Q_SIGNALS:
    void viewNoteRequested(CWizDocumentView* view, const WIZDOCUMENTDATA& doc);
    void viewNoteLoaded(CWizDocumentView* view, const WIZDOCUMENTDATA& doc);
};

} // namespace Core

#endif // CORE_ICORE_H
