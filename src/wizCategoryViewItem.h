#ifndef WIZCATEGORYVIEWITEM_H
#define WIZCATEGORYVIEWITEM_H

#include <QTreeWidgetItem>

#include "share/wizobject.h"

class CWizDatabase;
class CWizExplorerApp;
class CWizCategoryBaseView;

class CWizCategoryViewItemBase : public QTreeWidgetItem
{
public:
    CWizCategoryViewItemBase(CWizExplorerApp& app, const QString& strName = "", const QString& strKbGUID = "");
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos) = 0;
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument) = 0;
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data) { Q_UNUSED(db); Q_UNUSED(data); return false; }

    virtual void draw(QPainter* p, const QStyleOptionViewItemV4 *vopt) const;

    virtual QVariant data(int column, int role) const;
    virtual int getItemHeight(int hintHeight) const;
    virtual bool operator<(const QTreeWidgetItem &other) const;

    const QString& kbGUID() const { return m_strKbGUID; }
    const QString& name() const { return m_strName; }

    QString id() const;

    void setDocumentsCount(int nCurrent, int nTotal);

protected:
    CWizExplorerApp& m_app;
    QString m_strName;
    QString m_strKbGUID;

    // for quickly access by drawing
public:
    QString countString;
};

class CWizCategoryViewSpacerItem: public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewSpacerItem(CWizExplorerApp& app);

    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(db); Q_UNUSED(arrayDocument); }

    virtual int getItemHeight(int hintHeight) const;
};

class CWizCategoryViewSeparatorItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewSeparatorItem(CWizExplorerApp& app);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos) { Q_UNUSED(pCtrl); Q_UNUSED(pos); }
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument) { Q_UNUSED(db); Q_UNUSED(arrayDocument); }
    virtual int getItemHeight(int nHeight) const;
};

class CWizCategoryViewCategoryItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewCategoryItem(CWizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(CWizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(db); Q_UNUSED(arrayDocument); }
};

class CWizCategoryViewMessageItem : public CWizCategoryViewItemBase
{
public:
    enum FilterType {
        All,
        SendToMe,
        ModifyNote,
        Comment,
        SendFromMe
    };

    CWizCategoryViewMessageItem(CWizExplorerApp& app, const QString& strName, int nFilter);

    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);

private:
    int m_nFilter;
};

class CWizCategoryViewShortcutRootItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewShortcutRootItem(CWizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(CWizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(db); Q_UNUSED(arrayDocument); }
};

class CWizCategoryViewSearchRootItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewSearchRootItem(CWizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(CWizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(db); Q_UNUSED(arrayDocument); }
};

class CWizCategoryViewAllFoldersItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewAllFoldersItem(CWizExplorerApp& app, const QString& strName, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
};

class CWizCategoryViewFolderItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewFolderItem(CWizExplorerApp& app, const QString& strLocation, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool operator < (const QTreeWidgetItem &other) const;


    virtual QTreeWidgetItem* clone() const;

    QString location() const { return m_strName; }
    QString name() const;
};

class CWizCategoryViewAllTagsItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewAllTagsItem(CWizExplorerApp& app, const QString& strName, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
};

class CWizCategoryViewTagItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewTagItem(CWizExplorerApp& app, const WIZTAGDATA& tag, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);

    virtual QTreeWidgetItem *clone() const;

    void reload(CWizDatabase& db);
    const WIZTAGDATA& tag() const { return m_tag; }

private:
    WIZTAGDATA m_tag;
};

class CWizCategoryViewStyleRootItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewStyleRootItem(CWizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(CWizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(db); Q_UNUSED(arrayDocument); }
};

class CWizCategoryViewAllGroupsRootItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewAllGroupsRootItem(CWizExplorerApp& app,
                                      const QString& strName,
                                      const QString& strKbGUID);

    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(db); Q_UNUSED(arrayDocument); }
};

class CWizCategoryViewBizGroupRootItem : public CWizCategoryViewAllGroupsRootItem
{
public:
    CWizCategoryViewBizGroupRootItem(CWizExplorerApp& app,
                                     const QString& strName,
                                     const QString& strKbGUID);

    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(db); Q_UNUSED(arrayDocument); }
};

class CWizCategoryViewGroupRootItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewGroupRootItem(CWizExplorerApp& app,
                                  const QString& strName,
                                  const QString& strKbGUID);

    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
    void reload(CWizDatabase& db);
};

class CWizCategoryViewGroupNoTagItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewGroupNoTagItem(CWizExplorerApp& app, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos) {Q_UNUSED(pCtrl); Q_UNUSED(pos);}
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
};

class CWizCategoryViewGroupItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewGroupItem(CWizExplorerApp& app, const WIZTAGDATA& tag, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
    void reload(CWizDatabase& db);

    const WIZTAGDATA& tag() const { return m_tag; }

private:
    WIZTAGDATA m_tag;
};

class CWizCategoryViewTrashItem : public CWizCategoryViewFolderItem
{
public:
    CWizCategoryViewTrashItem(CWizExplorerApp& app, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
};

#endif // WIZCATEGORYVIEWITEM_H
