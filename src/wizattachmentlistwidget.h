#ifndef WIZATTACHMENTLISTWIDGET_H
#define WIZATTACHMENTLISTWIDGET_H

#include "share/wizpopupwidget.h"
#include "share/wizdatabase.h"
#include "share/wizmultilinelistwidget.h"

#include "share/wizfileiconprovider.h"

class CWizExplorerApp;
class CWizAttachmentListViewItem;

class CWizAttachmentListView : public CWizMultiLineListWidget
{
    Q_OBJECT
public:
    CWizAttachmentListView(CWizExplorerApp& app, QWidget* parent);
private:
    CWizExplorerApp& m_app;
    CWizDatabase& m_db;
    WIZDOCUMENTDATA m_document;
    CWizFileIconProvider m_iconProvider;
    QMenu* m_menu;
private:
    void resetAttachments();
public:
    void setDocument(const WIZDOCUMENTDATA& document);
    const CWizAttachmentListViewItem* attachmentItemFromIndex(const QModelIndex& index) const;
    void addAttachments();
    void openAttachment(CWizAttachmentListViewItem* item);
protected:
    virtual int wrapTextLineIndex() const;
    virtual bool imageAlignLeft() const;
    virtual int imageWidth() const;
    virtual QString itemText(const QModelIndex& index, int line) const;
    virtual QPixmap itemImage(const QModelIndex& index) const;
    //
    virtual void contextMenuEvent(QContextMenuEvent * e);
    //
    friend class CWizAttachmentListViewItem;
public slots:
    void on_action_addAttachment();
    void on_action_saveAttachmentAs();
    void on_action_openAttachment();
    void on_action_deleteAttachment();
    void on_list_itemDoubleClicked(QListWidgetItem* item);
};


class CWizAttachmentListWidget : public CWizPopupWidget
{
    Q_OBJECT
public:
    CWizAttachmentListWidget(CWizExplorerApp& app, QWidget* parent);
private:
    CWizAttachmentListView* m_list;
public:
    void setDocument(const WIZDOCUMENTDATA& document);
public slots:
    void on_addAttachment_clicked();
};

#endif // WIZATTACHMENTLISTWIDGET_H
