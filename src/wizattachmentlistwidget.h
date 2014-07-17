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
class CWizAttachmentListView;

class CWizAttachmentListViewItem : public QObject, public QListWidgetItem
{
    Q_OBJECT
public:
    enum LoadState{
        Unkonwn,
        Downloaded,
        Downloading,
        Uploading
    };

    CWizAttachmentListViewItem(const WIZDOCUMENTATTACHMENTDATA& att);
    const WIZDOCUMENTATTACHMENTDATA& attachment() const { return m_attachment; }

    QString detailText(const CWizAttachmentListView* view) const;

    bool isDownloading() const;
    void setIsDownloading(bool isDownloading);
    bool isUploading() const;
    void setIsUploading(bool isUploading);
    int loadProgress() const;

public slots:
    void on_downloadFinished(const WIZOBJECTDATA& data, bool bSucceed);
    void on_downloadProgress(QString objectGUID, int totalSize, int loadedSize);

signals:
    void updateRequet();

private:
    WIZDOCUMENTATTACHMENTDATA m_attachment;
    LoadState m_loadState;
    int m_loadProgress;
};


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
    void startDownLoad(CWizAttachmentListViewItem* item);
    CWizAttachmentListViewItem* newAttachmentItem(const WIZDOCUMENTATTACHMENTDATA& att);

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
    virtual bool itemExtraImage(const QModelIndex& index, const QRect& itemBound, QRect& rcImage, QPixmap& extraPix) const;

    virtual void contextMenuEvent(QContextMenuEvent * e);

    friend class CWizAttachmentListViewItem;

public Q_SLOTS:
    void on_action_addAttachment();
    void on_action_saveAttachmentAs();
    void on_action_openAttachment();
    void on_action_deleteAttachment();
    void on_list_itemDoubleClicked(QListWidgetItem* item);
    //
    void forceRepaint();
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
    QString m_currentDocument;

public Q_SLOTS:
    void on_addAttachment_clicked();
};

#endif // WIZATTACHMENTLISTWIDGET_H
