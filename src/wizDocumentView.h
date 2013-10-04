#ifndef WIZDOCUMENTVIEW_H
#define WIZDOCUMENTVIEW_H

#include <QWidget>
#include <QPointer>
#include <QTimer>

#include "wizdef.h"
#include "share/wizobject.h"
#include "share/wizsettings.h"
#include "share/wizuihelper.h"

class CWizScrollBar;
class CWizDocumentWebView;
class CWizDatabase;
class CWizTagListWidget;
class CWizAttachmentListWidget;
class CWizNoteInfoForm;
class CWizEditorToolBar;

class QScrollArea;
class QLineEdit;

class CWizTitleBar;
class CWizInfoToolBar;
class CWizNotifyToolbar;


class CWizDocumentView : public QWidget
{
    Q_OBJECT

public:
    CWizDocumentView(CWizExplorerApp& app, QWidget* parent = 0);
    virtual QSize sizeHint() const { return QSize(200, 1); }

    QWidget* client() const { return m_client; }
    CWizDocumentWebView* web() const { return m_web; }

protected:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
    CWizUserSettings& m_userSettings;
    CWizDocumentWebView* m_web;
    QPointer<CWizTitleBar> m_title;
    QPointer<QWidget> m_client;
    QPointer<CWizTagListWidget> m_tags;
    QPointer<CWizAttachmentListWidget> m_attachments;
    QPointer<CWizNoteInfoForm> m_info;

    // indicate current document editing status
    bool m_editingDocument;

    WizDocumentViewMode m_viewMode;

public:
    bool viewDocument(const WIZDOCUMENTDATA& data, bool forceEdit);
    void reloadDocument(bool bIncludeData);
    void setReadOnly(bool b, bool isGroup);
    void showClient(bool visible);
    void editDocument(bool editing);
    void setViewMode(WizDocumentViewMode mode);
    void setModified(bool modified);
    void settingsChanged();

public Q_SLOTS:
    void on_titleEdit_editingFinished();
    void on_titleEdit_returnPressed();

    void on_title_editButtonClicked();
    void on_title_tagButtonClicked();
    void on_title_attachButtonClicked();
    void on_title_infoButtonClicked();

    void on_document_modified(const WIZDOCUMENTDATA& documentOld,
                              const WIZDOCUMENTDATA& documentNew);
    void on_document_data_modified(const WIZDOCUMENTDATA& data);

    void on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attachment);

    void on_webview_focusIn();
    void on_webview_focusOut();
};

#endif // WIZDOCUMENTVIEW_H
