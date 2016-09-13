#ifndef WIZATTACHMENTLISTWIDGET_H
#define WIZATTACHMENTLISTWIDGET_H

#include "share/WizObject.h"
#include "share/WizMultiLineListWidget.h"
#include "share/WizPopupWidget.h"
#include "share/WizFileIconProvider.h"

class QMenu;

class WizDatabaseManager;
class WizAttachmentListViewItem;
class WizButton;
class WizObjectDownloaderHost;
class WizAttachmentListView;

class WizAttachmentListViewItem : public QObject, public QListWidgetItem
{
    Q_OBJECT
public:
    enum LoadState{
        Unkonwn,
        Downloaded,
        Downloading,
        Uploading
    };

    WizAttachmentListViewItem(const WIZDOCUMENTATTACHMENTDATA& att, QListWidget* view);
    const WIZDOCUMENTATTACHMENTDATA& attachment() const { return m_attachment; }

    QString detailText(const WizAttachmentListView* view) const;

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


class WizAttachmentListView : public WizMultiLineListWidget
{
    Q_OBJECT

public:
    WizAttachmentListView(QWidget* parent);
    const WIZDOCUMENTDATA& document() const { return m_document; }


public Q_SLOTS:
    void on_action_addAttachment();
//    void on_action_downloadAttachment();
    void on_action_saveAttachmentAs();
    void on_action_openAttachment();
    void on_action_deleteAttachment();
    void on_action_attachmentHistory();
    void on_list_itemDoubleClicked(QListWidgetItem* item);
    //
    void forceRepaint();
    //
    void resetAttachments();

public:
    QAction* findAction(const QString& strName);
    void setDocument(const WIZDOCUMENTDATA& document);
    const WizAttachmentListViewItem* attachmentItemFromIndex(const QModelIndex& index) const;
    void addAttachments();
    void openAttachment(WizAttachmentListViewItem* item);
    void downloadAttachment(WizAttachmentListViewItem* item);

signals:
    void closeRequest();

protected:
    virtual int wrapTextLineIndex() const;
    virtual bool imageAlignLeft() const;
    virtual int imageWidth() const;
    virtual QString itemText(const QModelIndex& index, int line) const;
    virtual QPixmap itemImage(const QModelIndex& index) const;
    virtual bool itemExtraImage(const QModelIndex& index, const QRect& itemBound, QRect& rcImage, QPixmap& extraPix) const;

    virtual void contextMenuEvent(QContextMenuEvent * e);

    friend class WizAttachmentListViewItem;

private:
    WizDatabaseManager& m_dbMgr;
    WIZDOCUMENTDATA m_document;
    WizFileIconProvider m_iconProvider;
    QMenu* m_menu;
    WizObjectDownloaderHost* m_downloaderHost;

    void resetPermission();
    void startDownload(WizAttachmentListViewItem* item);
    WizAttachmentListViewItem* newAttachmentItem(const WIZDOCUMENTATTACHMENTDATA& att);
    void waitForDownload();

    //
    bool isAttachmentModified(const WIZDOCUMENTATTACHMENTDATAEX& attachment);
    void updateAttachmentInfo(const WIZDOCUMENTATTACHMENTDATAEX& attachment);
};


class WizAttachmentListWidget : public WizPopupWidget
{
    Q_OBJECT

public:
    WizAttachmentListWidget(QWidget* parent);
    bool setDocument(const WIZDOCUMENTDATA& document);

    virtual QSize sizeHint() const;
signals:
    void widgetStatusChanged();

protected:
    void hideEvent(QHideEvent* ev);

private:
    WizAttachmentListView* m_list;
    WizButton* m_btnAddAttachment;
    //CWizImagePushButton* m_btnAddAttachment;

public Q_SLOTS:
    void on_addAttachment_clicked();
    void on_attachList_closeRequest();
};

#endif // WIZATTACHMENTLISTWIDGET_H
