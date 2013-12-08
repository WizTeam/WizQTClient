#ifndef CORE_TITLEBAR_H
#define CORE_TITLEBAR_H

#include <QWidget>
#include <QIcon>


struct WIZDOCUMENTDATA;
class CWizDatabase;
class CWizTagListWidget;
class CWizNoteInfoForm;
class CWizDocumentWebView;
class CWizAttachmentListWidget;

namespace Core {

namespace Internal {
class TitleEdit;
class InfoBar;
class NotifyBar;
class EditorToolBar;
class CellButton;
class CWizDocumentView;

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent);
    CWizDocumentView* noteView();
    void setLocked(bool bReadOnly, int nReason, bool bIsGroup);
    void setEditor(CWizDocumentWebView* editor);

    void setNote(const WIZDOCUMENTDATA& data, bool editing, bool locked);
    void updateInfo(const WIZDOCUMENTDATA& doc);
    void setEditingDocument(bool editing);
    void updateEditButton(bool bEditing);


private:
    CWizDocumentWebView* m_editor;

    TitleEdit* m_editTitle;
    InfoBar* m_infoBar;
    NotifyBar* m_notifyBar;
    EditorToolBar* m_editorBar;

    CellButton* m_editBtn;
    CellButton* m_tagBtn;
    CellButton* m_attachBtn;
    CellButton* m_infoBtn;

    CWizTagListWidget* m_tags;
    CWizAttachmentListWidget* m_attachments;
    CWizNoteInfoForm* m_info;

public Q_SLOTS:
    void onEditButtonClicked();
    void onTagButtonClicked();
    void onAttachButtonClicked();
    void onInfoButtonClicked();

    void onEditorChanged();
    void onEditorFocusIn();
    void onEditorFocusOut();
};

} //namesapce Internal
} // namespace Core

#endif // CORE_TITLEBAR_H
