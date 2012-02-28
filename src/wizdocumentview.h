#ifndef WIZDOCUMENTVIEW_H
#define WIZDOCUMENTVIEW_H

#ifndef QWIDGET_H
#include <QWidget>
#endif

#ifndef WIZDEF_H
#include "wizdef.h"
#endif

#ifndef WIZNOTESETTINGS_H
#include "wiznotesettings.h"
#endif

class CWizTitleBar;
class CWizDocumentWebView;
class CWizDatabase;
class CWizTagListWidget;
class CWizAttachmentListWidget;
//


class CWizDocumentView
    : public QWidget
{
    Q_OBJECT;
public:
    CWizDocumentView(CWizExplorerApp& app, QWidget* parent = 0);
protected:
    CWizDatabase& m_db;
    CWizTitleBar* m_title;
    CWizDocumentWebView* m_web;
    QWidget* m_client;
    CWizTagListWidget* m_tags;
    CWizAttachmentListWidget* m_attachments;
    //
    QWidget* createClient();
    //
    bool m_editingDocument;
    WizDocumentViewMode m_viewMode;
public:
    bool viewDocument(const WIZDOCUMENTDATA& data, bool forceEdit);
    //
    void showClient(bool visible);
    //
    const WIZDOCUMENTDATA& document();
    //
    void editDocument(bool editing);
    bool isEditingDocument() const { return m_editingDocument; }
    //
    void setViewMode(WizDocumentViewMode mode);
    void setModified(bool modified);
    //
    void settingsChanged();
    //
public slots:
    void on_titleEdit_textEdited ( const QString & text );
    void on_editDocumentButton_clicked();
    void on_tagsButton_clicked();
    void on_attachmentButton_clicked();
    void on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attachment);
    void on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew);
};

#endif // WIZDOCUMENTVIEW_H
