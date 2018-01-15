#ifndef WIZDOCUMENTLISTVIEW_H
#define WIZDOCUMENTLISTVIEW_H

#include <QListWidget>
#include <memory>

#include "WizDef.h"
#include "share/WizObject.h"
#include "share/WizUIHelper.h"
#include "WizDocumentListViewItem.h"

class WizTagListWidget;
class WizFolderSelector;
class WizScrollBar;
class CWizUserAvatarDownloaderHost;

#define WIZNOTE_CUSTOM_SCROLLBAR


#define DocumentLeadInfo_None                      0x0000
#define DocumentLeadInfo_PersonalRoot         0x0001
#define DocumentLeadInfo_PersonalFolder       0x0002
#define DocumentLeadInfo_PersonalTag           0x0004
#define DocumentLeadInfo_GroupRoot              0x0008
#define DocumentLeadInfo_GroupFolder            0x0010
#define DocumentLeadInfo_SearchResult           0x0020

enum DocumentsSortingType {
    SortingByCreatedTime = 1,
    SortingByModifiedTime,
    SortingByAccessedTime,
    SortingByTitle,
    SortingByLocation,
    SortingBySize,
    SortingAsAscendingOrder,
    SortingAsDescendingOrder
};


class WizDocumentListView : public QListWidget
{
    Q_OBJECT

public:
    enum ViewType {
        TypeThumbnail,
        TypeTwoLine,
        TypeOneLine,
        TypeSearchResult
    };

public:
    explicit WizDocumentListView(WizExplorerApp& app, QWidget *parent = 0);
    virtual ~WizDocumentListView();

    void resetItemsViewType(int type);

    void setLeadInfoState(int state);

    int sortingType() const { return m_nSortingType; }
    void resetItemsSortingType(int type);
    bool isSortedByAccessDate();

    int documentCount() const;
    void clearAllItems();

    //CWizThumbIndexCache* thumbCache() const { return m_thumbCache; }

    void setItemsNeedUpdate(const QString& strKbGUID = 0, const QString& strGUID = 0);
    void drawItem(QPainter*p, const QStyleOptionViewItem* vopt) const;
    void reloadItem(const QString& strKbGUID, const QString& strGUID);

    bool acceptAllSearchItems() const;
    void setAcceptAllSearchItems(bool bAccept);

protected:
    virtual void resizeEvent(QResizeEvent* event);
    //virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void paintEvent(QPaintEvent* event);

    virtual void startDrag(Qt::DropActions supportedActions);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent * event);


private:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
#ifdef WIZNOTE_CUSTOM_SCROLLBAR
    WizScrollBar* m_vScroll;
#endif

    WizDocumentListView::ViewType m_nViewType;
    int m_nSortingType;
    int m_nLeadInfoState;
    bool m_searchResult;

    QMenu* m_menuDocument;
    WizTagListWidget* m_tagList;

    QPoint m_dragStartPosition;

    QList<WizDocumentListViewDocumentItem*> m_rightButtonFocusedItems;

    QList<WizDocumentListViewSectionItem*> m_sectionItems;

//#ifndef Q_OS_MAC
    // used for smoothly scroll
    QTimer m_vscrollTimer;
    int m_vscrollOldPos;
    int m_vscrollDelta;
    int m_vscrollCurrent;
//#endif // Q_OS_MAC

    bool m_itemSelectionChanged;
    bool m_accpetAllSearchItems;
    //
    int m_nAddedDocumentCount;
    bool m_bSortDocumentsAfterAdded;

    QPointer<QPropertyAnimation> m_scrollAnimation;
    //
    QPixmap m_emptyFolder;
    QPixmap m_emptySearch;

    QAction* findAction(const QString& strName);

    void resetPermission();

    // Test documents property
    bool isDocumentsAllCanDelete(const CWizDocumentDataArray& arrayDocument);
    bool isDocumentsWithGroupDocument(const CWizDocumentDataArray& arrayDocument);
    bool isDocumentsWithDeleted(const CWizDocumentDataArray& arrayDocument);
    bool isDocumentsAlwaysOnTop(const CWizDocumentDataArray& arrayDocument);

public:
    void setDocuments(const CWizDocumentDataArray& arrayDocument, bool searchResult = false);
    void appendDocuments(const CWizDocumentDataArray& arrayDocument);
    void appendDocumentsNoSort(const CWizDocumentDataArray& arrayDocument);

    bool acceptDocument(const WIZDOCUMENTDATA& document);
    void addAndSelectDocument(const WIZDOCUMENTDATA& document);
    //
    bool isSearchResult() const { return m_searchResult; }
    ViewType viewType() const { if (m_searchResult) return TypeSearchResult; return m_nViewType; }

public:
    void getSelectedDocuments(CWizDocumentDataArray& arrayDocument);

    int documentIndexFromGUID(const QString &strGUID);
    WizDocumentListViewBaseItem *itemFromIndex(int index) const;
    WizDocumentListViewBaseItem* itemFromIndex(const QModelIndex &index) const;
    WizDocumentListViewDocumentItem* documentItemAt(int index) const;
    WizDocumentListViewDocumentItem *documentItemFromIndex(const QModelIndex &index) const;

    const WIZDOCUMENTDATA& documentFromIndex(const QModelIndex &index) const;
    const WizDocumentListViewItemData& documentItemDataFromIndex(const QModelIndex& index) const;

//#ifndef Q_OS_MAC
    // used for smoothly scroll
    void vscrollBeginUpdate(int delta);
    //virtual void updateGeometries();
//#endif // Q_OS_MAC

public Q_SLOTS:
    void on_itemSelectionChanged();

    void on_tag_created(const WIZTAGDATA& tag);
    void on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    void on_document_created(const WIZDOCUMENTDATA& document);
    void on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew);
    void on_document_param_modified(const WIZDOCUMENTPARAMDATA& param);
    void on_documentUploaded(const QString& kbGuid, const QString& docGuid);
    void on_document_deleted(const WIZDOCUMENTDATA& document);
    void on_documentAccessDate_changed(const WIZDOCUMENTDATA& document);
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
    void on_action_locate();
    void on_action_selectTags();
    void on_action_deleteDocument();
    void on_action_encryptDocument();
    void on_action_cancelEncryption();
    void on_action_combineNote();
    void on_action_alwaysOnTop();
    void on_action_addToShortcuts();

    void on_action_moveDocument();
    void on_action_moveDocument_confirmed(int result);

    void on_action_copyDocument();
    void on_action_copyDocument_confirmed(int result);

    void on_action_showDocumentInFloatWindow();
    void on_action_copyDocumentLink();
    void on_action_copyWebClientLink();
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
    void groupDocumentReadCountChanged(const QString& strKbGUID);
    void shareDocumentByLinkRequest(const QString& strKbGUID, const QString& strGUID);
    void changeUploadRequest(const QString& strKbGUID);
    void addDocumentToShortcutsRequest(const WIZDOCUMENTDATA& doc);
    void loacteDocumetRequest(const WIZDOCUMENTDATA& doc);

private:
    int numOfEncryptedDocuments(const CWizDocumentDataArray& docArray);
    void setEncryptDocumentActionEnable(bool enable);
    //    
    int addDocument(const WIZDOCUMENTDATAEX& data, bool sort);
    void addDocument(const WIZDOCUMENTDATAEX &doc);

    bool acceptDocumentChange(const WIZDOCUMENTDATA &document);
    bool acceptDocumentChange(const QString &documentGuid);

    void resetSectionData();
    //
    void moveDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder);
    void moveDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag);
    void copyDocumentsToPersonalFolder(const CWizDocumentDataArray& arrayDocument, const QString& targetFolder,
                                      bool keepDocTime, bool keepTag);
    void copyDocumentsToGroupFolder(const CWizDocumentDataArray& arrayDocument, const WIZTAGDATA& targetTag,
                                    bool keepDocTime);

    //
    void duplicateDocuments(const CWizDocumentDataArray& arrayDocument);   

    //
    void addSectionItem(const WizDocumentListViewSectionData& secData, const QString& text, int docCount);
    void updateSectionItems();
    bool getDocumentDateSections(QMap<QDate, int>& dateMap);
    bool getDocumentSizeSections(QMap<QPair<int, int>, int>& sizeMap);
    bool getDocumentTitleSections(QMap<QString, int>& titleMap);
    bool getDocumentLocationSections(QMap<QString, int>& locationMap);
//    bool getDocumentTagSections(QMap<QString, int>& tagMap);

};


#endif // WIZDOCUMENTLISTVIEW_H
