#ifndef WIZDOCUMENTVIEW_H
#define WIZDOCUMENTVIEW_H

#include <QWidget>
#include <QWebView>
#include <QPointer>
#include <QTimer>

#include "wizdef.h"
#include "share/wizobject.h"
#include "share/wizsettings.h"


class CWizDocumentWebView;
class CWizDatabase;
class CWizTagListWidget;
class CWizAttachmentListWidget;
class CWizNoteInfoForm;
class CWizEditorToolBar;

class CWizTitleBar;
class CWizInfoToolBar;

class CWizDocumentView : public QWidget
{
    Q_OBJECT

public:
    CWizDocumentView(CWizExplorerApp& app, QWidget* parent = 0);
    virtual QSize sizeHint() const { return QSize(200, 1); }

    QWidget* client() const { return m_client; }
    CWizEditorToolBar* editorToolBar() const;
    CWizDocumentWebView* web() const { return m_web; }

private:
    CWizExplorerApp& m_app;
    CWizUserSettings& m_userSettings;

protected:
    CWizDatabaseManager& m_dbMgr;
    QPointer<CWizTitleBar> m_title;
    QPointer<CWizInfoToolBar> m_infoToolBar;
    QPointer<CWizEditorToolBar> m_editorToolBar;
    CWizDocumentWebView* m_web;
    QPointer<QWidget> m_client;
    QPointer<CWizTagListWidget> m_tags;
    QPointer<CWizAttachmentListWidget> m_attachments;
    QPointer<CWizNoteInfoForm> m_info;

    bool m_editingDocument;
    WizDocumentViewMode m_viewMode;

    QTimer m_timerDelay;
    WIZDOCUMENTDATA m_dataDelay;

public:
    bool viewDocument(const WIZDOCUMENTDATA& data, bool forceEdit);
    void setReadOnly(bool b, bool isGroup);
    void showClient(bool visible);
    const WIZDOCUMENTDATA& document();
    void editDocument(bool editing);
    bool isEditingDocument() const { return m_editingDocument; }
    void setViewMode(WizDocumentViewMode mode);
    void setModified(bool modified);
    void settingsChanged();

public Q_SLOTS:
    void on_titleEdit_textChanged(const QString& strTitle);
    void on_titleEdit_textEdit_writeDelay();
    void on_editDocumentButton_clicked();
    void on_tagsButton_clicked();
    void on_attachmentButton_clicked();
    void on_infoButton_clicked();
    void on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew);

    void on_webview_focusIn();
};

#endif // WIZDOCUMENTVIEW_H
