#ifndef WIZDOCUMENTVIEW_H
#define WIZDOCUMENTVIEW_H

#include <QWidget>
#include <QWebView>

#include "wizdef.h"

class CWizTitleBar;
class CWizDocumentWebView;
class CWizDatabase;
class CWizTagListWidget;
class CWizAttachmentListWidget;

class CWizDocumentView : public QWidget
{
    Q_OBJECT

public:
    CWizDocumentView(CWizExplorerApp& app, QWidget* parent = 0);
    virtual QSize sizeHint() const { return QSize(800,700); }

    CWizDocumentWebView* view() const { return m_web; }
    QWidget* client() const { return m_client; }

private:
    CWizExplorerApp& m_app;
    CWizUserSettings& m_userSettings;

protected:
    CWizDatabase& m_db;
    CWizTitleBar* m_title;
    CWizDocumentWebView* m_web;
    QWidget* m_client;
    CWizTagListWidget* m_tags;
    CWizAttachmentListWidget* m_attachments;

    QWidget* createClient();

    bool m_editingDocument;
    WizDocumentViewMode m_viewMode;

public:
    bool viewDocument(const WIZDOCUMENTDATA& data, bool forceEdit);
    void showClient(bool visible);
    const WIZDOCUMENTDATA& document();
    void editDocument(bool editing);
    bool isEditingDocument() const { return m_editingDocument; }
    void setViewMode(WizDocumentViewMode mode);
    void setModified(bool modified);
    void settingsChanged();

public Q_SLOTS:
    void on_titleEdit_editingFinished();
    void on_editDocumentButton_clicked();
    void on_tagsButton_clicked();
    void on_attachmentButton_clicked();
    void on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew);

};

#endif // WIZDOCUMENTVIEW_H
