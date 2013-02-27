#ifndef WIZDOCUMENTLISTVIEW_H
#define WIZDOCUMENTLISTVIEW_H

#include <QtGui>

#include "wizdef.h"
#include "share/wizobject.h"
#include "share/wizThumbIndexCache.h"

class CWizDocumentListViewItem;
class CWizTagListWidget;

class CWizDocumentListView : public QListWidget
{
    Q_OBJECT

public:
    CWizDocumentListView(CWizExplorerApp& app, QWidget *parent = 0);
    //virtual QSize sizeHint() const { return QSize(300, 1); }

    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);

    virtual void startDrag(Qt::DropActions supportedActions);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent * event);

private:
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
    QPointer<CWizThumbIndexCache> m_thumbCache;

    QPointer<QMenu> m_menu;
    QAction* m_actionEncryptDocument;
    CWizTagListWidget* m_tagList;

    QPoint m_dragStartPosition;

#ifndef Q_OS_MAC
    // used for smoothly scroll
    QTimer m_vscrollTimer;
    int m_vscrollOldPos;
    int m_vscrollDelta;
    int m_vscrollCurrent;
#endif // Q_OS_MAC

    void resetPermission();
    QAction* findAction(const QString& strName);
    bool isSelectedAllCanDelete();
    bool isSelectedGroupDocument();

public:
    void setDocuments(const CWizDocumentDataArray& arrayDocument);
    void addDocuments(const CWizDocumentDataArray& arrayDocument);
    int addDocument(const WIZDOCUMENTDATA& data, bool sort);
    bool acceptDocument(const WIZDOCUMENTDATA& document);
    void addAndSelectDocument(const WIZDOCUMENTDATA& document);

public:
    void getSelectedDocuments(CWizDocumentDataArray& arrayDocument);

    int documentIndexFromGUID(const CString& strGUID);
    CWizDocumentListViewItem *documentItemAt(int index);
    CWizDocumentListViewItem *documentItemFromIndex(const QModelIndex &index) const;
    WIZDOCUMENTDATA documentFromIndex(const QModelIndex &index) const;
    WIZABSTRACT documentAbstractFromIndex(const QModelIndex &index) const;
    CString documentTagsFromIndex(const QModelIndex &index) const;

#ifndef Q_OS_MAC
    // used for smoothly scroll
    void vscrollBeginUpdate(int delta);
    virtual void updateGeometries();
    virtual void wheelEvent(QWheelEvent* event);
#endif // Q_OS_MAC

public Q_SLOTS:
    void on_tag_created(const WIZTAGDATA& tag);
    void on_tag_modified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    void on_document_created(const WIZDOCUMENTDATA& document);
    void on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew);
    void on_document_deleted(const WIZDOCUMENTDATA& document);
    //void on_document_AbstractModified(const WIZDOCUMENTDATA& document);
    void on_action_selectTags();
    void on_action_deleteDocument();
    void on_action_encryptDocument();

    void on_document_abstractLoaded(const WIZABSTRACT& abs);

#ifndef Q_OS_MAC
    // used for smoothly scroll
    void on_vscroll_valueChanged(int value);
    void on_vscroll_actionTriggered(int action);
    void on_vscroll_update();
#endif // Q_OS_MAC
};


#endif // WIZDOCUMENTLISTVIEW_H
