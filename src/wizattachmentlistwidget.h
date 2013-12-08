#ifndef WIZATTACHMENTLISTWIDGET_H
#define WIZATTACHMENTLISTWIDGET_H

#include "share/wizobject.h"
#include "share/wizmultilinelistwidget.h"
#include "share/wizpopupwidget.h"
#include "share/wizfileiconprovider.h"

class QMenu;

class CWizDatabaseManager;
class CWizAttachmentListViewItem;
class CWizButton;
class CWizObjectDataDownloaderHost;

class CWizAttachmentListView : public CWizMultiLineListWidget
{
    Q_OBJECT

public:
    CWizAttachmentListView(QWidget* parent);
    const WIZDOCUMENTDATA& document() const { return m_document; }

private:
    CWizDatabaseManager& m_dbMgr;
    WIZDOCUMENTDATA m_document;
    CWizFileIconProvider m_iconProvider;
    QMenu* m_menu;
    CWizObjectDataDownloaderHost* m_downloaderHost;

    void resetAttachments();
    void resetPermission();

public:
    QAction* findAction(const QString& strName);
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

    virtual void contextMenuEvent(QContextMenuEvent * e);

    friend class CWizAttachmentListViewItem;

public Q_SLOTS:
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
    CWizAttachmentListWidget(QWidget* parent);
    void setDocument(const WIZDOCUMENTDATA& document);

private:
    CWizAttachmentListView* m_list;
    CWizButton* m_btnAddAttachment;
    //CWizImagePushButton* m_btnAddAttachment;

public Q_SLOTS:
    void on_addAttachment_clicked();
};

#endif // WIZATTACHMENTLISTWIDGET_H
