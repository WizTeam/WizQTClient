#ifndef CORE_ICORE_H
#define CORE_ICORE_H

#include <QObject>


struct WIZDOCUMENTDATA;
class QSettings;

class CWizDocumentView;
class MainWindow;


class WizGlobal : public QObject
{
    Q_OBJECT

private:
    explicit WizGlobal();
public:
    ~WizGlobal();

public:
    static WizGlobal* instance();
    //
    static void setMainWindow(MainWindow* mw);
    static MainWindow* mainWindow();

    static void emitViewNoteRequested(CWizDocumentView* view, const WIZDOCUMENTDATA& doc, bool forceEditing);
    static void emitViewNoteLoaded(CWizDocumentView* view, const WIZDOCUMENTDATA& doc, bool bOk);
    static void emitCloseNoteRequested(CWizDocumentView* view);
    static void emitMarkdownSettingChanged();
    //
    static void setSettings(QSettings *settings);
    static QSettings *settings();
    static void setGlobalSettings(QSettings *settings);
    static QSettings *globalSettings();

private:
    //
    friend class MainWindow;
Q_SIGNALS:
    void viewNoteRequested(CWizDocumentView* view, const WIZDOCUMENTDATA& doc, bool forceEditing);
    void viewNoteLoaded(CWizDocumentView* view, const WIZDOCUMENTDATA& doc, bool bOk);
    void closeNoteRequested(CWizDocumentView* view);
    void markdownSettingChanged();
};


#endif // CORE_ICORE_H
