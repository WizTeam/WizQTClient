#ifndef WIZDOCUMENTLISTVIEW_H
#define WIZDOCUMENTLISTVIEW_H

#include <QListWidget>

#include "wizdef.h"
#include "share/wizobject.h"
#include "share/wizuihelper.h"
#include "wizDocumentListViewItem.h"

class CWizTagListWidget;
class CWizFolderSelector;
class CWizScrollBar;
class CWizUserAvatarDownloaderHost;

#ifdef Q_OS_LINUX
#define WIZNOTE_CUSTOM_SCROLLBAR
#else
//#if QT_VERSION < 0x050000
#define WIZNOTE_CUSTOM_SCROLLBAR
//#endif
#endif


class CWizDocumentListView : public QListWidget
{
    Q_OBJECT

public:
    enum ViewType {
        TypeThumbnail,
        TypeTwoLine,
        TypeOneLine
    };

public:
    explicit CWizDocumentListView(CWizExplorerApp& app, QWidget *parent = 0);
    virtual ~CWizDocumentListView();

    int viewType() const { return m_nViewType; }
    void resetItemsViewType(int type);
    QSize itemSizeFromViewType(CWizDocumentListView::ViewType type);

    int sortingType() const { return m_nSortingType; }
    void resetItemsSortingType(int type);
    bool isSortedByAccessDate();

    //CWizThumbIndexCache* thumbCache() const { return m_thumbCache; }

    void setItemsNeedUpdate(const QString& strKbGUID = 0, const QString& strGUID = 0);
    void drawItem(QPainter*p, const QStyleOptionViewItemV4* vopt) const;
    void reloadItem(const QString& strKbGUID, const QString& strGUID);

    void setAcceptAllSearchItems(bool bAccept);

protected:
    virtual void resizeEvent(QResizeEvent* event);
    //virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);

    virtual void startDrag(Qt::DropActions supportedActions);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent * event);

private:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    CWizScrollBar* m_vScroll;
#endif

    CWizDocumentListView::ViewType m_nViewType;
    int m_nSortingType;

    QMenu* m_menuDocument;
    CWizTagListWidget* m_tagList;

    QPoint m_dragStartPosition;

    QList<CWizDocumentListViewItem*> m_rightButtonFocusedItems;

//#ifndef Q_OS_MAC
    // used for smoothly scroll
    QTimer m_vscrollTimer;
    int m_vscrollOldPos;
    int m_vscrollDelta;
    int m_vscrollCurrent;
//#endif // Q_OS_MAC

    bool m_itemSelectionChanged;
    bool m_accpetAllSearchItems;

    QPointer<QPropertyAnimation> m_scrollAnimation;

    QAction* findAction(const QString& strName);

    void resetPermission();

    // Test documents property
    bool isDocumentsAllCanDelete(const CWizDocumentDataArray& arrayDocument);
    bool isDocumentsWithGroupDocument(const CWizDocumentDataArray& arrayDocument);
    bool isDocumentsWithDeleted(const CWizDocumentDataArray& arrayDocument);
    bool isDocumentsAlwaysOnTop(const CWizDocumentDataArray &arrayDocument);

public:
    void setDocuments(const CWizDocumentDataArray& arrayDocument);
    void appendDocuments(const CWizDocumentDataArray& arrayDocument);

    bool acceptDocument(const WIZDOCUMENTDATA& document);
    void addAndSelectDocument(const WIZDOCUMENTDATA& document);

public:
    void getSelectedDocuments(CWizDocumentDataArray& arrayDocument);

    int documentIndexFromGUID(const QString &strGUID);
    CWizDocumentListViewItem *documentItemAt(int index);

    // drawing passthrought methods
    CWizDocumentListViewItem *documentItemFromIndex(const QModelIndex &index) const;
    const WIZDOCUMENTDATA& documentFromIndex(const QModelIndex &index) const;
    const WizDocumentListViewItemData& documentItemDataFromIndex(const QModelIndex& index) const;

//#ifndef Q_OS_MAC
    // used for smoothly scroll
    void vscrollBeginUpdate(int delta);
    //virtual void updateGeometries();
    virtual void wheelEvent(QWheelEvent* event);
//#endif // Q_OS_MAC

public Q_SLOTS:
    void on_itemSelectionChanged();

    void on_tag_created(const WIZTAGDATA& tag);
    void on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    void on_document_created(const WIZDOCUMENTDATA& document);
    void on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew);
    void on_document_deleted(const WIZDOCUMENTDATA& document);
    void on_documentReadCount_changed(const WIZDOCUMENTDATA& document);

    // message related signals
    //void on_message_created(const WIZMESSAGEDATA& data);
    //void on_message_modified(const WIZMESSAGEDATA& oldMsg,
    //                         const WIZMESSAGEDATA& newMsg);
    //void on_message_deleted(const WIZMESSAGEDATA& data);

    // message context menu
    //void on_action_message_mark_read();
    //void on_action_message_delete();

    // document context menu
    void on_action_selectTags();
    void on_action_deleteDocument();
    void on_action_encryptDocument();
    void on_action_cancelEncryption();
    void on_action_alwaysOnTop();
    void on_action_addToShortcuts();

    void on_action_moveDocument();
    void on_action_moveDocument_confirmed(int result);

    void on_action_copyDocument();
    void on_action_copyDocument_confirmed(int result);

    void on_action_showDocumentInFloatWindow();
    void on_action_copyDocumentLink();
    void on_action_documentHistory();
    void on_action_shareDocumentByLink();

    void on_menu_aboutToHide();

    void on_document_abstractLoaded(const WIZABSTRACT& abs);
    void on_userAvatar_loaded(const QString& strUserGUID);
    void onThumbCacheLoaded(const QString& strKbGUID, const QString& strGUID);


//#ifndef Q_OS_MAC
    // used for smoothly scroll
    void on_vscroll_valueChanged(int value);
    void on_vscroll_actionTriggered(int action);
    void on_vscroll_update();
    void on_vscrollAnimation_valueChanged(const QVariant& value);
    void on_vscrollAnimation_finished();
//#endif // Q_OS_MAC

Q_SIGNALS:
    void documentCountChanged();
    void lastDocumentDeleted();
    void documentsSelectionChanged();
    void shareDocumentByLinkRequest(const QString& strKbGUID, const QString& strGUID);
    void changeUploadRequest(const QString& strKbGUID);
    void addDocumentToShortcutsRequest(const WIZDOCUMENTDATA& doc);

private:
    int numOfEncryptedDocuments(const CWizDocumentDataArray& docArray);
    void setEncryptDocumentActionEnable(bool enable);
    //    
    int addDocument(const WIZDOCUMENTDATA& data, bool sort);
    void addDocument(const WIZDOCUMENTDATA &doc);

    bool acceptDocumentChange(const WIZDOCUMENTDATA &document);

    //
    void moveDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder);
    void moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag);
    void copyDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder,
                                      bool keepDocTime, bool keepTag);
    void copyDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag,
                                    bool keepDocTime);

    //
    void duplicateDocuments(const CWizDocumentDataArray& arrayDocument);
};


#endif // WIZDOCUMENTLISTVIEW_H
