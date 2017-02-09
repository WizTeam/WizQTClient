#ifndef CORE_ICORE_H
#define CORE_ICORE_H

#include <QObject>


struct WIZDOCUMENTDATAEX;
class QSettings;

class WizDocumentView;
class WizMainWindow;


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
    static void setMainWindow(WizMainWindow* mw);
    static WizMainWindow* mainWindow();

    static void emitViewNoteRequested(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing);
    static void emitViewNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool bOk);
    static void emitCloseNoteRequested(WizDocumentView* view);
    static void emitMarkdownSettingChanged();
    //
    static void setSettings(QSettings *settings);
    static QSettings *settings();
    static void setGlobalSettings(QSettings *settings);
    static QSettings *globalSettings();

private:
    //
    friend class WizMainWindow;
Q_SIGNALS:
    void viewNoteRequested(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing);
    void viewNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool bOk);
    void closeNoteRequested(WizDocumentView* view);
    void markdownSettingChanged();
};


#endif // CORE_ICORE_H
